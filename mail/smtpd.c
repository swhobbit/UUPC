/*--------------------------------------------------------------------*/
/*       u u s m t p d . c                                            */
/*                                                                    */
/*       SMTP receiver for UUPC/extended                              */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1998 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: smtpd.c 1.10 1998/03/01 01:33:00 ahd Exp $
 *
 *    $Log: smtpd.c $
 *    Revision 1.10  1998/03/01 01:33:00  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.9  1997/12/14 02:41:14  ahd
 *    restore proper support for UUCP grades during delivery
 *
 *    Revision 1.8  1997/12/13 18:05:06  ahd
 *    Change parsing and passing of sender address information
 *
 *    Revision 1.7  1997/11/29 13:03:13  ahd
 *    Clean up single client (hot handle) mode for OS/2, including correct
 *    network initialization, use unique client id (pid), and invoke all
 *    routines needed in main client loop.
 *
 *    Revision 1.6  1997/11/28 23:11:38  ahd
 *    Additional SMTP auditing, normalize formatting, more OS/2 SMTP fixes
 *
 *    Revision 1.5  1997/11/28 04:52:10  ahd
 *    Initial UUSMTPD OS/2 support
 *
 *    Revision 1.4  1997/11/26 03:34:11  ahd
 *    Correct SMTP timeouts, break out protocol from rest of daemon
 *
 *    Revision 1.3  1997/11/25 05:05:06  ahd
 *    More robust SMTP daemon
 *
 *    Revision 1.2  1997/11/24 02:52:26  ahd
 *    First working SMTP daemon which delivers mail
 *
 *    Revision 1.1  1997/11/21 18:15:18  ahd
 *    Command processing stub SMTP daemon
 *
 *    Revision 1.2  1997/06/03 03:25:31  ahd
 *    First compiling SMTPD
 *
 *    Revision 1.1  1997/05/20 03:55:46  ahd
 *    Initial revision
 *
 */

/*--------------------------------------------------------------------*/
/*                         Standard includes                          */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"

#include <signal.h>
#include <limits.h>
#include <ctype.h>

#include "timestmp.h"
#include "catcher.h"

#include "smtpserv.h"
#include "deliver.h"
#include "getopt.h"
#include "logger.h"
#include "smtpcmds.h"

/*--------------------------------------------------------------------*/
/*                      Global defines/variables                      */
/*--------------------------------------------------------------------*/

RCSID("$Id: smtpd.c 1.10 1998/03/01 01:33:00 ahd Exp $");

currentfile();

/*--------------------------------------------------------------------*/
/*       c l i e n t M o d e                                          */
/*                                                                    */
/*       Handle requests from a single client                         */
/*--------------------------------------------------------------------*/

int
clientMode( int hotHandle, KWBoolean runUUXQT )
{

   static const char mName[] = "clientMode";
   SMTPClient *client;

   printmsg(1, "%s: Entering single client mode for handle %d",
               mName,
               hotHandle );

   client = initializeClient( (SOCKET) hotHandle, KWFalse );

   if ( client == NULL )
      return 4;

   while( isClientValid( client ))
   {
      flagReadyClientList( client );
      timeoutClientList( client );
      processReadyClientList( client );
   }

   freeClient( client );

   return 0;

} /* clientMode */

/*--------------------------------------------------------------------*/
/*    d a e m o n M o d e                                             */
/*                                                                    */
/*    Process clients connecting to our master socket until           */
/*    we are shutdown or the exit time arrives                        */
/*--------------------------------------------------------------------*/

int
daemonMode( char *port, time_t exitTime, KWBoolean runUUXQT )
{
   static const char mName[] = "daemonMode";
   SMTPClient *master = initializeMaster( port, exitTime );

   if ( master == NULL )
      return 4;

   printmsg(1,"%s: Beginning daemon mode processing.", mName);

/*--------------------------------------------------------------------*/
/*                      Our main processing loop                      */
/*--------------------------------------------------------------------*/

   while( ! terminate_processing &&
           isClientValid( master ) &&
           ! isClientTimedOut( master ))
   {
      flagReadyClientList( master );
      timeoutClientList( master );
      processReadyClientList( master );
      dropTerminatedClientList( master->next );

   } /* while( ! terminate_processing && isClientValid( master )) */

   dropAllClientList( master );

   if ( terminate_processing )
      return 100;
   else
      return 0;

} /* daemonMode */

