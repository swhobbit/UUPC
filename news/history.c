/*--------------------------------------------------------------------*/
/*    h i s t o r y . c                                               */
/*                                                                    */
/*    News history file maintenance for UUPC/extended.                */
/*--------------------------------------------------------------------*/

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

/* new version, rewritten for history-based news database
 *
 * Author:  Kai Uwe Rommel <rommel@ars.muc.de>
 * Created: Sun Aug 15 1993
 */

#include "uupcmoah.h"

static const char rcsid[] =
      "$Id: history.c 1.14 1995/12/03 13:51:44 ahd Exp $";

/*
 * $Log: history.c $
 * Revision 1.14  1995/12/03 13:51:44  ahd
 * Additional debugging cleanup
 *
 * Revision 1.13  1995/12/02 14:18:33  ahd
 * Use longer buffers, add new debugging messages
 *
 * Revision 1.12  1995/03/11 22:29:41  ahd
 * Use macro for file delete to allow special OS/2 processing
 *
 * Revision 1.11  1995/03/11 01:59:57  ahd
 * Return result of cancel to caller to allow message
 *
 * Revision 1.10  1995/01/29 14:03:29  ahd
 * Clean up IBM C/Set compiler warnings
 *
 * Revision 1.9  1995/01/07 16:21:23  ahd
 * Change KWBoolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 * Revision 1.8  1995/01/03 05:32:26  ahd
 * Further SYS file support cleanup
 *
 * Revision 1.7  1994/12/27 23:35:28  ahd
 * Various contributed news fixes; make processing consistent, improve logging,
 * use consistent host names
 *
 * Revision 1.6  1994/06/14 01:19:24  ahd
 * Clean yp RCS information
 * patches from Kai Uwe Rommel
 *
 * Revision 1.5  1994/02/19  04:21:49  ahd
 * Use standard first header
 *
 * Revision 1.5  1994/02/19  04:21:49  ahd
 * Use standard first header
 *
 * Revision 1.4  1994/01/18  13:29:22  ahd
 * Add standard UUPC/extended error logging routines for run time
 * library errors
 *
 * Revision 1.3  1993/11/06  17:54:55  rhg
 * Drive Drew nuts by submitting cosmetic changes mixed in with bug fixes
 *
 * Revision 1.2  1993/10/30  11:39:26  rommel
 * fixed some function error returns missing a value
 *
 * Revision 1.1  1993/09/05  10:56:49  rommel
 * Initial revision
 * */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include <fcntl.h>
#include <io.h>
#include <sys/stat.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "timestmp.h"
#include "active.h"
#include "history.h"
#include "importng.h"
#include "hdbm.h"

currentfile();

/*--------------------------------------------------------------------*/
/*    o p e n _ h i s t o r y                                         */
/*                                                                    */
/*    Open the history file.                                          */
/*--------------------------------------------------------------------*/

void *open_history(char *name)
{
   char hfile_name[FILENAME_MAX];
   DBM *hdbm_file = NULL;

   mkfilename(hfile_name, E_newsdir, name);
   hdbm_file = dbm_open(hfile_name, O_RDWR | O_CREAT, 0666);

   if (hdbm_file == NULL) {
      printmsg(0,"Unable to open history file");
      return NULL;
   }

   return hdbm_file;
}

/*--------------------------------------------------------------------*/
/*    c l o s e _ h i s t o r y                                       */
/*                                                                    */
/*    Close the history file.                                         */
/*--------------------------------------------------------------------*/

void close_history(void *hdbm_file)
{
  if (hdbm_file != NULL)
    dbm_close(hdbm_file);
}

/*--------------------------------------------------------------------*/
/*    g e t _ h i s t e n t r y                                       */
/*                                                                    */
/*    Check whether messageID is already in the history file.         */
/*--------------------------------------------------------------------*/

char *get_histentry(void *hdbm_file, const char *messageID)
{
  datum key, val;

  if (hdbm_file == NULL)
    return NULL;

  key.dptr = (char *) messageID;
  key.dsize = strlen(key.dptr) + 1;

  val = dbm_fetch(hdbm_file, key);

  return val.dptr;
}

/*--------------------------------------------------------------------*/
/*    g e t _ * _ h i s t e n t r y                                   */
/*                                                                    */
/*    Retrieve all entries from the history file in order.            */
/*--------------------------------------------------------------------*/

static int get_entry(void *hdbm_file, char **messageID, char **histentry,
                     datum (*dbm_getkey)(DBM *hdbm_file))
{
  datum key, val;

  if (hdbm_file == NULL)
    return KWFalse;

  key = dbm_getkey(hdbm_file);

  if (key.dptr == NULL)
    return KWFalse;

  val = dbm_fetch(hdbm_file, key);

  *messageID = key.dptr;
  *histentry = val.dptr;

  return KWTrue;
}

int get_first_histentry(void *hdbm_file, char **messageID, char **histentry)
{
  return get_entry(hdbm_file, messageID, histentry, dbm_firstkey);
}

int get_next_histentry(void *hdbm_file, char **messageID, char **histentry)
{
  return get_entry(hdbm_file, messageID, histentry, dbm_nextkey);
}

/*--------------------------------------------------------------------*/
/*    a d d _ h i s t e n t r y                                       */
/*                                                                    */
/*    Add messageID to the history file.                              */
/*--------------------------------------------------------------------*/

