/*--------------------------------------------------------------------*/
/*    d c p l i b . c                                                 */
/*                                                                    */
/*    DCP system-dependent library                                    */
/*                                                                    */
/*    Services provided by dcplib.c:                                  */
/*                                                                    */
/*    - login                                                         */
/*    - UNIX commands simulation                                      */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1993 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Copyright (c) Richard H. Lamb 1985, 1986, 1987                  */
/*    Changes Copyright (c) Stuart Lynne 1987                         */
/*--------------------------------------------------------------------*/

/*
 *    $Id: DCP.C 1.10 1993/04/11 00:35:46 ahd Exp $
 *
 *    $Log: DCP.C $

   Updated:

      14May89  - Added system name to login prompt - ahd
                 Added configuration file controlled user id, password
                 Added Kermit server option
      17May89  - Redo login processing to time out after five minutes;
                 after all, we have to exit someday.                    ahd
      22Sep89  - Add password file processing                           ahd
      24Sep89  - Modify login() to issue only one wait command for up
                 to 32K seconds; this cuts down LOGFILE clutter.        ahd
      01Oct89  - Re-do function headers to allow copying for function
                 prototypes in ulib.h                                   ahd
      17Jan90  - Filter unprintable characters from logged userid and
                 password to prevent premature end of file.             ahd
      18Jan90  - Alter processing of alternate shells to directly
                 invoke program instead of using system() call.         ahd
   6  Sep 90   - Change logging of line data to printable               ahd
      8 Sep 90 - Split ulib.c into dcplib.c and ulib.c                  ahd
      8 Oct 90 - Break rmail.com and rnews.com out of uuio
                 Add FIXED_SPEED option for no-autobauding              ahd
      10Nov 90 - Move sleep call into ssleep and rename                 ahd
*/

#include <ctype.h>
#include <direct.h>
#include <dos.h>
#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

#ifdef __TURBOC__
#include <sys/timeb.h>
#endif

#include "lib.h"
#include "arpadate.h"
#include "dcp.h"
#include "dcplib.h"
#include "dcpsys.h"
#include "hlib.h"
#include "hostable.h"
#include "import.h"
#include "modem.h"
#include "pushpop.h"
#include "security.h"
#include "ssleep.h"
#include "commlib.h"
#include "usertabl.h"
#include "timestmp.h"

/*--------------------------------------------------------------------*/
/*        Define current file name for panic() and printerr()         */
/*--------------------------------------------------------------------*/

currentfile();

/*--------------------------------------------------------------------*/
/*                    Internal function prototypes                    */
/*--------------------------------------------------------------------*/

static void LoginShell( const   struct UserTable *userp );

void motd( const char *fname, char *buf, const int bufsiz );

/*--------------------------------------------------------------------*/
/*    l o g i n                                                       */
/*                                                                    */
/*    Login handler                                                   */
/*--------------------------------------------------------------------*/

