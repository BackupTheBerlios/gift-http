/*
 * $Id: ht_utils.h,v 1.1 2003/09/01 10:33:28 mkern Exp $
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

#ifndef __HT_UTILS_H
#define __HT_UTILS_H

/*****************************************************************************/

// caller frees returned string
char *ht_utils_url_decode (char *encoded);

// caller frees returned string
char *ht_utils_url_encode (char *decoded);

/*****************************************************************************/

// caller frees returned string
char *ht_utils_base64_encode (const unsigned char *data, int src_len);

// caller frees returned string
unsigned char *ht_utils_base64_decode (const char *data, int *dst_len);

/*****************************************************************************/

#endif /* __HT_UTILS_H */
