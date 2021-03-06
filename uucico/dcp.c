/*--------------------------------------------------------------------*/
/*          d c p . c                                                 */
/*                                                                    */
/*          Main routines for UUCICO                                  */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-2002 by Kendra Electronic            */
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
 *    $Id: dcp.c 1.58 2001/03/12 13:55:29 ahd v1-13k $
 *
 *    $Log: dcp.c $
 *    Revision 1.58  2001/03/12 13:55:29  ahd
 *    Annual copyright update
 *
 *    Revision 1.57  2000/05/12 12:32:55  ahd
 *    Annual copyright update
 *
 *    Revision 1.56  1999/01/08 02:21:01  ahd
 *    Convert currentfile() to RCSID()
 *
 *    Revision 1.55  1999/01/04 03:53:57  ahd
 *    Annual copyright change
 *
 *    Revision 1.54  1998/11/05 03:29:03  ahd
 *    Don't abort if Win9X can't suspend (Windows 98 is missing files)
 *
 *    Revision 1.53  1998/07/27 01:03:54  ahd
 *    Disable logging during execute invocation
 *
 *        Revision 1.52  1998/03/01  01:39:14  ahd
 *        Annual Copyright Update
 *
 *    Revision 1.51  1997/04/24 01:33:12  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.50  1996/11/18 04:46:49  ahd
 *    Normalize arguments to bugout
 *    Reset title after exec of sub-modules
 *    Normalize host status names to use HS_ prefix
 *
 *    Revision 1.49  1996/01/01 21:19:54  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.48  1995/07/21 13:27:00  ahd
 *    If modem is unable to dial, be sure to resume suspended UUCICO if needed
 *
 *    Revision 1.47  1995/04/02 00:01:39  ahd
 *    Correct processing to not send files below requested call grade
 *
 *    Revision 1.46  1995/03/11 15:49:23  ahd
 *    Clean up compiler warnings, modify dcp/dcpsys/nbstime for better msgs
 *
 *    Revision 1.45  1995/02/22 12:14:24  ahd
 *    Correct 16 bit compiler warning errors
 *
 *    Revision 1.44  1995/02/14 04:38:42  ahd
 *    Correct problems with directory processing under NT
 *
 *    Revision 1.43  1995/01/30 04:08:36  ahd
 *    Additional compiler warning fixes
 *
 *    Revision 1.42  1995/01/13 14:02:36  ahd
 *    Correct NT VC++ 2.0 warnings
 *
 *    Revision 1.41  1995/01/07 17:40:36  ahd
 *    Change boolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.40  1995/01/07 16:37:55  ahd
 *    Change KWBoolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.39  1994/12/31 03:39:56  ahd
 *    Correct processing for systems with grade restrictions in the
 *    systems file when calling system "any"
 *
 *    Revision 1.38  1994/12/27 20:45:50  ahd
 *    Smoother call grading'
 *
 *    Revision 1.37  1994/12/22 00:34:31  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.36  1994/12/09 03:42:09  ahd
 *    Don't spawn UUXQT with handle open to serial port
 *
 *        Revision 1.35  1994/10/23  23:29:44  ahd
 *        Add new title information
 *
 *        Revision 1.34  1994/10/03  01:01:25  ahd
 *        Change title message to be more descriptive when waiting for
 *        phone to ring.
 *        Move logical release of port to physical release port in shutdown()
 *
 *        Revision 1.33  1994/08/07  21:45:09  ahd
 *        Correct selected changes in host title
 *
 *        Revision 1.32  1994/05/06  03:55:50  ahd
 *        Hot login support
 *
 *        Revision 1.31  1994/05/04  23:36:34  ahd
 *        Trap missing modem file
 *
 *        Revision 1.30  1994/05/04  01:56:22  ahd
 *        Clean up title messages
 *
 *        Revision 1.29  1994/05/01  21:59:06  dmwatt
 *        Trap errors from failure of suspend_init to create pipe
 *
 *        Revision 1.28  1994/04/27  00:02:15  ahd
 *        Pick one: Hot handles support, OS/2 TCP/IP support,
 *                  title bar support
 *
 * Revision 1.27  1994/02/20  19:11:18  ahd
 * IBM C/Set 2 Conversion, memory leak cleanup
 *
 * Revision 1.26  1994/02/19  05:06:20  ahd
 * Use standard first header
 *
 * Revision 1.25  1994/01/01  19:18:06  ahd
 * Annual Copyright Update
 *
 * Revision 1.24  1993/12/02  13:48:35  ahd
 * Don't release network ports after use
 *
 * Revision 1.23  1993/10/28  12:19:01  ahd
 * Cosmetic time formatting twiddles and clean ups
 *
 * Revision 1.22  1993/10/25  01:21:22  ahd
 * Restructure to suppress silly unable to perform global optimizations
 * message under MS C 6.0.
 * Trap and report invalid system names when dialing out.
 *
 * Revision 1.21  1993/10/12  01:32:46  ahd
 * Normalize comments to PL/I style
 *
 * Revision 1.20  1993/10/03  20:37:34  ahd
 * Don't attempt to suspend port if using network protocol suite
 *
 * Revision 1.19  1993/09/30  03:06:28  ahd
 * Move suspend signal handler into suspend2
 *
 * Revision 1.18  1993/09/29  13:18:06  ahd
 * Don't call suspend handler under DOS Turbo C++ (not impletemented, anyway)
 *
 * Revision 1.17  1993/09/29  04:52:03  ahd
 * Suspend port by port name, not modem file name
 *
 * Revision 1.16  1993/09/27  00:48:43  ahd
 * Control UUCICO in passive mode by K. Rommel
 *
 * Revision 1.15  1993/09/20  04:46:34  ahd
 * OS/2 2.x support (BC++ 1.0 support)
 * TCP/IP support from Dave Watt
 * 't' protocol support
 *
 * Revision 1.14  1993/08/02  03:24:59  ahd
 * Further changes in support of Robert Denny's Windows 3.x support
 *
 * Revision 1.13  1993/07/31  16:26:01  ahd
 * Changes in support of Robert Denny's Windows support
 *
 * Revision 1.12  1993/07/22  23:22:27  ahd
 * First pass at changes for Robert Denny's Windows 3.1 support
 *
 * Revision 1.11  1993/05/30  00:01:47  ahd
 * Allow tracing connection via UUCICO -t flag
 *
 * Revision 1.10  1993/04/11  00:35:46  ahd
 * Global edits for year, TEXT, etc.
 *
 * Revision 1.9  1993/04/05  04:32:19  ahd
 * Allow unique send and receive packet sizes
 *
 * Revision 1.8  1993/01/23  19:08:09  ahd
 * Don't update host status at sysend() if hostp is not initialized
 *
 * Revision 1.7  1992/12/18  12:05:57  ahd
 * Suppress duplicate machine state messages to improve OS/2 scrolling
 *
 * Revision 1.6  1992/12/01  04:37:03  ahd
 * Add standard comment block for header
 *
 * Revision 1.5  1992/11/28  19:51:16  ahd
 * If in multitask mode, only open syslog on demand basis
 *
 * Revision 1.4  1992/11/22  21:30:55  ahd
 * Do not bother to strdup() string arguments
 *
 * 25Aug87 - Added a version number - Jal
 * 25Aug87 - Return 0 if contact made with host, or 5 otherwise.
 * 04Sep87 - Bug causing premature sysend() fixed. - Randall Jessup
 * 13May89 - Add date to version message  - Drew Derbyshire
 * 17May89 - Add '-u' (until) option for login processing
 * 01 Oct 89      Add missing function prototypes
 * 28 Nov 89      Add parse of incoming user id for From record
 * 18 Mar 90      Change checktime() calls to Microsoft C 5.1
 */

