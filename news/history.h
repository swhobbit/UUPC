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
/*       Changes Copyright (c) 1989-1995 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/* $Id: history.h 1.3 1995/01/03 05:33:54 ahd v1-12n $ */

/* $Log: history.h $
/* Revision 1.3  1995/01/03 05:33:54  ahd
/* Make as many parameters CONST as possible
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

void *open_history(char *name);
void close_history(void *hdbm_file);

int add_histentry(void *hdbm_file,
                  const char *messageID,
                  const char *hist_record);
int delete_histentry(void *hdbm_file, const char *messageID);

char *get_histentry(void *hdbm_file,
                    const char *messageID);

int get_first_histentry(void *hdbm_file, char **messageID, char **histentry);
int get_next_histentry(void *hdbm_file, char **messageID, char **histentry);

int count_postings(char *histentry);
char *purge_article(char *histentry, char **groups);
KWBoolean cancel_article(void *hdbm_file, const char *messageID);
