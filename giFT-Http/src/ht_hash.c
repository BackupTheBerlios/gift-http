/*
 * $Id: ht_hash.c,v 1.1 2003/09/01 10:33:28 mkern Exp $
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
#include "ht_hash.h"
#include "md5.h"

#include <libgift/file.h>

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#ifndef WIN32
# include <sys/stat.h>
# include <sys/types.h>
# ifndef O_BINARY
#  define O_BINARY 0
# endif
#else
# include <io.h> /* open() */
# define open(p1, p2) _open(p1, p2)
# define read(p1, p2, p3) _read(p1, p2, p3)
# define close(p1) _close(p1)
#endif /* !WIN32 */

/*****************************************************************************/

static int md5_hash_file (unsigned char *md5, char *file);

/*****************************************************************************/

unsigned char *ht_giftcb_MD5 (const char *path, size_t *len)
{
	unsigned char *hash = malloc (MD5_HASH_LEN);

	md5_hash_file (hash, (char*)path);
	*len = MD5_HASH_LEN;

	return hash;
}

char *ht_giftcb_MD5_human (unsigned char *MD5)
{
	return strdup (ht_hash_get_string (MD5));
}

/*****************************************************************************/

/* hash file, ripped from OpenFT */
static int md5_hash_file (unsigned char *md5, char *file)
{
	MD5Context	  state;
	char         *buf;
	ssize_t       buf_n;
	int           fd;
    struct stat   st;
	off_t         st_size;
	unsigned long st_blksize;

	if (!file)
		return FALSE;

	/* TODO -- does windows have an fstat? */
	if (stat (file, &st) < 0)
	{
		HT_ERR_2 ("Can't stat %s: %s", file, GIFT_STRERROR ());
		return FALSE;
	}

	/* we need to open the file and get the size */
	if ((fd = open (file, O_RDONLY | O_BINARY)) < 0)
	{
		HT_ERR_2 ("Can't open %s: %s", file, GIFT_STRERROR ());
		return FALSE;
	}

	/* take in the stat suggestions */
	st_size    = st.st_size;
#ifndef WIN32
	st_blksize = st.st_blksize;        /* TODO: autoconf test */
#else /* WIN32 */
	st_blksize = 1024;
#endif /* !WIN32 */

	/* sanity clamp */
	if (st_blksize < 512)
		st_blksize = 1024;

	if (!(buf = malloc (st_blksize)))
		return FALSE;

	/* initialize the md5 state */
	MD5Init (&state);

	while ((buf_n = read (fd, buf, MIN ((off_t) st_blksize, st_size))) > 0)
	{
		MD5Update (&state, (unsigned char *) buf, buf_n);
		st_size -= buf_n;

		if (st_size <= 0)
			break;
	}

	MD5Final (md5, &state);

	/* cleanup */
	free (buf);
	close (fd);

	return TRUE;
}

/*****************************************************************************/

/* creates human readable string of hash
 * returned string is only valid till next call of function */
char *ht_hash_get_string (unsigned char *hash)
{
	static const char hex_string[] = "0123456789abcdefABCDEF";
	static char string[MD5_HASH_STR_LEN+1];
	char *p = string;
	int i;

	if (!hash)
		return NULL;

	for(i = 0; i < MD5_HASH_LEN; i++, p += 2)
	{
		p[0] = hex_string[hash[i] >> 4];
		p[1] = hex_string[hash[i] & 0x0F];
	}

	string[MD5_HASH_STR_LEN] = 0;

	return string;
}

/* sets hash from human readable string, */
int h_hash_set_string (unsigned char *hash, char* string)
{
	static const char hex_string[] = "0123456789abcdefABCDEF";
	char *p, *h;
	int i;
	unsigned char hi, lo;

	if (!hash || !string)
		return FALSE;

	for(i = 0, p = string; i < MD5_HASH_LEN; i++, p += 2)
	{
		/* high nibble */
		if( (h = strchr (hex_string, p[0])) == NULL)
			return FALSE;
		hi = (h - hex_string) > 16 ? (h - hex_string - 6) : (h - hex_string);
		/* low nibble */
		if ( (h = strchr (hex_string, p[1])) == NULL)
			return FALSE;
		lo = (h - hex_string) > 16 ? (h - hex_string - 6) : (h - hex_string);

		hash[i] = (hi << 4) | lo;
	}

	return TRUE;
}

/*****************************************************************************/
