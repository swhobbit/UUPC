/*--------------------------------------------------------------------*/
/*    m o d e m . c                                                   */
/*                                                                    */
/*    High level modem control routines for UUPC/extended             */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1995 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: modem.c 1.52 1994/12/09 03:45:50 ahd v1-12k $
 *
 *    Revision history:
 *    $Log: modem.c $
 *    Revision 1.52  1994/12/09 03:45:50  ahd
 *    Add more setTitle commands to better track call progress
 *    Better handling of login scripts which end with except string
 *
 *        Revision 1.51  1994/10/24  23:42:55  rommel
 *        Prevent suspend processing from hanging when file is rapidly
 *        opened and closed.
 *
 *        Revision 1.50  1994/10/23  23:29:44  ahd
 *        Better control of suspension of processing
 *
 *        Revision 1.49  1994/10/03  01:01:25  ahd
 *        Release port to any sleeping program as soon as port is closed
 *
 *        Revision 1.48  1994/08/07  21:45:09  ahd
 *        Don't report "modem ready" if really network connection
 *
 *        Revision 1.47  1994/05/08  02:43:07  ahd
 *        Handle carrier detect option internal to CD()
 *
 *        Revision 1.46  1994/04/27  00:02:15  ahd
 *        Pick one: Hot handles support, OS/2 TCP/IP support,
 *                  title bar support
 *
 * Revision 1.45  1994/03/09  04:17:41  ahd
 * Don't force interactive mode when modem is sleeping
 *
 * Revision 1.44  1994/02/20  19:11:18  ahd
 * IBM C/Set 2 Conversion, memory leak cleanup
 *
 * Revision 1.43  1994/02/19  05:14:10  ahd
 * Use standard first header
 *
 * Revision 1.42  1994/02/16  02:26:27  ahd
 * Correct delay before banner display to 2 seconds from 30
 *
 * Revision 1.41  1994/02/14  01:03:56  ahd
 * Add AnswerDelay variable to allow delay before presenting login:
 * prompt
 *
 * Revision 1.40  1994/01/01  19:20:13  ahd
 * Annual Copyright Update
 *
 * Revision 1.39  1993/12/30  03:26:21  ahd
 * Add timeout parameter for 'e' protocol
 *
 * Revision 1.38  1993/12/24  05:12:54  ahd
 * Support for checking echoing of transmitted characters
 *
 * Revision 1.37  1993/11/15  05:43:29  ahd
 * Drop BPS rate from connect messages
 *
 * Revision 1.37  1993/11/15  05:43:29  ahd
 * Drop BPS rate from connect messages
 *
 * Revision 1.36  1993/11/14  20:51:37  ahd
 * Drop modem speed from network dialing/connection messages
 * Normalize internal speed for network links to 115200 (a large number)
 *
 * Revision 1.35  1993/10/28  12:19:01  ahd
 * Cosmetic time formatting twiddles and clean ups
 *
 * Revision 1.35  1993/10/28  12:19:01  ahd
 * Cosmetic time formatting twiddles and clean ups
 *
 * Revision 1.34  1993/10/12  01:32:46  ahd
 * Normalize comments to PL/I style
 *
 * Revision 1.33  1993/10/09  22:21:55  rhg
 * ANSIfy source
 *
 * Revision 1.31  1993/10/03  22:34:33  ahd
 * Insure priority values are reset when loading modem information
 *
 * Revision 1.30  1993/10/03  20:43:08  ahd
 * Move slowWrite to script.c
 *
 * Revision 1.29  1993/10/01  01:17:44  ahd
 * Use atol() for reading port speed
 *
 * Revision 1.28  1993/09/29  13:18:06  ahd
 * Clear raise before calling script processor in shutDown
 *
 * Revision 1.28  1993/09/29  13:18:06  ahd
 * Clear raise before calling script processor in shutDown
 *
 * Revision 1.27  1993/09/29  05:25:21  ahd
 * Correct resetting of raised flag
 *
 * Revision 1.26  1993/09/29  04:49:20  ahd
 * Allow unique signal handler for suspend port processing
 *
 * Revision 1.25  1993/09/28  01:38:19  ahd
 * Add configurable timeout for conversation start up phase
 *
 * Revision 1.24  1993/09/27  04:04:06  ahd
 * Normalize references to modem speed to avoid incorrect displays
 *
 * Revision 1.23  1993/09/27  00:45:20  ahd
 * Add suspend of serial port processing from K. Rommel
 *
 * Revision 1.22  1993/09/25  03:07:56  ahd
 * Invoke set priority functions
 *
 * Revision 1.21  1993/09/23  03:26:51  ahd
 * Never try to autobaud a network connection
 *
 * Revision 1.20  1993/09/20  04:46:34  ahd
 * OS/2 2.x support (BC++ 1.0 support)
 * TCP/IP support from Dave Watt
 * 't' protocol support
 *
 * Revision 1.19  1993/08/03  03:11:49  ahd
 * Add Description= line
 *
 * Revision 1.18  1993/07/13  01:13:32  ahd
 * Correct message for systems waiting forever
 *
 * Revision 1.17  1993/07/05  14:47:05  ahd
 * Drop obsolete  tag from "variablepacket"
 * Set default timeout of 30 seconds for answer timeout
 *
 * Revision 1.16  1993/06/16  04:03:25  ahd
 * Lower max wait time for NT
 *
 * Revision 1.15  1993/05/30  15:25:50  ahd
 * Multiple driver support
 *
 * Revision 1.14  1993/05/30  00:04:53  ahd
 * Multiple communications drivers support
 *
 * Revision 1.13  1993/04/15  03:21:06  ahd
 * Add CD() call to hot login procedure
 *
 * Revision 1.12  1993/04/11  00:34:11  ahd
 * Global edits for year, TEXT, etc.
 *
 * Revision 1.11  1993/04/05  04:35:40  ahd
 * Set clear abort processing flag (norecover) from ulib.c
 *
 * Revision 1.10  1993/03/06  23:04:54  ahd
 * make modem connected messages consistent
 *
 * Revision 1.9  1993/01/23  19:08:09  ahd
 * Add additional shutDown() commands even when modem does not init
 *
 * Revision 1.8  1992/12/30  13:11:44  dmwatt
 * Check for NULL brand pointer before comparing
 *
 * Revision 1.7  1992/12/18  12:05:57  ahd
 * Flag variable packet as obsolete
 *
 * Revision 1.6  1992/11/28  19:51:16  ahd
 * Add program exit time to waiting for callin message
 * Make time parameter to callin() const
 *
 * Revision 1.5  1992/11/22  21:20:45  ahd
 * Use strpool for const string allocation
 *
 * Revision 1.4  1992/11/19  03:01:21  ahd
 * drop rcsid
 *
 * Revision 1.3  1992/11/18  03:48:24  ahd
 * Move check of call window to avoid premature lock file overhead
 *
 * Revision 1.2  1992/11/15  20:12:17  ahd
 * Clean up modem file support for different protocols
 *
 * Revision 1.1  1992/11/12  12:32:18  ahd
 * Initial revision
 *
 */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"

