/*--------------------------------------------------------------------*/
/*    m o d e m . c                                                   */
/*                                                                    */
/*    High level modem control routines for UUPC/extended             */
/*                                                                    */
/*    Copyright (c) 1991 by Andrew H. Derbyshire                      */
/*                                                                    */
/*    Change history:                                                 */
/*       21 Apr 91      Create from dcpsys.c                          */
/*--------------------------------------------------------------------*/


/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: MODEM.C 1.15 1993/05/30 15:25:50 ahd Exp $
 *
 *    Revision history:
 *    $Log: MODEM.C $
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
 * Add additional shutdown() commands even when modem does not init
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
 * Revision 1.1  1992/05/02  13:06:48  ahd
 * Initial revision
 *
 */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <limits.h>
#include <sys/types.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "arpadate.h"
#include "checktim.h"
#include "dater.h"
#include "dcp.h"
#include "dcpsys.h"
#include "hlib.h"
#include "hostable.h"
#include "modem.h"
#include "script.h"
#include "security.h"
#include "ssleep.h"
#include "catcher.h"
#include "commlib.h"

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

char *device = NULL;          /*Public to show in login banner     */

static char **answer, **initialize, **dropline, **ring, **connect;
static char **noconnect;
static char *dialPrefix, *dialSuffix;
static char *M_suite;

static INTEGER chardelay, dialTimeout, modemTimeout, scriptTimeout;
static INTEGER answerTimeout, inspeed;
static INTEGER gWindowSize, gPacketSize;
static INTEGER vWindowSize, vPacketSize;
static INTEGER GWindowSize, GPacketSize;

INTEGER M_fPacketSize;
INTEGER M_gPacketTimeout;        /* "g" procotol                  */
INTEGER M_fPacketTimeout;        /* "f" procotol                  */
INTEGER M_MaxErr= 10;         /* Allowed errors per single packet    */
INTEGER M_MaxErr;             /* Allowed errors per single packet    */
INTEGER M_xfer_bufsize;       /* Buffering used for file transfers */

boolean bmodemflag[MODEM_LAST];

static FLAGTABLE modemFlags[] = {
   { "carrierdetect",  MODEM_CD,          B_LOCAL },
   { "direct",         MODEM_DIRECT,      B_LOCAL },
   { "fixedspeed",     MODEM_FIXEDSPEED,  B_LOCAL },
   { "variablepacket", MODEM_VARIABLEPACKET, B_LOCAL | B_OBSOLETE },
   { "largepacket",    MODEM_LARGEPACKET, B_LOCAL | B_OBSOLETE },
   { nil(char) }
}           ;

static CONFIGTABLE modemtable[] = {
   { "answer",        (char **) &answer,       B_LIST   | B_UUCICO },
   { "answertimeout", (char **) &answerTimeout,B_INTEGER| B_UUCICO },
   { "biggpacketsize",(char **) &GPacketSize,  B_INTEGER| B_UUCICO },
   { "biggwindowsize",(char **) &GWindowSize,  B_INTEGER| B_UUCICO },
   { "chardelay",     (char **) &chardelay,    B_INTEGER| B_UUCICO },
   { "connect",       (char **) &connect,      B_LIST   | B_UUCICO },
   { "device",        &device,  B_TOKEN  | B_UUCICO | B_REQUIRED },
   { "dialprefix",    &dialPrefix, B_STRING | B_UUCICO | B_REQUIRED },
   { "dialsuffix",    &dialSuffix,             B_STRING | B_UUCICO },
   { "dialtimeout",   (char **) &dialTimeout,  B_INTEGER| B_UUCICO },
   { "fpacketsize",   (char **) &M_fPacketSize,B_INTEGER| B_UUCICO },
   { "fpackettimeout",(char **) &M_fPacketTimeout, B_INTEGER | B_UUCICO },
   { "gpacketsize",   (char **) &gPacketSize,  B_INTEGER| B_UUCICO },
   { "gpackettimeout",(char **) &M_gPacketTimeout, B_INTEGER | B_UUCICO },
   { "gwindowsize",   (char **) &gWindowSize,  B_INTEGER| B_UUCICO },
   { "hangup",        (char **) &dropline,     B_LIST   | B_UUCICO },
   { "initialize",    (char **) &initialize,   B_LIST   | B_UUCICO },
   { "inspeed",       (char **) &inspeed,      B_INTEGER| B_UUCICO },
   { "maximumerrors", (char **) &M_MaxErr,     B_INTEGER| B_UUCICO },
   { "modemtimeout",  (char **) &modemTimeout, B_INTEGER| B_UUCICO },
   { "noconnect",     (char **) &noconnect,    B_LIST   | B_UUCICO },
   { "options",       (char **) bmodemflag,    B_ALL    | B_BOOLEAN},
   { "porttimeout",   NULL,                    B_OBSOLETE },
   { "ring",          (char **) &ring,         B_LIST   | B_UUCICO },
   { "scripttimeout", (char **) &scriptTimeout,B_INTEGER| B_UUCICO },
   { "suite",         &M_suite,                B_TOKEN  | B_UUCICO },
   { "transferbuffer",(char **) &M_xfer_bufsize, B_INTEGER| B_UUCICO },
   { "vpacketsize",   (char **) &vPacketSize,  B_INTEGER| B_UUCICO },
   { "vwindowsize",   (char **) &vWindowSize,  B_INTEGER| B_UUCICO },
   { nil(char) }
}; /* modemtable */

