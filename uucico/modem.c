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
 *    $Header: E:\SRC\UUPC\UUCICO\RCS\modem.c 1.1 1992/11/12 12:32:18 ahd Exp ahd $
 *
 *    Revision history:
 *    $Log: modem.c $
 * Revision 1.1  1992/11/12  12:32:18  ahd
 * Initial revision
 *
 * Revision 1.1  1992/05/02  13:06:48  ahd
 * Initial revision
 *
 */

static char rcsid[] = "$Id: modem.c 1.1 1992/11/12 12:32:18 ahd Exp ahd $";

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <limits.h>
#include <sys/types.h>

#include "lib.h"
#include "arpadate.h"
#include "checktim.h"
#include "dcp.h"
#include "dcpsys.h"
#include "hlib.h"
#include "hostable.h"
#include "modem.h"
#include "script.h"
#include "security.h"
#include "ssleep.h"
#include "catcher.h"
#include "ulib.h"

#define MAX_MODEM 8           /* Max length of a modem name          */

char *device = NULL;          /*Public to show in login banner     */

static char **answer, **initialize, **dropline, **ring, **connect;
static char **noconnect;
static char *dialPrefix, *dialSuffix;

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
   { "variablepacket", MODEM_VARIABLEPACKET, B_LOCAL },
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

CONN_STATE callup(char callgrade)
{
   char *exp;
   int i;
   size_t speed;

/*--------------------------------------------------------------------*/
/*      Determine if the window for calling this system is open       */
/*--------------------------------------------------------------------*/

   if ( !callgrade && equal(flds[FLD_CCTIME],"Never" ))
   {
      hostp->hstatus = wrong_time;
      return CONN_INITIALIZE;
   }

   time(&hostp->hstats->ltime); /* Save time of last attempt to call   */

/*--------------------------------------------------------------------*/
/*    Check the time of day and whether or not we should call now.    */
/*                                                                    */
/*    If calling a system to set the clock and we determine the       */
/*    system clock is bad (we fail the sanity check of the last       */
/*    connected a host to being in the future), then we ignore the    */
/*    time check field.                                               */
/*--------------------------------------------------------------------*/

   if (!callgrade)
   {

      if ((*flds[FLD_PROTO] != '*') ||       /* Not setting clock?   */
          ((hostp->hstats->ltime >  hostp->hstats->lconnect) &&
           (hostp->hstats->ltime >  630720000L )))
                                             /* Clock okay?          */
      {                                      /* Yes--> Return        */
         hostp->hstatus = wrong_time;
         return CONN_INITIALIZE;
      }
   } /* if */

/*--------------------------------------------------------------------*/
/*             Announce we are trying to call the system              */
/*--------------------------------------------------------------------*/

   printmsg(1, "callup: calling \"%s\" via %s at %s on %s",
          rmtname, flds[FLD_TYPE], flds[FLD_SPEED], arpadate());

   speed = (size_t) atoi( flds[FLD_SPEED] );
   if (speed < 300)
   {
      printmsg(0,"callup: Modem speed \"%s\" is invalid.",
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

   if (openline(device, speed, bmodemflag[MODEM_DIRECT] ))
      panic();

/*--------------------------------------------------------------------*/
/*                          Initialize stats                          */
/*--------------------------------------------------------------------*/

   memset( &remote_stats, 0, sizeof remote_stats);
                              /* Clear remote stats for login        */
   time(&remote_stats.ltime); /* Remember time of last attempt conn  */
   remote_stats.calls ++ ;
   return CONN_HOTLOGIN;

} /* callhot */

/*--------------------------------------------------------------------*/
/*    c a l l i n                                                     */
/*                                                                    */
/*    Answer the modem in passive mode                                */
/*--------------------------------------------------------------------*/

CONN_STATE callin( time_t exit_time )
{
   char c;                    /* A character for input buffer        */

   int    offset;             /* Time to wait for telephone          */
   time_t left;

/*--------------------------------------------------------------------*/
/*    Determine how long we can wait for the telephone, up to         */
/*    MAX_INT seconds.  Aside from Turbo C limits, this insures we    */
/*    kick the modem once in a while.                                 */
/*--------------------------------------------------------------------*/

      left =  exit_time - time(NULL);
      if ( left < 0 )               /* Any time left?                */
         return CONN_EXIT;             /* No --> shutdown            */

      if ( left > INT_MAX)
         offset = INT_MAX;
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
      panic();
   }

/*--------------------------------------------------------------------*/
/*                   Wait for the telephone to ring                   */
/*--------------------------------------------------------------------*/

   printmsg(1,"callin: Waiting for answer on port %s device %s for %d minutes",
                     device, E_inmodem , (int) (offset / 60) );

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

   printmsg(14, "callin: got CONNECT");

   if (bmodemflag[MODEM_CD])
      CD();                   /* Set the carrier detect flags        */

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
   static char modem[MAX_MODEM+1] = "";
   FILE *fp;
   CONFIGTABLE *tptr;
   size_t subscript;
   boolean success;

/*--------------------------------------------------------------------*/
/*                      Validate the modem name                       */
/*--------------------------------------------------------------------*/

   if (strlen(brand) > MAX_MODEM)
   {
      printmsg(0,"getmodem: Invalid modem %s; must be %d characters or less",
         brand, modem);
      return FALSE;
   }

   if (equal(modem, brand))   /* Already initialized?                */
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

/*--------------------------------------------------------------------*/
/*                 Open the modem configuration file                  */
/*--------------------------------------------------------------------*/

   if (equaln(brand,"COM",3))
   {
      printmsg(0,"Modem type \"%s\" is invalid; Snuffles suspects \
your %s file is obsolete.", brand, SYSTEMS);
      panic();
   }

   sprintf(filename,"%s/%s.MDM",E_confdir, brand);
   if ((fp = FOPEN(filename, "r", TEXT)) == nil(FILE))
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

   if ( success )             /* Good modem setup?                   */
      strcpy( modem, brand);  /* Yes --> Remember it for next time   */

   return success;

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

   if (bmodemflag[MODEM_CD])
      CD();                   /* Set the carrier detect flags        */

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

   if ( ! port_active )          /* Allowed for Ctrl-Break           */
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
