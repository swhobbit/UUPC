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
/*    Changes Copyright (c) 1989-1999 by Kendra Electronic            */
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
 *    $Id: dcplib.c 1.31 1999/01/04 03:53:57 ahd Exp $
 *
 *    $Log: dcplib.c $
 *    Revision 1.31  1999/01/04 03:53:57  ahd
 *    Annual copyright change
 *
 *    Revision 1.30  1998/03/01 19:40:21  ahd
 *    First compiling POP3 server which accepts user id/password
 *
 *    Revision 1.29  1998/03/01 01:39:30  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.28  1997/04/24 01:33:40  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.27  1996/03/18 03:52:46  ahd
 *    Allow binary rmsg() processing without translation of CR/LF into \0
 *    Use enumerated list for synch types on rmsg() call
 *
 *    Revision 1.26  1996/01/01 21:20:56  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.25  1995/04/02 00:01:39  ahd
 *    Correct processing to not send files below requested call grade
 *
 *    Revision 1.24  1995/01/30 04:08:36  ahd
 *    Additional compiler warning fixes
 *
 *    Revision 1.23  1995/01/07 16:38:26  ahd
 *    Change boolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.22  1994/12/22 00:35:01  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.21  1994/05/08 22:46:32  ahd
 *    Add missing value for return.
 *
 *        Revision 1.20  1994/05/07  21:45:33  ahd
 *        Handle empty passwords different from blocked (asterisk) passwords
 *
 *        Revision 1.19  1994/05/06  03:55:50  ahd
 *        Force hot logins to always be UUCICO (internal) shell -- why else
 *        would we be called, and otherwise UUCICO can't call itself
 *        for testing.
 *
 *        Revision 1.18  1994/05/04  23:36:34  ahd
 *        Trap NULL Passwords
 *
 * Revision 1.17  1994/02/26  17:20:13  ahd
 * Change BINARY_MODE to IMAGE_MODE to avoid IBM C/SET 2 conflict
 *
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
RCSID("$Id$");
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

KWBoolean login(void)
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

   if ( ! bflag[F_SUPPRESSLOGININFO] )
   {

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
            M_device); /* Format a hello message            */
#endif

      wmsg(line, KWFalse);

   } /* if ( ! bflag[F_SUPPRESSLOGININFO] ) */

/*--------------------------------------------------------------------*/
/*    Display a login prompt until we get a printable character or    */
/*    the login times out                                             */
/*--------------------------------------------------------------------*/

   for ( attempts = 0; attempts < 5 ; attempts++ )
   {
      KWBoolean invalid = KWTrue;

      while (invalid)         /* Spin for a user id or timeout       */
      {
         memset(user, 0, sizeof user);

         wmsg("\r\nlogin: ", KWFalse);

         if (rmsg(user, SYNCH_ECHO, M_startupTimeout, sizeof user) == TIMEOUT)
                                    /* Did the user enter data?  */
            return KWFalse;  /* No --> Give up                */

         if (! CD() )
            return KWFalse;

         token = user;

         while ((*token != '\0') && invalid) /* Ignore empty lines   */
         {
            if ( isgraph(*token++) )
               invalid = KWFalse;
            else
               invalid = KWTrue;
         }

      } /* while */

      printmsg(14, "login: login=\"%s\"", user);

/*--------------------------------------------------------------------*/
/*       We have a user id token, now get a password unless its a     */
/*       valid user with an empty password and we don't prompt for    */
/*       such passwords.                                              */
/*--------------------------------------------------------------------*/

      userp = checkuser(user);      /* Locate user id in host table  */

      memset(pswd, 0, sizeof pswd );/* Initialize to nice default    */

      if ( (! bflag[F_SUPPRESSEMPTYPASSWORD]) ||
           (userp == BADUSER) ||
           (*(userp->password) != '\0'))
      {
         wmsg("\r\nPassword: ", KWFalse);
         if (rmsg(pswd, SYNCH_NONE, M_startupTimeout, sizeof pswd) == TIMEOUT)
            return KWFalse;

      }

      printmsg(14, "login: password=\"%s\"", pswd);

      if ( ! CD() )
         return KWFalse;

/*--------------------------------------------------------------------*/
/*                 Validate the user id and passowrd                  */
/*--------------------------------------------------------------------*/

      if (userp == BADUSER)            /* Does user id exist?          */
      {                                /* No --> Notify the user       */
         wmsg("\r\nlogin failed", KWFalse);

         token = user;
         while (!isalnum( *token ) && (*token !=  '\0'))
            token ++;                  /* Scan for first alpha-numeric */

         if (*token != '\0')           /* If at least one good char    */
            printmsg(0,"login: login for user %s failed, bad user id",
                  user);               /* Log the error for ourselves  */
      }
      else if ( userp->group && equal(POP3_GROUP, userp->group))
      {
         wmsg("\r\nlogin failed", KWFalse);
         printmsg(0,"login: login user %s (%s) failed, in group %s",
                  userp->uid,
                  userp->realname,
                  POP3_GROUP );
      }
      else if ( userp->password && equal(pswd, userp->password ))
                                       /* Correct password?            */
      {                                /* Yes --> Log the user "in"    */
         time_t now;
                   /*   . . ..+....1....  +....2....+....3....  + .   */
         if ( ! bflag[F_SUPPRESSLOGININFO] )
         {
            sprintf(line,"\r\n\nWelcome to %s; login complete at %s\r\n",
                    E_domain,
                    arpadate());
            wmsg(line, KWFalse);
         }

         time( &now );
         printmsg(0,"login: login user %s (%s) at %.24s",
                     userp->uid,
                     userp->realname,
                     ctime( &now ));

         if equal(userp->sh,UUCPSHELL) /* Standard uucp shell?       */
         {
            securep = userp->hsecure;
            printmsg(5,"Processing user via %s", UUCPSHELL);
            return CD();            /* Yes --> Startup the machine   */
         }
         else {                     /* No --> run special shell      */

            if ( E_motd != NULL )
               motd( E_motd, line, sizeof line );
            LoginShell( userp );
            return KWFalse;  /* Hang up phone and exit        */
         }
      }
      else {                        /* Password was wrong.  Report   */
         wmsg("\r\nlogin failed", KWFalse);
         printmsg(0,"login: login user %s (%s) failed, bad password %s",
                  userp->uid,
                  userp->realname,
                  pswd);
      }
   }  /* for */

