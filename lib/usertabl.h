/*--------------------------------------------------------------------*/
/*       u s e r t a b l .h                                           */
/*                                                                    */
/*       Routines included in usertabl.c                              */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1997 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: usertabl.h 1.7 1996/01/01 20:58:43 ahd v1-12r $
 *
 *    Revision history:
 *    $Log: usertabl.h $
 *    Revision 1.7  1996/01/01 20:58:43  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.6  1994/12/27 23:36:37  ahd
 *    Allow hostable.h, usertable.h to be included without others
 *
 *    Revision 1.5  1994/12/22 00:14:14  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.4  1994/04/26 03:34:33  ahd
 *    Lower case name
 *
 */

#ifndef __USERTABL
#define __USERTABLE

#define UUCPSHELL "uucp"         /* Shell for UUCP users               */

struct HostSecurity;             /* Suppress warning for undefined struct */

struct  UserTable {
      char  *uid;                /* User id for this host to login via */
      char  *beep;               /* Tone to beep at upon delivery      */
      char  *password;           /* Password for this host to login via */
      char  *realname;           /* Name of the host in question       */
      char  *homedir;            /* Home directory for user            */
      char  *sh;                 /* Shell for this user                */
      struct HostSecurity *hsecure; /* Pointer to security information */
   };

extern struct UserTable *users;        /* Public to allow alias.c to use it */
extern size_t userElements;            /* Public to allow alias.c to use it */

struct UserTable *checkuser ( const char *user );

char *NextField( char *input );

#define BADUSER NULL
#define EMPTY_GCOS "????"

#endif
