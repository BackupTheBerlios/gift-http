/*
 * $Id: ht_hash.h,v 1.1 2003/09/01 10:33:26 mkern Exp $
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

#ifndef __HT_HASH_H
#define __HT_HASH_H

#include "md5.h"

/*****************************************************************************/

#define MD5_HASH_STR_LEN	32	/* length of string representing hash without terminating '\0' */

/*****************************************************************************/

unsigned char *ht_giftcb_MD5 (const char *path, size_t *len);

char *ht_giftcb_MD5_human (unsigned char *MD5);

/*****************************************************************************/

/*
 * creates human readable string of hash
 * returned string is only valid till next call of function
 */
char *ht_hash_get_string (unsigned char *hash);

/* sets hash from human readable string, */
int ht_hash_set_string (unsigned char *hash, char *string);

/*****************************************************************************/

#endif /* __HT_HASH_H */
