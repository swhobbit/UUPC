/*--------------------------------------------------------------------*/
/*    h i s t o r y . h                                               */
/*                                                                    */
/*    News history file maintenance for UUPC/extended.                */
/*                                                                    */
/*--------------------------------------------------------------------*/

/* new version, rewritten for history-based news database
 * Author:  Kai Uwe Rommel <rommel@ars.muc.de>
 * Created: Sun Aug 15 1993
 */
 
/* $Id$ */

/* $Log$ */

void *open_history(char *name);
void close_history(void *hdbm_file);

int add_histentry(void *hdbm_file, char *messageID, char *hist_record);
int delete_histentry(void *hdbm_file, char *messageID);

char *get_histentry(void *hdbm_file, char *messageID);
int get_first_histentry(void *hdbm_file, char **messageID, char **histentry);
int get_next_histentry(void *hdbm_file, char **messageID, char **histentry);

int count_postings(char *histentry);
char *purge_article(char *histentry, char **groups);
void cancel_article(void *hdbm_file, char *messageID);