/*--------------------------------------------------------------------*/
/* This program implements a uucico type file transfer and remote     */
/* execution protocol.                                                */
/*                                                                    */
/* Usage:   UUCICO [-s sys]                                           */
/*                 [-d hhmm]                                          */
/*                 [-g grade]                                         */
/*                 [-h handle]                                        */
/*                 [-l logfile]                                       */
/*                 [-m modem]                                         */
/*                 [-r 0|1]                                           */
/*                 [-t]                                               */
/*                 [-U]                                               */
/*                 [-w userid]                                        */
/*                 [-x debug]                                         */
/*                 [-x debuglevel]                                    */
/*                 [-z bps]                                           */
/*                                                                    */
/* e.g.                                                               */
/*                                                                    */
/* UUCICO [-x n] -r 0 [-d hhmm]    client mode, wait for an incoming  */
/*                                 call for 'hhmm'.                   */
/* UUCICO [-x n] -s HOST     call the host "HOST".                    */
/* UUCICO [-x n] -s all      call all known hosts in the systems      */
/*                           file.                                    */
/* UUCICO [-x n] -s any      call any host we have work queued for.   */
/* UUCICO [-x n]             same as the above.                       */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"

#include <limits.h>

#ifdef _Windows
#include <Windows.h>
#endif

