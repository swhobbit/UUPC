/* cache.c
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

/*
 *    $Id$
 *
 * $Log: cache.c,v $
 * Revision 1.1  1995/02/19 19:18:12  rommel
 * Initial revision
 *
 */

#include "uupcmoah.h"

static const char rcsid[] =
   "$Id: cache.c,v 1.1 1995/02/19 19:18:12 rommel Exp rommel $";

#include <io.h>
#include <memory.h>

#include "cache.h"

currentfile();

/* I/O functions */

static int cache_read(CACHE *cache, int index, void *buffer)
{
  if (lseek(cache -> file, (long) (index * cache -> itemsize), SEEK_SET) == -1)
  {
    printerr("lseek");
    return -1;
  }

  if (read(cache -> file, buffer, cache -> itemsize) != cache -> itemsize)
  {
    printerr("read");
    return -1;
  }

  cache -> reads++;

  return 0;
}

static int cache_write(CACHE *cache, int index, void *buffer)
{

  if (lseek(cache -> file, (long) (index * cache -> itemsize), SEEK_SET) == -1)
  {
    printerr("lseek");
    return -1;
  }

  if (write(cache -> file, buffer, cache -> itemsize) != cache -> itemsize)
  {
    printerr("write");
    return -1;
  }

  cache -> writes++;

  return 0;
}

/* cache maintenance functions */

static void cache_alloc(CACHE *cache)
{
  CACHEITEM *new;
  int i;

  if (cache -> head != NULL)
    return;

  for (i = 0; i < cache -> items; i++)
  {
    new = malloc(sizeof(CACHEITEM));
    checkref(new);

    new -> buffer = malloc(cache -> itemsize);
    checkref(new -> buffer);

    new -> index = -1;
    new -> dirty = 0;

    if (cache -> head == NULL)
    {
      cache -> head = cache -> tail = new;
      new -> prev = new -> next = NULL;
    }
    else
    {
      new -> prev = NULL;
      new -> next = cache -> head;
      cache -> head -> prev = new;
      cache -> head = new;
    }
  }

  printmsg(9, "cache_alloc: buffers for file %d allocated", cache -> file);
}

static int cache_add(CACHE *cache, int index, void *buffer, int dirty)
{
  CACHEITEM *item;

  if (cache -> items > 0 && cache -> head == NULL)
    cache_alloc(cache); /* delayed until here in case it is never actually used */

  if ((item = cache -> tail) == NULL)
    return dirty ? cache_write(cache, index, buffer) : 0;

  if (item -> dirty)
    if (cache_write(cache, item -> index, item -> buffer) != 0)
      return -1;

  cache -> tail = item -> prev;
  cache -> tail -> next = NULL;

  item -> index = index;
  item -> dirty = dirty;
  memcpy(item -> buffer, buffer, cache -> itemsize);

  item -> prev = NULL;
  item -> next = cache -> head;
  cache -> head -> prev = item;
  cache -> head = item;

  return 0;
}

static CACHEITEM *cache_find(CACHE *cache, int index)
{
  CACHEITEM *item;

  for (item = cache -> head; item != NULL; item = item -> next)
    if (index == item -> index)
    {
      if (item -> prev != NULL) /* maintain LRU order */
      {
        item -> prev -> next = item -> next;
        if (item -> next != NULL)
          item -> next -> prev = item -> prev;
        else
          cache -> tail = item -> prev;

        item -> prev = NULL;
        item -> next = cache -> head;
        cache -> head -> prev = item;
        cache -> head = item;
      }

      return item;
    }

  return NULL;
}

/* interface functions */

CACHE *cache_init(int file, const long items, const size_t itemsize)
{
  CACHE *cache;

  cache = (CACHE *) malloc(sizeof(CACHE));
  checkref(cache);

  cache -> magic    = CACHE_MAGIC;
  cache -> file     = file;
  cache -> itemsize = itemsize;
  cache -> items    = items;

  cache -> reads = cache -> writes = cache -> total = 0;
  cache -> head = cache -> tail = NULL;

  printmsg(9, "cache_init: cache for file %d initialized", file);

  return cache;
}

void cache_flush(CACHE *cache)
{
  CACHEITEM *item;

  if (cache == NULL || cache -> magic != CACHE_MAGIC)
    return;

  for (item = cache -> head; item != NULL; item = item -> next)
    if (item -> dirty)
    {
      printmsg(9, "cache_flush: record %d written to disk", item -> index);
      cache_write(cache, item -> index, item -> buffer);
      item -> dirty = 0;
    }
}

void cache_exit(CACHE *cache)
{
  CACHEITEM *item, *next;
  long percent;

  if (cache == NULL || cache -> magic != CACHE_MAGIC)
    return;

  cache_flush(cache);

  if (cache -> total)
  {
    percent = 100 - (cache -> reads + cache -> writes) * 100 / cache -> total;
    printmsg(2,"cache_exit: %ld read and %ld write I/O calls, hit rate %ld%%",
             cache -> reads, cache -> writes, percent);
  }

  for (item = cache -> head; item != NULL; item = next)
  {
    next = item -> next;
    free(item -> buffer);
    free(item);
  }

  free(cache);
}

int cache_get(CACHE *cache, int index, void *buffer)
{
  CACHEITEM *item;
  int rc;

  if (cache == NULL || cache -> magic != CACHE_MAGIC)
    return -1;

  cache -> total++;

  if ((item = cache_find(cache, index)) != NULL)
  {
    printmsg(9, "cache_get: record %d found in cache", index);
    memcpy(buffer, item -> buffer, cache -> itemsize);
    return 0;
  }

  printmsg(9, "cache_get: reading record %d from disk", index);
  if ((rc = cache_read(cache, index, buffer)) == 0)
    rc = cache_add(cache, index, buffer, 0);

  return rc;
}

int cache_put(CACHE *cache, int index, void *buffer)
{
  CACHEITEM *item;

  if (cache == NULL || cache -> magic != CACHE_MAGIC)
    return -1;

  cache -> total++;

  if ((item = cache_find(cache, index)) != NULL)
  {
    printmsg(9, "cache_put: record %d found in cache", index);
    memcpy(item -> buffer, buffer, cache -> itemsize);
    item -> dirty = 1;
    return 0;
  }

  printmsg(9, "cache_put: writing record %d to cache", index);
  return cache_add(cache, index, buffer, 1);
}

/* end of cache.c */
