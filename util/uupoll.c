/*--------------------------------------------------------------------*/
/*    Program:    uupoll.c              22 September 1989             */
/*    Author:     Andrew H. Derbyshire                                */
/*                108 Decatur St                                      */
/*                Arlington, MA 02174                                 */
/*    Internet:   help@kew.com                                        */
/*    Function:   Performs autopoll functions for UUCICO              */
/*    Language:   Borland C++ 2.0                                     */
/*    Usage:      uupoll [-r 0] [-f hhmm] [-i hhmm|0400 ]             */
/*                       [-d hhmm] [-e hhmm]                          */
/*                       [-a hhmm] [-x debug] [-s systems]            */
/*                                                                    */
/*                Where:                                              */
/*                                                                    */
/*                -r 0     specifies that UUCICO is to run            */
/*                         into passive mode when waiting to          */
/*                         poll out                                   */
/*                                                                    */
/*                -r 1     specifies that UUCICO will not run         */
/*                         in passive mode while waiting to           */
/*                         poll out, but polling out will             */
/*                         occur.                                     */
/*                                                                    */
/*                -f hhmm  is the first time in the day that          */
/*                         UUCICO is to poll out.  If omitted,        */
/*                         polling begins after the interval          */
/*                         specified with -i.                         */
/*                                                                    */
/*                -i hhmm  the interval the UUCICO is to poll         */
/*                         out at.  If omitted, a period of 4         */
/*                         hours will be used.                        */
/*                                                                    */
/*                -d hhmm  Terminate polling after hhmm.              */
/*                         Default is not to terminate.               */
/*                                                                    */
/*                -e hhmm  Terminate polling at hhmm                  */
/*                         Default is not to terminate.               */
/*                                                                    */
/*                -a hhmm  Automatically poll actively using the      */
/*                         system name "any" after any                */
/*                         successful inbound poll if hhmm have       */
/*                         past since last poll.  hhmm may be         */
/*                         0000.                                      */
/*                                                                    */
/*                -c hhmm  Run a command at hhmm to clean the UUPC    */
/*                         spool at this time every day.  Default is  */
/*                         never.                                     */
/*                                                                    */
/*                -C cmd   Run command 'cmd' at the specified by      */
/*                         -c.  The default is UUCLEAN.               */
/*                                                                    */
/*                -B cmd   Run command cmd before each active call    */
/*                         out to batch up any work.  Default is      */
/*                         run nothing.                              */
/*                                                                    */
/*                In addition, the following flags will be passed     */
/*                to UUCICO:                                          */
/*                                                                    */
/*                -s system      system name to poll.  By default,    */
/*                               UUCICO will be invoked with          */
/*                               '-s all' followed by '-s any'.       */
/*                                                                    */
/*                -x n           debug level.   The default level     */
/*                               is 1.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: uupoll.c 1.11 1993/07/19 02:52:11 ahd Exp $
 *
 *    $Log: uupoll.c $
 * Revision 1.11  1993/07/19  02:52:11  ahd
 * Up memory access room
 *
 * Revision 1.10  1993/06/06  15:04:05  ahd
 * Allow for batch command to run before regularly scheduled poll out
 *
 * Revision 1.9  1993/05/13  01:39:04  ahd
 * Additional fix for correctly handling UUCLEAN once and only once daily
 *
 * Revision 1.8  1993/05/11  03:25:17  ahd
 * Don't loop when sleeping for autoclean -- get it over with
 *
 * Revision 1.7  1993/04/14  10:29:53  ahd
 * Correct invalid exit time if both -e and -f flags specified
 *
 * Revision 1.6  1993/04/10  21:25:16  dmwatt
 * Add Windows/NT support
 *
 * Revision 1.6  1993/04/10  21:25:16  dmwatt
 * Add Windows/NT support
 *
 * Revision 1.5  1993/04/04  04:57:01  ahd
 * Return exit code of UUCICO upon exit
 *
 * Revision 1.4  1993/03/06  23:04:54  ahd
 * Add cr after auto-clean message
 *
 * Revision 1.3  1992/11/20  12:41:01  ahd
 * Fix TZ change bug
 *
 * Revision 1.2  1992/11/15  04:45:46  ahd
 * Correct polling for days time zone changes
 *
 * Revision 1.1  1992/11/15  04:29:22  ahd
 * Initial revision
 *
 * Revision 1.1  1992/04/27  00:30:13  ahd
 * Initial revision
 *
 */

