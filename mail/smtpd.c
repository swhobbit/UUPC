/*--------------------------------------------------------------------*/
/*       u u s m t p d . c                                            */
/*                                                                    */
/*       SMTP receiver for UUPC/extended                              */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1997 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: smtpd.c 1.2 1997/11/24 02:52:26 ahd Exp $
 *
 *    $Log: smtpd.c $
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

#include "timestmp.h"
#include "catcher.h"

#include "smtpserv.h"
#include "getopt.h"
#include "logger.h"

/*--------------------------------------------------------------------*/
/*                      Global defines/variables                      */
/*--------------------------------------------------------------------*/

RCSID("$Id: smtpd.c 1.2 1997/11/24 02:52:26 ahd Exp $");

currentfile();

/*--------------------------------------------------------------------*/
/*       c l i e n t M o d e                                          */
/*                                                                    */
/*       Handle requests from a single client                         */
/*--------------------------------------------------------------------*/

int
clientMode( int hotHandle, KWBoolean runUUXQT )
{

   SMTPClient *client = initializeClient( (SOCKET) hotHandle, KWFalse );

   if ( client == NULL )
      return 4;

   while( isClientValid( client ))
      processClient( client );

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

   while( ! terminate_processing && isClientValid( master ))
   {
      flagReadyClientList( master );
      timeoutClientList( master );
      processReadyClientList( master );
      dropTerminatedClientList( master );

   } /* while( ! terminate_processing && isClientValid( master )) */

   dropAllClientList( master );

   if ( terminate_processing )
      return 100;
   else
      return 0;

} /* daemonMode */

/*--------------------------------------------------------------------*/
/*       m a i n                                                      */
/*                                                                    */
/*       Invocation of daemon for SMTP receipt                        */
/*--------------------------------------------------------------------*/

main( int argc, char ** argv )
{
   int exitStatus;
   char *logfile_name = NULL;
   char *port = "25";

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

/*--------------------------------------------------------------------*/
/*       Parse our options, which will dictate how we process user    */
/*       connections.                                                 */
/*--------------------------------------------------------------------*/

   while((option = getopt( argc, argv, "d:h:l:p:Ux:" )) != EOF)
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
         printmsg(0,"optarg = %s (%d), debuglevel = %d",
                     optarg,
                     atoi( optarg ),
                     debuglevel );
         break;

      default:
         fprintf( stdout, "Invalid option '%c'.\n", option);
         /* FALL THROUGH */

      case '?':
         puts("\nUsage:\tuusmtpd\t"
         "[-l logfile] [-t] [-U] [-x debug]\n"
         "\t\t[-h handle | -d hhmm]");
         exit(4);

      } /* switch( option ) */
   }

   printmsg(0,"Debuglevel is %d", debuglevel );

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

} /* main */
