/* idx.c
 *
 * simple index manager for UUPC news
 *
 * Author:  Kai Uwe Rommel <rommel@ars.muc.de>
 * Created: Sun Aug 15 1993
 */

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-2001 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"

RCSID("$Id: idx.c 1.18 2000/05/12 12:26:40 ahd v1-13g $");

/*
 * $Log: idx.c $
 * Revision 1.18  2000/05/12 12:26:40  ahd
 * Annual copyright update
 *
 * Revision 1.17  1999/01/08 02:20:52  ahd
 * Convert currentfile() to RCSID()
 *
 * Revision 1.16  1999/01/04 03:53:30  ahd
 * Annual copyright change
 *
 * Revision 1.15  1998/03/01 01:30:20  ahd
 * Annual Copyright Update
 *
 * Revision 1.14  1997/04/24 01:39:09  ahd
 * Annual Copyright Update
 *
 * Revision 1.13  1996/11/18 04:46:49  ahd
 * Normalize arguments to bugout
 * Reset title after exec of sub-modules
 * Normalize host status names to use HS_ prefix
 *
 * Revision 1.12  1996/01/01 21:07:15  ahd
 * Annual Copyright Update
 *
 * Revision 1.11  1995/09/24 19:10:36  ahd
 * Use standard buffer length for processing in all environments
 *
 * Revision 1.10  1995/02/20 00:03:07  ahd
 * Kai Uwe Rommel's cache support
 *
 * Revision 1.9  1995/01/29 14:03:29  ahd
 * Clean up IBM C/Set compiler warnings
 *
 * Revision 1.8  1995/01/03 05:32:26  ahd
 * Further SYS file support cleanup
 *
 * Revision 1.7  1994/05/04 02:40:52  ahd
 * Delete unreferenced variable
 *
 * Revision 1.6  1994/03/20  23:35:57  rommel
 * Handle 16/32 bit compiler differences
 *
 * Revision 1.5  1994/03/07  06:09:51  ahd
 * Add additional error messages to error returns
 *
 * Revision 1.4  1994/02/19  04:22:01  ahd
 * Use standard first header
 *
 * Revision 1.3  1993/11/20  13:47:06  rommel
 * Truncate keys at 80 characters
 *
 * Revision 1.2  1993/11/06  17:54:55  rhg
 * Drive Drew nuts by submitting cosmetic changes mixed in with bug fixes
 *
 * Revision 1.1  1993/09/05  10:56:49  rommel
 * Initial revision
 * */

#include <io.h>

#include "idx.h"
#include "cache.h"

/* page level primitives */

static long idx_new_page(IDX *idx)
{
  idx -> page_dirty = 0;
  idx -> page_number = (long) idx -> size++;

  return idx -> page_number;
}

static int idx_get_page(IDX *idx, const long number)
{
  idx -> page_dirty = 0;
  idx -> page_number = number;

  return cache_get(idx -> cache, idx -> page_number, &idx -> page);
}

static int idx_flush_page(IDX *idx)
{
  if (idx -> page_dirty)
  {
    idx -> page_dirty = 0;
    return cache_put(idx -> cache, idx -> page_number, &idx -> page);
  }

  return 0;
}

/* page stack */

static int idx_push_page(IDX *idx, const long number)
{
  idx_flush_page(idx);

  if (idx -> page_stacksize == IDX_MAXSTACK)
    return -1;

  idx -> page_stack[idx -> page_stacksize++] = idx -> page_number;

  return idx_get_page(idx, number);
}

static int idx_pop_page(IDX *idx)
{
  idx_flush_page(idx);

  if (idx -> page_stacksize == 0)
    return -1;

  return idx_get_page(idx, idx -> page_stack[--idx -> page_stacksize]);
}

/* the workhorses */

static int idx_search(IDX *idx, const char *key)
{

  int n, cmp;

  for (;;)
  {

    for (n = idx -> page.items - 1; n >= 0 ; n--)
    {

      cmp = strcmp(key, idx -> page.item[n].key);

      if (cmp == 0)
        return n;
      else if (cmp > 0)
      {
        if (idx -> page.item[n].child)
        {
          idx_push_page(idx, idx -> page.item[n].child);
          break;
        }
        else
          return -1;
      }

    } /* for (n = idx -> page.items - 1; n >= 0 ; n--) */

    if (n < 0)
    {
      if (idx -> page.child_0)
        idx_push_page(idx, idx -> page.child_0);
      else
        break;
    }

  } /* for (;;) */

  return -1;

} /* idx_search */