/*--------------------------------------------------------------------*/
/*                      UUPC/extended prototypes                      */
/*--------------------------------------------------------------------*/

#include "catcher.h"
#include "checktim.h"
#include "dcp.h"
#include "dcplib.h"
#include "dcpstats.h"
#include "dcpsys.h"
#include "dcpxfer.h"
#include "expath.h"
#include "getopt.h"
#include "hostable.h"
#include "hostatus.h"
#include "lock.h"
#include "logger.h"
#include "modem.h"
#include "security.h"
#include "ssleep.h"
#include "suspend.h"
#include "commlib.h"
#include "title.h"
#include "execute.h"
#include "pushpop.h"

#if defined(_Windows)
#include "winutil.h"
#endif

/*--------------------------------------------------------------------*/
/*    Define passive and active polling modes; passive is             */
/*    sometimes refered to as "slave", "active" as master.  Since     */
/*    the roles can actually switch during processing, we avoid       */
/*    the terms here                                                  */
/*--------------------------------------------------------------------*/

typedef enum {
      POLL_PASSIVE = 0,       /* We answer the telephone          */
      POLL_ACTIVE  = 1        /* We call out to another host      */
      } POLL_MODE ;

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

size_t s_pktsize;             /* send packet size for protocol       */
size_t r_pktsize;             /* receive packet size for protocol    */

FILE *xfer_stream = NULL;        /* stream for file being handled    */
KWBoolean callnow = KWFalse;         /* KWTrue = ignore time in L.SYS    */
FILE *fwork = NULL, *fsys= NULL ;
FILE *syslog = NULL;
char workfile[FILENAME_MAX];  /* name of current workfile         */
char *Rmtname = "any";        /* system we want to call           */
char rmtname[20];             /* system we end up talking to      */
struct HostTable *hostp;
struct HostStats remote_stats; /* host status, as defined by hostatus */

static KWBoolean dialed = KWFalse;/* True = We attempted a phone call */

RCSID("$Id: dcp.c 1.58 2001/03/12 13:55:29 ahd v1-13k $");

/*--------------------------------------------------------------------*/
/*                     Local function prototypes                      */
/*--------------------------------------------------------------------*/

static CONN_STATE process( const POLL_MODE poll_mode, const char callGrade );

static KWBoolean master( const char recvGrade,
                       const KWBoolean overrideGrade,
                       const KWBoolean runUUXQT );

static KWBoolean client( const time_t exitTime,
                       const char *hotUser,
                       const BPS hotBPS,
                       const int hotHandle,
                       const KWBoolean runUUXQT );

/*--------------------------------------------------------------------*/
/*    d c p m a i n                                                   */
/*                                                                    */
/*    main program for DCP, called by uuhost                          */
/*--------------------------------------------------------------------*/