boolean login(void)
{
   char line[BUFSIZ];                  /* Allow for long domain names!  */
   char user[50];
   char pswd[50];
   char attempts = 0;                  /* Allows login tries         */
   char *token;                        /* Pointer to returned token  */
   struct UserTable *userp;

   if ( E_banner != NULL )
      motd( E_banner, line, sizeof line );

/*--------------------------------------------------------------------*/
/*    Our modem is now connected.  Begin actual login processing      */
/*    by displaying a banner.                                         */
/*--------------------------------------------------------------------*/

   sprintf(line,"\r\n\n%s %d.%02d with %s %s (%s) (%s)\r\n",
#ifdef WIN32
            "Windows/NT(TM)",
            _osmajor,
#elif defined( __TURBOC__ )
            "MS-DOS(R)",
            _osmajor,
#else
            (_osmode == DOS_MODE) ? "MS-DOS(R)" : "OS/2(R)" ,
            (_osmode == DOS_MODE) ? _osmajor : ((int) _osmajor / 10 ),
#endif
       _osminor,
       compilep,
       compilev,
       E_domain, device); /* Print a hello message            */
   wmsg(line,0);
   ddelay(250);

/*--------------------------------------------------------------------*/
/*    Display a login prompt until we get a printable character or    */
/*    the login times out                                             */
/*--------------------------------------------------------------------*/

   for ( attempts = 0; attempts < 5 ; attempts++ )
   {
      boolean invalid = TRUE;
      while (invalid)         /* Spin for a user id or timeout       */
      {
         wmsg("\r\nlogin: ", 0);
         strcpy(user,"");
         if (rmsg(user, 2, 30, sizeof user) == TIMEOUT)
                                    /* Did the user enter data?  */
            return FALSE;   /* No --> Give up                */

         if (equal(user,"NO CARRIER"))
            return FALSE;

         token = user;
         while ((*token != '\0') && invalid) /* Ignore empty lines   */
            invalid = ! isgraph(*token++);
      } /* while */

      printmsg(14, "login: login=%s", user);

/*--------------------------------------------------------------------*/
/*               We have a user id, now get a password                */
/*--------------------------------------------------------------------*/

      wmsg("\r\nPassword: ", 0);
      strcpy(pswd,"");
      if (rmsg(pswd, 0, 30, sizeof pswd) == TIMEOUT)
         return FALSE;

/*--------------------------------------------------------------------*/
/*       Zap unprintable characters before we log the password        */
/*--------------------------------------------------------------------*/

      printmsg(14, "login: password=%s", pswd);

/*--------------------------------------------------------------------*/
/*                 Validate the user id and passowrd                  */
/*--------------------------------------------------------------------*/

      userp = checkuser(user);         /* Locate user id in host table  */

      if (userp == BADUSER)            /* Does user id exist?           */
      {                                /* No --> Notify the user        */
         wmsg("\r\nlogin failed",0);

         token = user;
         while (!isalnum( *token ) && (*token !=  '\0'))
            token ++;                  /* Scan for first alpha-numeric  */

         if (*token != '\0')           /* If at least one good char     */
            printmsg(0,"login: login for user %s failed, bad user id",
                  user);               /* Log the error for ourselves   */
      }
      else if ( equal(pswd,userp->password))   /* Correct password?     */
      {                                /* Yes --> Log the user "in"     */
                   /*   . . ..+....1....  +....2....+....3....  + .   */
         sprintf(line,"\r\n\nWelcome to %s; login complete at %s\r\n",
                  E_domain, arpadate());
         wmsg(line, 0);
         printmsg(0,"login: login user %s (%s) at %s",
                     userp->uid, userp->realname, arpadate());

         if equal(userp->sh,UUCPSHELL) /* Standard uucp shell?       */
         {
            securep = userp->hsecure;
            printmsg(5,"Processing user via %s", UUCPSHELL);
            return TRUE;            /* Yes --> Startup the machine   */
         }
         else {                     /* No --> run special shell      */

            if ( E_motd != NULL )
               motd( E_motd, line, sizeof line );
            LoginShell( userp );
            return FALSE;   /* Hang up phone and exit        */
         }
      }
      else {                        /* Password was wrong.  Report   */
         wmsg("\r\nlogin failed",0);
         printmsg(0,"login: login user %s (%s) failed, bad password %s",
                  userp->uid, userp->realname, pswd);
      }
   }  /* for */

/*-----------------------------------------------------------------*/
/*    If we fall through the loop, we have an excessive number of  */
/*    login attempts; hangup the telephone and try again.          */
/*-----------------------------------------------------------------*/

   return FALSE;                    /* Exit processing            */

} /*login*/

/*--------------------------------------------------------------------*/
/*    l o g i n b y p a s s                                           */
/*                                                                    */
/*    Initialize user setup when login is bypassed                    */
/*--------------------------------------------------------------------*/