static const char rcsid[] =
         "$Id: uupoll.c 1.11 1993/07/19 02:52:11 ahd Exp $";

/*--------------------------------------------------------------------*/
/*                        System include file                         */
/*--------------------------------------------------------------------*/

#include <ctype.h>
#include <dos.h>
#include <limits.h>
#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <conio.h>
#include <direct.h>
#include <signal.h>           /* Ctrl-Break handler               */

#if  defined(WIN32) || defined(FAMILYAPI) || defined(_Windows) || defined(__OS2__)
#define NOCBREAK
#elif defined(__TURBOC__)
#include <alloc.h>
unsigned _stklen = 3172;            /* Reduce memory usage           */
unsigned _heaplen = 2048;           /* Reduce memory usage           */
#else
static int setcbrk(char state);
#endif

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "getopt.h"
#include "lib.h"
#include "timestmp.h"
#include "ssleep.h"
#include "arpadate.h"
#include "safeio.h"
#include "dater.h"
#include "execute.h"

/*--------------------------------------------------------------------*/
/*                        Typedefs and macros                         */
/*--------------------------------------------------------------------*/

typedef int hhmm;

/*--------------------------------------------------------------------*/
/*                  Prototypes and global variables                   */
/*--------------------------------------------------------------------*/

void   Catcher( void );
 static active(char *Rmtname, int debuglevel, const char *logname);
static void    busywork( time_t next);
static int     runCommand( char *command );
static time_t  nextpoll( hhmm first, hhmm interval );
static boolean     notanumber( char *number);
static void    usage( char *name );

 static int passive( time_t next,
                     int debuglevel,
                     const char *logname,
                     const char *modem );

static hhmm    firstpoll(hhmm interval);
static void    uuxqt( int debuglevel);
static time_t LifeSpan( time_t duration, time_t stoptime );

static time_t now;            /* Current time, updated at start of
                                 program and by busywork() and
                                 runCommand()                        */

currentfile();