#include <limits.h>
#include <ctype.h>
#include <limits.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "hostable.h"
#include "catcher.h"
#include "checktim.h"
#include "commlib.h"
#include "dater.h"
#include "dcp.h"
#include "dcpsys.h"
#include "modem.h"
#include "script.h"
#include "security.h"
#include "ssleep.h"
#include "suspend.h"
#include "usrcatch.h"
#include "title.h"

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

char *M_device = NULL;          /*Public to show in login banner     */

static char **answer, **initialize, **dropline, **ring, **connect;
static char **noconnect;
static char *dialPrefix, *dialSuffix;
static char *M_suite;
static char *dummy;

static KEWSHORT dialTimeout, modemTimeout, scriptTimeout, scriptEchoTimeout;
static KEWSHORT answerTimeout;
static KEWSHORT answerDelay;
static BPS inspeed;
static KEWSHORT gWindowSize, gPacketSize;
static KEWSHORT vWindowSize, vPacketSize;
static KEWSHORT GWindowSize, GPacketSize;

KEWSHORT M_charDelay;
KEWSHORT M_fPacketSize;
KEWSHORT M_gPacketTimeout;       /* "g" procotol                  */
KEWSHORT M_ePacketTimeout;       /* "e" procotol                  */
KEWSHORT M_fPacketTimeout;       /* "f" procotol                  */
KEWSHORT M_tPacketTimeout;       /* "t" procotol                  */
KEWSHORT M_startupTimeout;       /* pre-procotol exchanges        */
KEWSHORT M_MaxErr= 10;        /* Allowed errors per single packet    */
KEWSHORT M_MaxErr;            /* Allowed errors per single packet    */
KEWLONG  M_xfer_bufsize;      /* Buffering used for file transfers */

static KEWSHORT M_priority = 999;
static KEWSHORT M_prioritydelta = 999;

boolean bmodemflag[MODEM_LAST];

static FLAGTABLE modemFlags[] = {
   { "carrierdetect",  MODEM_CARRIERDETECT,          B_LOCAL },
   { "direct",         MODEM_DIRECT,      B_LOCAL },
   { "fixedspeed",     MODEM_FIXEDSPEED,  B_LOCAL },
   { "variablepacket", MODEM_VARIABLEPACKET, B_LOCAL },
   { "largepacket",    MODEM_LARGEPACKET, B_LOCAL | B_OBSOLETE },
   { nil(char) }
}           ;

