/*--------------------------------------------------------------------*/
/*    a c t i v e . c                                                 */
/*                                                                    */
/*    Load and write UUPC/extended news active file                   */
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

/*
 *    $Id: active.c 1.12 1994/12/31 03:41:08 ahd Exp $
 *
 *    $Log: active.c $
 *    Revision 1.12  1994/12/31 03:41:08  ahd
 *    First pass of integrating Mike McLagan's news SYS file suuport
 *
 *    Revision 1.11  1994/12/22 00:07:04  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.10  1994/03/15 03:02:26  ahd
 *    Update copyright, move include for mother of all headers
 *
 *     Revision 1.9  1994/02/19  04:47:46  ahd
 *     Use standard first header
 *
 *     Revision 1.8  1994/02/19  04:12:59  ahd
 *     Use standard first header
 *
 *     Revision 1.7  1994/02/19  04:00:32  ahd
 *     Use standard first header
 *
 *     Revision 1.6  1994/02/18  23:04:47  ahd
 *     Use standard first header
 *
 *     Revision 1.5  1994/02/18  23:04:24  ahd
 *     Use standard first header
 *
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


#include "uupcmoah.h"

#include <io.h>
#include <conio.h>

#include <ctype.h>
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
   int line = 0;

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

/*--------------------------------------------------------------------*/
/*               No active file, die young, stay pretty               */
/*--------------------------------------------------------------------*/

   if ( g == NULL )
   {

      printerr(active_filename);
      panic();

   }  /* if ( g == NULL ) */

/*--------------------------------------------------------------------*/
/*            Build the list of groups in the active file             */
/*--------------------------------------------------------------------*/

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
      line++;

      if (i != 4)
      {
         printmsg(0,"active: incomplete line in %s, %d tokens found"
                    " on line %d",
                     active_filename,
                     i,
                     line );
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

/*--------------------------------------------------------------------*/
/*     Verify we had a valid active file with at least one entry      */
/*--------------------------------------------------------------------*/

   if (prev_grp == NULL)
   {
      printmsg(0,"active: Active file %s is empty, cannot continue",
                  active_filename );
      panic();
   }
   else {                        /* Drop extra slot we had allocated */
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

/*--------------------------------------------------------------------*/
/*    f i n d _ n e w s g r o u p                                     */
/*                                                                    */
/*    Locate a news group in our list                                 */
/*--------------------------------------------------------------------*/

struct grp *find_newsgroup(const char *grp)
{
   struct grp *cur = group_list;

   while (!equal(grp, cur->grp_name))
   {
      if (cur->grp_next != NULL)
      {
         cur = cur->grp_next;
      }
      else {
         return NULL;
      }
   }

   return cur;

}  /* find_newsgroup */

/*--------------------------------------------------------------------*/
/*    a d d _ n e w s g r o u p                                       */
/*                                                                    */
/*    Add a news group to our list                                    */
/*--------------------------------------------------------------------*/

boolean add_newsgroup(const char *grp, const boolean moderated)
{
   struct grp *cur = group_list;

   while ((strcmp(grp, cur->grp_name) != 0))
   {
      if (cur->grp_next != NULL)
      {
         if ( equali( cur->grp_name, grp ) ) /* Group exists?        */
            return FALSE;           /* Yes --> Cannot add it again   */

         cur = cur->grp_next;
      }
      else {
         cur->grp_next = (struct grp *) malloc(sizeof(struct grp));
         cur = cur->grp_next;
         checkref(cur);
         cur->grp_next = NULL;
         cur->grp_name = newstr(grp);
         cur->grp_high = 1;
         cur->grp_low  = 0;
         cur->grp_can_post = (char) (moderated ? 'm' : 'y');
         break;
      }
   }

} /* add_newsgroup */

/*--------------------------------------------------------------------*/
/*    d e l _ n e w s g r o u p                                       */
/*                                                                    */
/*    Remove a news group from our list                               */
/*--------------------------------------------------------------------*/

boolean del_newsgroup(const char *grp)
{
   struct grp *cur = group_list;
   struct grp *prev = NULL;

   while ((strcmp(grp, cur->grp_name) != 0))
   {
      if (cur->grp_next != NULL)
      {
         prev = cur;
         cur = cur->grp_next;
      }
      else {
         return FALSE;
      }
   }

   if (prev == NULL)
     group_list = cur->grp_next;
   else
     prev->grp_next = cur->grp_next;

   free(cur);

   /* name string is not free'ed because it's in the string pool */

   return TRUE;

} /* del_newsgroup */

/*--------------------------------------------------------------------*/
/*    g e t _ s n u m                                                 */
/*                                                                    */
/*    Get highest article number of newsgroup                         */
/*--------------------------------------------------------------------*/

boolean get_snum(const char *group, char *snum)
{
   struct grp *cur;

   strcpy(snum, "0");
   cur = find_newsgroup(group);
   if (cur == NULL)
      return FALSE;

   sprintf(snum, "%ld", cur->grp_high);
   return TRUE;

} /* snum */