/*--------------------------------------------------------------------*/
/*    m a i n                                                         */
/*                                                                    */
/*    main program                                                    */
/*--------------------------------------------------------------------*/

 void main( int argc , char *argv[] )
 {

   int option;
   hhmm first = - 1;
   hhmm interval = -1;
   hhmm stoptime = -1;
   hhmm duration = -1;
   hhmm autowait = -1;
   hhmm cleanup  = -1;
   time_t cleannext = LONG_MAX;

   time_t exittime;
   int nopassive = 2;
   boolean done = FALSE;
   char *Rmtname = NULL;
   char *CleanCommand = "uuclean";
   char *logname = NULL;
   char *modem   = NULL;
   char *batchCommand   = NULL;
   int returnCode = 0;

#ifndef NOCBREAK
   boolean cbrk;
#endif

   banner(argv);

   if (!configure( B_UUPOLL ))
      panic();

   tzset();
   time( &now );

    if( signal( SIGINT, Catcher ) == SIG_ERR )
    {
        fprintf( stderr, "uupoll: Couldn't set SIGINT\n" );
        panic();
    }

   while((option = getopt(argc, argv, "m:a:B:c:C:d:e:f:l:i:s:r:x:")) != EOF)
   switch(option)
   {

/*--------------------------------------------------------------------*/
/*       Automatically poll "any" after an incoming phone call        */
/*--------------------------------------------------------------------*/

      case 'a':
         if (notanumber(optarg))
            usage( argv[0] );
         autowait = (hhmm) hhmm2sec( atoi(optarg) );
         break;

/*--------------------------------------------------------------------*/
/*                    Batching command to execute                     */
/*--------------------------------------------------------------------*/

      case 'B':
         batchCommand = optarg;
         break;

/*--------------------------------------------------------------------*/
/*                         First time to poll                         */
/*--------------------------------------------------------------------*/

      case 'f':
         if (notanumber(optarg))
            usage( argv[0] );
         first = atoi(optarg);
         if ( interval == -1 )
            interval = 400;
         break;

/*--------------------------------------------------------------------*/
/*                         First time to clean                        */
/*--------------------------------------------------------------------*/

      case 'c':
         if (notanumber(optarg))
            usage( argv[0] );
         cleanup = atoi(optarg);
         break;

/*--------------------------------------------------------------------*/
/*                    Clean-up command to execute                     */
/*--------------------------------------------------------------------*/

      case 'C':
         CleanCommand = optarg;
         break;

/*--------------------------------------------------------------------*/
/*                          UUCICO log file                           */
/*--------------------------------------------------------------------*/

      case 'l':
         logname = optarg;
         break;


/*--------------------------------------------------------------------*/
/*                            Input modem                             */
/*--------------------------------------------------------------------*/

      case 'm':
         modem = optarg;
         break;

/*--------------------------------------------------------------------*/
/*                          Interval to poll                          */
/*--------------------------------------------------------------------*/

      case 'i':
         if (notanumber(optarg))
            usage( argv[0] );
         interval = atoi(optarg);
         nopassive = min(1,nopassive);
         break;

/*--------------------------------------------------------------------*/
/*                      Duration to poll                              */
/*--------------------------------------------------------------------*/

      case 'e':
         if (notanumber(optarg))
            usage( argv[0] );
         stoptime = atoi(optarg);
         break;

/*--------------------------------------------------------------------*/
/*                      Time to exit                                  */
/*--------------------------------------------------------------------*/

      case 'd':
         if (notanumber(optarg))
            usage( argv[0] );
         duration = atoi(optarg);
         break;


/*--------------------------------------------------------------------*/
/*                        System name to poll                         */
/*--------------------------------------------------------------------*/

      case 's':
         Rmtname = strdup(optarg);
         break;

/*--------------------------------------------------------------------*/
/*                            Debug level                             */
/*--------------------------------------------------------------------*/

      case 'x':
         if (notanumber(optarg))
            usage( argv[0] );
         debuglevel = atoi(optarg);
         break;

/*--------------------------------------------------------------------*/
/*                       Passive polling option                       */
/*--------------------------------------------------------------------*/

      case 'r':
         if (notanumber(optarg))
            usage( argv[0] );
         nopassive = atoi(optarg);
         break;

/*--------------------------------------------------------------------*/
/*                                Help                                */
/*--------------------------------------------------------------------*/

      default:
      case '?':
         usage( argv[0] );
   } /* switch */

/*--------------------------------------------------------------------*/
/*             Terminate with error if too many arguments             */
/*--------------------------------------------------------------------*/

   if (optind != argc)
   {
      puts("Extra parameters on command line.");
      usage( argv[0] );
   }

/*--------------------------------------------------------------------*/
/* Terminate if neither active polling nor passive polling requested  */
/*--------------------------------------------------------------------*/

   if ( nopassive == 2 && (first < 0))
   {
      puts("Must specify -r 0, -f hhmm, or -i hhmm");
      usage( argv[0] );
   }

/*--------------------------------------------------------------------*/
/*                        Compute time to exit                        */
/*--------------------------------------------------------------------*/

   exittime = LifeSpan( duration, stoptime );

/*--------------------------------------------------------------------*/
/*                   Comput first time to poll out                    */
/*--------------------------------------------------------------------*/

   if ( (interval > 0) && (first < 0))
      first = firstpoll(interval);

/*--------------------------------------------------------------------*/
/*            If running under MS-DOS, enable Cntrl-Break.            */
/*--------------------------------------------------------------------*/

#ifndef NOCBREAK
#if defined(__TURBOC__)

   cbrk = getcbrk();                /* Get original Cntrl-Break setting */

   if (!cbrk)
      setcbrk(1);                   /* Turn it on to allow abort        */

#else /*dmw*/

   cbrk = setcbrk(1);      /* Turn it on to allow abort; get previous state */

#endif
#endif

/*--------------------------------------------------------------------*/
/*          Determine first time to perform clean-up, if any          */
/*--------------------------------------------------------------------*/

      if (cleanup >= 0)
      {
         cleannext = nextpoll(cleanup, 2400);
         printf("Automatic cleanup via \"%s\" scheduled for %s",
                  CleanCommand, ctime(&cleannext));
      }

/*--------------------------------------------------------------------*/
/*                       Beginning of main loop                       */
/*--------------------------------------------------------------------*/

   while ( !done && (exittime > now))
   {
      time_t next = LONG_MAX;
      time_t autonext  = now + autowait;
      time_t wait = 10;      /* Time to wait after first panic()    */
      returnCode = 0;

/*--------------------------------------------------------------------*/
/*        Determine length of passive poll or wasting of time         */
/*--------------------------------------------------------------------*/

      if (first >= 0)
      {
         next = nextpoll(first,interval);
         if ( next > exittime )
            next = exittime;
      }
      else
         next = exittime;

/*--------------------------------------------------------------------*/
/*              Disable OS/2 undelete support if desired              */
/*--------------------------------------------------------------------*/

   if ( !bflag[ F_UNDELETE ] )
      putenv( "DELDIR=");

/*--------------------------------------------------------------------*/
/*    Begin passive polling (with optional active calls system        */
/*    "any") until next active poll or exit time is reached.          */
/*--------------------------------------------------------------------*/

      while ((now < next) && ! done )
      {

         if ( cleannext <= now )
         {
            printf("Performing auto-clean with command: %s\n",
                     CleanCommand );
            if (runCommand( CleanCommand ))
               printerr( CleanCommand );
            cleannext = nextpoll(cleanup,  2400);
         }

         if (nopassive)
            busywork(next < cleannext ? next : cleannext);
         else {
            time_t spin;
            returnCode = passive(next < cleannext ? next : cleannext ,
                                 debuglevel, logname , modem );

            if (returnCode == 69 )  /* Error in UUCICO?              */
            {                       /* Yes --> Allow time to fix it  */
               spin = now + wait;   /* Figure next wait              */
               wait *= 2 ;          /* Double wait for next time     */
               busywork( spin > next ? next : spin );
                                    /* But only wait till next poll  */
            } /* if (returnCode == 69 ) */
            else {
               wait = 10;

               if ((returnCode == 0) && (autowait != -1) &&
                   (now >= autonext) && (now < next))
               {
                  returnCode = active("any",debuglevel, logname);
                  autonext = now + autowait;
               } /* if */

            } /* else */

            if ( (now > exittime) && (now < next))
               done = TRUE;
            else if ( returnCode == 100 )
               done = TRUE;

         } /* else */

      } /* while */

/*--------------------------------------------------------------------*/
/*                      Actively poll if needed                       */
/*--------------------------------------------------------------------*/

      if ( ! done && (first >= 0) )
      {
         if ( batchCommand != NULL )
         {
            returnCode = runCommand( batchCommand );
            if ( returnCode != 0 )
            {
               printmsg(0,
                        "%s exited with %d status, UUPOLL exiting",
                        batchCommand,
                        returnCode);
               break;
            }
         }

         returnCode = active(Rmtname,debuglevel,logname);

         if ( returnCode == 100 )
            done = TRUE;

      } /* if ( ! done && (first >= 0) ) */

   } /* while */

/*--------------------------------------------------------------------*/
/*                          End of main loop                          */
/*--------------------------------------------------------------------*/

   uuxqt( debuglevel );          /* One last call to UUXQT                 */

#ifndef NOCBREAK
   if (!cbrk)
      setcbrk(0);                /* Restore original Cntrl-Break setting   */
#endif

   printmsg(2,"UUPOLL exiting with return code %d", returnCode );

   exit(returnCode);
 } /* main */

