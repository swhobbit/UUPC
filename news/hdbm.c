/* hdbm.c
 *
 * simple history database manager for UUPC news
 *
 * Author:  Kai Uwe Rommel <rommel@ars.muc.de>
 * Created: Sun Aug 15 1993
 */
 
static char *rcsid = "$Id: HDBM.C 1.1 1993/09/05 10:56:49 rommel Exp $";
static char *rcsrev = "$Revision: 1.1 $";

/* $Log: HDBM.C $
 * Revision 1.1  1993/09/05  10:56:49  rommel
 * Initial revision
 * */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>

#include "hdbm.h"
#include "idx.h"

datum nullitem = {NULL, 0};

DBM *dbm_open(char *name, int flags, int mode)
{
  DBM *db;
  char filename[_MAX_PATH];

  if ((db = (DBM *) malloc(sizeof(DBM))) == NULL)
    return NULL;

  strcpy(filename, name);
  strcat(filename, DBM_EXT_DBF);

  if ((db -> dbffile = open(filename, flags | O_BINARY, mode)) == -1)
    return NULL;

  strcpy(filename, name);
  strcat(filename, DBM_EXT_IDX);

  if ((db -> idxfile = open(filename, flags | O_BINARY, mode)) == -1)
    return close(db -> dbffile), NULL;

  if ((db -> idx = idx_init(db -> idxfile)) == NULL)
    return close(db -> dbffile), close(db -> idxfile), NULL;

  db -> magic = DBM_MAGIC;

  return db;
}

void dbm_close(DBM *db)
{
  if (db == NULL || db -> magic != DBM_MAGIC)
    return;

  idx_exit(db -> idx);

  close(db -> idxfile);
  close(db -> dbffile);

  free(db);
}

#ifdef __TURBOC__
#pragma argsused
#elif _MSC_VER >= 700
#pragma warning(disable:4100)   /* suppress unref'ed formal param. warnings */
#endif

int dbm_store(DBM *db, datum key, datum val, int flag)
{
  char buffer[BUFSIZ];
  long offset;
  int size;

  if (db == NULL || db -> magic != DBM_MAGIC)
    return -1;

  if ((offset = lseek(db -> dbffile, 0, SEEK_END)) == -1L)
    return -1;

  memcpy(buffer, key.dptr, key.dsize);
  size = key.dsize;
  buffer[size - 1] = ' '; /* replace zero */
  memcpy(buffer + size, val.dptr, val.dsize);
  size += val.dsize;
  buffer[size - 1] = '\n';

  if (idx_addkey(db -> idx, key.dptr, offset, size) == -1)
    return -1;

  if (write(db -> dbffile, buffer, size) != size)
    return -1;

  return 0;
}

#if _MSC_VER >= 700
#pragma warning(default:4100)   /* restore unref'ed formal param. warnings */
#endif

int dbm_delete(DBM *db, datum key)
{
  char buffer[BUFSIZ];
  long offset;
  int size;

  if (db == NULL || db -> magic != DBM_MAGIC)
    return -1;

  if (idx_delkey(db -> idx, key.dptr, &offset, &size) != -1)
  {
    if ((offset = lseek(db -> dbffile, offset, SEEK_SET)) == -1L)
      return -1;

    memset(buffer, ' ', size - 1);
    buffer[size - 1] = '\n';

    if (write(db -> dbffile, buffer, size) != size)
      return -1;
  }

  return 0;
}

datum dbm_fetch(DBM *db, datum key)
{
  datum val = nullitem;
  long offset;
  int size;

  if (db == NULL || db -> magic != DBM_MAGIC)
    return nullitem;

  if (db -> stream && strcmp(key.dptr, db -> buffer) == 0)
  {
    val.dptr = db -> value;
    val.dsize = strlen(val.dptr) + 1;
  }
  else if (idx_getkey(db -> idx, key.dptr, &offset, &size) != -1)
  {
    if ((offset = lseek(db -> dbffile, offset, SEEK_SET)) == -1L)
      return nullitem;

    if (read(db -> dbffile, db -> buffer, size) != size)
      return nullitem;

    db -> buffer[size - 1] = 0; /* delete \n */

    val.dptr = strchr(db -> buffer, ' ') + 1;
    val.dsize = strlen(val.dptr) + 1;
  }

  return val;
}

/* Accessing the database sequentially is not as easy as the records
 * are of variable length to save space and make it look like a text
 * file. So we just put a stream on top of it and read it this way. */

datum dbm_firstkey(DBM *db)
{
  datum val = nullitem;
  char *ptr;
  int handle;

  if (db == NULL || db -> magic != DBM_MAGIC)
    return nullitem;

  if (lseek(db -> dbffile, 0, SEEK_SET) == -1L)
    return nullitem;

  if ((handle = dup(db -> dbffile)) == -1)
    return nullitem;

  if ((db -> stream = fdopen(handle, "rb")) == NULL)
    return nullitem;

  do /* skip blanked out records */
  {
    if (fgets(db -> buffer, sizeof(db -> buffer), db -> stream) == NULL)
      return nullitem;
  } while (db -> buffer[0] == ' ');

  if ((ptr = strchr(db -> buffer, ' ')) == NULL)
    return nullitem;

  db -> buffer[strlen(db -> buffer) - 1] = 0; /* delete \n */

  *ptr = 0;
  db -> value = ptr + 1;

  val.dptr = db -> buffer;
  val.dsize = strlen(db -> buffer) + 1;

  return val;
}

datum dbm_nextkey(DBM *db)
{
  datum val = nullitem;
  char *ptr;

  if (db == NULL || db -> magic != DBM_MAGIC || db -> stream == NULL)
    return nullitem;

  do /* skip blanked out records */
  {
    if (fgets(db -> buffer, sizeof(db -> buffer), db -> stream) == NULL)
      return fclose(db -> stream), (db -> stream = NULL), nullitem;
  } while (db -> buffer[0] == ' ');

  if ((ptr = strchr(db -> buffer, ' ')) == NULL)
    return nullitem;

  db -> buffer[strlen(db -> buffer) - 1] = 0; /* delete \n */

  *ptr = 0;
  db -> value = ptr + 1;

  val.dptr = db -> buffer;
  val.dsize = strlen(db -> buffer) + 1;

  return val;
}

/* end of hdbm.c */
