/* hdbm.c
 *
 * simple history database manager for UUPC news
 *
 * Author:  Kai Uwe Rommel <rommel@ars.muc.de>
 * Created: Sun Aug 15 1993
 */

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-2002 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"

RCSID("$Id: hdbm.c 1.24 2001/03/12 13:54:04 ahd v1-13k $");

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 * $Id: hdbm.c 1.24 2001/03/12 13:54:04 ahd v1-13k $
 *
 * $Log: hdbm.c $
 * Revision 1.24  2001/03/12 13:54:04  ahd
 * Annual Copyright update
 *
 * Revision 1.23  2000/05/12 12:26:40  ahd
 * Annual copyright update
 *
 * Revision 1.22  1999/01/08 02:20:52  ahd
 * Convert currentfile() to RCSID()
 *
 * Revision 1.21  1999/01/04 03:53:30  ahd
 * Annual copyright change
 *
 * Revision 1.20  1998/03/01 01:30:15  ahd
 * Annual Copyright Update
 *
 * Revision 1.19  1997/12/22 14:15:29  ahd
 * Correct subroutine name
 *
 * Revision 1.18  1997/12/22 14:12:44  ahd
 * Correct off-by-2 error in computing length of string to allocate
 * Add debugging information for failure to backup/rename files
 *
 * Revision 1.17  1997/04/24 00:56:54  ahd
 * Delete MAKEBUF/FREEBUF support
 *
 * Revision 1.16  1997/03/31 06:56:40  ahd
 * Annual Copyright Update
 *
 * Revision 1.15  1996/11/18 04:46:49  ahd
 * Normalize arguments to bugout
 * Reset title after exec of sub-modules
 * Normalize host status names to use HS_ prefix
 *
 * Revision 1.14  1996/01/01 21:06:57  ahd
 * Annual Copyright Update
 *
 * Revision 1.13  1995/12/02 14:18:33  ahd
 * New debugging messages
 *
 * Revision 1.12  1995/09/24 19:10:36  ahd
 * Use standard buffer length for processing in all environments
 *
 * Revision 1.11  1995/03/11 12:39:24  ahd
 * Correct compiler warnings for size mismatches
 *
 * Revision 1.10  1995/03/11 00:18:45  rommel
 * Trap possible spaces in message ids
 *
 * Revision 1.9  1995/01/29 14:03:29  ahd
 * Clean up IBM C/Set compiler warnings
 *
 * Revision 1.8  1995/01/03 05:32:26  ahd
 * Further SYS file support cleanup
 *
 * Revision 1.7  1994/12/22 00:24:36  ahd
 * Annual Copyright Update
 *
 * Revision 1.6  1994/06/14 01:19:24  ahd
 * Clean yp RCS information
 * patches from Kai Uwe Rommel
 *
 * Revision 1.5  1994/03/07  06:09:51  ahd
 * Add additional error messages to error returns
 *
 * Revision 1.4  1994/02/19  04:21:38  ahd
 * Use standard first header
 *
 * Revision 1.4  1994/02/19  04:21:38  ahd
 * Use standard first header
 *
 * Revision 1.3  1994/01/18  13:29:22  ahd
 * Add standard UUPC/extended error logging routines for run time
 * library errors
 *
 * Revision 1.2  1993/11/06  17:54:55  rhg
 * Drive Drew nuts by submitting cosmetic changes mixed in with bug fixes
 *
 * Revision 1.1  1993/09/05  10:56:49  rommel
 * Initial revision
 * */

#include <io.h>
#include <fcntl.h>

#include "hdbm.h"
#include "idx.h"

datum nullitem = {NULL, 0};

DBM *dbm_open(const char *name, const unsigned int flags, const int mode)
{
  DBM *db;
  char filename[ FILENAME_MAX ];

  db = (DBM *) malloc(sizeof(DBM));
  checkref( db );             /* Panic if malloc() failed      */

  strcpy(filename, name);
  strcat(filename, DBM_EXT_DBF);

  if ((db -> dbffile = open(filename, (int) (flags | O_BINARY), mode)) == -1)
  {
    printerr( filename );
    free(db);
    return NULL;
  }

  strcpy(filename, name);
  strcat(filename, DBM_EXT_IDX);

  if ((db -> idxfile = open(filename, (int) (flags | O_BINARY), mode)) == -1)
  {
    printerr( filename );
    close(db -> dbffile);
    db -> dbffile = -1;
    free(db);
    return (DBM *) NULL;
  }

  if ((db -> idx = idx_init(db -> idxfile)) == NULL)
  {
    printmsg(0,"Unable to initialize index");
    close(db -> dbffile);
    close(db -> idxfile);
    db -> idxfile = -1;
    db -> dbffile = -1;
    free(db);
    return (DBM *) NULL;
  }

  db -> magic = DBM_MAGIC;

  return db;

} /* dbm_open */

