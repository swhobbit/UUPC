/* idx.h
 *
 * simple index manager for UUPC news
 *
 * Author:  Kai Uwe Rommel <rommel@ars.muc.de>
 * Created: Sun Aug 15 1993
 */
 
/* $Id: IDX.H 1.1 1993/09/05 10:56:49 rommel Exp rommel $ */

/* $Log: IDX.H $
 * Revision 1.1  1993/09/05  10:56:49  rommel
 * Initial revision
 * */

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
  short size;
  long child;
}
ITEM;

typedef struct
{
  short items;
  ITEM item[IDX_MAXITEM];
  long child_0;
}
PAGE;
#pragma pack()

typedef struct
{
  int magic;
  int file;
  long size;
  PAGE page;
  long page_number;
  int page_dirty;
  long page_stack[IDX_MAXSTACK];
  int page_stacksize;
}
IDX;

extern IDX *idx_init(int file);
extern void idx_exit(IDX *idx);
extern int idx_addkey(IDX *idx, char *key, long offset, int size);
extern int idx_getkey(IDX *idx, char *key, long *offset, int *size);
extern int idx_delkey(IDX *idx, char *key, long *offset, int *size);

#endif /* _IDX_H */

/* end of idx.h */