static CONFIGTABLE modemtable[] = {
   { "answer",        (char **) &answer,       B_LIST   | B_UUCICO },
   { "answerdelay",   (char **) &answerDelay,  B_SHORT  | B_UUCICO },
   { "answertimeout", (char **) &answerTimeout,B_SHORT  | B_UUCICO },
   { "biggpacketsize",(char **) &GPacketSize,  B_SHORT  | B_UUCICO },
   { "biggwindowsize",(char **) &GWindowSize,  B_SHORT  | B_UUCICO },
   { "chardelay",     (char **) &M_charDelay,  B_SHORT  | B_UUCICO },
   { "connect",       (char **) &connect,      B_LIST   | B_UUCICO },
   { "description",   &dummy,                  B_TOKEN  },
   { "device",        &M_device,               B_TOKEN  | B_UUCICO | B_REQUIRED },
   { "dialprefix",    &dialPrefix,             B_STRING | B_UUCICO | B_REQUIRED },
   { "dialsuffix",    &dialSuffix,             B_STRING | B_UUCICO },
   { "dialtimeout",   (char **) &dialTimeout,  B_SHORT  | B_UUCICO },
   { "epackettimeout",(char **) &M_ePacketTimeout, B_SHORT | B_UUCICO },
   { "fpacketsize",   (char **) &M_fPacketSize,B_SHORT  | B_UUCICO },
   { "fpackettimeout",(char **) &M_fPacketTimeout, B_SHORT | B_UUCICO },
   { "gpacketsize",   (char **) &gPacketSize,  B_SHORT  | B_UUCICO },
   { "gpackettimeout",(char **) &M_gPacketTimeout, B_SHORT | B_UUCICO },
   { "gwindowsize",   (char **) &gWindowSize,  B_SHORT  | B_UUCICO },
   { "hangup",        (char **) &dropline,     B_LIST   | B_UUCICO },
   { "initialize",    (char **) &initialize,   B_LIST   | B_UUCICO },
   { "inspeed",       (char **) &inspeed,      B_LONG   | B_UUCICO },
   { "maximumerrors", (char **) &M_MaxErr,     B_SHORT  | B_UUCICO },
   { "modemtimeout",  (char **) &modemTimeout, B_SHORT  | B_UUCICO },
   { "noconnect",     (char **) &noconnect,    B_LIST   | B_UUCICO },
   { "options",       (char **) bmodemflag,    B_ALL    | B_BOOLEAN},
   { "porttimeout",   NULL,                    B_OBSOLETE },
   { "priority",      (char **) &M_priority,    B_SHORT | B_UUCICO},
   { "prioritydelta", (char **) &M_prioritydelta,B_SHORT |B_UUCICO},
   { "ring",          (char **) &ring,         B_LIST   | B_UUCICO },
   { "scripttimeout", (char **) &scriptTimeout,B_SHORT  | B_UUCICO },
   { "scriptechotimeout", (char **) &scriptEchoTimeout,B_SHORT| B_UUCICO },
   { "startuptimeout",(char **) &M_startupTimeout, B_SHORT | B_UUCICO },
   { "suite",         &M_suite,                B_TOKEN  | B_UUCICO },
   { "transferbuffer",(char **) &M_xfer_bufsize, B_LONG| B_UUCICO },
   { "tpackettimeout",(char **) &M_tPacketTimeout, B_SHORT | B_UUCICO },
   { "version",       &dummy,                  B_TOKEN  },
   { "vpacketsize",   (char **) &vPacketSize,  B_SHORT  | B_UUCICO },
   { "vwindowsize",   (char **) &vWindowSize,  B_SHORT  | B_UUCICO },
   { nil(char) }

}; /* modemtable */

static boolean reEnable = FALSE;

/*--------------------------------------------------------------------*/
/*                    Internal function prototypes                    */
/*--------------------------------------------------------------------*/

static boolean dial(char *number, const BPS speed);

static boolean sendlist( char **list, int timeout, int lasttimeout,
                         char **failure);

static boolean sendalt( char *string, int timeout, char **failure);

static void autobaud( const BPS speed);

/*--------------------------------------------------------------------*/
/*              Define current file name for references               */
/*--------------------------------------------------------------------*/

currentfile();

/*--------------------------------------------------------------------*/
/*    c a l l u p                                                     */
/*                                                                    */
/*    script processor - nothing fancy!                               */
/*--------------------------------------------------------------------*/

