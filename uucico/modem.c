/*--------------------------------------------------------------------*/
/*    m o d e m . c                                                   */
/*                                                                    */
/*    High level modem control routines for UUPC/extended             */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1999 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: modem.c 1.74 1998/11/24 13:49:41 ahd v1-13f ahd $
 *
 *    Revision history:
 *    $Log: modem.c $
 *    Revision 1.74  1998/11/24 13:49:41  ahd
 *    Disable optimization under VC++ 5.0 for modem packet size computation;
 *    panic if code loops excessively anyway.
 *
 *    Revision 1.73  1998/04/27 01:55:28  ahd
 *    Allow defaulting selected options to enabled
 *
 *    Revision 1.72  1998/04/20 02:47:57  ahd
 *    TAPI/Windows 32 BIT GUI display support
 *
 *    Revision 1.71  1998/04/19 23:55:58  ahd
 *    *** empty log message ***
 *
 *    Revision 1.70  1998/03/01 01:39:53  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.69  1997/06/03 03:25:31  ahd
 *    First compiling SMTPD
 *
 *    Revision 1.68  1997/05/11 04:28:26  ahd
 *    SMTP client support for RMAIL/UUXQT
 *
 *    Revision 1.67  1997/04/24 01:34:23  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.66  1996/11/18 04:46:49  ahd
 *    Normalize arguments to bugout
 *    Reset title after exec of sub-modules
 *    Normalize host status names to use HS_ prefix
 *
 *    Revision 1.65  1996/03/18 03:52:46  ahd
 *    Don't suspend/resume network ports
 *
 *    Revision 1.64  1996/01/04 04:00:46  ahd
 *    Use sorted list of boolean options with binary search and computed
 *    table size.
 *
 *    Revision 1.63  1996/01/02 02:51:53  ahd
 *    Sort security, modem configuration tables
 *
 *    Revision 1.62  1996/01/02 00:00:24  ahd
 *    Break out search loop for configuration file keywords from
 *    processing of them.
 *    Use proper binary search for configuration file keywords rather
 *    than lineaer search.  Also includes pre-computing size of configuration
 *    tables.
 *
 *    Revision 1.61  1995/07/21 13:27:00  ahd
 *    If modem is unable to dial, be sure to resume suspended UUCICO if needed
 *
 *    Revision 1.60  1995/02/26 02:51:34  ahd
 *    Reduce default packet size to 512 bytes - 1024 is 3 mile freight train
 *
 *    Revision 1.59  1995/02/25 18:21:44  ahd
 *    Prevent UUCICO from looping without time limit
 *
 *    Revision 1.58  1995/02/21 03:30:52  ahd
 *    More compiler warning cleanup, drop selected messages at compile
 *    time if not debugging.
 *
 *    Revision 1.57  1995/02/17 23:54:56  ahd
 *    Terminate processing on a minute boundary
 *
 *    Revision 1.56  1995/02/12 23:37:04  ahd
 *    compiler cleanup, NNS C/news support, optimize dir processing
 *
 *    Revision 1.55  1995/01/29 14:07:59  ahd
 *    Clean up most IBM C/Set Compiler Warnings
 *
 *    Revision 1.54  1995/01/07 16:39:06  ahd
 *    Change boolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.53  1994/12/22 00:35:37  ahd
 *    Annual Copyright Update
 *
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

#ifdef TAPI_SUPPORT
#include "uutapi.h"
#endif

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

char *M_device = NULL;          /*Public to show in login banner     */

static char **answer, **initialize, **dropline, **ring, **connect;
static char **noconnect;
static char *dialPrefix, *dialSuffix;
static char *M_suite;

static KEWSHORT dialTimeout, modemTimeout, scriptTimeout, scriptEchoTimeout;
static KEWSHORT answerTimeout;
static KEWSHORT answerDelay;
static BPS inspeed;
static KEWSHORT gWindowSize, gPacketSize;
static KEWSHORT vWindowSize, vPacketSize;
static KEWSHORT GWindowSize, GPacketSize;

KEWSHORT M_charDelay;
KEWSHORT M_portNumber;           /* TCP/IP only                   */
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

