/*
 * $Id: ht_http.c,v 1.1 2003/09/01 10:33:27 mkern Exp $
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

#include <signal.h>

#include "ht_http.h"

/*****************************************************************************/

Protocol *ht_proto = NULL;

/*****************************************************************************/

/* allocate and init plugin */
static int ht_giftcb_start (Protocol *p)
{
	HTPlugin *plugin = malloc (sizeof (HTPlugin));

	HT_DBG ("starting up");

	/* set protocol pointer */
	p->udata = (void*)plugin;

	/* we're all set, now just wait for downloads */

	return TRUE;
}

/* destroy plugin */
static void ht_giftcb_destroy (Protocol *p)
{
	HT_DBG ("shutting down");

	if (!HT_PLUGIN)
		return;

	free (HT_PLUGIN);
}

/*****************************************************************************/

int ht_giftcb_source_cmp (Protocol *p, Source *a, Source *b)
{
	return strcmp (a->url, b->url);
}

/*****************************************************************************/

static void ht_plugin_setup_functbl (Protocol *p)
{
	/*
	 * communicate special properties of this protocol which will modify
	 * giFT's behaviour
 	 * NOTE: most of these dont do anything yet
	 */

	p->support (p, "range-get", TRUE);
	p->support (p, "hash-unique", FALSE);

	/*
	 * Finally, assign the support communication structure.
	 */

	/* fst_hash.c */
	p->hash_handler (p, (const char*)"MD5", HASH_PRIMARY, (HashFn)ht_giftcb_MD5, (HashDspFn)ht_giftcb_MD5_human);

	/* fst_openft.c: */
	p->start          = ht_giftcb_start;
	p->destroy        = ht_giftcb_destroy;

	/* fst_search.c: */
/*
*	p->search         = ht_giftcb_search;
*	p->browse         = ht_giftcb_browse;
*	p->locate         = ht_giftcb_locate;
*	p->search_cancel  = ht_giftcb_search_cancel;
*/
	/* fst_xfer.c: */
	p->download_start = ht_giftcb_download_start;
	p->download_stop  = ht_giftcb_download_stop;
	p->source_remove  = ht_giftcb_source_remove;
/*
*	p->upload_stop    = ht_giftcb_upload_stop;
*	p->upload_avail   = ht_giftcb_upload_avail;
*	p->chunk_suspend  = ht_giftcb_chunk_suspend;
*	p->chunk_resume   = ht_giftcb_chunk_resume;
*/
	p->source_cmp     = ht_giftcb_source_cmp;
/*	
	p->user_cmp       = ht_giftcb_user_cmp;
*/	

	/* fst_share.c: */
/*
*	p->share_new      = ht_giftcb_share_new;
*	p->share_free     = ht_giftcb_share_free;
*	p->share_add      = ht_giftcb_share_add;
*	p->share_remove   = ht_giftcb_share_remove;
*	p->share_sync     = ht_giftcb_share_sync;
*	p->share_hide     = ht_giftcb_share_hide;
*	p->share_show     = ht_giftcb_share_show;
*/
	/* fst_stats.c: */
/*
*	p->stats          = ht_giftcb_stats;
*/
}

int Http_init (Protocol *p)
{
	/* make sure we're loaded with the correct plugin interface version */
	if (protocol_compat (p, LIBGIFTPROTO_MKVERSION (0, 11, 3)) != 0)
		return FALSE;
	
	/* tell giFT about our version
	 * VERSION is defined in config.h. e.g. "0.0.1"
	 */ 
	p->version_str = strdup (VERSION);
	
	/* put protocol in global variable so we always have access to it */
	ht_proto = p;

	/* setup giFT callbacks */
	ht_plugin_setup_functbl (p);

	return TRUE;
}

/*****************************************************************************/