CONN_STATE callup( void )
{
   char *exp;
   int i;
   BPS speed = 115200;              /* Bogus network speed default   */
   time_t now;

/*--------------------------------------------------------------------*/
/*             Announce we are trying to call the system              */
/*--------------------------------------------------------------------*/

   time( &now );
   if ( IsNetwork() )
      printmsg(1,"callup: Connecting to %s via %s on %.24s",
                rmtname,
                flds[FLD_TYPE],
                ctime( &now ));
   else {
      printmsg(1, "callup: Calling %s via %s at %s on %.24s",
          rmtname,
          flds[FLD_TYPE],
          flds[FLD_SPEED],
          ctime( &now ));

      speed = (BPS) atol( flds[FLD_SPEED] );

      if (speed < 300)
      {
         printmsg(0,"callup: Modem speed %s is invalid.",
                     flds[FLD_SPEED]);
         hostp->status.hstatus = invalid_device;
         return CONN_INITIALIZE;

      } /* if (speed < 300) */

   } /* else */

/*--------------------------------------------------------------------*/
/*                         Dial the telephone                         */
/*--------------------------------------------------------------------*/

   if (! dial(flds[FLD_PHONE], speed))
      return CONN_DROPLINE;

/*--------------------------------------------------------------------*/
/*             The modem is connected; now login the host             */
/*--------------------------------------------------------------------*/

   setTitle("Logging in to %s", rmtname );

   i = FLD_EXPECT;

   while (i < kflds )
   {

      exp = flds[i];
      printmsg(2, "expecting %d of %d \"%s\"",
               i,
               kflds,
               exp);

      if (!sendalt( exp, scriptTimeout , noconnect))
      {
         printmsg(0, "SCRIPT FAILED");
         hostp->status.hstatus =  script_failed;
         return CONN_DROPLINE;
      } /* if */

      if ( ++i < kflds )
      {

         printmsg(2, "callup: sending %d of %d \"%s\"",
                      i,
                      kflds,
                      flds[i]);

         if (!sendstr(flds[i++], scriptEchoTimeout, noconnect ))
            return CONN_DROPLINE;

      } /* if ( ++i < kflds ) */

   } /* while (i < kflds ) */

   return CONN_PROTOCOL;

} /*callup*/

/*--------------------------------------------------------------------*/
/*    c a l l h o t                                                   */
/*                                                                    */
/*    Initialize processing when phone is already off the hook        */
/*--------------------------------------------------------------------*/

CONN_STATE callhot( const BPS xspeed, const int hotHandle )
{
   BPS speed;

/*--------------------------------------------------------------------*/
/*                        Open the serial port                        */
/*--------------------------------------------------------------------*/

   if (E_inmodem == NULL)
   {
      printmsg(0,"callin: No modem name supplied for incoming calls!");
      panic();
   } /* if */

/*--------------------------------------------------------------------*/
/*                        Set the modem speed                         */
/*--------------------------------------------------------------------*/

   if (( xspeed == 0 ) && ( hotHandle == -1 ))
      speed = inspeed;
   else
      speed = xspeed;

/*--------------------------------------------------------------------*/
/*                    Open the communications port                    */
/*--------------------------------------------------------------------*/

   norecovery = FALSE;           /* Shutdown gracefully as needed     */
   reEnable   = FALSE;           /* Don't reenable port, we own it!   */

   if ( hotHandle != -1 )
      SetComHandle( hotHandle );

   if (passiveopenline(M_device, speed, bmodemflag[MODEM_DIRECT] ))
      panic();

/*--------------------------------------------------------------------*/
/*                          Initialize stats                          */
/*--------------------------------------------------------------------*/

   memset( &remote_stats, 0, sizeof remote_stats);
                              /* Clear remote stats for login        */
   time(&remote_stats.ltime); /* Remember time of last attempt conn  */
   remote_stats.calls ++ ;

   CD();                      /* Set the carrier detect flags        */

   return CONN_HOTLOGIN;

} /* callhot */

/*--------------------------------------------------------------------*/
/*    c a l l i n                                                     */
/*                                                                    */
/*    Answer the modem in passive mode                                */
/*--------------------------------------------------------------------*/