KWBoolean bmodemflag[MODEM_LAST];

static FLAGTABLE modemFlags[] = {
   { "carrierdetect",  MODEM_CARRIERDETECT,  B_LOCAL },
   { "direct",         MODEM_DIRECT,         B_LOCAL },
   { "fixedspeed",     MODEM_FIXEDSPEED,     B_LOCAL },
   { "variablepacket", MODEM_VARIABLEPACKET, B_LOCAL }
};

static size_t modemFlagsSize = sizeof modemFlags / (sizeof (FLAGTABLE));

static CONFIGTABLE modemTable[] = {
   { "answer",         &answer,           0, B_LIST   },
   { "answerdelay",    &answerDelay,      0, B_SHORT  },
   { "answertimeout",  &answerTimeout,    0, B_SHORT  },
   { "biggpacketsize", &GPacketSize,      0, B_SHORT  },
   { "biggwindowsize", &GWindowSize,      0, B_SHORT  },
   { "chardelay",      &M_charDelay,      0, B_SHORT  },
   { "connect",        &connect,          0, B_LIST   },
   { "description",    0,                 0, B_TOKEN  },
   { "device",         &M_device,         0, B_TOKEN  | B_REQUIRED },
   { "dialprefix",     &dialPrefix,       0, B_STRING | B_REQUIRED },
   { "dialsuffix",     &dialSuffix,       0, B_STRING },
   { "dialtimeout",    &dialTimeout,      0, B_SHORT  },
   { "epackettimeout", &M_ePacketTimeout, 0, B_SHORT  },
   { "fpacketsize",    &M_fPacketSize,    0, B_SHORT  },
   { "fpackettimeout", &M_fPacketTimeout, 0, B_SHORT  },
   { "gpacketsize",    &gPacketSize,      0, B_SHORT  },
   { "gpackettimeout", &M_gPacketTimeout, 0, B_SHORT  },
   { "gwindowsize",    &gWindowSize,      0, B_SHORT  },
   { "hangup",         &dropline,         0, B_LIST   },
   { "initialize",     &initialize,       0, B_LIST   },
   { "inspeed",        &inspeed,          0, B_LONG   },
   { "maximumerrors",  &M_MaxErr,         0, B_SHORT  },
   { "modemtimeout",   &modemTimeout,     0, B_SHORT  },
   { "noconnect",      &noconnect,        0, B_LIST   },
   { "options",        bmodemflag,        0, B_BOOLEAN},
   { "porttimeout",    0,                 0, B_OBSOLETE },
   { "portnumber",     &M_portNumber,     0, B_SHORT  },
   { "priority",       &M_priority,       0, B_SHORT  },
   { "prioritydelta",  &M_prioritydelta,  0, B_SHORT  },
   { "ring",           &ring,             0, B_LIST   },
   { "scriptechotimeout",  &scriptEchoTimeout,0, B_SHORT },
   { "scripttimeout",  &scriptTimeout,    0, B_SHORT  },
   { "startuptimeout", &M_startupTimeout, 0, B_SHORT  },
   { "suite",          &M_suite,          0, B_TOKEN  },
   { "tpackettimeout", &M_tPacketTimeout, 0, B_SHORT  },
   { "transferbuffer", &M_xfer_bufsize,   0, B_LONG   },
   { "version",        0,                 0, B_TOKEN  },
   { "vpacketsize",    &vPacketSize,      0, B_SHORT  },
   { "vwindowsize",    &vWindowSize,      0, B_SHORT  }
}; /* modemTable */

static size_t modemTableSize = sizeof modemTable / (sizeof (CONFIGTABLE));

static KWBoolean reEnable = KWFalse;

/*--------------------------------------------------------------------*/
/*                    Internal function prototypes                    */
/*--------------------------------------------------------------------*/

static KWBoolean dial(char *number, const BPS speed);

static KWBoolean sendlist( char **list,
                           unsigned int timeout,
                           unsigned int lasttimeout,
                           char **failure);

static KWBoolean sendalt( char *string,
                          unsigned int timeout,
                          char **failure);

static void autobaud( const BPS speed);



#ifdef TAPI_SUPPORT
static CONN_STATE answerTAPI(time_t offset);
#endif

