/*--------------------------------------------------------------------*/
/*    h i s t o r y . c                                               */
/*                                                                    */
/*    News history file maintenance for UUPC/extended.                */
/*                                                                    */
/*--------------------------------------------------------------------*/

/* new version, rewritten for history-based news database
 *
 * Author:  Kai Uwe Rommel <rommel@ars.muc.de>
 * Created: Sun Aug 15 1993
 */

static char *rcsid = "$Id$";
static char *rcsrev = "$Revision$";

/* $Log$ */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "hlib.h"
#include "timestmp.h"
#include "active.h"
#include "history.h"
#include "importng.h"
#include "hdbm.h"

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

char *get_histentry(void *hdbm_file, char *messageID)
{
  datum key, val;

  if (hdbm_file == NULL)
    return NULL;

  key.dptr = messageID;
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
    return FALSE;

  key = dbm_getkey(hdbm_file);

  if (key.dptr == NULL)
    return FALSE;

  val = dbm_fetch(hdbm_file, key);

  *messageID = key.dptr;
  *histentry = val.dptr;

  return TRUE;
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

int add_histentry(void *hdbm_file, char *messageID, char *hist_record)
{
  datum key, val;

  if (hdbm_file == NULL)
    return;

  key.dptr = messageID;
  key.dsize = strlen(key.dptr) + 1;
  val.dptr = hist_record;
  val.dsize = strlen(val.dptr) + 1;

  if (dbm_store(hdbm_file, key, val, DBM_REPLACE))
    return FALSE;

  return TRUE;
}

/*--------------------------------------------------------------------*/
/*    d e l e t e _ h i s t e n t r y                                 */
/*                                                                    */
/*    Delete messageID from the history file.                         */
/*--------------------------------------------------------------------*/

int delete_histentry(void *hdbm_file, char *messageID)
{
  datum key;

  if (hdbm_file == NULL)
    return;

  key.dptr = messageID;
  key.dsize = strlen(key.dptr) + 1;

  if (dbm_delete(hdbm_file, key))
    return FALSE;

  return TRUE;
}

/*--------------------------------------------------------------------*/
/*    c o u n t _ p o s t i n g s                                     */
/*                                                                    */
/*    Count number of postings in a history entry                     */
/*--------------------------------------------------------------------*/

int count_postings(char *histentry)
{
  char value[BUFSIZ], *ptr, *num;
  int count;
  long article;

  strcpy(value, histentry);
  strtok(value, " ");	/* strip off date */
  strtok(NULL, " ");	/* strip off size */
  count = 0;
  
  while ((ptr = strtok(NULL, " ,\n")) != NULL) 
  {
    num = strchr(ptr, ':') + 1;

    if ((article = atol(num)) != 0)
      count++;
  }

  return count;
}

/*--------------------------------------------------------------------*/
/*    p u r g e _ a r t i c l e                                       */
/*                                                                    */
/*    Actually delete an article's file(s) and return remaining ones. */
/*--------------------------------------------------------------------*/

static int matches(char *group, char **grouplist)
{
  int len1 = strlen(group), len2, subgroups;

  if (grouplist == NULL)
    return TRUE;

  for (; *grouplist != NULL; grouplist++)
  {
    len2 = strlen(*grouplist);
    subgroups = FALSE;

    if (*grouplist[len2 - 1] == '*')
      len2--, subgroups = TRUE;

    if (strnicmp(group, *grouplist, min(len1, len2)) == 0)
    {
      if (len1 < len2)
	continue;
      else if (len1 == len2)
	return TRUE;
      else /* len1 > len2 */
      {
	if (group[len2] == '.' && subgroups)
	  return TRUE;
      }

      return TRUE;
    }
  }

  return FALSE;
}

char *purge_article(char *histentry, char **groups)
{
  static char remain[BUFSIZ];
  char value[BUFSIZ];
  char filename[FILENAME_MAX];
  char *group, *num;
  long article, remaining;

  strcpy(value, histentry);
  num = strtok(value, " ");	/* strip off date */
  strcpy(remain, num);
  strcat(remain, " ");
  num = strtok(NULL, " ");	/* strip off size */
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
	unlink(filename);
      }
    }
    else
    {
      strcat(remain, remaining ? "," : "");
      strcat(remain, group);
      strcat(remain, ":");
      strcat(remain, num);
      remaining++;
    }
  }

  return remaining ? remain : NULL;
}

/*--------------------------------------------------------------------*/
/*    c a n c e l _ a r t i c l e                                     */
/*                                                                    */
/*    Cancel an article in the database                               */
/*--------------------------------------------------------------------*/

void cancel_article(void *hdbm_file, char *messageID)
{
  datum key, val;
  char *groups;

  if (hdbm_file == NULL)
    return NULL;

  key.dptr = messageID;
  key.dsize = strlen(key.dptr) + 1;

  val = dbm_fetch(hdbm_file, key);

  if (val.dptr == NULL) 
    printmsg(4,"Cannot find article to cancel in history");
  else
  {
    groups = strchr(val.dptr, ' ') + 1;  /* date */
    groups = strchr(groups, ' ') + 1;    /* size */
    printmsg(0,"cancelling %s", groups);
    delete_histentry(hdbm_file, messageID);
    purge_article(val.dptr, NULL);
  }
}
