/*--------------------------------------------------------------------*/
/*          d c p . c                                                 */
/*                                                                    */
/*          Main routines for UUCICO                                  */
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
 *    $Id: dcp.c 1.12 1993/07/22 23:22:27 ahd Exp $
 *
 *    $Log: dcp.c $
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
/*                 [-r 0|1]                                           */
/*                 [-x debug]                                         */
/*                 [-d hhmm]                                          */
/*                 [-m modem]                                         */
/*                 [-l logfile]                                       */
/*                 [-x debuglevel]                                    */
/*                 [-w userid]                                        */
/*                 [-z bps]                                           */
/*                 [-t]                                               */
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <limits.h>
#include <time.h>

#ifdef _Windows
#include <Windows.h>
#endif

/*--------------------------------------------------------------------*/
/*                      UUPC/extended prototypes                      */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "arpadate.h"
#include "catcher.h"
#include "checktim.h"
#include "dcp.h"
#include "dcplib.h"
#include "dcpstats.h"
#include "dcpsys.h"
#include "dcpxfer.h"
#include "expath.h"
#include "getopt.h"
#include "hlib.h"
#include "hostable.h"
#include "hostatus.h"
#include "lock.h"
#include "logger.h"
#include "modem.h"
#include "security.h"
#include "ssleep.h"
#include "commlib.h"

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
boolean callnow = FALSE;           /* TRUE = ignore time in L.SYS        */
FILE *fwork = NULL, *fsys= NULL ;
FILE *syslog = NULL;
char workfile[FILENAME_MAX];  /* name of current workfile         */
char *Rmtname = nil(char);    /* system we want to call           */
char rmtname[20];             /* system we end up talking to      */
struct HostTable *hostp;
struct HostStats remote_stats; /* host status, as defined by hostatus */

static boolean dialed = FALSE;/* True = We attempted a phone call */

currentfile();

/*--------------------------------------------------------------------*/
/*                     Local function prototypes                      */
/*--------------------------------------------------------------------*/

static CONN_STATE process( const POLL_MODE poll_mode, const char callgrade );

/*--------------------------------------------------------------------*/
/*    d c p m a i n                                                   */
/*                                                                    */
/*    main program for DCP, called by uuhost                          */
/*--------------------------------------------------------------------*/