/*--------------------------------------------------------------------*/
/*    L i f e  S p a n                                                */
/*                                                                    */
/*    Compute time to run program                                     */
/*--------------------------------------------------------------------*/

static time_t LifeSpan( time_t duration, time_t stoptime )
{
   time_t exittime = LONG_MAX;
   struct tm  *time_record;
   time_t quit;

/*--------------------------------------------------------------------*/
/*             Compute relative quitting time, if desired             */
/*--------------------------------------------------------------------*/

   if (duration != -1)
      exittime = hhmm2sec( duration ) + (now / 60L) * 60L;

/*--------------------------------------------------------------------*/
/*    Compute absolute quitting time, if desired.  Must terminate     */
/*    before midnight unless relative time was given as well, in      */
/*    which case it must terminate before relative time would         */
/*    expire.                                                         */
/*--------------------------------------------------------------------*/

   if (stoptime != -1)
   {

/*--------------------------------------------------------------------*/
/*               Compute the absolute termination time                */
/*--------------------------------------------------------------------*/

      time_record = localtime(&now);
      time_record->tm_sec = 0;
      time_record->tm_min = (int) (stoptime % 100);
      time_record->tm_hour= (int) (stoptime / 100);
      quit = mktime(time_record);

      if ( quit < now )             /* Number should be in future    */
         quit += hhmm2sec( 2400 );

/*--------------------------------------------------------------------*/
/*              Compute default relative time, if needed              */
/*--------------------------------------------------------------------*/

      if ( duration == -1 )
      {
         time_record->tm_min = 0;
         time_record->tm_hour= 24;
         exittime = mktime(time_record);
      }

/*--------------------------------------------------------------------*/
/*                  Determine if we should use time                   */
/*--------------------------------------------------------------------*/

      if ( exittime < quit )
      {

         printf("*** Exiting ***\tProgram was to execute until %s",
                     ctime(&quit));

         printf("\t\twhich would exceed retirement at %s",
                     ctime(&exittime));
         exit(10);

      } /* if ( exittime < now ) */
      else
         exittime = quit;

   } /* else if (stoptime != -1) */

   if (exittime != LONG_MAX)
      printf("Will terminate upon completion of first event after %s",
              ctime(&exittime));

   return exittime;

} /* LifeSpan */