int dcpmain(int argc, char *argv[])
{

   char *logfile_name = NULL;
   KWBoolean  contacted = KWFalse;

   int option;
   int pollMode = POLL_ACTIVE;   /* Default = dial out to system     */
   time_t exitTime = LONG_MAX;

   char recvGrade = ALL_GRADES;
   KWBoolean overrideGrade = KWFalse;
   KWBoolean runUUXQT = KWFalse;

   char *hotUser = NULL;
   BPS  hotBPS = 0;
   int  hotHandle = -1;

   fwork = nil(FILE);

/*--------------------------------------------------------------------*/
/*                        Process our options                         */
/*--------------------------------------------------------------------*/

   while ((option = getopt(argc, argv, "d:g:h:m:l:r:s:tUw:x:z:n?")) != EOF)
      switch (option)
      {

      case 'd':
         exitTime = atoi( optarg );
         exitTime = time(NULL) + hhmm2sec(exitTime);
         pollMode = POLL_PASSIVE;  /* Implies passive polling       */
         break;

      case 'g':
         if (strlen(optarg) == 1 )
            recvGrade = *optarg;
         else {
            recvGrade = checktime( optarg );
                                 /* Get restriction for this hour */
            if ( ! recvGrade )   /* If no class, use the default  */
               recvGrade = ALL_GRADES;
         }
         overrideGrade = KWTrue;
         break;

      case 'h':
         hotHandle = atoi( optarg );   /* Handle opened for us       */
         pollMode = POLL_PASSIVE;  /* Implies passive polling       */
         break;

      case 'm':                     /* Override in modem name     */
         E_inmodem = optarg;
         pollMode = POLL_PASSIVE;  /* Implies passive polling       */
         break;

      case 'l':                     /* Log file name              */
         logfile_name = optarg;
         break;

      case 'n':
         callnow = KWTrue;
         break;

      case 'r':
         pollMode = atoi(optarg);
         break;

      case 's':
         Rmtname = optarg;
         break;

      case 't':
         traceEnabled = KWTrue;
         break;

      case 'U':
         runUUXQT = KWTrue;
         break;

      case 'x':
         debuglevel = atoi(optarg);
         break;

      case 'z':
         hotBPS = (BPS) atoi(optarg);
         pollMode = POLL_PASSIVE;  /* Implies passive polling       */
         break;

      case 'w':
         pollMode = POLL_PASSIVE;  /* Implies passive polling       */
         hotUser = optarg;
         break;

      case '?':
         puts("\nUsage:\tuucico\t"
         "[-s [all | any | sys]] [-r 1|0] [-d hhmm]\n"
         "\t\t[-l logfile] [-n] [-t] [-U] [-x debug]\n"
         "\t\t[-h handle] [-m modem] [-z bps]");
         return 4;
      }

/*--------------------------------------------------------------------*/
/*                Abort if any options were left over                 */
/*--------------------------------------------------------------------*/

   if (optind != argc) {
      puts("Extra parameter(s) at end.");
      return 4;
   }

/*--------------------------------------------------------------------*/
/*        Initialize logging and the name of the systems file         */
/*--------------------------------------------------------------------*/

   openlog( logfile_name );

   if (bflag[F_SYSLOG] && ! bflag[F_MULTITASK])
   {
      syslog = FOPEN(SYSLOG, "a",TEXT_MODE);
      if ((syslog == nil(FILE)) || setvbuf( syslog, NULL, _IONBF, 0))
      {
         printerr( SYSLOG );
         panic();
      }
   }

   PushDir(E_spooldir);
   atexit( PopDir );

   if ( terminate_processing )
      return 100;

/*--------------------------------------------------------------------*/
/*                        Initialize security                         */
/*--------------------------------------------------------------------*/

   if ( !LoadSecurity())
   {
      printmsg(0,"Unable to initialize security, see previous message");
      panic();
   }

   if ( terminate_processing )
      return 100;

#if defined(_Windows)
   atexit(CloseEasyWin);       /* Auto-close EasyWin window on exit   */
#endif

   atexit( shutDown );        /* Insure port is closed by panic()    */
   remote_stats.hstatus = HS_NOCALL;
                              /* Known state for automatic status
                                 update                              */

/*--------------------------------------------------------------------*/
/*                     Begin main processing loop                     */
/*--------------------------------------------------------------------*/

   if (pollMode == POLL_ACTIVE)
      contacted = master(recvGrade, overrideGrade, runUUXQT );
   else if (pollMode == POLL_PASSIVE)
      contacted = client(exitTime,
                         hotUser,
                         hotBPS,
                         hotHandle,
                         runUUXQT);
   else {
      printmsg(0,"Invalid -r flag, must be 0 or 1");
      panic();
   }

/*--------------------------------------------------------------------*/
/*                         Report our results                         */
/*--------------------------------------------------------------------*/

   if (!contacted && (pollMode == POLL_ACTIVE))
   {
      if (dialed)
         printmsg(0, "Could not connect to remote system.");
      else
         printmsg(0,
               "No work for requested system or wrong time to call.");
   }

   dcupdate();

   if (bflag[F_SYSLOG] && ! bflag[F_MULTITASK])
      fclose(syslog);

   return terminate_processing ? 100 : (contacted ? 0 : 5);

} /* dcpmain */

