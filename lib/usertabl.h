/*
   u s e r t a b l .h

   routines included in usertabl.c

   Copyright (c) 1989, 1990  Andrew H. Derbyshire

   Update History:

   18Mar90     Create from router.c                                  ahd
 */

#ifndef __USERTABL
#define __USERTABLE

#define UUCPSHELL "uucp"         /* Shell for UUCP users                */

struct  UserTable {
      char  *uid;                /* User id for this host to login via  */
      char  *beep;               /* Tone to beep at upon delivery       */
      char  *password;           /* Password for this host to login via */
      char  *realname;           /* Name of the host in question        */
      char  *homedir;            /* Home directory for user             */
      char  *sh;                 /* Shell for this user                 */
      struct HostSecurity *hsecure; /* Pointer to security information  */
   };

extern struct UserTable *users;         /* Public to allow router.c to use it  */
extern size_t UserElements;            /* Public to allow router.c to use it  */

struct UserTable *checkuser ( const char *user );

char *NextField( char *input );

#define BADUSER NULL
#define EMPTY_GCOS "????"

#endif
