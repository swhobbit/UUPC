#ifndef _HDBM_H
#define _HDBM_H

/* hdbm.h
 *
 * simple history database manager for UUPC news
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

/*
 * $Id: hdbm.h 1.10 1996/11/19 00:23:52 ahd Exp $
 *
 * $Log: hdbm.h $
 * Revision 1.10  1996/11/19 00:23:52  ahd
 * Revamp memory management, remove compile warnings
 *
 * Revision 1.9  1996/01/01 21:10:25  ahd
 * Annual Copyright Update
 *
 * Revision 1.8  1995/09/24 19:11:38  ahd
 * Use standard length buffer
 *
 * Revision 1.7  1995/03/11 12:41:10  ahd
 * Correct compiler warnings
 *
 * Revision 1.6  1995/01/29 14:04:21  ahd
 * Clean up IBM C/Set compiler warnings
 *
 * Revision 1.5  1995/01/09 12:35:33  ahd
 * Correct VC++ compiler warnings
 *
 * Revision 1.4  1995/01/03 05:33:54  ahd
 * Make as many parameters CONST as possible
 *
 * Revision 1.3  1994/12/31 03:51:25  ahd
 * First pass of integrating Mike McLagan's news SYS file suuport
 *
 * Revision 1.2  1994/06/14 01:20:12  ahd
 * Clean up RCS information
 *
 * Revision 1.1  1993/09/05  10:56:49  rommel
 * Initial revision
 *
 * Revision 1.1  1993/09/05  10:56:49  rommel
 * Initial revision
 * */

#define DBM_BUFSIZ      2048

#define DBM_MAGIC       4711

#define DBM_EXT_DBF     ".pag"
#define DBM_EXT_IDX     ".dir"

#define DBM_INSERT      0
#define DBM_REPLACE     1

#include "idx.h"

typedef struct _DBM
{
  int magic;
  int dbffile;
  int idxfile;
  IDX *idx;
  FILE *stream;
  char buffer[DBM_BUFSIZ];
  char *value;
} DBM;

typedef struct
{
  char *dptr;
  unsigned int dsize;
}
datum;

extern datum nullitem;

extern DBM *dbm_open(const char *name,
                     const unsigned int flags,
                     const int mode);

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