int add_histentry(void *hdbm_file,
                  const char *messageID,
                  const char *hist_record)
{
  datum key, val;

  if (hdbm_file == NULL)
  {

#ifdef UDEBUG
     printmsg(2,"add_histentry: Invalid call, no file passed");
     panic();
#endif

    return KWFalse;
  }

  key.dptr = (char *) messageID;
  key.dsize = strlen(key.dptr) + 1;
  val.dptr = (char *) hist_record;
  val.dsize = strlen(val.dptr) + 1;

/*--------------------------------------------------------------------*/
/*       Since the DBM_REPLACE keyword doesn't actually work, any     */
/*       attempt to replace a history record will fail.  Thus, we     */
/*       don't actually bother to print the error message below.      */
/*--------------------------------------------------------------------*/

  if (dbm_store(hdbm_file, key, val, DBM_REPLACE))
  {

#ifdef UDEBUG2
     printmsg(2,"add_histentry: dbm_store failed for %s", messageID);
#endif

    return KWFalse;
  }

  return KWTrue;
}

/*--------------------------------------------------------------------*/
/*    d e l e t e _ h i s t e n t r y                                 */
/*                                                                    */
/*    Delete messageID from the history file.                         */
/*--------------------------------------------------------------------*/

int delete_histentry(void *hdbm_file, const char *messageID)
{
  datum key;

  if (hdbm_file == NULL)
    return KWFalse;

  key.dptr = (char *) messageID;
  key.dsize = strlen(key.dptr) + 1;

  if (dbm_delete(hdbm_file, key))
    return KWFalse;

  return KWTrue;
}

/*--------------------------------------------------------------------*/
/*    c o u n t _ p o s t i n g s                                     */
/*                                                                    */
/*    Count number of postings in a history entry                     */
/*--------------------------------------------------------------------*/

int count_postings(char *histentry)
{
  char value[DBM_BUFSIZ], *ptr, *num;
  int count;

  strcpy(value, histentry);
  strtok(value, " ");   /* strip off date */
  strtok(NULL, " ");    /* strip off size */
  count = 0;

  while ((ptr = strtok(NULL, " ,\n")) != NULL)
  {
    num = strchr(ptr, ':') + 1;

    if (atol(num))
      count++;
  }

  return count;
}

static int matches(const char *group, char **grouplist)
{
  size_t len1 = strlen(group);
  KWBoolean subgroups;

  if (grouplist == NULL)
    return KWTrue;

  for (; *grouplist != NULL; grouplist++)
  {
    size_t len2 = strlen(*grouplist);
    subgroups = KWFalse;

    if (len2 && ((*grouplist)[len2 - 1] == '*'))
    {
       len2--;
       subgroups = KWTrue;
    }

    if (strnicmp(group, *grouplist, min(len1, len2)) == 0)
    {
      if (len1 < len2)
        continue;
      else if (len1 == len2)
        return KWTrue;
      else /* len1 > len2 */
      {
        if (group[len2] == '.' && subgroups)
          return KWTrue;
      }

      return KWTrue;
    }
  }

  return KWFalse;

} /* matches */

/*--------------------------------------------------------------------*/
/*    p u r g e _ a r t i c l e                                       */
/*                                                                    */
/*    Actually delete an article's file(s) and return remaining ones. */
/*--------------------------------------------------------------------*/

char *purge_article(char *histentry, char **groups)
{
  static char *remain = NULL;
  char value[DBM_BUFSIZ];
  char filename[FILENAME_MAX];
  char *group, *num;
  long article, remaining;

  if ( remain == NULL )
  {
      remain = malloc( DBM_BUFSIZ );
      checkref( remain );
  }

  strcpy(value, histentry);
  num = strtok(value, " ");     /* strip off date */
  strcpy(remain, num);
  strcat(remain, " ");
  num = strtok(NULL, " ");      /* strip off size */
  strcat(remain, num);
  strcat(remain, " ");
  remaining = 0;

  while ((group = strtok(NULL, " ,\n")) != NULL)
  {
    num = strchr(group, ':');
    *num++ = 0;

    if (matches(group, groups))
    {
      if ((article = atol(num)) != 0)
      {
        ImportNewsGroup(filename, group, article);
        REMOVE(filename);
      }
    }
    else
    {
      strcat(remain, remaining ? "," : "");
      strcat(remain, group);
      strcat(remain, ":");
      strcat(remain, num);
      remaining++;

    }   /* else */

  } /* while ((group = strtok(NULL, " ,\n")) != NULL) */

/*--------------------------------------------------------------------*/
/*       If no articles remain name, free the persistent buffer       */
/*       and return a NULL to the caller; otherwise, just return      */
/*       the list.                                                    */
/*--------------------------------------------------------------------*/

  if ( ! remaining )
  {
      free( remain );
      remain = NULL;
  }

  return remain;

} /* purge_article */

/*--------------------------------------------------------------------*/
/*    c a n c e l _ a r t i c l e                                     */
/*                                                                    */
/*    Cancel an article in the database                               */
/*--------------------------------------------------------------------*/

KWBoolean
cancel_article(void *hdbm_file, const char *messageID)
{
  datum key, val;
  char *groups;

  if (hdbm_file == NULL)
    return KWFalse;

  key.dptr = (char *) messageID;
  key.dsize = strlen(key.dptr) + 1;

  val = dbm_fetch(hdbm_file, key);

  if (val.dptr == NULL)
  {
    printmsg(4,"Cannot find article to cancel in history");
    return KWFalse;
  }

    groups = strchr(val.dptr, ' ') + 1;  /* date */
    groups = strchr(groups, ' ') + 1;    /* size */
    printmsg(0,"cancelling %s", groups);
    purge_article(val.dptr, NULL);
    delete_histentry(hdbm_file, messageID);

  return KWTrue;

} /* cancel_article */