/*--------------------------------------------------------------------*/
/*    a c t i v e                                                     */
/*                                                                    */
/*    Perform an active (outgoing) poll of other hosts                */
/*--------------------------------------------------------------------*/

 static active(char *Rmtname, int debuglevel, const char *logname)
 {
   int result;

   if (Rmtname == NULL)             /* Default?                         */
   {                                /* Yes --> do -s all and -s any     */
      if (active("all",debuglevel, logname ) < 100)
         return active("any",debuglevel, logname);
      else
         return 100;
   }
   else {
      char buf[128];
      sprintf(buf,"uucico -r 1 -s %s -x %d",Rmtname,debuglevel);

      if ( logname != NULL )
         strcat( strcat( buf, " -l ") , logname );
      result = runCommand(buf);
      if ( result == 0 )
         uuxqt( debuglevel );


      printmsg(2,"active: Return code = %d", result );

      return result;
   }
 } /* active */

/*--------------------------------------------------------------------*/
/*    b u s y w o r k                                                 */
/*                                                                    */
/*    Waits for next time to poll without answering the telephone.    */
/*    Maybe we should at least beep on the hour?  :-)                 */
/*--------------------------------------------------------------------*/

static void busywork( time_t next)
{
   time_t naptime;
   time_t hours, minutes, seconds;

   naptime = next - now;

   hours   = (naptime / 3600) % 24;    /* Get pretty time to display... */
   minutes = (naptime / 60) % 60;
   seconds = naptime % 60;

   printf("Going to sleep for %02ld:%02ld:%02ld, next poll is %s",
             hours, minutes, seconds, ctime(&next) );
   ssleep( naptime );

   time( & now );
}