/*-----------------------------------------------------------------*/
/*    If we fall through the loop, we have an excessive number of  */
/*    login attempts; hangup the telephone and try again.          */
/*-----------------------------------------------------------------*/

   return KWFalse;                   /* Exit processing            */

} /*login*/

/*--------------------------------------------------------------------*/
/*    l o g i n b y p a s s                                           */
/*                                                                    */
/*    Initialize user setup when login is bypassed                    */
/*--------------------------------------------------------------------*/

KWBoolean loginbypass(const char *user)
{
   struct UserTable *userp;
   char line[BUFSIZ];                  /* Allow for long domain names! */

   printmsg(14, "loginbypass: login=\"%s\"", user);

/*--------------------------------------------------------------------*/
/*                 Validate the user id                               */
/*--------------------------------------------------------------------*/

   userp = checkuser(user);         /* Locate user id in host table  */

   if (userp == BADUSER)            /* Does user id exist?           */
   {                                /* No --> Notify the user        */
      wmsg("\r\nUUCICO login failed", KWFalse);

      printmsg(0,"loginbypass: login for user %s failed, bad user id",
               user);               /* Log the error for ourselves   */
      return KWFalse;                /* Hang up phone and exit        */
   }
   else {
                                    /* Yes --> Log the user "in"     */
                /*   . . ..+....1....  +....2....+....3....  + .   */
      sprintf(line,"\r\n\nWelcome to %s; login complete at %s\r\n",
               E_domain, arpadate());
      wmsg(line, KWFalse);
      printmsg(0,"loginbypass: login user %s (%s) at %s",
                  userp->uid, userp->realname, arpadate());

      securep = userp->hsecure;

      return CD();            /* Yes --> Startup the machine   */

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
   wmsg(line, KWFalse);
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
      size_t len;

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

         case 'x':                  /* Debug level */
            sprintf( line,"%ld", (long) debuglevel );
            insert = line;
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
   rc = execute( path, args ? argstring : NULL, NULL, NULL, KWTrue, KWFalse );

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
      wmsg( fname, KWFalse);
      wmsg( ": ", KWFalse);
      wmsg( strerror( errno ), KWFalse);
      wmsg( "\n\r", KWFalse);
      return;
   } /* if ( stream == NULL ) */

   while(( fgets( buf, bufsiz, stream ) != NULL ) && CD())
      wmsg( buf, KWFalse);

   fclose( stream );

} /* motd */
