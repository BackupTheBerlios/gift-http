/*
 * $Id: ht_download.c,v 1.1 2003/09/01 10:33:27 mkern Exp $
 *
 * Copyright (C) 2003 giFT-HTTP project
 * http://developer.berlios.de/projects/gift-http
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */

#include "ht_http.h"
#include "ht_download.h"

/*****************************************************************************/

#define DOWNLOAD_BUF_SIZE 4069

/*****************************************************************************/

/* called by gift to start downloading of a chunk */
int ht_giftcb_download_start (Protocol *p, Transfer *transfer, Chunk *chunk, Source *source)
{
	HTDownload *download = ht_download_create (chunk);
	
	if (!download)
		return FALSE;

	ht_download_start (download);

	return TRUE;
}

/* called by gift to stop download */
void ht_giftcb_download_stop (Protocol *p, Transfer *transfer, Chunk *chunk, Source *source, int complete)
{
	HTDownload *download;

	if(!chunk || !chunk->udata)
	{
		HT_HEAVY_DBG ("ht_giftcb_download_stop: chunk->udata == NULL, no action taken");
		return;
	}

	download = (HTDownload*)chunk->udata;

	if (complete)
	{
		HT_HEAVY_DBG_2 ("removing completed download from %s:%d", download->host, download->port);
		HT_PROTO->source_status (HT_PROTO, chunk->source, SOURCE_COMPLETE, "Complete");
		ht_download_stop (download);
	}
	else
	{
		HT_HEAVY_DBG_2 ("removing cancelled download from %s:%d", download->host, download->port);
		HT_PROTO->source_status (HT_PROTO, chunk->source, SOURCE_CANCELLED, "Cancelled");
		ht_download_stop (download);
	}
}

/* called by gift to remove source */
void ht_giftcb_source_remove (Protocol *p, Transfer *transfer, Source *source)
{
	HTDownload *download;

	if (!source || !source->chunk || !source->chunk->udata)
	{
		HT_DBG ("ht_giftcb_source_remove: invalid source, no action taken");
		return;
	}

	download = (HTDownload*)source->chunk->udata;

	HT_DBG_2 ("removing source %s:%d", download->host, download->port);
	HT_PROTO->source_status (HT_PROTO, source->chunk->source, SOURCE_CANCELLED, "Cancelled");
	ht_download_stop (download);
}

/*****************************************************************************/

static void download_connected (int fd, input_id input, HTDownload *download);
static void download_read_header (int fd, input_id input, HTDownload *download);
static void download_read_body (int fd, input_id input, HTDownload *download);
static void download_write_gift (HTDownload *download, unsigned char *data, unsigned int len);
static void download_error_gift (HTDownload *download, int remove_source, unsigned short klass, char *error);
static char *download_parse_url (char *url, char **host, unsigned short *port);

/*****************************************************************************/

// alloc and init download
HTDownload *ht_download_create (Chunk *chunk)
{
	HTDownload *dl = malloc (sizeof (HTDownload));

	dl->state = DownloadNew;
	dl->tcpcon = NULL;
	dl->hdrstr = string_new (NULL, 0, 0, TRUE);

	dl->chunk = chunk;
	dl->host = NULL;
	dl->uri = download_parse_url (chunk->source->url, &dl->host, &dl->port);

	if(!dl->uri || !dl->host)
	{
		ht_download_free (dl);
		return NULL;
	}

	/* make chunk refer back to us */
	chunk->udata = (void*)dl;
	
	return dl;
}

/* free download, stop it if necessary */
void ht_download_free (HTDownload *download)
{
	if (!download)
		return;

	tcp_close (download->tcpcon);
	string_free (download->hdrstr);
	free (download->uri);
	free (download->host);

	/* unref chunk */
	if (download->chunk)
		download->chunk->udata = NULL;

	free (download);
}

// start download
int ht_download_start (HTDownload *download)
{
	struct hostent *he;

	if (!download || download->state != DownloadNew || !download->chunk)
		return FALSE;

	HT_HEAVY_DBG_2 ("connecting to %s:%d", download->host, download->port);

	download->state = DownloadConnecting;
	HT_PROTO->source_status (HT_PROTO, download->chunk->source, SOURCE_WAITING, "Connecting");

	// TODO: make this non-blocking
	if (!(he = gethostbyname (download->host)))
	{
		HT_WARN_1 ("gethostbyname failed for host %s", download->host);
		download_error_gift (download, FALSE, SOURCE_TIMEOUT, "DNS failed");
		return FALSE;
	}

	download->tcpcon = tcp_open (*((in_addr_t*)he->h_addr_list[0]), download->port, FALSE);
	
	if (download->tcpcon == NULL)
	{
		HT_DBG_2 ("ERROR: tcp_open() failed for %s:%d", download->host, download->port);
		download_error_gift (download, FALSE, SOURCE_TIMEOUT, "Connect failed");
		return FALSE;
	}

	download->tcpcon->udata = (void*)download;

	/* wait for connected */
	input_add (download->tcpcon->fd, (void*)download, INPUT_WRITE, (InputCallback)download_connected, HT_DOWNLOAD_CONNECT_TIMEOUT);

	return TRUE;
}

