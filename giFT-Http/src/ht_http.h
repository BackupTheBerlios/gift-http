/*
 * $Id: ht_http.h,v 1.1 2003/09/01 10:33:26 mkern Exp $
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

#ifndef __HT_HTTP_H
#define __HT_HTTP_H

/*****************************************************************************/

#if HAVE_CONFIG_H
#  include <config.h>
#endif /* HAVE_CONFIG_H */



#define FILE_LINE_FUNC __FILE__,__LINE__,__PRETTY_FUNCTION__

/* The default shall be debugging on, unless it is a stable release */
#ifdef DEBUG
#define HT_DBG(fmt)				HT_PROTO->trace(HT_PROTO,FILE_LINE_FUNC,fmt)
#define HT_DBG_1(fmt,a)			HT_PROTO->trace(HT_PROTO,FILE_LINE_FUNC,fmt,a)
#define HT_DBG_2(fmt,a,b)		HT_PROTO->trace(HT_PROTO,FILE_LINE_FUNC,fmt,a,b)
#define HT_DBG_3(fmt,a,b,c)		HT_PROTO->trace(HT_PROTO,FILE_LINE_FUNC,fmt,a,b,c)
#define HT_DBG_4(fmt,a,b,c,d)	HT_PROTO->trace(HT_PROTO,FILE_LINE_FUNC,fmt,a,b,c,d)
#define HT_DBG_5(fmt,a,b,c,d,e)	HT_PROTO->trace(HT_PROTO,FILE_LINE_FUNC,fmt,a,b,c,d,e)
#else
#define HT_DBG(fmt)
#define HT_DBG_1(fmt,a)
#define HT_DBG_2(fmt,a,b)
#define HT_DBG_3(fmt,a,b,c)
#define HT_DBG_4(fmt,a,b,c,d)
#define HT_DBG_5(fmt,a,b,c,d,e)
#endif /* DEBUG */


#ifdef HEAVY_DEBUG
# define HT_HEAVY_DBG(fmt)				HT_PROTO->trace(HT_PROTO,FILE_LINE_FUNC,fmt)
# define HT_HEAVY_DBG_1(fmt,a)			HT_PROTO->trace(HT_PROTO,FILE_LINE_FUNC,fmt,a)
# define HT_HEAVY_DBG_2(fmt,a,b)		HT_PROTO->trace(HT_PROTO,FILE_LINE_FUNC,fmt,a,b)
# define HT_HEAVY_DBG_3(fmt,a,b,c)		HT_PROTO->trace(HT_PROTO,FILE_LINE_FUNC,fmt,a,b,c)
# define HT_HEAVY_DBG_4(fmt,a,b,c,d)	HT_PROTO->trace(HT_PROTO,FILE_LINE_FUNC,fmt,a,b,c,d)
# define HT_HEAVY_DBG_5(fmt,a,b,c,d,e)	HT_PROTO->trace(HT_PROTO,FILE_LINE_FUNC,fmt,a,b,c,d,e)
#else
# define HT_HEAVY_DBG(fmt)
# define HT_HEAVY_DBG_1(fmt,a)
# define HT_HEAVY_DBG_2(fmt,a,b)
# define HT_HEAVY_DBG_3(fmt,a,b,c)
# define HT_HEAVY_DBG_4(fmt,a,b,c,d)
# define HT_HEAVY_DBG_5(fmt,a,b,c,d,e)
#endif /* HEAVY_DEBUG */


#define HT_WARN(fmt)			HT_PROTO->warn(HT_PROTO,fmt)
#define HT_WARN_1(fmt,a)		HT_PROTO->warn(HT_PROTO,fmt,a)
#define HT_WARN_2(fmt,a,b)		HT_PROTO->warn(HT_PROTO,fmt,a,b)
#define HT_WARN_3(fmt,a,b,c)	HT_PROTO->warn(HT_PROTO,fmt,a,b,c)

#define HT_ERR(fmt)				HT_PROTO->err(HT_PROTO,fmt)
#define HT_ERR_1(fmt,a)			HT_PROTO->err(HT_PROTO,fmt,a)
#define HT_ERR_2(fmt,a,b)		HT_PROTO->err(HT_PROTO,fmt,a,b)
#define HT_ERR_3(fmt,a,b,c)		HT_PROTO->err(HT_PROTO,fmt,a,b,c)

#define GIFT_PLUGIN
#include <libgift/libgift.h>

#include <libgift/giftconfig.h>

#if TIME_WITH_SYS_TIME
# include <time.h>
# include <sys/time.h>
#else /* !TIME_WITH_SYS_TIME */
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif /* TIME_WITH_SYS_TIME */

#ifdef HAVE_STDINT_H
# include <stdint.h>
#endif

/* Just a hack 'til we fix it properly */
#ifndef _MSC_VER
typedef int8_t   ht_int8;
typedef uint8_t  ht_uint8;
typedef int16_t  ht_int16;
typedef uint16_t ht_uint16;
typedef int32_t  ht_int32;
typedef uint32_t ht_uint32;

#else
#define ht_int8 int8_t
#define ht_uint8 uint8_t
#define ht_int16 int16_t
#define ht_uint16 uint16_t
#define ht_int32 int32_t
#define ht_uint32 uint32_t
#endif /* _MSC_VER */

/*****************************************************************************/

#include <ctype.h>
#include <string.h>

#include <libgift/file.h>
#include <libgift/parse.h>
#include <libgift/network.h>
#include <libgift/dataset.h>
#include <libgift/tcpc.h>
#include <libgift/proto/transfer_api.h>
#include <libgift/proto/protocol.h>

#include "ht_hash.h"
#include "ht_download.h"
#include "ht_utils.h"

/*****************************************************************************/

#define HT_PLUGIN ( (HTPlugin*)ht_proto->udata)
#define HT_PROTO (ht_proto)

#define HT_DOWNLOAD_CONNECT_TIMEOUT		(15*SECONDS)
#define HT_DOWNLOAD_HANDSHAKE_TIMEOUT	(10*SECONDS)
#define HT_DOWNLOAD_DATA_TIMEOUT		(10*SECONDS)

/*****************************************************************************/

typedef struct
{
/*
	Config *conf;					// ~/.giFT/HTTP/HTTP.conf
*/
	int dummy; /* so we may not be empty */

} HTPlugin;

/*****************************************************************************/

extern Protocol *ht_proto;			// global pointer to plugin struct

// called by gift to init plugin
#ifdef WIN32
int __declspec(dllexport) Http_init (Protocol *p);
#else
int Http_init (Protocol *p);
#endif

/*****************************************************************************/

#endif /* __HT_HTTP_H */