/*--------------------------------------------------------------------*/
/*                    Internal function prototypes                    */
/*--------------------------------------------------------------------*/

static boolean getmodem( const char *brand);

static boolean dial(char *number, const size_t speed);

static boolean sendlist( char **list, int timeout, int lasttimeout,
                         char **failure);

static boolean sendalt( char *string, int timeout, char **failure);

static void autobaud( const size_t speed);

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
   size_t speed;

/*--------------------------------------------------------------------*/
/*             Announce we are trying to call the system              */
/*--------------------------------------------------------------------*/

   printmsg(1, "callup: Calling %s via %s at %s on %s",
          rmtname, flds[FLD_TYPE], flds[FLD_SPEED], arpadate());

   speed = (size_t) atoi( flds[FLD_SPEED] );
   if (speed < 300)
   {
      printmsg(0,"callup: Modem speed %s is invalid.",
                  flds[FLD_SPEED]);
      hostp->hstatus = invalid_device;
      return CONN_INITIALIZE;
   }

/*--------------------------------------------------------------------*/
/*                     Get the modem information                      */
/*--------------------------------------------------------------------*/

   if (!getmodem(flds[FLD_TYPE]))
   {
      hostp->hstatus = invalid_device;
      return CONN_INITIALIZE;
   }

/*--------------------------------------------------------------------*/
/*                         Dial the telephone                         */
/*--------------------------------------------------------------------*/

   if (! dial(flds[FLD_PHONE],speed))
      return CONN_DROPLINE;

/*--------------------------------------------------------------------*/
/*             The modem is connected; now login the host             */
/*--------------------------------------------------------------------*/

   for (i = FLD_EXPECT; i < kflds; i += 2) {

      exp = flds[i];
      printmsg(2, "expecting %d of %d \"%s\"", i, kflds, exp);
      if (!sendalt( exp, scriptTimeout , noconnect))
      {
         printmsg(0, "SCRIPT FAILED");
         hostp->hstatus =  script_failed;
         return CONN_DROPLINE;
      } /* if */

      printmsg(2, "callup: sending %d of %d \"%s\"",
                   i + 1, kflds, flds[i + 1]);
      sendstr(flds[i + 1]);

   } /*for*/

   return CONN_PROTOCOL;

} /*callup*/

/*--------------------------------------------------------------------*/
/*    c a l l h o t                                                   */
/*                                                                    */
/*    Initialize processing when phone is already off the hook        */
/*--------------------------------------------------------------------*/

CONN_STATE callhot( const BPS xspeed )
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

   if (!getmodem(E_inmodem))  /* Initialize modem configuration      */
      panic();                /* Avoid loop if bad modem name        */

/*--------------------------------------------------------------------*/
/*                        Set the modem speed                         */
/*--------------------------------------------------------------------*/

   if ( xspeed == 0)
      speed = inspeed;
   else
      speed = xspeed;

/*--------------------------------------------------------------------*/
/*                    Open the communications port                    */
/*--------------------------------------------------------------------*/

   norecovery = FALSE;           // Shutdown gracefully as needed
   if (openline(device, speed, bmodemflag[MODEM_DIRECT] ))
      panic();