/* stop download */
int ht_download_stop (HTDownload *download)
{
	ht_download_free (download);

	return TRUE;
}

/*****************************************************************************/

static void download_connected (int fd, input_id input, HTDownload *download)
{
	HTHttpRequest *request;
	String *hdrstr;

	input_remove (input);

	if (net_sock_error (download->tcpcon->fd))
	{
		HT_HEAVY_DBG_2 ("connection to %s:%d failed -> removing source", download->host, download->port);
		download_error_gift (download, TRUE, SOURCE_TIMEOUT, "Connect failed");
		return;
	}

	download->state = DownloadRequesting;
	HT_PROTO->source_status (HT_PROTO, download->chunk->source, SOURCE_WAITING, "Requesting");

	/* create http request */
	request = ht_http_request_create ("GET", download->uri);

	/* add http headers */
	ht_http_request_set_header (request, "UserAgent", "giFT-HTTP");
	ht_http_request_set_header (request, "Connection", "close");
	ht_http_request_set_header (request, "Host", stringf ("%s:%d", download->host, download->port));

	/* range, http range is inclusive! */
	/* IMPORTANT: use chunk->start + chunk->transmit for starting point, rather non-intuitive */
	ht_http_request_set_header (request, "Range", stringf ("bytes=%d-%d", (int)(download->chunk->start + download->chunk->transmit), (int)download->chunk->stop - 1));

	/* compile and send request */
	hdrstr = string_new (NULL, 0, 0, TRUE);
	ht_http_request_compile (request, hdrstr);
	ht_http_request_free (request);

	if (tcp_send(download->tcpcon, hdrstr->str, hdrstr->len) != hdrstr->len)
	{
		download_error_gift (download, FALSE, SOURCE_TIMEOUT, "Request failed");
		string_free (hdrstr);
		return;
	}

	string_free (hdrstr);

	/* wait for header */
	input_add (download->tcpcon->fd, (void*)download, INPUT_READ, (InputCallback)download_read_header, HT_DOWNLOAD_HANDSHAKE_TIMEOUT);
}

static void download_read_header (int fd, input_id input, HTDownload *download)
{
	HTHttpReply *reply;
	char *p, *buf;
	int len;

	input_remove (input);

	if (net_sock_error (download->tcpcon->fd))
	{
		HT_HEAVY_DBG_2 ("read error while downloading from %s:%d -> removing source", download->host, download->port);
		download_error_gift (download, FALSE, SOURCE_TIMEOUT, "Request Failed");
		return;
	}

	/* read data */
	buf = malloc (1024);

	if ((len = tcp_recv(download->tcpcon, buf, 1024)) <= 0)
	{
		HT_DBG_2 ("read error while getting header from %s:%d -> aborting", download->host, download->port);
		download_error_gift (download, FALSE, SOURCE_TIMEOUT, "Request Failed");
		return;
	}
	string_appendu (download->hdrstr, buf, len);
	free (buf);

	reply = ht_http_reply_create ();

	if ((len = ht_http_reply_parse (reply, download->hdrstr->str, download->hdrstr->len)) == 0)
	{	
		ht_http_reply_free (reply);

		if (download->hdrstr->len > 4096)
		{
			HT_WARN ("Didn't get whole http header and received more than 4K, closing connection");
			download_error_gift (download, TRUE, SOURCE_TIMEOUT, "Invalid response");
			return;
		}

		HT_DBG_2 ("didn't get whole header from %s:%d -> waiting for more", download->host, download->port);
		/* wait for rest of header */
		input_add (download->tcpcon->fd, (void*)download, INPUT_READ, (InputCallback)download_read_header, HT_DOWNLOAD_HANDSHAKE_TIMEOUT);
		return;
	}

	/* TODO: handle redirects */
	if (reply->code != 200 && reply->code != 206) /* 206 == partial content */
	{
		HT_HEAVY_DBG_4 ("%s:%d replied with %d (\"%s\") -> aborting", download->host, download->port, reply->code, reply->code_str);

		if (reply->code == 503)
			download_error_gift (download, FALSE, SOURCE_QUEUED_REMOTE, "Remotely queued");
		else if (reply->code == 404)
			download_error_gift (download, TRUE, SOURCE_CANCELLED, "File not found");
		else
		{
			download_error_gift (download, TRUE, SOURCE_CANCELLED, "Weird http code");
			HT_DBG_4 ("weird http code from %s:%d: %d (\"%s\") -> aborting", download->host, download->port, reply->code, reply->code_str);
		}

		ht_http_reply_free (reply);
		return;
	}

	HT_HEAVY_DBG_4 ("%s:%d replied with %d (\"%s\") -> downloading", download->host, download->port, reply->code, reply->code_str);

	/* check that recevied ranges are correct */
	if ( (p = ht_http_reply_get_header (reply, "content-range")))
	{
		int start, stop;
		sscanf (p, "bytes %d-%d", &start, &stop);

		/* longer ranges than requested should be ok since giFT handles this */
		if (start != download->chunk->start + download->chunk->transmit || stop < download->chunk->stop - 1)
		{
			HT_WARN ("Removing source due to range mismatch");
			HT_WARN_2 ("\trequested range: %d-%d", download->chunk->start + download->chunk->transmit, download->chunk->stop - 1);
			HT_WARN_2 ("\treceived range: %d-%d", start, stop);
			if ( (p = ht_http_reply_get_header (reply, "content-length")))
				HT_WARN_1 ("\tcontent-length: %s", p);

			ht_http_reply_free (reply);
			download_error_gift (download, TRUE, SOURCE_CANCELLED, "Range mismatch");
			return;
		}
	}
	else
	{
		HT_WARN ("Server didn't sent content-range header, file may end up corrupted");
/*		
 *		ht_http_reply_free (reply);
 *		download_error_gift (download, TRUE, SOURCE_CANCELLED, "Missing Content-Range");
 *		return;
 */		
	}

#if 0
	/* HACKHACK: set transfer->total based on content-length*/
	if ( (p = ht_http_reply_get_header (reply, "content-length")))
	{
		int len = ATOI(p);

		download->chunk->transfer->total = len;

		HT_HEAVY_DBG_1 ("setting transfer->total to %d", len);
	}
#endif

	/* update status */
	download->state = DownloadRunning;
	HT_PROTO->source_status (HT_PROTO, download->chunk->source, SOURCE_ACTIVE, "Active");

	/* free reply */
	ht_http_reply_free (reply);

	/* wait for body */
	input_add (download->tcpcon->fd, (void*)download, INPUT_READ, (InputCallback)download_read_body, HT_DOWNLOAD_DATA_TIMEOUT);

	/* write parts of body we already received with header to file */
	download_write_gift (download, download->hdrstr->str + len, download->hdrstr->len - len);
}