CONN_STATE callin( const time_t exit_time )
{
   char c;                    /* A character for input buffer        */

   int    offset;             /* Time to wait for telephone          */
   time_t now, left;

/*--------------------------------------------------------------------*/
/*    Determine how long we can wait for the telephone, up to         */
/*    SHRT_MAX seconds.  Aside from Turbo C limits, this insures we   */
/*    kick the modem once in a while.                                 */
/*--------------------------------------------------------------------*/

      if ((now = time(NULL)) > exit_time) /* Any time left?          */
         return CONN_EXIT;             /* No --> shutdown            */

      if ( (left = exit_time - now) > SHRT_MAX)
         offset = SHRT_MAX;
      else
         offset = (int) left;

/*--------------------------------------------------------------------*/
/*                        Open the serial port                        */
/*--------------------------------------------------------------------*/

   if (E_inmodem == NULL)
   {
      printmsg(0,"callin: No modem name supplied for incoming calls!");
      panic();
   } /* if */

/*--------------------------------------------------------------------*/
/*                    Open the communications port                    */
/*--------------------------------------------------------------------*/

   norecovery = FALSE;           /* Shutdown gracefully as needed    */
   reEnable   = FALSE;           /* Don't reenable port, we own it!   */

   echoCheck( 0 );               /* Disable echo checking            */

/*--------------------------------------------------------------------*/
/*              Flush the input buffer of any characters              */
/*--------------------------------------------------------------------*/

   if ( IsNetwork() )
   {
      if (passiveopenline(M_device, inspeed, bmodemflag[MODEM_DIRECT]))
         panic();
   }
   else {
      if (((ring == NULL) || (inspeed == 0)))
      {
         printmsg(0,"callin: Missing inspeed and/or ring values in modem "
                    "configuration file.");
         panic();
      } /* if */

      if (passiveopenline(M_device, inspeed, bmodemflag[MODEM_DIRECT]))
         panic();

      while (sread(&c ,1,0)); /* Discard trailing trash from modem
                                 connect message                     */

/*--------------------------------------------------------------------*/
/*                        Initialize the modem                        */
/*--------------------------------------------------------------------*/

      if (!sendlist( initialize, modemTimeout, modemTimeout, NULL))
      {
         printmsg(0,"callin: Modem failed to initialize");
         shutDown();
         panic();
      }

   } /* else */

/*--------------------------------------------------------------------*/
/*                   Wait for the telephone to ring                   */
/*--------------------------------------------------------------------*/

   suspend_ready();

   printmsg(1,"Monitoring port %s device %s"
                     " for %d minutes until %s",
                     M_device, E_inmodem , (int) (offset / 60),
                     (left > hhmm2sec(10000)) ?
                              "user hits Ctrl-Break" :
                              dater( exit_time , NULL));

   interactive_processing = FALSE;

   if (IsNetwork())
   {                          /* Network connect is different        */
      if (!WaitForNetConnect(offset))
      {
         interactive_processing = TRUE;

         shutDown();

         if ( suspend_processing )        /* Give up modem for another process?  */
         {
           return CONN_WAIT;
         }
         return CONN_INITIALIZE;
      }

      interactive_processing = TRUE;
      printmsg(14, "callin: Network reports connected");

   }
   else {
      if (!sendlist( ring,modemTimeout, offset, noconnect))
      {                          /* Did it ring?                       */
         raised = 0;
         shutDown();
         if ( suspend_processing )        /* Give up modem for another process?  */
            return CONN_WAIT;
         else
            return CONN_INITIALIZE;     /* No --> Return to caller    */
      }

      interactive_processing = TRUE;

      setPrty(M_priority, M_prioritydelta );
                              /* Into warp drive for actual transfers  */

      setTitle("Answering port %s", M_device);

      if(!sendlist(answer, modemTimeout,answerTimeout, noconnect))
      {                           /* Pick up the telephone            */
         printmsg(1,"callin: Modem failed to connect to incoming call");
         shutDown();
         return CONN_INITIALIZE;
      }

      printmsg(14, "callin: Modem reports connected");

      autobaud(inspeed);      /* autobaud the modem                  */

/*--------------------------------------------------------------------*/
/*        Flush the input buffer of any other input characters        */
/*--------------------------------------------------------------------*/

      while (sread(&c ,1,0)); /* Discard trailing trash from modem
                                 connect message                     */

      ssleep( answerDelay );  /* Delay before presenting prompt, if
                                 if needed                           */

   } /* else */

   memset( &remote_stats, 0, sizeof remote_stats);
                              /* Clear remote stats for login        */

   time(&remote_stats.ltime); /* Remember time of last attempt conn  */
   remote_stats.calls ++ ;
   return CONN_LOGIN;

} /* callin */

/*--------------------------------------------------------------------*/
/*    g e t m o d e m                                                 */
/*                                                                    */
/*    Read a modem configuration file                                 */
/*--------------------------------------------------------------------*/