/*--------------------------------------------------------------------*/
/*    r u n C o m m a n d                                             */
/*                                                                    */
/*    Executes a command via a spawn() system call.  This avoids      */
/*    the storage overhead of COMMAND.COM and returns the actual      */
/*    return code from the command executed.                          */
/*                                                                    */
/*    Note that this does not allow quoted command arguments, which   */
/*    not a problem for the intended argv[0]s of UUCICO.              */
/*--------------------------------------------------------------------*/

 static int runCommand( char *command )
 {
   int result;

#ifdef DEBUG
   FILE *stream = NULL;
#endif

   printf("Executing command: %s\n",command);
#ifdef DEBUG                  /* ahd */
   stream = fopen("UUPOLL.LOG","a");
   if (stream == NULL)
   {
      printerr("UUPOLL.LOG");
      panic();
   } /* if */
   fprintf(stream, "%s: %s\n",arpadate(), command);
   fclose(stream);
#endif /* DEBUG */

   result = executeCommand( command, NULL, NULL, TRUE, FALSE );

   if ( result < 0 )
   {
      printf("\a\nCommand \"%s\" failed completely.\n\a", command);
      panic();
   }

   time( & now );

   return result;
} /* runCommand */

/*--------------------------------------------------------------------*/
/*    n e x t p o l l                                                 */
/*                                                                    */
/*    Returns next time to poll in seconds                            */
/*                                                                    */
/*    modified 14 October 1990 By Ed Keith.                           */
/*    modified 4 November 1990 by Drew Derbyshire.                    */
/*--------------------------------------------------------------------*/

static time_t nextpoll( hhmm first, hhmm interval )
{
   time_t sfirst;
   time_t sinterval = hhmm2sec( interval );
   time_t today;
   time_t tomorrow;
   struct tm  *time_record;   /* Ed K. 10/14/1990 */

   time_record = localtime(&now); /* Ed K. 10/14/1990 */
   time_record->tm_sec = 0;   /* Ed K. 10/14/1990 */
   time_record->tm_min = 0;   /* Ed K. 10/14/1990 */
   time_record->tm_hour= 0;   /* Ed K. 10/14/1990 */
   today = mktime(time_record);

/*--------------------------------------------------------------------*/
/*    We could just add hhmm2sec(2400) (24 hours) except this         */
/*    doesn't work during days when we do a daylight savings          */
/*    shift.  So we let the run time library compute midnight.        */
/*--------------------------------------------------------------------*/

   time_record->tm_hour = 23;   /* Advance to midnight     */
   time_record->tm_min  = 59;
   tomorrow = mktime(time_record) + 60;   /* Add a minute to 23:59   */

   sfirst = today + hhmm2sec(first);

   while (sfirst <= now)         // Insure we never return "now" for next
      sfirst += sinterval;

/*--------------------------------------------------------------------*/
/*    Since we restart the polling of each day anew, reset the        */
/*    next poll time based if it is after midnight.  Note the         */
/*    funny double compare, which handles the stricter of the two     */
/*    tests for "tomorrow".                                           */
/*--------------------------------------------------------------------*/

   if ((sfirst > tomorrow) || (sfirst > (today + hhmm2sec(2400))))
      sfirst = tomorrow + hhmm2sec(first);

   return sfirst;

} /* nextpoll */


/*--------------------------------------------------------------------*/
/*    f i r s t p o l l                                               */
/*                                                                    */
/*    Determine first time to poll if not specified                   */
/*--------------------------------------------------------------------*/

