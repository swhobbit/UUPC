/* hdbm.h
 *
 * simple history database manager for UUPC news
 *
 * Author:  Kai Uwe Rommel <rommel@ars.muc.de>
 * Created: Sun Aug 15 1993
 */
 
/* $Id: hdbm.h 1.1 1993/09/05 10:56:49 rommel Exp $ */

/* $Log: hdbm.h $
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

#define DBM_INSERT	0
#define DBM_REPLACE	1

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

extern DBM *dbm_open(char *name, int flags, int mode);
extern void dbm_close(DBM *db);
extern int dbm_store(DBM *db, datum key, datum val, int flag);
extern int dbm_delete(DBM *db, datum key);
extern datum dbm_fetch(DBM *db, datum key);
extern datum dbm_firstkey(DBM *db);
extern datum dbm_nextkey(DBM *db);

#endif /* _HDBM_H */

/* end of hdbm.h */
