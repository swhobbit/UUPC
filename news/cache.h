/* cache.h
 *
 * simple index cache for UUPC news
 *
 * Author:  Kai Uwe Rommel <rommel@ars.muc.de>
 * Created: Sun Jan 08 1995
 */

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1995 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/* $Id: cache.h,v 1.1 1995/02/19 19:18:12 rommel Exp rommel $ */

/*
 * $Log: cache.h,v $
 * Revision 1.1  1995/02/19 19:18:12  rommel
 * Initial revision
 *
 */

#ifndef _CACHE_H
#define _CACHE_H

#define CACHE_MAGIC      4713

typedef struct _CACHEITEM
{
  struct _CACHEITEM *next;
  struct _CACHEITEM *prev;
  long index;
  int dirty;
  void *buffer;
}
CACHEITEM;

typedef struct
{
  int magic;
  int file;
  size_t itemsize;
  long items;
  long reads;
  long writes;
  long total;
  CACHEITEM *head;
  CACHEITEM *tail;
}
CACHE;

extern CACHE *cache_init(int file, const long items, const size_t itemsize);

extern void cache_flush(CACHE *cache);

extern void cache_exit(CACHE *cache);

extern int cache_get(CACHE *cache, int item, void *buffer);
extern int cache_put(CACHE *cache, int item, void *buffer);

#endif /* _CACHE_H */

/* end of cache.h */
