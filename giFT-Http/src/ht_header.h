/*
 * $Id: ht_header.h,v 1.1 2003/09/01 10:33:27 mkern Exp $
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

#ifndef __HT_HEADER_H
#define __HT_HEADER_H

#include <libgift/dataset.h>
#include <libgift/strobj.h>

/*****************************************************************************/

typedef struct
{
	char *method;
	char *uri;

	Dataset *headers;

} HTHttpRequest;

typedef struct
{
	int code;
	char *code_str;

	Dataset *headers;

} HTHttpReply;

/*****************************************************************************/

/* alloc and init request */
HTHttpRequest *ht_http_request_create (char *method, char *uri);

/* free request */
void ht_http_request_free (HTHttpRequest *request);

/* add header to request */
void ht_http_request_set_header (HTHttpRequest *request, char *name, char *value);

/* compile request and append it to libgift String */
int ht_http_request_compile (HTHttpRequest *request, String *str);

/*****************************************************************************/

/* alloc an init reply */
HTHttpReply *ht_http_reply_create ();

/* free reply */
void ht_http_reply_free (HTHttpReply *reply);

/* retrieve header, do not modify/free returned string! */
char *ht_http_reply_get_header (HTHttpReply *reply, char *name);

/* parses reply and returns size of header in bytes or 0 for incomplete header */
int ht_http_reply_parse (HTHttpReply *reply, char *data, int data_len);

/*****************************************************************************/

#endif /* __HT_HEADER_H */
