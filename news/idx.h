/* idx.h
 *
 * simple index manager for UUPC news
 *
 * Author:  Kai Uwe Rommel <rommel@ars.muc.de>
 * Created: Sun Aug 15 1993
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

/* $Id: idx.h 1.8 1996/11/19 00:23:52 ahd Exp $ */

/* $Log: idx.h $
/* Revision 1.8  1996/11/19 00:23:52  ahd
/* Revamp memory management, remove compile warnings
/*
/* Revision 1.7  1996/01/01 21:09:54  ahd
/* Annual Copyright Update
/*
/* Revision 1.6  1995/09/24 19:11:38  ahd
/* Use standard length buffer
/*
/* Revision 1.5  1995/02/20 00:05:26  ahd
/* Kai Uwe Rommel's cache support
/*
 * Revision 1.4  1995/01/29 14:04:21  ahd
 * Clean up IBM C/Set compiler warnings
 *
 * Revision 1.3  1995/01/03 05:33:54  ahd
 * Make as many parameters CONST as possible
 *
 * Revision 1.2  1994/03/20 23:37:12  rommel
 * Correct 16/32 bit compiler differences
 *
 * Revision 1.1  1993/09/05  10:56:49  rommel
 * Initial revision
 * */

#include "cache.h"

#ifndef _IDX_H
#define _IDX_H

#define IDX_MAGIC      4712

#define IDX_MAXKEY     80
#define IDX_MINITEM    10
#define IDX_MAXITEM    (IDX_MINITEM * 2)
#define IDX_MAXSTACK   20

#pragma pack(1)
typedef struct
{
  char key[IDX_MAXKEY];
  long offset;
  unsigned short size;
  long child;
}
ITEM;

typedef struct
{
  unsigned short items;
  ITEM item[IDX_MAXITEM];
  long child_0;
}
PAGE;
#pragma pack()

typedef struct
{
  int magic;
  int file;
  unsigned long size;
  CACHE UUFAR *cache;
  PAGE page;
  long page_number;
  int page_dirty;
  long page_stack[IDX_MAXSTACK];
  size_t page_stacksize;
}
IDX;

extern IDX *idx_init(const int file);

extern void idx_exit(IDX *idx);

extern int idx_addkey(IDX *idx,
                      const char *key,
                      const long offset,
                      const size_t size);

extern int idx_getkey(IDX *idx,
                      const char *key,
                      long *offset,
                      size_t *size);

extern int idx_delkey(IDX *idx,
                      const char *key,
                      long *offset,
                      size_t *size);

#endif /* _IDX_H */

/* end of idx.h */