/*--------------------------------------------------------------------*/
/*              Define current file name for references               */
/*--------------------------------------------------------------------*/

currentfile();
RCSID("$Id: modem.c 1.74 1998/11/24 13:49:41 ahd v1-13f ahd $");

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
         hostp->status.hstatus = HS_INVALID_DEVICE;
         return CONN_INITIALIZE;

      } /* if (speed < 300) */

   } /* else */

/*--------------------------------------------------------------------*/
/*                         Dial the telephone                         */
/*--------------------------------------------------------------------*/

   if (! dial(flds[FLD_PHONE], speed))
   {
      if ( ! portActive )           /* Will shutdown() close line?   */
         suspend_other(KWFalse, M_device );  /* No--> Resume modem   */

      return CONN_DROPLINE;
   }

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
         hostp->status.hstatus = HS_SCRIPT_FAILED;
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

   norecovery = KWFalse;          /* Shutdown gracefully as needed     */
   reEnable   = KWFalse;          /* Don't reenable port, we own it!   */

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
   char *until;

   unsigned int    offset;    /* Time to wait for telephone          */
   time_t now = time(NULL);
   time_t left;

   if (now >= exit_time)            /* Any time left?                */
      return CONN_EXIT;             /* No --> shutdown               */

/*--------------------------------------------------------------------*/
/*                Round the time up to the next minute                */
/*--------------------------------------------------------------------*/

   left = exit_time - now;

   if ( (left+60) < SHRT_MAX )
   {
      struct tm  *time_record;
      time_t stop_time = exit_time + 59;

      time_record = localtime(&stop_time);
      time_record->tm_sec = 0;
      stop_time = mktime(time_record);

      until = dater( stop_time , NULL);
      left = stop_time - now;
      offset = (unsigned int) left;
   }
   else {
      until = "user hits Ctrl-Break";
      offset = SHRT_MAX;
   }


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

   norecovery = KWFalse;            /* Shutdown gracefully as needed  */
   reEnable   = KWFalse;            /* Don't reenable port, we own it!*/

   echoCheck( 0 );                  /* Disable echo checking         */

/*--------------------------------------------------------------------*/
/*              Flush the input buffer of any characters              */
/*--------------------------------------------------------------------*/

   if ( IsNetwork() )
   {
      if (passiveopenline(M_device, M_portNumber, KWFalse ))
         panic();
   }
   else {

      if (((ring == NULL) || (inspeed == 0)))
      {
         printmsg(0,"callin: Missing inspeed and/or ring values in modem "
                    "configuration file.");
         panic();
      } /* if */

      if (passiveopenline(M_device,
                          IsNetwork() ? M_portNumber : inspeed,
                          IsNetwork() ? KWFalse : bmodemflag[MODEM_DIRECT]))
         panic();


      if (!IsTAPI())
      {
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

      } /* if (!IsTAPI()) */

      suspend_ready();

   } /* else */

/*--------------------------------------------------------------------*/
/*                   Wait for the telephone to ring                   */
/*--------------------------------------------------------------------*/

   printmsg(1,"Monitoring port %s device %s"
                     " for %d minutes until %s",
                     M_device,
                     E_inmodem ,
                     (int) (offset / 60),
                     until );

   interactive_processing = KWFalse;

   if (IsNetwork())
   {                          /* Network connect is different        */
      if (!WaitForNetConnect(offset))
      {
         interactive_processing = KWTrue;

         shutDown();

         return CONN_INITIALIZE;
      }

      interactive_processing = KWTrue;
      printmsg(14, "callin: Network reports connected");

   }
#ifdef TAPI_SUPPORT
   else if (IsTAPI())
   {
      CONN_STATE state = answerTAPI(offset);

      if (state != CONN_NO_RETURN)
         return state;
   } /* if (IsTAPI()) */
