/*--------------------------------------------------------------------*/
/*    a c t i v e . h                                                 */
/*                                                                    */
/*    Structures and function prototypes for active file              */
/*    processing in UUPC/extended.  Written by Mike Lipsie            */
/*--------------------------------------------------------------------*/

struct grp {
   struct grp *grp_next;   /* Next group */
   char *grp_name;      /* Group name -- "ba.food" for example */
   long   grp_high;  /* Number for next article to be stored */
   long   grp_low;       /* Lowest numbered article not expired */
   char  grp_can_post;
   };

extern struct grp *group_list;   /* List of all groups */

void get_active(void);     /* Read in the active newsgroups file */
void put_active(void);     /* Write the active newsgroups file */
void validate_newsgroups(void);  /* Make sure the directory structure exists */

#ifndef MAXGRP
#define MAXGRP FILENAME_MAX   /* Max length of a news group name     */
#endif
