/* cache.h
 *
 * simple index cache for UUPC news
 *
 * Author:  Kai Uwe Rommel <rommel@ars.muc.de>
 * Created: Sun Jan 08 1995
 */

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1997 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/* $Id: cache.h 1.4 1996/11/19 00:23:52 ahd Exp $ */

/*
 * $Log: cache.h $
 * Revision 1.4  1996/11/19 00:23:52  ahd
 * Revamp memory management, remove compile warnings
 *
 * Revision 1.3  1996/01/01 21:10:08  ahd
 * Annual Copyright Update
 *
 * Revision 1.2  1995/12/03 13:52:32  ahd
 * Use longs for processing cashe items
 *
 * Revision 1.1  1995/02/20 00:05:26  ahd
 * Initial revision
 *
 * Revision 1.1  1995/02/19 19:18:12  rommel
 * Initial revision
 *
 */

#ifndef _CACHE_H
#define _CACHE_H

#define CACHE_MAGIC      4713

typedef struct _CACHEITEM
{
  struct _CACHEITEM UUFAR *next;
  struct _CACHEITEM UUFAR *prev;
  long index;
  int dirty;
  void UUFAR *buffer;
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
  CACHEITEM UUFAR *head;
  CACHEITEM UUFAR *tail;
}
CACHE;

extern CACHE UUFAR *cache_init(int file,
                                const long items,
                                const size_t itemsize);

extern void cache_flush(CACHE *cache);

extern void cache_exit(CACHE *cache);

extern int cache_get(CACHE *cache, long item, void *buffer);
extern int cache_put(CACHE *cache, long item, void *buffer);

#endif /* _CACHE_H */

/* end of cache.h */