/*--------------------------------------------------------------------*/
/*       m a s t e r                                                  */
/*                                                                    */
/*       Call out to other sites                                      */
/*--------------------------------------------------------------------*/

static KWBoolean master( const char recvGrade,
                       const KWBoolean overrideGrade,
                       const KWBoolean runUUXQT )
{

   CONN_STATE m_state = CONN_INITSTAT;
   CONN_STATE old_state = CONN_EXIT;

   char sendGrade = ALL_GRADES;

   KWBoolean contacted = KWFalse;
   KWBoolean needUUXQT = KWFalse;

/*--------------------------------------------------------------------*/
/*                    Validate the system to call                     */
/*--------------------------------------------------------------------*/

   if ( !equal( Rmtname, "any" ) && !equal( Rmtname, "all" ))
   {
      if ( checkreal( Rmtname ) == NULL )
      {
         printmsg(0,"%s is not \"any\", \"all\", or a valid system to call",
                     Rmtname);
         printmsg(0,"Run UUNAME for a list of callable systems");
         panic();
      }

   }

   if ((fsys = FOPEN(E_systems, "r",TEXT_MODE)) == nil(FILE))
   {
      printerr(E_systems);
      panic();
   }

   setvbuf( fsys, NULL, _IONBF, 0);

   while (m_state != CONN_EXIT )
   {
      printmsg(old_state == m_state ? 10 : 4 ,
               "M state = %c", m_state);
      old_state = m_state;

      switch (m_state)
      {
         case CONN_INITSTAT:
            HostStatus();
            m_state = CONN_INITIALIZE;
            break;

         case CONN_INITIALIZE:
            setTitle("Determining system to call");
            hostp = NULL;

            if ( locked )
               UnlockSystem();

            m_state = getsystem(recvGrade);
            if ( hostp != NULL )
               remote_stats.hstatus = hostp->status.hstatus;
            break;

         case CONN_CHECKTIME:
            sendGrade = checktime(flds[FLD_CCTIME]);

            if ( (overrideGrade && sendGrade) || callnow )
               sendGrade = recvGrade;

            if ( !CallWindow( sendGrade ))
               m_state = CONN_INITIALIZE;
            else if ( LockSystem( hostp->hostname , B_UUCICO))
            {
               dialed = KWTrue;
               time(&hostp->status.ltime);
                              /* Save time of last attempt to call  */
               hostp->status.hstatus = HS_AUTODIAL;
               m_state = CONN_MODEM;
            }
            else
               m_state = CONN_INITIALIZE;

            break;

         case CONN_NOGRADE:
            CallWindow( 0 );        /* Simply to update last time called */
            m_state = CONN_INITIALIZE;
            break;

         case CONN_MODEM:
            if (getmodem(flds[FLD_TYPE]))
               m_state = CONN_DIALOUT;
            else {
               hostp->status.hstatus = HS_INVALID_DEVICE;
               m_state = CONN_INITIALIZE;
            }
            break;

         case CONN_DIALOUT:
            if ( !IsNetwork() )
            {
               setTitle( "Allocating modem on %s", M_device);
               if (suspend_other(KWTrue, M_device ) < 0 )
               {
                  hostp->status.hstatus = HS_NODEVICE;
                  suspend_other(KWFalse, M_device );  /* Resume modem   */
                  m_state = CONN_INITIALIZE;    /* Try next system      */
                  break;
               }
            } /* if */

            setTitle( "Calling %s on %s", rmtname, M_device );
            m_state = callup( );
            break;

         case CONN_PROTOCOL:
            m_state = startup_server( (char)
                                       (bflag[F_SYMMETRICGRADES] ?
                                       sendGrade  : recvGrade) );
            break;

         case CONN_SERVER:
            if (bflag[F_MULTITASK])
               dcupdate();

            setTitle("%s connected to %s", securep->myname, hostp->via );
            m_state = process( POLL_ACTIVE, sendGrade );
            contacted = KWTrue;
            break;

         case CONN_TERMINATE:
            m_state = sysend();

            if ( hostp != NULL )
            {
               if (hostp->status.hstatus == HS_INPROGRESS)
                  hostp->status.hstatus = HS_CALL_FAILED;
               dcstats();
               needUUXQT = KWTrue;
            }
            break;

         case CONN_TIMESET:
            contacted = KWTrue;
            m_state = CONN_DROPLINE;
            break;

         case CONN_DROPLINE:
            setTitle("Closing port %s", M_device);
            shutDown();
            UnlockSystem();
            setTitle("Not connected");

            m_state = CONN_CLEANUP;
            break;

         case CONN_CLEANUP:
            if ( runUUXQT && needUUXQT )
               m_state = CONN_UUXQT;
            else
               m_state = CONN_INITIALIZE;
            break;

         case CONN_UUXQT:
            {
               char buf[100];
               sprintf( buf, "-s %s -x %d", rmtname, debuglevel );
               copylog();
               execute( "uuxqt", buf, NULL, NULL, KWFalse, KWFalse );
               openlog(NULL);
            }
            needUUXQT = KWFalse;
            m_state = CONN_INITIALIZE;
            break;

         case CONN_EXIT:
            break;

         default:
            printmsg(0,"dcpmain: Unknown master state = %c",m_state );
            panic();
            break;
      } /* switch */

      if ( terminate_processing )
         m_state = CONN_EXIT;

   } /* while */

   setTitle("Exiting");

   fclose(fsys);

   return contacted;

} /* master */

