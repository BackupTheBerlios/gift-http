/*
 * $Id: ht_download.h,v 1.1 2003/09/01 10:33:26 mkern Exp $
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

#ifndef __HT_DOWNLOAD_H
#define __HT_DOWNLOAD_H

#include <libgift/strobj.h>
#include "ht_http.h"
#include "ht_header.h"

/*****************************************************************************/

typedef enum { DownloadNew, DownloadConnecting, DownloadRequesting, DownloadRunning, DownloadComplete } HTDownloadState;

typedef struct
{
	HTDownloadState state;

	TCPC *tcpcon;

	Chunk *chunk;

	String *hdrstr;

	/* parsed url */
	char *host;
	unsigned short port;
	char *uri;

} HTDownload;

/*****************************************************************************/

/* called by gift to start downloading of a chunk */
int ht_giftcb_download_start (Protocol *p, Transfer *transfer, Chunk *chunk, Source *source);

/* called by gift to stop download */
void ht_giftcb_download_stop (Protocol *p, Transfer *transfer, Chunk *chunk, Source *source, int complete);

/* called by gift to remove source */
void ht_giftcb_source_remove (Protocol *p, Transfer *transfer, Source *source);

/*****************************************************************************/

/* alloc and init download */
HTDownload *ht_download_create (Chunk *chunk);

/* free download, stop it if necessary */
void ht_download_free (HTDownload *download);

/* start download */
int ht_download_start (HTDownload *download);

/* stop download */
int ht_download_stop (HTDownload *download);

/*****************************************************************************/

#endif /* __HT_DOWNLOAD_H */
