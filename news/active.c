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
 *    $Id: active.c 1.16 1995/01/07 16:31:34 ahd Exp $
 *
 *    $Log: active.c $
 *    Revision 1.16  1995/01/07 16:31:34  ahd
 *    Change KWBoolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.15  1995/01/07 16:11:41  ahd
 *    Change KWBoolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.14  1995/01/03 05:32:26  ahd
 *    Drop validate function, we don't exploit it anyway
 *
 *    Revision 1.13  1995/01/02 05:03:27  ahd
 *    Pass 2 of integrating SYS file support from Mike McLagan
 *
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

struct grp *group_list = NULL;      /* List of all groups */

/*--------------------------------------------------------------------*/
/*    g e t _ a c t i v e                                             */
/*                                                                    */
/*    This function opens <newsdir>/active and extracts all the       */
/*    information about the newsgroup we currently maintain           */
/*--------------------------------------------------------------------*/

void get_active( const KWBoolean mustExist )
{
   char active_filename[FILENAME_MAX];
   char grp_name_tmp[MAXGRP];

   FILE *g;
   struct grp *cur_grp;
   struct grp *prev_grp = NULL;
   int i;
   int line = 0;

   mkfilename(active_filename, E_confdir, ACTIVE);
   g = FOPEN(active_filename,"r",TEXT_MODE);

/*--------------------------------------------------------------------*/
/*       If we have no active file, we will allow processing to       */
/*       continue only if we were not expecting it to exist, such     */
/*       as it was being used for for remote batching only.           */
/*--------------------------------------------------------------------*/

   if ( g == NULL )
   {
      if ( mustExist || (debuglevel > 1 ))
         printerr(active_filename);

      if ( mustExist )
         panic();
      else
         return;

   }  /* if ( g == NULL ) */


/*--------------------------------------------------------------------*/
/*              Initialize the first group to be processed            */
/*--------------------------------------------------------------------*/

   group_list = (struct grp *) malloc(sizeof(struct grp));
   cur_grp = group_list;

   memset( cur_grp, 0, sizeof cur_grp );

/*--------------------------------------------------------------------*/
/*       Loop to read in all groups in the file.  Note at the top     */
/*       of the loop we always have one extra link in the list        */
/*       allocated to accept the new entry.                           */
/*--------------------------------------------------------------------*/

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

      memset( cur_grp, 0, sizeof cur_grp );

   } /* while */

/*--------------------------------------------------------------------*/
/*           Done loading groups, close up the active file            */
/*--------------------------------------------------------------------*/

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
   struct grp *cur_grp = group_list;

   if ( cur_grp == NULL )
   {
      printmsg(0, "put_active: Attempt to update empty ACTIVE file");
      panic();
   }

   mkfilename(active_filename, E_confdir, ACTIVE);

   filebkup( active_filename );

   g = FOPEN(active_filename, "w", TEXT_MODE);

   if (g == NULL)
   {
      printmsg(0, "rnews: Cannot open active %s", active_filename );
      printerr(active_filename);
      panic();
   }

/*--------------------------------------------------------------------*/
/*           Loop to actually write out the updated groups            */
/*--------------------------------------------------------------------*/

   while (cur_grp != NULL)
   {
      fprintf(g, "%s %ld %ld %c\n", cur_grp->grp_name,
                              cur_grp->grp_high-1,
                              cur_grp->grp_low,
                              cur_grp->grp_can_post);
      cur_grp = cur_grp->grp_next;
   }

   fclose(g);

} /* put_active */

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

KWBoolean add_newsgroup(const char *grp, const KWBoolean moderated)
{
   struct grp *cur = group_list;

   while ((strcmp(grp, cur->grp_name) != 0))
   {
      if (cur->grp_next != NULL)
      {
         if ( equali( cur->grp_name, grp ) ) /* Group exists?        */
            return KWFalse;          /* Yes --> Cannot add it again   */

         cur = cur->grp_next;
      }
      else                          /* Add group at end              */
         break;

   } /* while ((strcmp(grp, cur->grp_name) != 0)) */

/*--------------------------------------------------------------------*/
/*       We're at the end of the chain with no group found, add it    */
/*       at the end                                                   */
/*--------------------------------------------------------------------*/

   cur->grp_next = (struct grp *) malloc(sizeof(struct grp));
   cur = cur->grp_next;
   checkref(cur);
   cur->grp_next = NULL;
   cur->grp_name = newstr(grp);
   cur->grp_high = 1;
   cur->grp_low  = 0;
   cur->grp_can_post = (char) (moderated ? 'm' : 'y');

/*--------------------------------------------------------------------*/
/*                We added the group, return success.                 */
/*--------------------------------------------------------------------*/

   return KWTrue;

} /* add_newsgroup */

/*--------------------------------------------------------------------*/
/*    d e l _ n e w s g r o u p                                       */
/*                                                                    */
/*    Remove a news group from our list                               */
/*--------------------------------------------------------------------*/

KWBoolean del_newsgroup(const char *grp)
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
         return KWFalse;
      }
   }

   if (prev == NULL)
     group_list = cur->grp_next;
   else
     prev->grp_next = cur->grp_next;

   free(cur);

   /* name string is not free'ed because it's in the string pool */

   return KWTrue;

} /* del_newsgroup */

/*--------------------------------------------------------------------*/
/*    g e t _ s n u m                                                 */
/*                                                                    */
/*    Get highest article number of newsgroup                         */
/*--------------------------------------------------------------------*/

KWBoolean get_snum(const char *group, char *snum)
{
   struct grp *cur;

   strcpy(snum, "0");
   cur = find_newsgroup(group);
   if (cur == NULL)
      return KWFalse;

   sprintf(snum, "%ld", cur->grp_high);
   return KWTrue;

} /* snum */
