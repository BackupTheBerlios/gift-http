/*
 * $Id: ht_header.c,v 1.1 2003/09/01 10:33:28 mkern Exp $
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
#include "ht_header.h"

/*****************************************************************************/

/* alloc and init request */
HTHttpRequest *ht_http_request_create (char *method, char *uri)
{
	HTHttpRequest *request = malloc (sizeof (HTHttpRequest));

	request->method = strdup (method);
	request->uri = strdup (uri);
	request->headers = dataset_new (DATASET_HASH);

	return request;
}

/* free request */
void ht_http_request_free (HTHttpRequest *request)
{
	if (!request)
		return;

	free (request->method);
	free (request->uri);
	dataset_clear (request->headers);

	free (request);
}

/* add header to request */
void ht_http_request_set_header (HTHttpRequest *request, char *name, char *value)
{
	dataset_insertstr (&request->headers, name, value);
}

void http_reply_compile_header (ds_data_t *key, ds_data_t *value, String *str)
{
	string_appendf (str, "%s: %s\r\n", (char*)key->data, (char*)value->data);
}

/* compile request and append it to libgift String */
int ht_http_request_compile (HTHttpRequest *request, String *str)
{
	if(!request || !str)
		return FALSE;

	/* compile first line */
	string_appendf (str, "%s %s HTTP/1.1\r\n", request->method, request->uri);

	/* add headers */
	dataset_foreach (request->headers, DS_FOREACH(http_reply_compile_header), (void*)str);

	/* add empty line for header termination */
	string_append (str, "\r\n");

	return TRUE;
}

/*****************************************************************************/

/* alloc an init reply */
HTHttpReply *ht_http_reply_create ()
{
	HTHttpReply *reply = malloc (sizeof (HTHttpReply));

	reply->code = -1;
	reply->code_str = NULL;
	reply->headers = NULL;

	return reply;
}

/* free reply */
void ht_http_reply_free (HTHttpReply *reply)
{
	if (!reply)
		return;

	if (reply->code_str)
		free (reply->code_str);
	dataset_clear (reply->headers);

	free (reply);
}

/* retrieve header, do not modify/free returned string! */
char *ht_http_reply_get_header (HTHttpReply *reply, char *name)
{
	char *value, *low_name;

	if (!reply->headers)
		return NULL;

	low_name = strdup (name);
	string_lower (low_name);
	value = dataset_lookupstr (reply->headers, low_name);
	free (low_name);

	return value;
}

/* parses reply and returns size of header in bytes or 0 for incomplete header */
int ht_http_reply_parse (HTHttpReply *reply, char *data, int data_len)
{
	char *header, *tmp, *p, *line;
	int i, len;

	/* free previously used stuff */
	dataset_clear (reply->headers);
	reply->headers = dataset_new (DATASET_HASH);
	if (reply->code_str)
	{
		free (reply->code_str);
		reply->code_str = NULL;
	}

	/* check if packet contains entire header */
	p = data;
	len = data_len - 2;

	/* what a mess :-p */
	for(i = 0; ; i++, p++)
	{
		if (p[0] == '\r' && p[1] == '\n')
		{
			if (p[2] == 0x0a)	/* "\r\n\n", kazaa weirdness */
			{
				i += 3;
				break;
			}
			if (len - i >= 2 && p[2] == '\r' && p[3] == '\n')
			{
				i += 4;
				break;
			}
		}
		if (i == len)
			return FALSE;
	}

	/* create working copy of header */
	header = tmp = malloc (i + 1);
	memcpy (header, data, i);
	header[i] = 0;

	/* parse first line */
	if ( (line = string_sep_set (&tmp, "\r\n")))
	{
		string_sep (&line, " ");						/* shift past HTTP/1.1 */
		reply->code = ATOI (string_sep (&line, " "));	/* shift past 200 */
		reply->code_str = strdup (line);
	}

	/* parse header fields */
	while ( (line = string_sep_set (&tmp, "\r\n")))
	{
		p = string_sep (&line, ": ");

		if (!p || !line)
			continue;

		string_lower (p);
		dataset_insertstr (&reply->headers, p, line);
	}

	free (header);

	return i; // actual header length
}

/*****************************************************************************/
