#ifndef _HISTORY_H
#define _HISTORY_H

/*--------------------------------------------------------------------*/
/*    h i s t o r y . h                                               */
/*                                                                    */
/*    News history file maintenance for UUPC/extended.                */
/*--------------------------------------------------------------------*/

/* new version, rewritten for history-based news database
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

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/* $Id: history.h 1.11 2001/03/12 13:51:16 ahd v1-13k $
 *
 * $Log: history.h $
 * Revision 1.11  2001/03/12 13:51:16  ahd
 * Annual copyright update
 *
 * Revision 1.10  2000/05/12 12:27:02  ahd
 * Annual copyright update
 *
 * Revision 1.9  1999/01/04 03:55:33  ahd
 * Annual copyright change
 *
 * Revision 1.8  1998/03/01 01:31:26  ahd
 * Annual Copyright Update
 *
 * Revision 1.7  1997/04/24 01:40:16  ahd
 * Annual Copyright Update
 *
 * Revision 1.6  1996/11/19 00:23:52  ahd
 * Revamp memory management, remove compile warnings
 *
 * Revision 1.5  1996/01/01 21:09:45  ahd
 * Annual Copyright Update
 *
 * Revision 1.4  1995/03/11 02:02:28  ahd
 * Return success of cancel to allow caller to report
 *
 * Revision 1.3  1995/01/03 05:33:54  ahd
 * Make as many parameters CONST as possible
 *
 * Revision 1.2  1994/06/14 01:20:12  ahd
 * Clean up RCS information
 *
 * Revision 1.1  1993/09/05  10:56:49  rommel
 * Initial revision
 *
 * Revision 1.1  1993/09/05  10:56:49  rommel
 * Initial revision
 *
 */

#include "hdbm.h"

typedef DBM NEWS_HISTORY;

DBM *open_history(char *name);

void close_history(DBM *hdbm_file);

int add_histentry(DBM *hdbm_file,
                  const char *messageID,
                  const char *hist_record);
int delete_histentry(DBM *hdbm_file, const char *messageID);

char *get_histentry(DBM *hdbm_file,
                    const char *messageID);

int get_first_histentry(DBM *hdbm_file, char **messageID, char **histentry);

int get_next_histentry(DBM *hdbm_file, char **messageID, char **histentry);

int count_postings(char *histentry);

char *purge_article(char *histentry, char **groups);

KWBoolean cancel_article(DBM *hdbm_file, const char *messageID);

#endif