static hhmm firstpoll(hhmm interval)
{
   struct tm  *time_record;
   time_t sfirst;
   hhmm first;

   time_record = localtime(&now);
   sfirst = ((time_t) time_record->tm_hour * 3600L +
             (time_t) time_record->tm_min * 60L);
   sfirst  = sfirst % hhmm2sec(interval);
   first = (hhmm) ((sfirst / 3600L) * 100L + (sfirst % 3600L) / 60L);

   printf("First polling time computed to be %-2.2d:%-2.2d\n",
         first / 100, first % 100);

   return first;

} /* firstpoll */

/*--------------------------------------------------------------------*/
/*    n o t a n u m b e r                                             */
/*                                                                    */
/*    Examines string, returns true if non-numeric                    */
/*--------------------------------------------------------------------*/

 static boolean notanumber( char *start)
 {
   char *number = start;
   while (*number != '\0')
   {
      if (!isdigit(*number))
      {
         printf("Parameter must be numeric, was %s\n",start);
         return TRUE;
      }
      number++;
   }
   return FALSE;
 } /* notanumber */


/*--------------------------------------------------------------------*/
/*    p a s s i v e                                                   */
/*                                                                    */
/*    Invoke UUCICO in passive mode until next active poll (if any).  */
/*--------------------------------------------------------------------*/

 static int passive( time_t next,
                     int debuglevel,
                     const char *logname,
                     const char *modem )
 {
   char buf[128];             /* Buffer for runCommand() commands       */
   time_t seconds = (next - now + 59);
   time_t minutes;
   int result;

   if ( seconds > INT_MAX)
      seconds = (INT_MAX / 3600) * 3600;

   minutes = seconds / 60;

   sprintf(buf,"uucico -r 0 -x %d -d %02ld%02ld",
               debuglevel,
               minutes / 60, minutes % 60);

   if ( logname != NULL )
      strcat( strcat( buf, " -l ") , logname );

   if ( modem != NULL )
      strcat( strcat( buf, " -m ") , modem );

   result = runCommand(buf);
   if ( result == 0 )
      uuxqt( debuglevel );

   printmsg(2,"passive: Return code = %d", result );
   return result;

 } /* passive */

/*--------------------------------------------------------------------*/
/*    u u x q t                                                       */
/*                                                                    */
/*    Execute the UUXQT program to run files received by UUCICO       */
/*--------------------------------------------------------------------*/

 static void uuxqt( int debuglevel)
 {
   int result;
   char buf[128];             /* Buffer for runCommand() commands       */

   sprintf(buf,"uuxqt -x %d", debuglevel);
   result = runCommand(buf);

   if ( result != 0 )
   {
      printf("UUXQT failed with a return code of %d\n",result);
      panic();
   } /* if ( result != 0 ) */

 } /* uuxqt */

/*--------------------------------------------------------------------*/
/*    C a t c h e r                                                   */
/*                                                                    */
/*    Catch Ctrl-Break                                                */
/*--------------------------------------------------------------------*/

 void Catcher( void )
 {

    safeout("uupoll: Program aborted by user\r\n");

    _exit(100);
 } /* Catcher */

/*--------------------------------------------------------------------*/
/*    u s a g e                                                       */
/*                                                                    */
/*    Report correct usage of the program and then exit.              */
/*--------------------------------------------------------------------*/

 static void usage( char *name )
 {
   printf("Usage:\t%s"
          "\t[-a hhmm] [-d hhmm | -e hhmm] [-f hhmm] [-i hhmm]\n"
          "\t\t[-l logname] [-c hhmm] [-C command]\n"
          "\t\t[-r 0 | 1] [-s system] [-x n]\n",name);
   exit(4);
 }

#ifndef NOCBREAK
#ifndef __TURBOC__
/*--------------------------------------------------------------------*/
/*    s e t c b r k                                                   */
/*                                                                    */
/*    Enable Cntrl-Break                                              */
/*                                                                    */
/*    Written by Dave Watt                                            */
/*--------------------------------------------------------------------*/

static int setcbrk(char state)
{
   union REGS inregs, outregs;

   inregs.x.ax = 0x3302;
   inregs.h.dl = state;
   intdos(&inregs, &outregs);

   return outregs.h.dl;
}
#endif
#endif
