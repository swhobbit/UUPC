/* idx.c
 *
 * simple index manager for UUPC news
 *
 * Author:  Kai Uwe Rommel <rommel@ars.muc.de>
 * Created: Sun Aug 15 1993
 */

#include "uupcmoah.h"

static char *rcsid = "$Id: idx.c 1.4 1994/02/19 04:22:01 ahd Exp $";
static char *rcsrev = "$Revision: 1.4 $";

/* $Log: idx.c $
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

currentfile();

/* page level primitives */

static long idx_new_page(IDX *idx)
{
  idx -> page_dirty = 0;
  idx -> page_number = idx -> size++;

  return idx -> page_number;
}

static int idx_get_page(IDX *idx, long number)
{
  long offset;

  idx -> page_dirty = 0;
  idx -> page_number = number;
  offset = idx -> page_number * sizeof(PAGE);

  if (lseek(idx -> file, offset, SEEK_SET) == -1)
  {
    printerr("lseek");
    return -1;
  }

  if (read(idx -> file, &idx -> page, sizeof(PAGE)) != sizeof(PAGE))
  {
    printerr("read");
    return -1;
  }

  return 0;
}

static int idx_flush_page(IDX *idx)
{
  long offset;

  if (idx -> page_dirty)
  {
    idx -> page_dirty = 0;
    offset = idx -> page_number * sizeof(PAGE);

    if (lseek(idx -> file, offset, SEEK_SET) == -1)
    {
      printerr("lseek");
      return -1;
    }

    if (write(idx -> file, &idx -> page, sizeof(PAGE)) != sizeof(PAGE))
    {
      printerr("write");
      return -1;
    }
  }

  return 0;
}

static int idx_push_page(IDX *idx, long number)
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

static int idx_search(IDX *idx, char *key)
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
    }

    if (n < 0)
      if (idx -> page.child_0)
        idx_push_page(idx, idx -> page.child_0);
      else
        return -1;
  }
}

static int idx_add(IDX *idx, ITEM new)
{
  int i, n;

  for (;;)
  {
    for (n = idx -> page.items; n > 0; n--)
      if ( strcmp(new.key, idx -> page.item[n - 1].key) > 0 )
        break;

    if (idx -> page.items < IDX_MAXITEM)
    {
      /* we are lucky */

      for (i = idx -> page.items; i > n; i--)
      {
        idx -> page.item[i] = idx -> page.item[i - 1];
        idx -> page.item[i] = idx -> page.item[i - 1];
      }

      idx -> page.items++;
      idx -> page.item[n] = new;
      idx -> page_dirty = 1;

      idx_flush_page(idx);

      return 0;
    }
    else
    {
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

          idx -> page.item[n] = new;
          new = up;
        }

        for (i = 0; i < IDX_MINITEM; i++)
          newpage.item[i] = idx -> page.item[IDX_MINITEM + i];
      }
      else
      {
        up = idx -> page.item[IDX_MINITEM];
        n -= IDX_MINITEM;

        for (i = 0; i < n - 1; i++)
          newpage.item[i] = idx -> page.item[IDX_MINITEM + i + 1];

        newpage.item[n - 1] = new;

        for (i = n; i < IDX_MINITEM; i++)
          newpage.item[i] = idx -> page.item[IDX_MINITEM + i];

        new = up;
      }

      newpage.child_0 = new.child;
      idx -> page.items = newpage.items = IDX_MINITEM;

      if (idx -> page_stacksize == 0)
      {
        /* grow tree height */

        /* write both pieces onto new pages */

        child_0 = idx_new_page(idx);

        idx -> page_dirty = 1;
        idx_flush_page(idx);

        new.child = idx_new_page(idx);
        memcpy(&idx -> page, &newpage, sizeof(PAGE));

        idx -> page_dirty = 1;
        idx_flush_page(idx);

        /* and keep the root on page zero */

        idx -> page.items = 1;
        idx -> page.item[0] = new;
        idx -> page.child_0 = child_0;

        idx -> page_number = 0;
        idx -> page_dirty = 1;
        idx_flush_page(idx);

        return 0;
      }
      else
      {
        /* write lower half onto old page */

        idx -> page_dirty = 1;
        idx_flush_page(idx);

        /* and upper one onto new one */

        new.child = idx_new_page(idx);
        memcpy(&idx -> page, &newpage, sizeof(PAGE));

        idx -> page_dirty = 1;
        idx_flush_page(idx);

        /* and insert middle key into parent page */

        idx_pop_page(idx);
      }
    }
  }
}

/* interface functions */

IDX *idx_init(int file)
{
  IDX *idx;
  long size;

  if ((idx = (IDX *) malloc(sizeof(IDX))) == NULL)
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
    return free(idx), (IDX *) NULL; /* consistency check */

  idx -> size = size / sizeof(PAGE);

  if (idx -> size == 0) /* new (empty) index needs initialization */
  {
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

  free(idx);
}

int idx_addkey(IDX *idx, char *key, long offset, int size)
{
  int pos;
  ITEM new;

  if (idx == NULL || idx -> magic != IDX_MAGIC)
    return -1;

  idx_get_page(idx, 0);
  idx -> page_stacksize = 0;

  if ((pos = idx_search(idx, key)) != -1)
    return -1;

  strncpy(new.key, key, IDX_MAXKEY - 1);
  new.key[IDX_MAXKEY - 1] = 0;
  new.offset = offset;
  new.size   = (short) size;
  new.child  = 0;

  if (idx_add(idx, new) == -1)
    return -1;

  return 0;
}

int idx_getkey(IDX *idx, char *key, long *offset, int *size)
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

int idx_delkey(IDX *idx, char *key, long *offset, int *size)
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
