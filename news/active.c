/*--------------------------------------------------------------------*/
/*    a c t i v e . c                                                 */
/*                                                                    */
/*    Load and write UUPC/extended news active file                   */
/*--------------------------------------------------------------------*/

/*
 *    $Id: active.c 1.4 1993/04/11 00:31:31 dmwatt Exp $
 *
 *    $Log: active.c $
 *     Revision 1.4  1993/04/11  00:31:31  dmwatt
 *     Global edits for year, TEXT, etc.
 *
 * Revision 1.3  1993/03/06  22:48:23  ahd
 * Correct header files
 *
 * Revision 1.2  1992/11/23  03:56:06  ahd
 * Use strpool for news group names
 *
 */

/*
   This file contains routines that muck with the "active" file.

   The file is named "active" and is in the configuration directory
   defined in UUPC.RC.

   The file is a direct copy of the UNIX active file which is
   defined and described in the Nutshell book "Managing UUCP
   and Usenet".

   IMPORTANT:
   ----------
      Almost no checking is performed on the contents of the file.
      It is critically important that the system administrator
      maintain the file carefully.

   The file consists of one line for each newsgroup to be received.
   Articles destined for newsgroups which do not have a line in the
   active file are lost.

   Each line consists of four fields separated by spaces.

   The fields are:

      group high low post

   where:
      group the newsgroup name.  Case is important.  E.g., ba.food
            This field is maintained by the administrator.
            This field must be less than 50 characters long.

      high  highest article number in the group.  (Zero for a
            new group.)  This field is maintained by rnews.

      low   lowest article number in the group.  (Zero for a
            new group.)  This field is maintained by expire.

      post  can the user post to this newsgroup.  A single
            character. 'y' for yes; 'n' for no; 'm'
            for moderated.  This field is maintained
            by the administrator.

*/


#ifndef __GNUC__

#include "uupcmoah.h"
#include <io.h>
#include <conio.h>
#endif

#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "timestmp.h"
#include "active.h"
#include "importng.h"

#include "getopt.h"

currentfile();

static boolean fallback = FALSE;

struct grp *group_list = NULL;      /* List of all groups */

/*--------------------------------------------------------------------*/
/*    g e t _ a c t i v e                                             */
/*                                                                    */
/*    This function opens <newsdir>/active and extracts all the       */
/*    information about the newsgroup we currently maintain           */
/*--------------------------------------------------------------------*/

void get_active( void )
{
   char active_filename[FILENAME_MAX];
   char grp_name_tmp[51];     /* Space to hold the group field being read in */
   FILE *g;
   struct grp *cur_grp;
   struct grp *prev_grp;
   int i;

/*--------------------------------------------------------------------*/
/*    Open the active file and extract all the newsgroups and         */
/*    their next number.                                              */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*     Try configuration directory first, then try news directory     */
/*--------------------------------------------------------------------*/

   mkfilename(active_filename, E_confdir, ACTIVE);
   g = FOPEN(active_filename,"r",TEXT_MODE);

   if (g == NULL)
   {
      printerr(active_filename);

      mkfilename(active_filename, E_newsdir, ACTIVE);
      fallback= TRUE;
      g = FOPEN(active_filename,"r",TEXT_MODE);
   } /* if */

   if (g == NULL) {
      printerr(active_filename);
      panic();
   }

   prev_grp = NULL;

   group_list = (struct grp *) malloc(sizeof(struct grp));
   cur_grp = group_list;

   cur_grp->grp_next = NULL;
   cur_grp->grp_name = NULL;
   cur_grp->grp_low  = 0;
   cur_grp->grp_high = 0;
   cur_grp->grp_can_post = ' ';

   while ((i = fscanf(g, "%s %ld %ld %1s\n", &grp_name_tmp[0],
            &cur_grp->grp_high,
            &cur_grp->grp_low,
            &cur_grp->grp_can_post)) != EOF)
   {
      if (i != 4)
      {
         printmsg(0,"rnews: incomplete line in %s, %d tokens found",
                     active_filename, i);
         panic();
      }

      cur_grp->grp_name = newstr(grp_name_tmp);

      cur_grp->grp_high++;    /* It is stored as one less than we want it */

      prev_grp = cur_grp;
      cur_grp = (struct grp *) malloc(sizeof(struct grp));
      checkref(cur_grp);
      prev_grp->grp_next = cur_grp;

      cur_grp->grp_next = NULL;
      cur_grp->grp_name = NULL;
      cur_grp->grp_low  = 0;
      cur_grp->grp_high = 0;
      cur_grp->grp_can_post = ' ';

   } /* while */

   if (fclose(g))
      printerr( active_filename );

   if (prev_grp != NULL) {
      prev_grp->grp_next = NULL;
      free(cur_grp);
   }

   return;
} /* get_active */

/*--------------------------------------------------------------------*/
/*    p u t _ a c t i v e                                             */
/*                                                                    */
/*    Update current active file                                      */
/*--------------------------------------------------------------------*/

void put_active()
{
   char active_filename[FILENAME_MAX];
   FILE *g;
   struct grp *cur_grp;

   mkfilename(active_filename, E_confdir, ACTIVE);

   filebkup( active_filename );

   g = FOPEN(active_filename,"w",TEXT_MODE);

   if (g == NULL) {
      printmsg(0, "rnews: Cannot update active %s", active_filename );
      printerr(active_filename);
      panic();
   }

   cur_grp = group_list;

/*--------------------------------------------------------------------*/
/*           Loop to actually write out the updated groups            */
/*--------------------------------------------------------------------*/

   while ((cur_grp != NULL) && (cur_grp->grp_name != NULL))
   {
      fprintf(g, "%s %ld %ld %c\n", cur_grp->grp_name,
                              cur_grp->grp_high-1,
                              cur_grp->grp_low,
                              cur_grp->grp_can_post);
      cur_grp = cur_grp->grp_next;
   }

   fclose(g);

/*--------------------------------------------------------------------*/
/*    Delete old (now obsolete) active file in the news directory     */
/*--------------------------------------------------------------------*/

   if ( fallback )
   {
      mkfilename(active_filename, E_newsdir, ACTIVE);
      filebkup( active_filename );
   }

} /* put_active */

/*--------------------------------------------------------------------*/
/*    v a l i d a t e _ n e w s g r o u p s                           */
/*                                                                    */
/*    Verify all the directories for news groups exist                */
/*--------------------------------------------------------------------*/

void validate_newsgroups( void )
{
   char full_dirname[FILENAME_MAX];

   struct stat buff;

   struct grp *cur_grp;
   int i;

   cur_grp = group_list;
   while (cur_grp != NULL) {
      ImportNewsGroup( full_dirname , cur_grp->grp_name, 0 );

      i = stat(full_dirname, &buff);
      if (i != 0) {
         /* Directory does not exist, create it */
         printmsg(4,"Directory %s does not exist for group %s",
                     full_dirname, cur_grp->grp_name );

#ifdef WASTE_SPACE
         i = MKDIR(full_dirname);
         if (i != 0) {
            printf("Unable to create %s\n", full_dirname);
            panic();
         }
#endif

      } else {
         /* It exists.  Ensure that it is a directory */
         if (!(buff.st_mode & S_IFDIR)) {
            /* Yukk! */
            printmsg(0,"validate_newsgroups: %s is a file not a directory",
                   full_dirname);
            panic();
         }
       }
      cur_grp = cur_grp->grp_next;
   }

   return;
} /* validate_newsgroups */