/*--------------------------------------------------------------------*/
/*       c l i e n t                                                  */
/*                                                                    */
/*       Allow other systems to call us                               */
/*--------------------------------------------------------------------*/

static KWBoolean client( const time_t exitTime,
                       const char *hotUser,
                       const BPS hotBPS,
                       const int hotHandle,
                       const KWBoolean runUUXQT )
{

   CONN_STATE s_state = CONN_INITIALIZE;
   CONN_STATE old_state = CONN_EXIT;

   KWBoolean contacted = KWFalse;
   KWBoolean needUUXQT = KWFalse;

   char sendGrade = ALL_GRADES;

/*--------------------------------------------------------------------*/
/*                      Trap missing modem entry                      */
/*--------------------------------------------------------------------*/

   if ( E_inmodem == NULL )
   {
      printmsg(0,"No modem specified in configuration file or command line");
      panic();
   }

   if (!getmodem(E_inmodem))  /* Initialize modem configuration     */
      panic();                /* Avoid loop if bad modem name       */

   if ( ! IsNetwork() &&
        ( hotUser == NULL ) &&
        (hotHandle == -1 ) &&
        ! suspend_init(M_device))
   {

#ifdef WIN32
      if (!isWinNT())
      {
         printmsg(0,"Unable to set up pipe for suspending; "
                    "may be unsupported in this environment (use NT for modem sharing).");
      }
      else
#endif
      /* else under WIN32, otherwise unconditional */
      {
         printmsg(0,"Unable to set up pipe for suspending; "
                    "is another UUCICO running?" );
         panic();
      }
   }

   while (s_state != CONN_EXIT )
   {
      printmsg(s_state == old_state ? 10 : 4 ,
               "S state = %c", s_state);
      old_state = s_state;

      switch (s_state)
      {
         case CONN_INITIALIZE:
            if (( hotUser == NULL ) && (hotHandle == -1 ))
               s_state = CONN_ANSWER;
            else
               s_state = CONN_HOTMODEM;
            break;

         case CONN_WAIT:
#if !defined(__TURBOC__) || defined(BIT32ENV)
            setTitle("Port %s suspended", M_device);
           s_state = suspend_wait();
#else
           panic();                 /* Why are we here?!           */
#endif
           break;

         case CONN_ANSWER:
            setTitle("Monitoring port %s", M_device);
            s_state = callin( exitTime );
            break;

         case CONN_HOTMODEM:
            s_state = callhot( hotBPS, hotHandle );
            break;

         case CONN_HOTLOGIN:
            if ( hotUser == NULL )        /* User specified to login? */
               s_state = CONN_LOGIN;      /* No --> Process normally  */
            else if ( loginbypass( hotUser ) )
               s_state = CONN_INITSTAT;
            else
               s_state = CONN_DROPLINE;
            break;

         case CONN_LOGIN:
            setTitle("Processing login on %s",
                      M_device );
            if ( login( ) )
               s_state = CONN_INITSTAT;
            else
               s_state = CONN_DROPLINE;
            break;

         case CONN_INITSTAT:
            HostStatus();
            s_state = CONN_PROTOCOL;
            break;

         case CONN_PROTOCOL:
            setTitle("Establishing connection on %s",
                      M_device);
            s_state = startup_client(&sendGrade);
            break;

         case CONN_CLIENT:
            contacted = KWTrue;
            if (bflag[F_MULTITASK])
               dcupdate();

            setTitle("%s connected to %s",
                      securep->myname,
                      hostp->via,
                      M_device);
            s_state = process( POLL_PASSIVE, sendGrade );
            break;

         case CONN_TERMINATE:
            s_state = sysend();
            if ( hostp != NULL )
            {
               dcstats();
               needUUXQT = KWTrue;
            }
            break;

         case CONN_DROPLINE:
            shutDown();
            if ( locked )     /* Cause could get here w/o
                                 locking                    */
               UnlockSystem();
            s_state = CONN_CLEANUP;
            break;

         case CONN_CLEANUP:
            if ( runUUXQT && needUUXQT )
               s_state = CONN_UUXQT;
            else
               s_state = CONN_EXIT;
            break;

         case CONN_UUXQT:
            {
               char buf[100];
               sprintf( buf, "-s %s -x %d", rmtname, debuglevel );
               copylog();
               execute( "uuxqt", buf, NULL, NULL, KWFalse, KWFalse );
               openlog(NULL);
            }
            needUUXQT = KWFalse;
            s_state = CONN_EXIT;
            break;

         case CONN_EXIT:
            break;

         default:
            printmsg(0,"dcpmain: Unknown slave state = %c",s_state );
            panic();
            break;

      } /* switch */

      if ( terminate_processing )
         s_state = CONN_EXIT;

   } /* while */

   return contacted;

} /* client */