boolean loginbypass(const char *user)
{
   struct UserTable *userp;
   char line[BUFSIZ];                  /* Allow for long domain names!  */

   printmsg(14, "loginbypass: login=%s", user);

/*--------------------------------------------------------------------*/
/*                 Validate the user id                               */
/*--------------------------------------------------------------------*/

   userp = checkuser(user);         /* Locate user id in host table  */

   if (userp == BADUSER)            /* Does user id exist?           */
   {                                /* No --> Notify the user        */
      wmsg("\r\nUUCICO login failed",0);

      printmsg(0,"loginbypass: login for user %s failed, bad user id",
               user);               /* Log the error for ourselves   */
      return FALSE;                 /* Hang up phone and exit        */
   }
   else {
                                    /* Yes --> Log the user "in"     */
                /*   . . ..+....1....  +....2....+....3....  + .   */
      sprintf(line,"\r\n\nWelcome to %s; login complete at %s\r\n",
               E_domain, arpadate());
      wmsg(line, 0);
      printmsg(0,"loginbypass: login user %s (%s) at %s",
                  userp->uid, userp->realname, arpadate());

      if equal(userp->sh,UUCPSHELL) /* Standard uucp shell?       */
      {
         securep = userp->hsecure;
         return TRUE;            /* Yes --> Startup the machine   */
      } /* if equal(userp->sh,UUCPSHELL) */
      else {                     /* No --> run special shell      */
         LoginShell( userp );
         return FALSE;           /* Hang up phone and exit        */
      } /* else */
   } /* else */

} /*loginbypass*/

/*--------------------------------------------------------------------*/
/*    L o g i n S h e l l                                             */
/*                                                                    */
/*    Execute a non-default remote user shell                         */
/*--------------------------------------------------------------------*/

static void LoginShell( const   struct UserTable *userp )
{
   char *shellstring;
   char *path;
   char *args;
   int   rc;

/*--------------------------------------------------------------------*/
/*              Get the program to run and its arguments              */
/*--------------------------------------------------------------------*/

   shellstring = strdup(userp->sh);
                           /* Copy user shell for parsing   */
   path = strtok(shellstring," \t");   /* Get program name  */
   args = strtok(NULL,"");    /* Get rest of arg string     */

   printmsg(1,"LoginShell: Invoking %s in directory %s",
         userp->sh, userp->homedir);

   ddelay(250);            /* Wait for port to stablize     */

/*--------------------------------------------------------------------*/
/*       Run the requested program in the user's home directory       */
/*--------------------------------------------------------------------*/

   PushDir(userp->homedir);/* Switch to user's home dir     */
   if (args == NULL)
      rc = spawnl(P_WAIT, path, path, NULL);
   else
      rc = spawnl(P_WAIT, path, path, args, NULL);

   PopDir();               /* Return to original directory  */

/*--------------------------------------------------------------------*/
/*                     Report any errors we found                     */
/*--------------------------------------------------------------------*/

   if ( rc < 0 )           /* Error condition?              */
   {                        /* Yes --> Report it to the user */
      printmsg(0,"LoginShell: Unable to execute user shell");
      printerr(path);
   }
   else                    /* No --> Report normal result   */
      printmsg(rc == 0 ? 4 : 0,"LoginShell: %s return code is %d", path, rc);

} /* LoginShell */

/*--------------------------------------------------------------------*/
/*    m o t d                                                         */
/*                                                                    */
/*    Display a message of the day to the remote login                */
/*--------------------------------------------------------------------*/

void motd( const char *fname, char *buf, const int bufsiz )
{
   FILE *stream = FOPEN( fname, "r", BINARY_MODE );  /* Leave CRLF in data */

   if ( stream == NULL )
   {
      perror( fname );
      wmsg( fname,0 );
      wmsg( ": ", 0);
      wmsg( strerror( errno ),0);
      wmsg( "\n\r",0);
      return;
   } /* if ( stream == NULL ) */

   while( fgets( buf, bufsiz, stream ) != NULL )
      wmsg( buf, 0 );

   fclose( stream );

} /* motd */