#endif
   else {

      if (!sendlist( ring, modemTimeout, offset, noconnect))
      {                          /* Did it ring?                       */
         raised = 0;
         shutDown();
         if ( suspend_processing )        /* Give up modem for another process?  */
            return CONN_WAIT;
         else
            return CONN_INITIALIZE;     /* No --> Return to caller    */
      }

      interactive_processing = KWTrue;

      setPrty(M_priority, M_prioritydelta );
                              /* Into warp drive for actual transfers  */

      setTitle("Answering port %s", M_device);

      if(!sendlist(answer, modemTimeout, answerTimeout, noconnect))
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

KWBoolean getmodem( const char *brand)
{
   char filename[FILENAME_MAX];
   static char *modem = NULL;
   FILE *fp;
   size_t subscript;
   KWBoolean success;

/*--------------------------------------------------------------------*/
/*                      Validate the modem name                       */
/*   Added check for validity of modem ptr -- makes NT happier - dmw  */
/*--------------------------------------------------------------------*/

   if ((modem != NULL) && equal(modem, brand)) /* Already initialized?*/
      return KWTrue;           /* Yes --> Don't process it again      */

/*--------------------------------------------------------------------*/
/*                        Initialize the table                        */
/*--------------------------------------------------------------------*/

   for (subscript = 0; subscript < modemTableSize; subscript++ )
      if (modemTable[subscript].loc &&
          (modemTable[subscript].flag & (B_TOKEN | B_STRING | B_LIST | B_CLIST)))
         *((char **) modemTable[subscript].loc) = nil(char);

/*--------------------------------------------------------------------*/
/*             Reset boolean options to preferred values              */
/*--------------------------------------------------------------------*/

   resetOptions(modemFlags, bmodemflag, modemFlagsSize);

/*--------------------------------------------------------------------*/
/*                    Reset other preferred values                    */
/*--------------------------------------------------------------------*/

   M_charDelay = 00;          /* Default is no delay between chars    */
   dialTimeout = 40;          /* Default is 40 seconds to dial phone  */
   gPacketSize = SMALL_PACKET;
   vPacketSize = 512;
   GPacketSize = 512;
   gWindowSize = 0;
   vWindowSize = 0;
   GWindowSize = 0;
   M_fPacketSize = MAXPACK;
   M_fPacketTimeout = 20;
   M_gPacketTimeout = 10;
   M_ePacketTimeout = 60;
   M_tPacketTimeout = 60;
   M_portNumber = 0;
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
      return KWFalse;
   }

/*--------------------------------------------------------------------*/
/*                We got the file open, now process it                */
/*--------------------------------------------------------------------*/

   printmsg(3,"getmodem: loading modem configuration file %s", filename);
   success = getconfig(fp,
                       MODEM_CONFIG,
                       0,
                       modemTable,
                       modemTableSize,
                       modemFlags,
                       modemFlagsSize);
   fclose(fp);

   if (!success)
      return KWFalse;

/*--------------------------------------------------------------------*/
/*         Verify all required modem parameters were supplied         */
/*--------------------------------------------------------------------*/

   success = KWTrue;

   for (subscript = 0; subscript < modemTableSize; subscript++ )
   {

      if ((modemTable[subscript].flag & (B_REQUIRED | B_FOUND)) == B_REQUIRED)
      {
         printmsg(0, "getmodem: configuration parameter \"%s\" must be set.",
            modemTable[subscript].sym);
         success = KWFalse;
      } /* if */

   } /* for */

   if ( ! success )           /* Missing any required inputs?         */
      return success;         /* Yes --> Return failure to caller     */

/*--------------------------------------------------------------------*/
/*       The strings are valid, try to initialize the pointers to     */
/*       the processing routines.                                     */
/*--------------------------------------------------------------------*/

   if ( ! chooseCommunications( M_suite,
                                bmodemflag[MODEM_CARRIERDETECT] ,
                                &M_device ))
      return KWFalse;

/*--------------------------------------------------------------------*/
/*       We have success, save modem name for next time to speed      */
/*       initialize.                                                  */
/*--------------------------------------------------------------------*/

   modem = newstr(brand);  /* Yes --> Remember it for next time   */

   return KWTrue;

} /* getmodem */

/*--------------------------------------------------------------------*/
/*    d i a l                                                         */
/*                                                                    */
/*    Generic modem dialer; only major limitation is that autoabaud   */
/*    strings are not configurable                                    */
/*--------------------------------------------------------------------*/

