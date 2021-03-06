/* cache.c
 *
 * simple index cache for UUPC news
 *
 * Author:  Kai Uwe Rommel <rommel@ars.muc.de>
 * Created: Sun Jan 08 1995
 */

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-2002 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: cache.c 1.13 2001/03/12 13:54:04 ahd v1-13k $
 *
 * $Log: cache.c $
 * Revision 1.13  2001/03/12 13:54:04  ahd
 * Annual Copyright update
 *
 * Revision 1.12  2000/05/12 12:26:40  ahd
 * Annual copyright update
 *
 * Revision 1.11  1999/01/08 02:20:52  ahd
 * Convert currentfile() to RCSID()
 *
 * Revision 1.10  1999/01/04 03:53:30  ahd
 * Annual copyright change
 *
 * Revision 1.9  1998/03/01 01:30:06  ahd
 * Annual Copyright Update
 *
 * Revision 1.8  1997/12/14 21:17:03  ahd
 * Consistently use UUFAR for cache pointers
 *
 * Revision 1.7  1997/04/24 00:56:54  ahd
 * correct if ferror() to if ( ferror() )
 *
 * Revision 1.6  1996/11/18 04:46:49  ahd
 * Normalize arguments to bugout
 * Reset title after exec of sub-modules
 * Normalize host status names to use HS_ prefix
 *
 * Revision 1.5  1996/01/01 21:06:09  ahd
 * Annual Copyright Update
 *
 * Revision 1.4  1995/12/12 13:48:54  ahd
 * Use binary tree for news group active file
 * Use large buffers in news programs to avoid overflow of hist db recs
 * Use true recursive function to walk entire active file
 *
 * Revision 1.3  1995/12/03 13:51:44  ahd
 * Additional debugging cleanup
 *
 * Revision 1.2  1995/12/02 14:18:33  ahd
 * New debugging messages
 *
 * Revision 1.1  1995/02/20 00:03:07  ahd
 * Initial revision
 *
 * Revision 1.1  1995/02/19 19:18:12  rommel
 * Initial revision
 *
 */

#include "uupcmoah.h"

RCSID("$Id: cache.c 1.13 2001/03/12 13:54:04 ahd v1-13k $");

#include <io.h>
#include <memory.h>
#include <malloc.h>

#include "cache.h"

/* I/O functions */

static int cache_read(CACHE UUFAR *cache, long index, void *buffer)
{
  if (lseek(cache->file, (long) (index * cache->itemsize), SEEK_SET) == -1)
  {
    printerr("lseek");
    return -1;
  }

  if (read(cache->file, buffer, cache->itemsize) != (long) cache->itemsize)
  {
    printerr("read");
    return -1;
  }

  cache->reads++;

  return 0;
}

static int cache_write(CACHE UUFAR *cache, long index, void *buffer)
{

  if (lseek(cache->file, (long) (index * cache->itemsize), SEEK_SET) == -1)
  {
    printerr("lseek");
    return -1;
  }

  if (write(cache->file, buffer, cache->itemsize) != (long) cache->itemsize)
  {
    printerr("write");
    return -1;
  }

  cache->writes++;

  return 0;

} /* cache_write */

/* cache maintenance functions */

static void cache_alloc(CACHE UUFAR *cache)
{
  CACHEITEM UUFAR *newItem;
  long i;

  if (cache->head != NULL)
    return;

  for (i = 0; i < cache->items; i++)
  {
    newItem = (CACHEITEM UUFAR *) MALLOC(sizeof(CACHEITEM));
    checkref(newItem);

    newItem->buffer = MALLOC(cache->itemsize);
    checkref(newItem->buffer);

    newItem->index = -1;
    newItem->dirty = 0;

    if (cache->head == NULL)
    {
      cache->head = cache->tail = newItem;
      newItem->prev = newItem->next = NULL;
    }
    else
    {
      newItem->prev = NULL;
      newItem->next = cache->head;
      cache->head->prev = newItem;
      cache->head = newItem;
    }

  } /* for (i = 0; i < cache->items; i++) */

  printmsg(4, "cache_alloc: %ld cache items with %d byte buffers "
              "for file %d allocated",
               (long) cache->items,
               (int) cache->itemsize,
               cache->file);

} /* cache_alloc */

static int cache_add(CACHE UUFAR *cache, long index, void *buffer, int dirty)
{
  CACHEITEM UUFAR *item;

  if (cache->items > 0 && cache->head == NULL)
    cache_alloc(cache); /* delayed until here in case it is never actually used */

  if ((item = cache->tail) == NULL)
    return dirty ? cache_write(cache, index, buffer) : 0;

  if (item->dirty)
  {
    int rc;

    char *tempBuffer = (char *) malloc( cache->itemsize );

    checkref( tempBuffer );
    MEMCPY( tempBuffer, item->buffer, cache->itemsize );
    rc = cache_write(cache, item->index, tempBuffer);
    free( tempBuffer );

    if (rc != 0)
       return -1;

  } /* if (item->dirty) */

  cache->tail = item->prev;
  cache->tail->next = NULL;

  item->index = index;
  item->dirty = dirty;
  MEMCPY(item->buffer, buffer, cache->itemsize);

  item->prev = NULL;
  item->next = cache->head;
  cache->head->prev = item;
  cache->head = item;

  return 0;

} /* cache_add */

