/*--------------------------------------------------------------------*/
/*       s y s . h                                                    */
/*                                                                    */
/*       Header file for SYS.C news system file reading               */
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
 *    $Id: lib.h 1.25 1994/12/27 20:50:28 ahd Exp $
 *
 *    Revision history:
 *    $Log: lib.h $
 */

struct sys {
   struct sys *next;
   char        sysname[40];

   boolean     bExclude;
   long        excl_from,excl_to;

   boolean     bGroups;
   long        grp_from,grp_to;

   boolean     bDistrib;
   long        dist_from,dist_to;

   char        flags[6];
   char        command[128];
};

void    init_sys(FILE *sys_file);

boolean check_sys(struct sys *entry,char *group,char *distrib,char *path);

void    exit_sys(void);

extern struct sys *sys_list;