static void download_read_body (int fd, input_id input, HTDownload *download)
{
	char *data;
	int len;

	if (net_sock_error (download->tcpcon->fd))
	{
		HT_HEAVY_DBG_2 ("read error while downloading from %s:%d -> aborting", download->host, download->port);
		input_remove (input);

		/* this makes giFT call gift_cb_download_stop(), which closes connection and frees download */
		download_error_gift (download, FALSE, SOURCE_CANCELLED, "Download Failed");
		return;
	}
	
	data = malloc (DOWNLOAD_BUF_SIZE);

	if ( (len = tcp_recv (download->tcpcon, data, DOWNLOAD_BUF_SIZE)) <= 0)
	{
		HT_HEAVY_DBG_2 ("download_read_body: tcp_recv() <= 0 for %s:%d", download->host, download->port);
		input_remove (input);

		/* this makes giFT call gift_cb_download_stop(), which closes connection and frees download */
		download_error_gift (download, FALSE, SOURCE_CANCELLED, "Download Error");
		return;
	}
	
	/* write data to file through giFT, this calls ht_giftcb_download_stop() if download is complete */
	download_write_gift (download, data, len);

	free (data);
}

/*****************************************************************************/

static void download_write_gift (HTDownload *download, unsigned char *data, unsigned int len)
{
	HT_PROTO->chunk_write (HT_PROTO, download->chunk->transfer, download->chunk, download->chunk->source, data, len);
}

static void download_error_gift (HTDownload *download, int remove_source, unsigned short klass, char *error)
{
	if (remove_source)
	{
		HT_DBG_1 ("download error (%s), removing source", error);
		HT_PROTO->source_status (HT_PROTO, download->chunk->source, klass, error);
		HT_PROTO->source_abort (HT_PROTO, download->chunk->transfer, download->chunk->source);
	}
	else
	{
		HT_PROTO->source_status (HT_PROTO, download->chunk->source, klass, error);
		download->chunk->udata = NULL;

		/* tell giFT an error occured with this download */
		download_write_gift (download, NULL, 0);
		ht_download_free (download);
	}
}


/* parses url, returns uri which caller frees or NULL on failure */
static char *download_parse_url (char *url, char **host, unsigned short *port)
{
	char *tmp, *uri, *host_str, *port_str;

	if (!url)
		return NULL;

	tmp = uri = strdup (url);

	string_sep (&uri, "://");       /* get rid of this useless crap */

	if (uri)
	{
		/* divide the string into two sides */
		port_str = string_sep (&uri, "/");

		/* pull off the left-hand operands */
		host_str = string_sep (&port_str, ":");

		if (host)
			*host = strdup(host_str);

		if (port_str && port)
			*port = ATOI (port_str);
		else if (port)
			*port = 80;		/* default http port */

		if(uri)
		{
			uri--; *uri = '/';
			uri = strdup (uri);
		}
		else
		{
			uri = strdup ("/");
		}

		free (tmp);
		return uri;
	}

	free (tmp);
	return NULL;
}

/*****************************************************************************/
