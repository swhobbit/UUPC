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
 *    $Id: sys.h 1.8 1995/01/13 14:03:25 ahd Exp $
 *
 *    Revision history:
 *    $Log: sys.h $
 *    Revision 1.8  1995/01/13 14:03:25  ahd
 *    Add canonical_news_name() macro
 *
 *    Revision 1.7  1995/01/08 19:54:01  ahd
 *    NNS support
 *
 *    Revision 1.6  1995/01/07 20:48:48  ahd
 *    Correct 16 bit compiler warnings
 *
 *    Revision 1.5  1995/01/07 16:21:58  ahd
 *    Change KWBoolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.4  1995/01/07 15:49:37  ahd
 *    Covnert SYS file to keep strings in memory, not on disk
 *
 *    Revision 1.3  1995/01/03 05:33:54  ahd
 *    Add process count to struct
 *
 *    Revision 1.2  1995/01/02 05:04:57  ahd
 *    Pass 2 of integrating SYS file support from Mike McLagan
 *
 *    Revision 1.1  1994/12/31 03:51:25  ahd
 *    First pass of integrating Mike McLagan's news SYS file suuport
 *
 */

typedef struct sys
{
   long        processed;     /* Number of articles processed        */
   char        *exclude;
   char        *groups;
   char        *distribution;

   size_t      maximumHops;

   struct sys  *next;
   char        *sysname;
   char        *command;

/*--------------------------------------------------------------------*/
/*       Flag bits, most of which are set from the third (flag)       */
/*       field in the SYS file by letters of the same name.           */
/*--------------------------------------------------------------------*/

   struct
   {
      KWBoolean local;         /* sysname is local system name        */
      KWBoolean batch;         /* F, f, I, or I flags specified       */
      KWBoolean c;             /* Do not compress batch
                                  (supported by UUPC/extended only)   */
      KWBoolean B;             /* Do not send underlength batch
                                  (supported by UUPC/extended only)   */
      KWBoolean F;             /* Batch, write file name only         */
      KWBoolean J;             /* NSS mode, just write batches w/o
                                  transmitting them anywhere          */
      KWBoolean f;             /* Batch, write file name and length   */
      KWBoolean I;             /* Batch, write article message id     */
      KWBoolean n;             /* Batch, write file name and msg ID   */
      KWBoolean m;             /* Only batch moderated articles       */
      KWBoolean u;             /* Only batch unmoderated articles     */
   }  flag;

} NEWS_SYS;

KWBoolean init_sys( void );

KWBoolean check_sys(struct sys *entry, char *group, char *distrib, char *path);

struct sys *get_sys( const char *name );

void    exit_sys(void);

extern struct sys *sys_list;

#define OUTGOING_NEWS "going.out"   /* It could be out.going on UNIX or HPFS,
                                       but isn't for DOS compatibility        */

#define canonical_news_name()    E_nodename

#endif /* _SYS_H */