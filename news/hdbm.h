/* hdbm.h
 *
 * simple history database manager for UUPC news
 *
 * Author:  Kai Uwe Rommel <rommel@ars.muc.de>
 * Created: Sun Aug 15 1993
 */

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1995 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/* $Id: hdbm.h 1.3 1994/12/31 03:51:25 ahd Exp $ */

/* $Log: hdbm.h $
/* Revision 1.3  1994/12/31 03:51:25  ahd
/* First pass of integrating Mike McLagan's news SYS file suuport
/*
/* Revision 1.2  1994/06/14 01:20:12  ahd
/* Clean up RCS information
/*
 * Revision 1.1  1993/09/05  10:56:49  rommel
 * Initial revision
 *
 * Revision 1.1  1993/09/05  10:56:49  rommel
 * Initial revision
 * */

#ifndef _HDBM_H
#define _HDBM_H

#define DBM_MAGIC       4711

#define DBM_EXT_DBF     ".pag"
#define DBM_EXT_IDX     ".dir"

#define DBM_INSERT      0
#define DBM_REPLACE     1

typedef struct
{
  int magic;
  int dbffile;
  int idxfile;
  void *idx;
  FILE *stream;
  char buffer[BUFSIZ];
  char *value;
}
DBM;

typedef struct
{
  char *dptr;
  int dsize;
}
datum;

extern datum nullitem;

extern DBM *dbm_open(const char *name, int flags, int mode);
extern void dbm_close(DBM *db);
extern int dbm_store(DBM *db,
                     const datum key,
                     const datum val,
                     const int flag);
extern int dbm_delete(DBM *db, const datum key);
extern datum dbm_fetch(DBM *db, const datum key);
extern datum dbm_firstkey(DBM *db);
extern datum dbm_nextkey(DBM *db);

#endif /* _HDBM_H */

/* end of hdbm.h */