static KWBoolean dial(char *number, const BPS speed)
{
   char buf[81];

/*--------------------------------------------------------------------*/
/*                        Open the serial port                        */
/*--------------------------------------------------------------------*/

   norecovery = KWFalse;          /* Shutdown gracefully as needed     */
   reEnable   = KWTrue;           /* Automatically reenable port       */

   echoCheck( 0 );               /* Disable echo checking            */

/*--------------------------------------------------------------------*/
/*              Flush the input buffer of any characters              */
/*--------------------------------------------------------------------*/

   if ( IsNetwork() )
   {
      if (activeopenline(number, M_portNumber, bmodemflag[MODEM_DIRECT]))
      {
         hostp->status.hstatus = HS_NODEVICE;
         return KWFalse;
      }
   }
   else {


      if (activeopenline(M_device, speed, bmodemflag[MODEM_DIRECT]))
      {

         hostp->status.hstatus = HS_NODEVICE;
         return KWFalse;
      }

      /* Set up the number to dial */
      strcpy(buf, dialPrefix);
      strcat(buf, number);

      if (dialSuffix != NULL)
         strcat(buf, dialSuffix);

      /* Handle dialing for TAPI and normal cases */
      if (IsTAPI())
      {
#ifdef TAPI_SUPPORT

         SetComHandle((int) Tapi_DialCall(M_device, number, dialTimeout));

         if (GetComHandle() == INVALID_HANDLE_VALUE)
         {
             if (TapiMsg)
                printmsg(0,"Tapi: Dial Failed: %s",TapiMsg);
             hostp->status.hstatus =  HS_DIAL_FAILED;
             return KWFalse;
         }

         if (activeopenline(M_device, speed, bmodemflag[MODEM_DIRECT]))
            panic();                /* tapi port open 2nd phase      */

#else

      printmsg(0,"dial: No compiled TAPI support");
      panic();

#endif /*  TAPI_SUPPORT */

      }
      else {
         char c;

         while (sread(&c,1,0));     /* Discard trailing trash from
                                       modem connect message         */

         /* Initialize the modem */
         if (!sendlist( initialize, modemTimeout, modemTimeout, noconnect))
         {
            printmsg(0,"dial: Modem failed to initialize");
            shutDown();
            hostp->status.hstatus = HS_DIAL_SCRIPT_FAILED;
            return KWFalse;
         }

         /* Send the dial command to the modem  */
         if (!sendstr( buf, modemTimeout, noconnect ))
            return KWFalse;

         if (!sendlist(connect,  modemTimeout, dialTimeout, noconnect))
         {
            hostp->status.hstatus = HS_DIAL_FAILED;
            return KWFalse;
         }
      } /* else */

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

   return KWTrue;           /* Dial succeeded    */

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
   static KWBoolean recurse = KWFalse;

   echoCheck( 0 );

   if ( ! portActive )          /* Allowed for Ctrl-Break           */
      return;

   if ( !recurse )
   {
      KWBoolean aborted = terminate_processing;
      unsigned long saveRaised = raised;
      recurse = KWTrue;
      terminate_processing = KWFalse;
      raised = 0;
      hangup();
      resetPrty();               /* Drop out of hyperspace            */

      if (!IsTAPI())
         sendlist( dropline, modemTimeout, modemTimeout, NULL);
      recurse = KWFalse;

      if ( aborted )
         terminate_processing = KWTrue;

      if ( saveRaised && ! raised )
         raised = saveRaised;
   }

/*--------------------------------------------------------------------*/
/*                          Release the port                          */
/*--------------------------------------------------------------------*/

   closeline();

/*--------------------------------------------------------------------*/
/*              Give port back to original owner, if any              */
/*--------------------------------------------------------------------*/

   if (!IsNetwork() && reEnable )
      suspend_other(KWFalse, M_device);

   norecovery = KWTrue;

}  /* shutDown */

/*--------------------------------------------------------------------*/
/*    s e n d l i s t                                                 */
/*                                                                    */
/*    Send a NULL terminated list of send/expect strings              */
/*--------------------------------------------------------------------*/

static KWBoolean sendlist(  char **list,
                           unsigned int timeout,
                           unsigned int lasttimeout,
                           char **failure)
{
   KWBoolean expect = KWTrue;

   if (list == NULL)          /* Was the field supplied?             */
      return KWTrue;           /* No --> Must be optional, return     */

/*--------------------------------------------------------------------*/
/*     Run through the list, alternating expect and send strings      */
/*--------------------------------------------------------------------*/

   while( *list != NULL)
   {

      if (expect)
      {
         char *exp = strdup( *list );
         KWBoolean success;
         checkref( exp );
         success = sendalt( exp,
                            (*(++list) == NULL) ? lasttimeout : timeout,
                            failure);
         free( exp );
         if (!success)
            return KWFalse;
      }
      else
         if (!sendstr( *list++, timeout, failure ))
            return KWFalse;

      if ( expect )
         expect = KWFalse;
      else
         expect = KWTrue;

   } /* while */

/*--------------------------------------------------------------------*/
/*    If we made it this far, success is at hand; return to caller    */
/*--------------------------------------------------------------------*/

   return KWTrue;

} /* sendlist */

/*--------------------------------------------------------------------*/
/*    s e n d a l t                                                   */
/*                                                                    */
/*    Expect a string, with alternates                                */
/*--------------------------------------------------------------------*/

static KWBoolean sendalt( char *exp, unsigned int timeout, char **failure)
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
         break;
      }

      if ( ok == 1 )
         return KWTrue;

      if (ok || (alternate == nil(char)))
         return KWFalse;

      if (!CD())
      {
         printmsg(0,"sendalt: Serial port reports modem not ready");
         break;
      }

      exp = strchr(alternate, '-');
      if (exp != nil(char))
         *exp++ = '\0';

      printmsg(0, "sending alternate");

      if ( !sendstr(alternate, timeout, failure) )
         break;

   } /*for*/

   return KWFalse;

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