/*--------------------------------------------------------------------*/
/*                          Initialize stats                          */
/*--------------------------------------------------------------------*/

   memset( &remote_stats, 0, sizeof remote_stats);
                              /* Clear remote stats for login        */
   time(&remote_stats.ltime); /* Remember time of last attempt conn  */
   remote_stats.calls ++ ;

   if (bmodemflag[MODEM_CD])
      CD();                   /* Set the carrier detect flags        */

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
   time_t left;

/*--------------------------------------------------------------------*/
/*    Determine how long we can wait for the telephone, up to         */
/*    SHRT_MAX seconds.  Aside from Turbo C limits, this insures we   */
/*    kick the modem once in a while.                                 */
/*--------------------------------------------------------------------*/

      left =  exit_time - time(NULL);
      if ( left < 0 )               /* Any time left?                */
         return CONN_EXIT;             /* No --> shutdown            */

      if ( left > SHRT_MAX)
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

   if (!getmodem(E_inmodem))  /* Initialize modem configuration      */
      panic();                /* Avoid loop if bad modem name        */

   if ((ring == NULL) || (inspeed == 0))
   {
      printmsg(0,"callin: Missing inspeed and/or ring values in modem \
configuration file.");
      panic();
   } /* if */

/*--------------------------------------------------------------------*/
/*                    Open the communications port                    */
/*--------------------------------------------------------------------*/

   norecovery = FALSE;           // Shutdown gracefully as needed
   if (openline(device, inspeed, bmodemflag[MODEM_DIRECT]))
      panic();

/*--------------------------------------------------------------------*/
/*              Flush the input buffer of any characters              */
/*--------------------------------------------------------------------*/

   while (sread(&c ,1,0));    /* Discard trailing trash from modem
                                 connect message                     */

/*--------------------------------------------------------------------*/
/*                        Initialize the modem                        */
/*--------------------------------------------------------------------*/

   if (!sendlist( initialize, modemTimeout, modemTimeout, NULL))
   {
      printmsg(0,"callin: Modem failed to initialize");
      shutdown();
      panic();
   }

/*--------------------------------------------------------------------*/
/*                   Wait for the telephone to ring                   */
/*--------------------------------------------------------------------*/

   printmsg(1,"callin: Monitoring port %s device %s"
                     " for %d minutes until %s",
                     device, E_inmodem , (int) (offset / 60),
                      dater( exit_time , NULL));

   interactive_processing = FALSE;

   if (!sendlist( ring,modemTimeout, offset, noconnect))
                              /* Did it ring?                        */
   {
      interactive_processing = TRUE;
      shutdown();
      return CONN_INITIALIZE;     /* No --> Return to caller       */
   }

   interactive_processing = TRUE;

   if(!sendlist(answer, modemTimeout,answerTimeout, noconnect))
                              /* Pick up the telephone               */
   {
      printmsg(1,"callin: Modem failed to connect to incoming call");
      shutdown();
      return CONN_INITIALIZE;
   }

/*--------------------------------------------------------------------*/
/*           The modem is connected; now try to autobaud it           */
/*--------------------------------------------------------------------*/

   printmsg(14, "callin: Modem reports connected");


   autobaud(inspeed);         /* autobaud the modem                  */

/*--------------------------------------------------------------------*/
/*        Flush the input buffer of any other input characters        */
/*--------------------------------------------------------------------*/

   while (sread(&c ,1,0));    /* Discard trailing trash from modem
                                 connect message                     */

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

static boolean getmodem( const char *brand)
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

   chardelay = 00;            /* Default is no delay between chars    */
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
   modemTimeout  = 3;         /* Default is 3 seconds for modem cmds  */
   scriptTimeout = 30;        /* Default is 30 seconds for script data*/
   M_xfer_bufsize = BUFSIZ;   /* Buffering used for file transfers    */
   M_MaxErr= 10;              /* Allowed errors per single packet     */
   M_suite = NULL;            // Use default suite for communications

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

   if ( ! success )           // Missing any required inputs?
      return success;         // Yes --> Return failure to caller

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