void dbm_close(DBM *db)
{
  static const char mName[] = "void dbm_close";

  if (db == NULL || db -> magic != DBM_MAGIC)
  {
    printmsg(0,"%s: Internal error, invalid DBM file",
               mName );
    panic();
  }

  idx_exit(db -> idx);

  close(db -> idxfile);
  close(db -> dbffile);
  db -> idxfile = -1;
  db -> dbffile = -1;

  free(db);
}

#ifdef __TURBOC__
#pragma argsused
#elif _MSC_VER >= 700
#pragma warning(disable:4100)   /* suppress unref'ed formal param. warnings */
#endif

int dbm_store(DBM *db, const datum key, const datum val, const int flag)
{
  char buffer[ DBM_BUFSIZ ];
  long offset;
  size_t size;

  if (db == NULL || db -> magic != DBM_MAGIC)
    return -1;

  if ((offset = lseek(db -> dbffile, 0, SEEK_END)) == -1L)
  {
     printerr( "dbm_store");
     return -1;
  }

  memcpy(buffer, key.dptr, key.dsize);
  size = key.dsize;
  buffer[size - 1] = ' '; /* replace zero */
  memcpy(buffer + size, val.dptr, val.dsize);
  size += val.dsize;
  buffer[size - 1] = '\n';

  if (idx_addkey(db -> idx, key.dptr, offset, size) == -1)
  {
#ifdef UDEBUG
     printmsg(10,"dbm_store: idx_addkey failed for key %s", key.dptr );
#endif

    return -1;
  }

  if (write(db -> dbffile, buffer, size) != (int) size)
  {
    printerr( "dbm_store" );
    return -1;
  }

  return 0;

} /* dbm_store */

#if _MSC_VER >= 700
#pragma warning(default:4100)   /* restore unref'ed formal param. warnings */
#endif

int dbm_delete(DBM *db, const datum key)
{
  char buffer[ DBM_BUFSIZ ];
  long offset;
  size_t size;

  if (db == NULL || db -> magic != DBM_MAGIC)
    return -1;

  if (idx_delkey(db -> idx, key.dptr, &offset, &size) != -1)
  {
    if ((offset = lseek(db -> dbffile, offset, SEEK_SET)) == -1L)
    {
       return -1;
    }

    memset(buffer, ' ', size - 1);
    buffer[size - 1] = '\n';

    if (write(db -> dbffile, buffer, size) != (int) size)
    {
       return -1;
    }
  }

  return 0;

} /* dbm_delete */

datum dbm_fetch(DBM *db, const datum key)
{
  datum val = nullitem;
  long offset;
  size_t size;

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

    if (read(db -> dbffile, db -> buffer, size) != (int) size)
      return nullitem;

    db -> buffer[size - 1] = 0; /* delete \n */

    if ((val.dptr = strchr(db -> buffer, '>')) != NULL)
      val.dptr += 2;
    else if ((val.dptr = strchr(db -> buffer, ' ')) != NULL)
       val.dptr++;
    else
       return nullitem;

    val.dsize = strlen(val.dptr) + 1;
  }

  return val;

} /* dbm_fetch */

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
  {
    printerr( "dbm_firstkey" );
    return nullitem;
  }

  do /* skip blanked out records */
  {
    if (fgets(db -> buffer, sizeof(db -> buffer), db -> stream) == NULL)
      return nullitem;
  } while (db -> buffer[0] == ' ');

  if ((ptr = strchr(db -> buffer, '>')) != NULL)
    ptr++;                          /* Move on to space              */
  else if ((ptr = strchr(db -> buffer, ' ')) == NULL)
     return nullitem;

  db -> buffer[strlen(db -> buffer) - 1] = 0; /* delete \n */

  *ptr = 0;
  db -> value = ptr + 1;

  val.dptr = db -> buffer;
  val.dsize = strlen(db -> buffer) + 1;

  return val;

} /* dbm_firstkey */

datum dbm_nextkey(DBM *db)
{
  datum val = nullitem;
  char *ptr;

  if (db == NULL || db -> magic != DBM_MAGIC || db -> stream == NULL)
    return nullitem;

  do /* skip blanked out records */
  {
    if (fgets(db -> buffer, sizeof(db -> buffer), db -> stream) == NULL)
    {
      fclose(db -> stream);
      db -> stream = NULL;
      return nullitem;
    }
  } while (db -> buffer[0] == ' ');

  if ((ptr = strchr(db -> buffer, '>')) != NULL)
    ptr++;                          /* Move on to space              */
  else if ((ptr = strchr(db -> buffer, ' ')) == NULL)
    return nullitem;

  db -> buffer[strlen(db -> buffer) - 1] = 0; /* delete \n */

  *ptr = 0;
  db -> value = ptr + 1;

  val.dptr = db -> buffer;
  val.dsize = strlen(db -> buffer) + 1;

  return val;

} /* dbm_nextkey */

/* end of hdbm.c */