#if defined(WIN32)
#pragma optimize("",off)   /* VC++ screws up bit shift below */
#endif

/*--------------------------------------------------------------------*/
/*    G e t G P a c k e t                                             */
/*                                                                    */
/*    Return the allowed packet size for the "g" procotol             */
/*--------------------------------------------------------------------*/


KEWSHORT GetGPacket( KEWSHORT maxvalue , const char protocol)
{
   KEWSHORT savePacketSize ;
   KEWSHORT ourPacketSize = 0;
   int bits = 6;     /* Minimum Packet Size is 64 bytes     */

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

   while((ourPacketSize >> (bits+1)) > 0)
   {
      if (bits > (sizeof ourPacketSize * 8))
         panic();

      bits++;
   }

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

#ifdef TAPI_SUPPORT

/*--------------------------------------------------------------------*/
/*    a n s w e r T A P I                                             */
/*                                                                    */
/*    Answer a TAPI telephone call                                    */
/*--------------------------------------------------------------------*/

static CONN_STATE
answerTAPI(time_t offset)
{

   time_t stop_time = time(NULL) + offset;

/*--------------------------------------------------------------------*/
/*       Every 5 secs AnswerCall returns to allow us to decr          */
/*       timeout and check for user aborts.                           */
/*--------------------------------------------------------------------*/

   for (;;)
   {
      long TapiShutdown = 0;
      long h = Tapi_AnswerCall(M_device, 5000, &TapiShutdown);

      /* If the handle opens, we 're done */
      if ( h != INVALID_HANDLE_VALUE)
      {
         SetComHandle(h);
         printmsg(2, "Tapi line connected");

         // do the inits to the port
         if (passiveopenline(M_device, inspeed, bmodemflag[MODEM_DIRECT]))
             panic();

         return CONN_NO_RETURN;
      }

      if (TapiShutdown || (time(NULL) > stop_time) || terminate_processing)
      {
         if (TapiShutdown && TapiMsg)
             printmsg(0,"Tapi: %s",TapiMsg);
         shutDown();
         return CONN_INITIALIZE;
      }

      if (suspend_processing)
      {
         shutDown();
         return CONN_WAIT;
      }

   } /* for (;;) */

} /* answerTAPI() */

#endif /* TAPI_SUPPORT */