boolean getmodem( const char *brand)
{
   char filename[FILENAME_MAX];
   static char *modem = NULL;
   FILE *fp;
   CONFIGTABLE *tptr;
   size_t subscript;
   boolean success;

/*--------------------------------------------------------------------*/
/*                      Validate the modem name                       */
/*   Added check for validity of modem ptr -- makes NT happier - dmw  */
/*--------------------------------------------------------------------*/

   if ((modem != NULL) && equal(modem, brand)) /* Already initialized?*/
      return TRUE;            /* Yes --> Don't process it again      */

/*--------------------------------------------------------------------*/
/*                        Initialize the table                        */
/*--------------------------------------------------------------------*/

   for (tptr = modemtable; tptr->sym != nil(char); tptr++)
      if (tptr->bits & (B_TOKEN | B_STRING | B_LIST | B_CLIST))
         *(tptr->loc) = nil(char);

   for (subscript = 0; subscript < MODEM_LAST; subscript++)
      bmodemflag[subscript] = FALSE;

   M_charDelay = 00;          /* Default is no delay between chars    */
   dialTimeout = 40;          /* Default is 40 seconds to dial phone  */
   gPacketSize = SMALL_PACKET;
   vPacketSize = MAXPACK;
   GPacketSize = MAXPACK;
   gWindowSize = 0;
   vWindowSize = 0;
   GWindowSize = 0;
   M_fPacketSize = MAXPACK;
   M_fPacketTimeout = 20;
   M_gPacketTimeout = 10;
   M_ePacketTimeout = 60;
   M_tPacketTimeout = 60;
   modemTimeout  = 3;         /* Default is 3 seconds for modem cmds  */
   scriptTimeout = 30;        /* Default is 30 seconds for script data*/
   scriptEchoTimeout = 5;     /* Default is 5 seconds for script echo */
   answerTimeout = 30;        /* Default is 30 seconds to answer phone*/
   answerDelay   = 2;         /* No default delay before presenting
                                 login prompt to remote system        */
   M_xfer_bufsize = BUFSIZ;   /* Buffering used for file transfers    */
   M_MaxErr= 10;              /* Allowed errors per single packet     */
   M_suite = NULL;            /* Use default suite for communications */
   M_startupTimeout = 40;     /* 40 seconds per message to exchange protocols  */

   M_priority = 999;
   M_prioritydelta = 999;

/*--------------------------------------------------------------------*/
/*                 Open the modem configuration file                  */
/*--------------------------------------------------------------------*/

   if (equaln(brand,"COM",3))
   {
      printmsg(0,"Modem type %s is invalid; Snuffles suspects "
               "your %s file is obsolete.", brand, E_systems);
      panic();
   }

   sprintf(filename,"%s/%s.MDM",E_confdir, brand);
   if ((fp = FOPEN(filename, "r",TEXT_MODE)) == nil(FILE))
   {
      printmsg(0,"getmodem: Unable to locate configuration for %s",
               brand);
      printerr( filename );
      return FALSE;
   }

/*--------------------------------------------------------------------*/
/*                We got the file open, now process it                */
/*--------------------------------------------------------------------*/

   printmsg(3,"getmodem: loading modem configuration file %s", filename);
   success = getconfig(fp, MODEM_CONFIG, B_UUCICO, modemtable, modemFlags);
   fclose(fp);
   if (!success)
      return FALSE;

/*--------------------------------------------------------------------*/
/*         Verify all required modem parameters were supplied         */
/*--------------------------------------------------------------------*/

   success = TRUE;
   for (tptr = modemtable; tptr->sym != nil(char); tptr++) {
      if ((tptr->bits & (B_REQUIRED | B_FOUND)) == B_REQUIRED)
      {
         printmsg(0, "getmodem: configuration parameter \"%s\" must be set.",
            tptr->sym);
         success = FALSE;
      } /* if */
   } /* for */

   if ( ! success )           /* Missing any required inputs?         */
      return success;         /* Yes --> Return failure to caller     */

/*--------------------------------------------------------------------*/
/*       The strings are valid, try to initialize the pointers to     */
/*       the processing routines.                                     */
/*--------------------------------------------------------------------*/

   if ( ! chooseCommunications( M_suite ))
      return FALSE;

/*--------------------------------------------------------------------*/
/*       We have success, save modem name for next time to speed      */
/*       initialize.                                                  */
/*--------------------------------------------------------------------*/

   modem = newstr(brand);  /* Yes --> Remember it for next time   */

   return TRUE;

} /* getmodem */

/*--------------------------------------------------------------------*/
/*    d i a l                                                         */
/*                                                                    */
/*    Generic modem dialer; only major limitation is that autoabaud   */
/*    strings are not configurable                                    */
/*--------------------------------------------------------------------*/