static CACHEITEM UUFAR *cache_find(CACHE UUFAR *cache, long index)
{
  CACHEITEM UUFAR *item;

  for (item = cache->head; item != NULL; item = item->next)
  {
    if (index == item->index)
    {
      if (item->prev != NULL) /* maintain LRU order */
      {
        item->prev->next = item->next;

        if (item->next != NULL)
          item->next->prev = item->prev;
        else
          cache->tail = item->prev;

        item->prev = NULL;
        item->next = cache->head;
        cache->head->prev = item;
        cache->head = item;
      }

      return item;                  /* Return success to caller      */

    }

  } /* for */

/*--------------------------------------------------------------------*/
/*                    Report failure to the caller                    */
/*--------------------------------------------------------------------*/

  return NULL;

} /* cache_find */

/* interface functions */

CACHE UUFAR *cache_init(int file, const long items, const size_t itemsize)
{
  CACHE UUFAR *cache = (CACHE UUFAR *) MALLOC(sizeof(CACHE));

  checkref(cache);

  cache->magic    = CACHE_MAGIC;
  cache->file     = file;
  cache->itemsize = itemsize;

  if ( items < 4 )            /* Avoid funky boundry conditions   */
     cache->items    = 4;
   else
     cache->items    = items;

  cache->reads = cache->writes = cache->total = 0;
  cache->head = cache->tail = NULL;

#ifdef UDEBUG
  printmsg(4, "cache_init: %ld item cache for file %d initialized",
                           items,
                           file);
#endif

  return cache;

} /* cache_init */

void cache_flush(CACHE UUFAR *cache)
{
  CACHEITEM UUFAR *item;
  char *tempBuffer;

  if (cache == NULL || cache->magic != CACHE_MAGIC)
    return;

  tempBuffer = (char *) malloc( cache->itemsize );
  checkref( tempBuffer );

  for (item = cache->head; item != NULL; item = item->next)
    if (item->dirty)
    {
      MEMCPY( tempBuffer, item->buffer, cache->itemsize );
      if ( cache_write(cache, item->index, tempBuffer) == -1 )
      {
         printmsg(0,"cache_flush: Unable to flush record %ld", item->index );
      }
      else {

#ifdef UDEBUG
      printmsg(8, "cache_flush: record %d written to disk", item->index);
#endif
         item->dirty = 0;
      }
    }

    free( tempBuffer );

} /* cache_flush */

void cache_exit(CACHE UUFAR *cache)
{
  CACHEITEM UUFAR *item;
  CACHEITEM UUFAR *next;
  long percent;

  if (cache == NULL || cache->magic != CACHE_MAGIC)
    return;

  cache_flush(cache);

  if (cache->total)
  {
    percent = 100 - (cache->reads + cache->writes) * 100 / cache->total;
    printmsg(2,"cache_exit: %ld read and %ld write I/O calls, hit rate %ld%%",
             cache->reads, cache->writes, percent);
  }

  for (item = cache->head; item != NULL; item = next)
  {
    next = item->next;
    FREE(item->buffer);
    FREE(item);
  }

  FREE(cache);

} /* cache_exit */

int cache_get(CACHE UUFAR *cache, long index, void *buffer)
{
  CACHEITEM UUFAR *item;
  int rc;

  if (cache == NULL || cache->magic != CACHE_MAGIC)
    return -1;

  cache->total++;

  if ((item = cache_find(cache, index)) != NULL)
  {

#ifdef UDEBUG
    printmsg(10, "cache_get: record %d found in cache", index);
#endif

    MEMCPY(buffer, item->buffer, cache->itemsize);
    return 0;
  }

#ifdef UDEBUG
  printmsg(8, "cache_get: reading record %d from disk", index);
#endif

  if ((rc = cache_read(cache, index, buffer)) == 0)
    rc = cache_add(cache, index, buffer, 0);

  return rc;
}

int cache_put(CACHE UUFAR *cache, long index, void *buffer)
{
  CACHEITEM UUFAR *item;

  if (cache == NULL || cache->magic != CACHE_MAGIC)
    return -1;

  cache->total++;

  if ((item = cache_find(cache, index)) != NULL)
  {
#ifdef UDEBUG
    printmsg(10, "cache_put: record %ld found in cache", index);
#endif

    MEMCPY(item->buffer, buffer, cache->itemsize);
    item->dirty = 1;
    return 0;
  }

#ifdef UDEBUG
  printmsg(8, "cache_put: writing record %ld to cache", index);
#endif

  return cache_add(cache, index, buffer, 1);

} /* cache_put */

/* end of cache.c */
