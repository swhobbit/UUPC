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
/*    Changes Copyright (c) 1989-1994 by Kendra Electronic            */
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
 *    $Id: dcplib.c 1.16 1994/02/20 19:11:18 ahd Exp $
 *
 *    $Log: dcplib.c $
 * Revision 1.16  1994/02/20  19:11:18  ahd
 * IBM C/Set 2 Conversion, memory leak cleanup
 *
 * Revision 1.15  1994/01/04  03:56:38  ahd
 * Add missing break after modem insert
 *
 * Revision 1.14  1994/01/02  04:27:23  ahd
 * Allow alias of 'w' for 'u' (userid) insertion to be compatible
 * with UUCICO command line.
 *
 * Revision 1.13  1994/01/01  19:19:00  ahd
 * Annual Copyright Update
 *
 * Revision 1.12  1993/12/09  12:48:33  rommel
 * Move extra terminating character when inserting substitution strings
 *
 * Revision 1.12  1993/12/09  12:48:33  rommel
 * Move extra terminating character when inserting substitution strings
 *
 * Revision 1.11  1993/11/30  04:18:14  ahd
 * Correct spelling error in comment
 *
 * Revision 1.10  1993/11/20  14:48:53  ahd
 * Add support for passing port name/port handle/port speed/user id to child
 *
 * Revision 1.9  1993/10/28  12:19:01  ahd
 * Cosmetic time formatting twiddles and clean ups
 *
 * Revision 1.8  1993/10/12  01:33:59  ahd
 * Normalize comments to PL/I style
 *
 * Revision 1.7  1993/09/29  04:52:03  ahd
 * Make device name use standard modem file configuration prefix
 *
 * Revision 1.6  1993/09/20  04:48:25  ahd
 * TCP/IP support from Dave Watt
 * 't' protocol support
 * OS/2 2.x support (BC++ 1.0 for OS/2)
 *
 * Revision 1.5  1993/07/24  03:40:55  ahd
 * Correct #ifif to #ifel
 *
 * Revision 1.4  1993/07/22  23:22:27  ahd
 * First pass at changes for Robert Denny's Windows 3.1 support
 *
 * Revision 1.3  1993/05/30  00:01:47  ahd
 * Multiple commuications drivers support
 *
 *
 * Updated:
 *
 *    14May89  - Added system name to login prompt - ahd
 *               Added configuration file controlled user id, password
 *               Added Kermit server option
 *    17May89  - Redo login processing to time out after five minutes;
 *               after all, we have to exit someday.                    ahd
 *    22Sep89  - Add password file processing                           ahd
 *    24Sep89  - Modify login() to issue only one wait command for up
 *               to 32K seconds; this cuts down LOGFILE clutter.        ahd
 *    01Oct89  - Re-do function headers to allow copying for function
 *               prototypes in ulib.h                                   ahd
 *    17Jan90  - Filter unprintable characters from logged userid and
 *               password to prevent premature end of file.             ahd
 *    18Jan90  - Alter processing of alternate shells to directly
 *               invoke program instead of using system() call.         ahd
 * 6  Sep 90   - Change logging of line data to printable               ahd
 *    8 Sep 90 - Split ulib.c into dcplib.c and ulib.c                  ahd
 *    8 Oct 90 - Break rmail.com and rnews.com out of uuio
 *               Add FIXED_SPEED option for no-autobauding              ahd
 *    10Nov 90 - Move sleep call into ssleep and rename                 ahd
 */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"

#include <ctype.h>
#include <direct.h>

#ifdef __TURBOC__
#include <sys/timeb.h>
#endif

#if defined(_Windows)
#include <windows.h>
#elif !defined(__32BIT__)
#include <dos.h>
#endif

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "arpadate.h"
#include "dcp.h"
#include "dcplib.h"
#include "dcpsys.h"
#include "execute.h"
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

#if !defined(_Windows)
currentfile();
#endif

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
   char line[BUFSIZ];                  /* Allow for long domain names! */
   char user[50];
   char pswd[50];
   char attempts = 0;                  /* Allows login tries         */
   char *token;                        /* Pointer to returned token  */
   struct UserTable *userp;

#if defined(_Windows)
   WORD wVersion;
#endif

   if ( E_banner != NULL )
      motd( E_banner, line, sizeof line );

/*--------------------------------------------------------------------*/
/*    Our modem is now connected.  Begin actual login processing      */
/*    by displaying a banner.                                         */
/*--------------------------------------------------------------------*/

#if defined(_Windows)
    wVersion = LOWORD(GetVersion());
    sprintf(line, "\r\n\nMS-Windows(TM) %d.%02d with %s %s (%s) (%s)\r\n",
         (WORD)(LOBYTE(wVersion)),
         (WORD)(HIBYTE(wVersion)),
          compilep,
          compilev,
         E_domain,
         M_device);           /* Print a hello message                 */

#else /* not Windows */

   sprintf(line,"\r\n\n%s %d.%02d with %s %s (%s) (%s)\r\n",
#ifdef WIN32
            "Windows NT(TM)",
            _winmajor,
            _winminor,
#elif defined(__OS2__)
            "OS/2(R)",
            (int) _osmajor / 10,
            _osminor,
#elif defined( __TURBOC__ )
            "MS-DOS(R)",
            _osmajor,
            _osminor,
#else
            (_osmode == DOS_MODE) ? "DOS" : "OS/2(R)" ,
            (_osmode == DOS_MODE) ? (int) _osmajor : ((int) _osmajor / 10 ),
            _osminor,
#endif
            compilep,
            compilev,
            E_domain,
            M_device); /* Print a hello message            */