static boolean dial(char *number, const BPS speed)
{
   char buf[81];

/*--------------------------------------------------------------------*/
/*                        Open the serial port                        */
/*--------------------------------------------------------------------*/

   norecovery = FALSE;           /* Shutdown gracefully as needed     */
   reEnable   = TRUE;            /* Automatically reenable port       */

   echoCheck( 0 );               /* Disable echo checking            */

/*--------------------------------------------------------------------*/
/*              Flush the input buffer of any characters              */
/*--------------------------------------------------------------------*/

   if ( IsNetwork() )
   {
      if (activeopenline(number, speed, bmodemflag[MODEM_DIRECT]))
      {
         hostp->status.hstatus =  nodevice;
         return FALSE;
      }
   }
   else {

      if (activeopenline(M_device, speed, bmodemflag[MODEM_DIRECT]))
      {

         hostp->status.hstatus =  nodevice;
         return FALSE;
      }

      while (sread(buf,1,0)); /* Discard trailing trash from modem
                                 connect message                     */

/*--------------------------------------------------------------------*/
/*                        Initialize the modem                        */
/*--------------------------------------------------------------------*/

      if (!sendlist( initialize, modemTimeout, modemTimeout, noconnect))
      {
         printmsg(0,"dial: Modem failed to initialize");
         shutDown();
         hostp->status.hstatus =  dial_script_failed;
         return FALSE;
      }

/*--------------------------------------------------------------------*/
/*           Setup the dial string and then dial the modem            */
/*--------------------------------------------------------------------*/

      strcpy(buf, dialPrefix);
      strcat(buf, number);
      if (dialSuffix != NULL)
         strcat(buf, dialSuffix);

      if (!sendstr( buf, modemTimeout, noconnect ))
         return FALSE;
                              /* Send the dial command to the modem  */

      if (!sendlist(connect,  modemTimeout, dialTimeout, noconnect))
      {
         hostp->status.hstatus =  dial_failed;
         return FALSE;
      }

   }  /* if ( !IsNetwork() ) */

   printmsg(3, "dial: %s reports connected",
               IsNetwork() ? "Network" : "Modem" );

   time( &remote_stats.lconnect );
   remote_stats.calls ++ ;

   if ( !IsNetwork() )
      autobaud(speed);     /* Reset modem speed, if desired          */

   setPrty(M_priority, M_prioritydelta );

/*--------------------------------------------------------------------*/
/*                      Report success to caller                      */
/*--------------------------------------------------------------------*/

   return TRUE;            /* Dial succeeded    */

} /* dial */

/*--------------------------------------------------------------------*/
/*    a u t o b a u d                                                 */
/*                                                                    */
/*    autobaud a modem which has just connected                       */
/*--------------------------------------------------------------------*/

static void autobaud( const BPS speed )
{
   char buf[10];

   ssleep(1);                 /*  Allow modem port to stablize       */

   CD();                      /* Set the carrier detect flags        */

/*--------------------------------------------------------------------*/
/*                  Autobaud the modem if requested                   */
/*--------------------------------------------------------------------*/

   if (!bmodemflag[MODEM_FIXEDSPEED])
   {
      size_t len = 0;

      memset( buf, '\0', sizeof( buf ));  /* Zero buffer               */
      while ((len < sizeof buf) && sread( &buf[len],1,0))
         len = strlen( buf );             /* Get speed into buffer     */

      if (len > 5)
      {
         char  *token;           /* Pointer to buffer value */
         token = strtok(buf,WHITESPACE);
         if (strlen(token))
         {
            BPS new_speed = (unsigned) atol(token);
            if ((new_speed != speed) && (new_speed > 300))
            {
               printmsg(2, "autobaud: speed select %s", token);
               SIOSpeed(new_speed);
            } /* if */
         } /* if */
      } /* if */
      else
         printmsg(3, "autobaud: unable to speed select, using %d", speed);
   } /* if */

} /* autobaud */

/*--------------------------------------------------------------------*/
/*    s h u t d o w n                                                 */
/*                                                                    */
/*    Terminate modem processing via hangup                           */
/*--------------------------------------------------------------------*/

void shutDown( void )
{
   static boolean recurse = FALSE;

   echoCheck( 0 );

   if ( ! portActive )          /* Allowed for Ctrl-Break           */
      return;

   if ( !recurse )
   {
      boolean aborted = terminate_processing;
      unsigned long saveRaised = raised;
      recurse = TRUE;
      terminate_processing = FALSE;
      raised = 0;
      hangup();
      resetPrty();               /* Drop out of hyperspace            */
      sendlist( dropline, modemTimeout, modemTimeout, NULL);
      recurse = FALSE;
      terminate_processing |= aborted;
      saveRaised |= raised;
   }

/*--------------------------------------------------------------------*/
/*                          Release the port                          */
/*--------------------------------------------------------------------*/

   closeline();

/*--------------------------------------------------------------------*/
/*              Give port back to original owner, if any              */
/*--------------------------------------------------------------------*/

   if (!IsNetwork() && reEnable )
      suspend_other(FALSE, M_device);

   norecovery = TRUE;

}  /* shutDown */

/*--------------------------------------------------------------------*/
/*    s e n d l i s t                                                 */
/*                                                                    */
/*    Send a NULL terminated list of send/expect strings              */
/*--------------------------------------------------------------------*/