int dcpmain(int argc, char *argv[])
{

   char *logfile_name = NULL;
   boolean  Contacted = FALSE;

   int option;
   int poll_mode = POLL_ACTIVE;   /* Default = dial out to system     */
   time_t exit_time = LONG_MAX;

   char recvgrade = ALL_GRADES;
   boolean override_grade = FALSE;
   char sendgrade = ALL_GRADES;

   char *hotuser = NULL;
   BPS  hotbaud = 0;

   fwork = nil(FILE);

/*--------------------------------------------------------------------*/
/*                        Process our options                         */
/*--------------------------------------------------------------------*/

   while ((option = getopt(argc, argv, "d:g:m:l:r:s:tw::x:z:n?")) != EOF)
      switch (option)
      {

      case 'd':
         exit_time = atoi( optarg );
         exit_time = time(NULL) + hhmm2sec(exit_time);
         break;

      case 'g':
         if (strlen(optarg) == 1 )
            recvgrade = *optarg;
         else {
            recvgrade = checktime( optarg );
                                 /* Get restriction for this hour */
            if ( ! recvgrade )   /* If no class, use the default  */
               recvgrade = ALL_GRADES;
         }
         override_grade = TRUE;
         break;

      case 'm':                     /* Override in modem name     */
         E_inmodem = optarg;
         poll_mode = 0;             /* Presume passive polling */
         break;

      case 'l':                     /* Log file name              */
         logfile_name = optarg;
         break;

      case 'n':
         callnow = TRUE;
         break;

      case 'r':
         poll_mode = atoi(optarg);
         break;

      case 's':
         Rmtname = optarg;
         break;

      case 't':
         traceEnabled = TRUE;
         break;

      case 'x':
         debuglevel = atoi(optarg);
         break;

      case 'z':
         hotbaud = atoi(optarg);
         break;

      case 'w':
         poll_mode = 0;       /* Presume passive polling */
         hotuser = optarg;
         break;

      case '?':
         puts("\nUsage:\tuucico\t"
         "[-s [all | any | sys]] [-r 1|0] [-x debug] [-d hhmm]\n"
         "\t\t[-n] [-t] [-w user] [-l logfile] [-m modem] [-z bps]");
         return 4;
      }

/*--------------------------------------------------------------------*/
/*                Abort if any options were left over                 */
/*--------------------------------------------------------------------*/

   if (optind != argc) {
      puts("Extra parameter(s) at end.");
      return 4;
   }

   if (Rmtname == nil(char))
      Rmtname = "any";

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
   atexit(CloseEasyWin);       // Auto-close EasyWin window on exit
#endif

   atexit( shutdown );        /* Insure port is closed by panic()    */
   remote_stats.save_hstatus = nocall;
                              /* Known state for automatic status
                                 update                              */

/*--------------------------------------------------------------------*/
/*                     Begin main processing loop                     */
/*--------------------------------------------------------------------*/

   if (poll_mode == POLL_ACTIVE) {

      CONN_STATE m_state = CONN_INITSTAT;
      CONN_STATE old_state = CONN_EXIT;

      printmsg(2, "calling \"%s\", debug=%d", Rmtname, debuglevel);

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

         if (bflag[F_MULTITASK] &&
              (hostp != NULL ) &&
              (remote_stats.save_hstatus != hostp->hstatus ))
         {
            dcupdate();
            remote_stats.save_hstatus = hostp->hstatus;
         }

         switch (m_state)
         {
            case CONN_INITSTAT:
               HostStatus();
               m_state = CONN_INITIALIZE;
               break;

            case CONN_INITIALIZE:
               hostp = NULL;

               if ( locked )
                  UnlockSystem();

               m_state = getsystem(recvgrade);
               if ( hostp != NULL )
                  remote_stats.save_hstatus = hostp->hstatus;
               break;

            case CONN_CALLUP1:
               sendgrade = checktime(flds[FLD_CCTIME]);

               if ( (override_grade && sendgrade) || callnow )
                  sendgrade = recvgrade;

               if ( !CallWindow( sendgrade ))
                  m_state = CONN_INITIALIZE;
               else if ( LockSystem( hostp->hostname , B_UUCICO))
               {
                  dialed = TRUE;
                  time(&hostp->hstats->ltime);
                                 /* Save time of last attempt to call   */
                  hostp->hstatus = autodial;
                  m_state = CONN_CALLUP2;
               }
               else
                  m_state = CONN_INITIALIZE;

               break;

            case CONN_CALLUP2:
               m_state = callup( );

               break;

            case CONN_PROTOCOL:
               m_state = startup_server( (char)
                                          (bflag[F_SYMMETRICGRADES] ?
                                          sendgrade  : recvgrade) );
               break;

            case CONN_SERVER:
               m_state = process( poll_mode, recvgrade );
               Contacted = TRUE;
               break;

            case CONN_TERMINATE:
               m_state = sysend();
               if ( hostp != NULL )
               {
                  if (hostp->hstatus == inprogress)
                     hostp->hstatus = call_failed;
                  dcstats();
               }
               break;

            case CONN_DROPLINE:
               shutdown();
               UnlockSystem();
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
      fclose(fsys);

   }
   else { /* client mode */

      CONN_STATE s_state = CONN_INITIALIZE;
      CONN_STATE old_state = CONN_EXIT;

      while (s_state != CONN_EXIT )
      {
         printmsg(s_state == old_state ? 10 : 4 ,
                  "S state = %c", s_state);
         old_state = s_state;

         if (bflag[F_MULTITASK] &&
              (hostp != NULL ) &&
              (remote_stats.save_hstatus != hostp->hstatus ))
         {
            printmsg(2, "Updating status for host %s, status %d",
                        hostp->hostname ,
                        (int) hostp->hstatus );
            dcupdate();
            remote_stats.save_hstatus = hostp->hstatus;
         }

         switch (s_state) {
            case CONN_INITIALIZE:
               if ( hotuser == NULL )
                  s_state = CONN_ANSWER;
               else
                  s_state = CONN_HOTMODEM;
               break;

            case CONN_ANSWER:
               s_state = callin( exit_time );
               break;

            case CONN_HOTMODEM:
               s_state = callhot( hotbaud );
               break;

            case CONN_HOTLOGIN:
               if ( loginbypass( hotuser ) )
                  s_state = CONN_INITSTAT;
               else
                  s_state = CONN_DROPLINE;
               break;

            case CONN_LOGIN:
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
               s_state = startup_client(&sendgrade);
               break;

            case CONN_CLIENT:
               Contacted = TRUE;
               s_state = process( poll_mode, sendgrade );
               break;

            case CONN_TERMINATE:
               s_state = sysend();
               if ( hostp != NULL )
                  dcstats();
               break;

            case CONN_DROPLINE:
               shutdown();
               if ( locked )     /* Cause could get here w/o
                                    locking                    */
                  UnlockSystem();
               s_state = CONN_EXIT;

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
   } /* else */

/*--------------------------------------------------------------------*/
/*                         Report our results                         */
/*--------------------------------------------------------------------*/

   if (!Contacted && (poll_mode == POLL_ACTIVE))
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

   return terminate_processing ? 100 : (Contacted ? 0 : 5);

} /*dcpmain*/


/*--------------------------------------------------------------------*/
/*    p r o c e s s                                                   */
/*                                                                    */
/*    The procotol state machine                                      */
/*--------------------------------------------------------------------*/

static CONN_STATE process( const POLL_MODE poll_mode, const char callgrade )
{
   boolean master  = ( poll_mode == POLL_ACTIVE );
   boolean aborted = FALSE;
   XFER_STATE state =  master ? XFER_SENDINIT : XFER_RECVINIT;
   XFER_STATE old_state = XFER_EXIT;
                              /* Initialized to any state but the
                                 original value of "state"           */
   XFER_STATE save_state = XFER_EXIT;

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
            master = TRUE;
            state = XFER_NEXTJOB;
            break;

         case XFER_SLAVE:     /* Begin slave mode                    */
            master = FALSE;
            state = XFER_RECVHDR;
            break;

         case XFER_NEXTJOB:   /* Look for work in local queue        */
            state = scandir( rmtname, callgrade );
            break;

         case XFER_REQUEST:   /* Process next file in current job
                                 in queue                            */
            state = newrequest();
            break;

         case XFER_PUTFILE:   /* Got local tranmit request           */
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
            state = master ? XFER_REQUEST : XFER_RECVHDR;
            break;

         case XFER_NOLOCAL:   /* No local work, remote have any?     */
            state = sbreak();
            break;

         case XFER_NOREMOTE:  /* No remote work, local have any?     */
            state = schkdir( poll_mode == POLL_ACTIVE, callgrade );
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
            hostp->hstatus = call_failed;
            state = XFER_EXIT;
            break;

         case XFER_ABORT:     /* Internal error, flame out           */
            printmsg(0,"process: Aborting connection to %s, "
                       "previous system state = %c",
                       rmtname, save_state );
            hostp->hstatus = call_failed;
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