#endif

   wmsg(line,0);

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

      userp = checkuser(user);         /* Locate user id in host table */

      if (userp == BADUSER)            /* Does user id exist?          */
      {                                /* No --> Notify the user       */
         wmsg("\r\nlogin failed",0);

         token = user;
         while (!isalnum( *token ) && (*token !=  '\0'))
            token ++;                  /* Scan for first alpha-numeric */

         if (*token != '\0')           /* If at least one good char    */
            printmsg(0,"login: login for user %s failed, bad user id",
                  user);               /* Log the error for ourselves  */
      }
      else if ( equal(pswd,userp->password))   /* Correct password?    */
      {                                /* Yes --> Log the user "in"    */
         time_t now;
                   /*   . . ..+....1....  +....2....+....3....  + .   */
         sprintf(line,"\r\n\nWelcome to %s; login complete at %s\r\n",
                  E_domain, arpadate());
         wmsg(line, 0);

         time( &now );
         printmsg(0,"login: login user %s (%s) at %.24s",
                     userp->uid, userp->realname, ctime( &now ));

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
   char line[BUFSIZ];                  /* Allow for long domain names! */

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

   char line[128];

#if defined(_Windows)

/*--------------------------------------------------------------------*/
/*           No special shell support under Windows, sorry!           */
/*--------------------------------------------------------------------*/

   sprintf(line,
          "LoginShell: special shell %s not supported. Goodbye.\r\n",
           userp->sh);
   wmsg(line, 0);
   printmsg(0, "Login with special shell %s, not supported.", userp->sh);
   return;

#else

   char *shellstring;
   char *path;
   char *args;
   int   rc;
   char argstring[255];
   char *s = argstring;

/*--------------------------------------------------------------------*/
/*              Get the program to run and its arguments              */
/*--------------------------------------------------------------------*/

   shellstring = strdup(userp->sh);
                              /* Copy user shell for parsing   */
   checkref(shellstring);
   path = strtok(shellstring,WHITESPACE);   /* Get program name  */

   args = strtok(NULL,"");    /* Get rest of arg string     */

   if ( args == NULL )
      *argstring = '\0';
   else {
      strncpy( argstring, args, sizeof argstring - 1 );
      argstring[ sizeof argstring - 1 ] = '\0';
   }

   path = newstr(path);
   free( shellstring );

/*--------------------------------------------------------------------*/
/*                 Perform command line substitution                  */
/*--------------------------------------------------------------------*/

   printmsg(4,"LoginShell: command %s, parameters %s",
            path, argstring );

   if ( args != NULL )
   while( (s = strchr( s, '%')) != NULL ) /* Get next percent on line */
   {
      char *insert;
      int len;

      switch( s[1] )                /* Look at next character        */
      {
         case '%':
            insert = "%";           /* Literal percent sign          */
            break;

         case 'l':                  /* Com port handle               */
            sprintf(line,"%d", GetComHandle());
            insert = line;
            break;

         case 'm':
            insert = E_inmodem;     /* Modem file name               */
            break;

         case 'p':
            insert = M_device;      /* Port name                     */
            break;

         case 's':
            sprintf( line,"%lu", (long unsigned) GetSpeed());
            insert = line;
            break;                  /* Current line speed            */

         case 'w':                  /* Alias for user id             */
         case 'u':                  /* User id                       */
            insert = (char *) userp->uid;
            break;

         default:
            printmsg(0,"LoginShell: Unknown substitution character %c",
                     s[1] );
            line[0] = s[1];         /* Perform a self-insert         */
            line[1] = '\0';
            insert = line;
            break;

      } /* switch */

      len = strlen( insert );
      printmsg(4,"Inserting %s into %s",
                 insert,
                 argstring );
      if ( len != 2 )               /* Make room for new string      */
         memmove( s + len, s + 2, strlen(s + 2) + 1);

      memcpy( s, insert, len );     /* Move in the string            */

      s += len;                     /* Step past the string          */

   } /* while */

/*--------------------------------------------------------------------*/
/*       Run the requested program in the user's home directory       */
/*--------------------------------------------------------------------*/

   PushDir(userp->homedir);         /* Switch to user's home dir     */

   printmsg(1,"LoginShell: Invoking %s %s in directory %s",
         path, argstring, userp->homedir);
   rc = execute( path, args ? argstring : NULL, NULL, NULL, TRUE, FALSE );

   PopDir();               /* Return to original directory  */

/*--------------------------------------------------------------------*/
/*                     Report any errors we found                     */
/*--------------------------------------------------------------------*/

   if ( rc > 0 )              /* Error condition?              */
      printmsg(rc == 0 ? 4 : 0,"LoginShell: %s return code is %d",
               path,
               rc);

#endif

} /* LoginShell */

/*--------------------------------------------------------------------*/
/*    m o t d                                                         */
/*                                                                    */
/*    Display a message of the day to the remote login                */
/*--------------------------------------------------------------------*/

void motd( const char *fname, char *buf, const int bufsiz )
{
   FILE *stream = FOPEN( fname, "r", IMAGE_MODE );    /* Leave CRLF in data */

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