static boolean sendlist(   char **list,
                           int timeout,
                           int lasttimeout,
                           char **failure)
{
   boolean expect = TRUE;

   if (list == NULL)          /* Was the field supplied?             */
      return TRUE;            /* No --> Must be optional, return     */

/*--------------------------------------------------------------------*/
/*     Run through the list, alternating expect and send strings      */
/*--------------------------------------------------------------------*/

   while( *list != NULL)
   {

      if (expect)
      {
         char *exp = strdup( *list );
         boolean success;
         checkref( exp );
         success = sendalt( exp,
                            (*(++list) == NULL) ? lasttimeout : timeout,
                            failure);
         free( exp );
         if (!success)
            return FALSE;
      }
      else
         if (!sendstr( *list++, timeout, failure ))
            return FALSE;

      expect = ! expect;

   } /* while */

/*--------------------------------------------------------------------*/
/*    If we made it this far, success is at hand; return to caller    */
/*--------------------------------------------------------------------*/

   return TRUE;

} /* sendlist */

/*--------------------------------------------------------------------*/
/*    s e n d a l t                                                   */
/*                                                                    */
/*    Expect a string, with alternates                                */
/*--------------------------------------------------------------------*/

static boolean sendalt( char *exp, int timeout, char **failure)
{
   int ok;

   for ( ;; )
   {
      char *alternate = strchr(exp, '-');

      if (alternate != nil(char))
         *alternate++ = '\0';

      ok = expectstr(exp, timeout, failure);

      if ( terminate_processing || raised )
      {
         shutDown();
         return FALSE;
      }

      if (ok || (alternate == nil(char)))
         return (ok == 1);

      if (!CD())
      {
         printmsg(0,"sendalt: Serial port reports modem not ready");
         return FALSE;
      }

      exp = strchr(alternate, '-');
      if (exp != nil(char))
         *exp++ = '\0';

      printmsg(0, "sending alternate");

      if ( !sendstr(alternate,timeout, failure) )
         return FALSE;

   } /*for*/

} /* sendalt */

/*--------------------------------------------------------------------*/
/*    G e t G W i n d o w                                             */
/*                                                                    */
/*    Report the size of the allowed window for the "g" protocol      */
/*--------------------------------------------------------------------*/

KEWSHORT GetGWindow(  KEWSHORT maxvalue , const char protocol )
{
   KEWSHORT ourWindowSize = 0;

   switch( protocol )
   {
      case 'g':
         ourWindowSize = gWindowSize;
         break;

      case 'G':
         ourWindowSize = GWindowSize;
         break;

      case 'v':
         ourWindowSize = vWindowSize;
         break;

      default:
         printmsg(0,"GetGWindow: Invalid protocol %c",protocol);
         panic();
   }

   if ( (ourWindowSize < 1 ) || (ourWindowSize > maxvalue))
      return maxvalue;
   else
      return ourWindowSize;

} /* GetGWindow */

/*--------------------------------------------------------------------*/
/*    G e t G P a c k e t                                             */
/*                                                                    */
/*    Return the allowed packet size for the "g" procotol             */
/*--------------------------------------------------------------------*/

KEWSHORT GetGPacket( KEWSHORT maxvalue , const char protocol)
{
   KEWSHORT savePacketSize ;
   KEWSHORT ourPacketSize = 0;
   int bits = 6;              /* Minimum Packet Size is 64 bytes     */

   switch( protocol )
   {
      case 'g':
         ourPacketSize = gPacketSize;
         break;

      case 'G':
         ourPacketSize = GPacketSize;
         break;

      case 'v':
         ourPacketSize = vPacketSize;
         break;

      default:
         printmsg(0,"GetGPacket: Invalid protocol %c",protocol);
         panic();
   }

   savePacketSize = ourPacketSize;

/*--------------------------------------------------------------------*/
/*                 Insure the value is a power of two                 */
/*--------------------------------------------------------------------*/

   while( (ourPacketSize >> (bits+1)) > 0 )
      bits++;

   ourPacketSize = (KEWSHORT) ((ourPacketSize >> bits) << bits);
   if ( savePacketSize != ourPacketSize )
      printmsg(0,"packetsize for %c protocol rounded down from %d to %d",
               protocol,
               (int) savePacketSize, (int) ourPacketSize );

/*--------------------------------------------------------------------*/
/*    Return the smaller of the argument (the largest packet size     */
/*    the packet driver supports) or what the modem file allows.      */
/*--------------------------------------------------------------------*/

   if ( (ourPacketSize < 1 ) || (ourPacketSize > maxvalue))
      return maxvalue;
   else
      return ourPacketSize;

} /* GetGPacket */
