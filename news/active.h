/*--------------------------------------------------------------------*/
/*    a c t i v e . h                                                 */
/*                                                                    */
/*    Structures and function prototypes for active file              */
/*    processing in UUPC/extended.  Written by Mike Lipsie            */
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
 *    $Id: lib.h 1.27 1995/01/07 16:16:05 ahd Exp $
 *
 *    Revision history:
 *    $Log: lib.h $
 */

struct grp
{
   struct grp *grp_next;      /* Next group                          */
   char *grp_name;            /* Group name -- "ba.food" for example */
   long   grp_high;           /* Number of next article to be stored */
   long   grp_low;            /* Lowest numbered article not expired */
   char  grp_can_post;
};

extern struct grp *group_list;   /* List of all groups               */

void
get_active( const KWBoolean willUpdate );
                              /* Read in the active newsgroups file  */

void put_active(void);        /* Write the active newsgroups file    */

struct grp *find_newsgroup(const char *grp);
                              /* Find pointer for specified group grp*/

KWBoolean add_newsgroup(const char *grp, const KWBoolean moderated);

KWBoolean del_newsgroup(const char *grp);

KWBoolean get_snum(const char *group, char *snum);

#ifndef MAXGRP
#define MAXGRP FILENAME_MAX   /* Max length of a news group name      */
#endif