static boolean dial(char *number, const size_t speed)
{
   char buf[81];

/*--------------------------------------------------------------------*/
/*                        Open the serial port                        */
/*--------------------------------------------------------------------*/

   norecovery = FALSE;           // Shutdown gracefully as needed
   if (openline(device, speed, bmodemflag[MODEM_DIRECT]))
   {

      hostp->hstatus =  nodevice;
      return FALSE;
   }

/*--------------------------------------------------------------------*/
/*              Flush the input buffer of any characters              */
/*--------------------------------------------------------------------*/

   while (sread(buf,1,0));    /* Discard trailing trash from modem
                                 connect message                     */

/*--------------------------------------------------------------------*/
/*                        Initialize the modem                        */
/*--------------------------------------------------------------------*/

   if (!sendlist( initialize, modemTimeout, modemTimeout, noconnect))
   {
      printmsg(0,"dial: Modem failed to initialize");
      shutdown();
      hostp->hstatus =  dial_script_failed;
      return FALSE;
   }

/*--------------------------------------------------------------------*/
/*           Setup the dial string and then dial the modem            */
/*--------------------------------------------------------------------*/

   strcpy(buf, dialPrefix);
   strcat(buf, number);
   if (dialSuffix != NULL)
      strcat(buf, dialSuffix);

   sendstr( buf );         /* Send the command to the telephone      */

   if (!sendlist(connect,  modemTimeout, dialTimeout, noconnect))
   {
      hostp->hstatus =  dial_failed;
      return FALSE;
   }
   printmsg(3, "dial: Modem reports connected");

   time( &remote_stats.lconnect );
   remote_stats.calls ++ ;

   autobaud(speed);        /* Reset modem speed, if desired          */

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

static void autobaud( const size_t speed )
{
   char buf[10];

   ssleep(1);                 /*  Allow modem port to stablize       */

   if (bmodemflag[MODEM_CD])
      CD();                   /* Set the carrier detect flags        */

/*--------------------------------------------------------------------*/
/*                  Autobaud the modem if requested                   */
/*--------------------------------------------------------------------*/

   if (!bmodemflag[MODEM_FIXEDSPEED])
   {
      size_t len = 0;

      memset( buf, '\0', sizeof( buf ));  /* Zero buffer                */
      while ((len < sizeof buf) && sread( &buf[len],1,0))
         len = strlen( buf );             /* Get speed into buffer      */

      if (len > 5)
      {
         char  *token;           /* Pointer to buffer value */
         token = strtok(buf,WHITESPACE);
         if (strlen(token))
         {
            size_t new_speed = (unsigned) atoi(token);
            if ((new_speed != speed) && (new_speed > 300))
            {
               printmsg(2, "autobaud: speed select %s", token);
               SIOSpeed(atoi(token));
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

void shutdown( void )
{
   static boolean recurse = FALSE;

   if ( ! portActive )          /* Allowed for Ctrl-Break           */
      return;

   if ( !recurse )
   {
      boolean aborted = terminate_processing;
      recurse = TRUE;
      terminate_processing = FALSE;
      hangup();
      sendlist( dropline, modemTimeout, modemTimeout, NULL);
      recurse = FALSE;
      terminate_processing |= aborted;
   }

   closeline();
   norecovery = TRUE;
}

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
         sendstr( *list++ );
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

      if ( terminate_processing )
      {
         shutdown();
         return FALSE;
      }

      if (ok || (alternate == nil(char)))
         return (ok == 1);


      if (bmodemflag[MODEM_CD] && ! CD())
      {
         printmsg(0,"sendalt: Serial port reports modem not ready");
         return FALSE;
      }

      exp = strchr(alternate, '-');
      if (exp != nil(char))
         *exp++ = '\0';

      printmsg(0, "sending alternate");
      sendstr(alternate);
   } /*for*/

} /* sendalt */

/*--------------------------------------------------------------------*/
/*    s l o w w r i t e                                               */
/*                                                                    */
/*    Write characters to the serial port at a configurable           */
/*    snail's pace.                                                   */
/*--------------------------------------------------------------------*/

void slowwrite( char *s, int len)
{
   swrite( s , len );
   if (chardelay > 0)
      ddelay(chardelay);
} /* slowwrite */

/*--------------------------------------------------------------------*/
/*    G e t G W i n d o w                                             */
/*                                                                    */
/*    Report the size of the allowed window for the "g" protocol      */
/*--------------------------------------------------------------------*/

INTEGER  GetGWindow(  INTEGER maxvalue , const char protocol )
{
   INTEGER ourWindowSize = 0;

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

INTEGER  GetGPacket( INTEGER maxvalue , const char protocol)
{
   INTEGER savePacketSize ;
   INTEGER ourPacketSize = 0;
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

   ourPacketSize = (ourPacketSize >> bits) << bits;
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