/*--------------------------------------------------------------------*/
/*    p r o c e s s                                                   */
/*                                                                    */
/*    The procotol state machine                                      */
/*--------------------------------------------------------------------*/

static CONN_STATE process( const POLL_MODE pollMode, const char callGrade )
{
   KWBoolean master  = (KWBoolean) ( pollMode == POLL_ACTIVE ?
                                       KWTrue : KWFalse );
   KWBoolean aborted = KWFalse;
   XFER_STATE state =  (XFER_STATE) (master ? XFER_SENDINIT : XFER_RECVINIT);
   XFER_STATE old_state = XFER_EXIT;
                              /* Initialized to any state but the
                                 original value of "state"           */
   XFER_STATE save_state = XFER_EXIT;
   char currentGrade = (char) ((unsigned char) 0xff);

/*--------------------------------------------------------------------*/
/*  Yea old state machine for the high level file transfer procotol   */
/*--------------------------------------------------------------------*/

   while( state != XFER_EXIT )
   {
      printmsg(state == old_state ? 14 : 4 ,
               "process: Machine state is = %c", state );
      old_state = state;

      if ( terminate_processing != aborted )
      {
         aborted = terminate_processing;
         state = XFER_ABORT;
      }

      switch( state )
      {

         case XFER_SENDINIT:  /* Initialize outgoing protocol        */
            state = sinit();
            break;

         case XFER_RECVINIT:  /* Initialize Receive protocol         */
            state = rinit();
            break;

         case XFER_MASTER:    /* Begin master mode                   */
            master = KWTrue;
            state = XFER_NEXTJOB;
            resetGrade( );    /* Reset best grade status             */
            currentGrade = E_firstGrade;
            break;

         case XFER_SLAVE:     /* Begin slave mode                    */
            master = KWFalse;
            state = XFER_RECVHDR;
            break;

         case XFER_NEXTJOB:   /* Look for work in local queue        */
            state = scandir( rmtname, currentGrade );
            break;

         case XFER_REQUEST:   /* Process next file in current job
                                 in queue                            */
            state = newrequest();
            break;

         case XFER_PUTFILE:   /* Got local transmit request          */
            state = ssfile();
            break;

         case XFER_GETFILE:   /* Got local tranmit request           */
            state = srfile();
            break;

         case XFER_SENDDATA:  /* Remote accepted our work, send data */
            state = sdata();
            break;

         case XFER_SENDEOF:   /* File xfer complete, send EOF        */
            state = seof( master );
            break;

         case XFER_FILEDONE:  /* Receive or transmit is complete     */
            state = (XFER_STATE) (master ? XFER_REQUEST : XFER_RECVHDR);
            break;

         case XFER_NEXTGRADE: /* Process next grade of local files   */
            currentGrade = nextGrade( callGrade );

            if ( currentGrade )
               state = XFER_NEXTJOB;
            else
               state = XFER_NOLOCAL;
            break;

         case XFER_NOLOCAL:   /* No local work, remote have any?     */
            state = sbreak();
            break;

         case XFER_NOREMOTE:  /* No remote work, local have any?     */
            state = schkdir( (KWBoolean) (pollMode == POLL_ACTIVE ?
                                             KWTrue : KWFalse ),
                              callGrade );
            break;

         case XFER_RECVHDR:   /* Receive header from other host      */
            state = rheader();
            break;

         case XFER_TAKEFILE:  /* Set up to receive remote requested
                                 file transfer                       */
            state = rrfile();
            break;

         case XFER_GIVEFILE:  /* Set up to transmit remote
                                 requuest file transfer              */
            state = rsfile();
            break;

         case XFER_RECVDATA:  /* Receive file data from other host   */
            state = rdata();
            break;

         case XFER_RECVEOF:
            state = reof();
            break;

         case XFER_LOST:      /* Lost the other host, flame out      */
            printmsg(0,"process: Connection lost to %s, "
                       "previous system state = %c",
                       rmtname, save_state );
            hostp->status.hstatus = HS_CALL_FAILED;
            state = XFER_EXIT;
            break;

         case XFER_ABORT:     /* Internal error, flame out           */
            printmsg(0,"process: Aborting connection to %s, "
                       "previous system state = %c",
                       rmtname, save_state );
            hostp->status.hstatus = HS_CALL_FAILED;
            state = XFER_ENDP;
            break;

         case XFER_ENDP:      /* Terminate the protocol              */
            state = endp();
            break;

         default:
            printmsg(0,"process: Unknown state = %c, "
                       "previous system state = %c",
                       state, save_state );
            state = XFER_ABORT;
            break;
      } /* switch */

      save_state = old_state; /* Used only if we abort               */

   } /* while( state != XFER_EXIT ) */

/*--------------------------------------------------------------------*/
/*           Protocol is complete, terminate the connection           */
/*--------------------------------------------------------------------*/

   return CONN_TERMINATE;

} /* process */
