# $Id: Makefile.am,v 1.1 2003/09/01 10:33:26 mkern Exp $
AM_CFLAGS = -Wall

plugin_LTLIBRARIES = libHttp.la

libHttp_la_SOURCES = \
	ht_download.c	ht_download.h \
	ht_http.c	ht_http.h \
	ht_hash.c	ht_hash.h \
	ht_header.c	ht_header.h \
	ht_utils.c	ht_utils.h \
	md5.c		md5.h

INCLUDES = \
	@GIFT_CFLAGS@

libHttp_la_LDFLAGS = @GIFT_LIBS@ -module -avoid-version