static int idx_add(IDX *idx, ITEM object)
{
  int i, n;

  for (;;)
  {

    for (n = idx -> page.items; n > 0; n--)
    {

      if ( strcmp(object.key, idx -> page.item[n - 1].key) > 0 )
        break;

    } /* for (n = idx -> page.items; n > 0; n--) */

    if (idx -> page.items < IDX_MAXITEM)
    {
      /* we are lucky */

      for (i = idx -> page.items; i > n; i--)
      {
        idx -> page.item[i] = idx -> page.item[i - 1];
        idx -> page.item[i] = idx -> page.item[i - 1];
      }

      idx -> page.items++;
      idx -> page.item[n] = object;
      idx -> page_dirty = 1;

      idx_flush_page(idx);

      break;
    }
    else {
      /* split page */

      ITEM up;
      PAGE newpage;
      long child_0;

      if (n <= IDX_MINITEM)
      {
        if (n != IDX_MINITEM)
        {
          up = idx -> page.item[IDX_MINITEM - 1];

          for (i = IDX_MINITEM - 1; i > n; i--)
            idx -> page.item[i] = idx -> page.item[i - 1];

          idx -> page.item[n] = object;
          object = up;
        }

        for (i = 0; i < IDX_MINITEM; i++)
          newpage.item[i] = idx -> page.item[IDX_MINITEM + i];
      }
      else {
        up = idx -> page.item[IDX_MINITEM];
        n -= IDX_MINITEM;

        for (i = 0; i < n - 1; i++)
          newpage.item[i] = idx -> page.item[IDX_MINITEM + i + 1];

        newpage.item[n - 1] = object;

        for (i = n; i < IDX_MINITEM; i++)
          newpage.item[i] = idx -> page.item[IDX_MINITEM + i];

        object = up;

      } /* else */

      newpage.child_0 = object.child;
      idx -> page.items = newpage.items = IDX_MINITEM;

      if (idx -> page_stacksize == 0)
      {
        /* grow tree height */

        /* write both pieces onto new pages */

        child_0 = idx_new_page(idx);

        idx -> page_dirty = 1;
        idx_flush_page(idx);

        object.child = idx_new_page(idx);
        memcpy(&idx -> page, &newpage, sizeof(PAGE));

        idx -> page_dirty = 1;
        idx_flush_page(idx);

        /* and keep the root on page zero */

        idx -> page.items = 1;
        idx -> page.item[0] = object;
        idx -> page.child_0 = child_0;

        idx -> page_number = 0;
        idx -> page_dirty = 1;
        idx_flush_page(idx);

        break;
      }
      else {
        /* write lower half onto old page */

        idx -> page_dirty = 1;
        idx_flush_page(idx);

        /* and upper one onto new one */

        object.child = idx_new_page(idx);
        memcpy(&idx -> page, &newpage, sizeof(PAGE));

        idx -> page_dirty = 1;
        idx_flush_page(idx);

        /* and insert middle key into parent page */

        idx_pop_page(idx);

      } /* else */

    } /* else */

  } /* for (;;) */

 return 0;

} /* idx_add */

/* interface functions */

IDX *idx_init(const int file)
{
  IDX *idx;
  long size;

  idx = (IDX *) malloc(sizeof(IDX));
  checkref( idx );

  idx -> magic = IDX_MAGIC;
  idx -> file = file;

  if ((size = lseek(idx -> file, 0, SEEK_END)) == -1)
  {
    printerr( "lseek" );
    free(idx);
    return (IDX *) NULL;
  }

  if (size % sizeof(PAGE) != 0)
  {
    free(idx);
    return (IDX *) NULL; /* consistency check */
  }

  idx -> size = (unsigned long) size / sizeof(PAGE);

  if (idx -> size == 0) /* new (empty) index file needs initialization */
  {
    if (chsize(idx -> file, sizeof(PAGE)) != 0)
    {                         /* let it contain exactly the root page */
      printerr( "chsize" );
      free(idx);
      return (IDX *) NULL;
    }

    if (idx_new_page(idx) != 0)
    {
       free(idx);
       return (IDX *) NULL;
    }
  }

  if (lseek(idx -> file, 0, SEEK_SET) == -1)
  {
    printerr( "lseek" );
    free(idx);
    return (IDX *) NULL;
  }

  if ((idx -> cache = cache_init(idx -> file,
                                 E_newsCache,
                                 sizeof(PAGE))) == NULL)
  {
    printerr( "cache_init" );
    free(idx);
    return (IDX *) NULL;
  }

  memset(&idx -> page, 0, sizeof(PAGE));
  idx -> page_number = -1;
  idx -> page_stacksize = 0;

  return idx;
}

void idx_exit(IDX *idx)
{
  if (idx == NULL || idx -> magic != IDX_MAGIC)
    return;

  idx_flush_page(idx);

  cache_exit(idx -> cache);

  free(idx);
}

int idx_addkey(IDX *idx,
               const char *key,
               const long offset,
               const size_t size)
{
  ITEM object;

  if (idx == NULL || idx -> magic != IDX_MAGIC)
    return -1;

  idx_get_page(idx, 0);
  idx -> page_stacksize = 0;

  if (idx_search(idx, key) != -1)
    return -1;

  strncpy(object.key, key, IDX_MAXKEY - 1);
  object.key[IDX_MAXKEY - 1] = 0;
  object.offset = offset;
  object.size   = (unsigned short) size;
  object.child  = 0;

  if (idx_add(idx, object) == -1)
    return -1;

  return 0;

} /* idx_addkey */

int idx_getkey(IDX *idx, const char *key, long *offset, size_t *size)
{
  int pos;

  if (idx == NULL || idx -> magic != IDX_MAGIC)
    return -1;

  idx_get_page(idx, 0);
  idx -> page_stacksize = 0;

  if ((pos = idx_search(idx, key)) == -1)
    return -1;

  if (idx -> page.item[pos].offset == -1L)
    return -1;

  *offset = idx -> page.item[pos].offset;
  *size   = idx -> page.item[pos].size;

  return 0;
}

int idx_delkey(IDX *idx, const char *key, long *offset, size_t *size)
{
  int pos;

  if (idx == NULL || idx -> magic != IDX_MAGIC)
    return -1;

  idx_get_page(idx, 0);
  idx -> page_stacksize = 0;

  if ((pos = idx_search(idx, key)) == -1)
    return -1;

  if (idx -> page.item[pos].offset == -1L)
    return -1;

  *offset = idx -> page.item[pos].offset;
  *size   = idx -> page.item[pos].size;

  idx -> page.item[pos].offset = -1L;
  idx -> page.item[pos].size   = 0;

  idx -> page_dirty = 1;
  idx_flush_page(idx);

  return 0;
}

/* end of idx.c */
