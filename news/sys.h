#ifndef _SYS_H
#define _SYS_H

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
 *    $Id: sys.h 1.1 1994/12/31 03:51:25 ahd Exp $
 *
 *    Revision history:
 *    $Log: sys.h $
 *    Revision 1.1  1994/12/31 03:51:25  ahd
 *    First pass of integrating Mike McLagan's news SYS file suuport
 *
 */

typedef struct sys
{
   long        excl_from;
   long        excl_to;

   long        grp_from;
   long        grp_to;

   long        dist_from;
   long        dist_to;

   size_t      maximumHops;

   struct sys  *next;
   char        *sysname;
   char        *command;

   struct
   {
      boolean c;              /* Do not compress batch               */
                              /* (supported by UUPC/extended only)   */
      boolean B;              /* Do not send underlength batch       */
                              /* (supported by UUPC/extended only)   */
      boolean F;              /* Batch, write file name only         */
      boolean f;              /* Batch, write file name and length   */
      boolean I;              /* Batch, write article message id     */
      boolean n;              /* Batch, write file name and msg ID   */
      boolean m;              /* Only batch moderated articles       */
      boolean u;              /* Only batch unmoderated articles     */
   }  flag;

   boolean     bExclude;
   boolean     bGroups;
   boolean     bDistrib;

} NEWS_SYS;

void    init_sys( );

boolean check_sys(struct sys *entry,char *group,char *distrib,char *path);

void    exit_sys(void);

extern struct sys *sys_list;

#define OUTGOING_NEWS "out.go"   /* It would be out.going on UNIX or HPFS  */

#endif /* _SYS_H */
