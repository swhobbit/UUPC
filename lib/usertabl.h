/*--------------------------------------------------------------------*/
/*       u s e r t a b l .h                                           */
/*                                                                    */
/*       Routines included in usertabl.c                              */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-2001 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: usertabl.h 1.12 2000/05/12 12:40:50 ahd v1-13g $
 *
 *    Revision history:
 *    $Log: usertabl.h $
 *    Revision 1.12  2000/05/12 12:40:50  ahd
 *    Annual copyright update
 *
 *    Revision 1.11  1999/01/04 03:55:03  ahd
 *    Annual copyright change
 *
 *    Revision 1.10  1998/03/01 19:43:52  ahd
 *    First compiling POP3 server which accepts user id/password
 *
 *    Revision 1.9  1998/03/01 01:28:01  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.8  1997/03/31 07:07:33  ahd
 *    Annual Copyright Update
 *
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

#define UUCPSHELL "uucp"         /* Shell for UUCP users             */
#define POP3_GROUP "pop3"        /* Required group for POP3 users    */
#define UUCP_GROUP "uucp"        /* Required group for normal users  */

struct HostSecurity;             /* Suppress warning for undefined struct */

struct  UserTable {
      char  *uid;                /* User id for this host to login via */
      char  *beep;               /* Tone to beep at upon delivery      */
      char  *group;              /* Group user is member of (for POP3) */
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