/*--------------------------------------------------------------------*/
/*       u s a g e                                                    */
/*                                                                    */
/*       Print command line options and exit                          */
/*--------------------------------------------------------------------*/

void
usage( const char *myName )
{
   fprintf(stderr, "\nUsage:\t%s\t"
            "[-l logfile] [-t] [-U] [-x debug]\n"
            "\t\t[-h handle | -d hhmm]",
            myName );
   exit(4);
}

/*--------------------------------------------------------------------*/
/*       m a i n                                                      */
/*                                                                    */
/*       Invocation of daemon for SMTP receipt                        */
/*--------------------------------------------------------------------*/

main( int argc, char ** argv )
{
   int exitStatus;
   char *logfile_name = NULL;
   char *port = defaultPortName;

   int option;
   time_t exitTime = LONG_MAX;
   KWBoolean runUUXQT = KWFalse;
   int  hotHandle = -1;

   logfile = stderr;

/*--------------------------------------------------------------------*/
/*          Report our version number and date/time compiled          */
/*--------------------------------------------------------------------*/

   banner( argv );

#if defined(__CORE__)
   copywrong = strdup(copyright);
   checkref(copywrong);
#endif

   if (!configure( B_UUSMTPD ))
      panic();

   setDeliveryGrade( E_mailGrade );

/*--------------------------------------------------------------------*/
/*       Parse our options, which will dictate how we process user    */
/*       connections.                                                 */
/*--------------------------------------------------------------------*/

   while((option = getopt( argc, argv, "d:g:h:l:p:Ux:" )) != EOF)
   {
      switch( option )
      {
         case 'd':
            exitTime = atoi( optarg );
            exitTime = time(NULL) + hhmm2sec(exitTime);
            break;

         case 'h':
            hotHandle = atoi( optarg );   /* Handle opened for us       */
            break;

         case 'g':
            if ( isalnum(*optarg) && ( strlen( optarg) == 1 ))
               setDeliveryGrade(*optarg);
            else {
               printmsg(0,"Invalid grade for mail: %s", optarg );
               usage( argv[0] );
            }
            break;

         case 'l':                     /* Log file name              */
            logfile_name = optarg;
            break;

         case 'p':
            port = optarg;
            break;

         case 'U':
            runUUXQT = KWTrue;
            break;

         case 'x':
            debuglevel = atoi(optarg);
            break;

         default:
            fprintf( stdout, "Invalid option '%c'.\n", option);
            /* FALL THROUGH */

         case '?':
            usage( argv[0] );
            break;

      } /* switch( option ) */
   }

/*--------------------------------------------------------------------*/
/*                Abort if any options were left over                 */
/*--------------------------------------------------------------------*/

   if (optind != argc)
   {
      puts("Extra parameter(s) at end.");
      return 4;
   }

/*--------------------------------------------------------------------*/
/*        Initialize logging and the name of the systems file         */
/*--------------------------------------------------------------------*/

   openlog( logfile_name );

/*--------------------------------------------------------------------*/
/*                        Trap control C exits                        */
/*--------------------------------------------------------------------*/

    if( signal( SIGINT, ctrlchandler ) == SIG_ERR )
    {
        printerr("signal");
        printmsg( 0, "Couldn't set SIGINT\n" );
        panic();
    }

#if defined(__OS2__) || defined(FAMILYAPI) || defined(WIN32)

    if( signal( SIGTERM, ctrlchandler ) == SIG_ERR )
    {
        printerr("signal");
        printmsg( 0, "Couldn't set SIGTERM\n" );
        panic();
    }

#endif

#if defined(__OS2__)

    if( signal( SIGBREAK , ctrlchandler ) == SIG_ERR )
    {
        printerr("signal");
        printmsg( 0, "Couldn't set SIGBREAK\n" );
        panic();
    }

#endif

   interactive_processing = KWFalse;

/*--------------------------------------------------------------------*/
/*                If loaded for single client, handle it              */
/*--------------------------------------------------------------------*/

   if ( hotHandle == -1 )
      exitStatus = daemonMode( port, exitTime, runUUXQT );
   else
      exitStatus = clientMode( hotHandle, runUUXQT );

   exit( exitStatus );
   return exitStatus;               /* Suppress compiler warning */

} /* main */
