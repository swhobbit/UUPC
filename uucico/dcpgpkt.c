/*
   dcpgpkt.c

   Revised edition of dcp

   Stuart Lynne May/87

   Copyright (c) Richard H. Lamb 1985, 1986, 1987
   Changes Copyright (c) Stuart Lynne 1987
   Changes Copyright (c) Andrew H. Derbyshire 1989
   Changes Copyright (c) Kendra Electronic Wonderworks 1990-1992

   Maintenance notes:

   25Aug87 - Allow for up to 7 windows - Jal
   01Nov87 - those strncpy's should really be memcpy's! - Jal
   11Sep89 - Raise TimeOut to 15 - ahd
   30Apr90 - Add Jordon Brown's fix for short packet retries.
             Reduce retry limit to 20                             ahd
   22Jul90 - Change error retry limit from per host to per
             packet.                                              ahd
   22Jul90 - Add error message for number of retries exceeded     ahd
   08Sep90 - Drop memmove to memcpy change supplied by Jordan
             Brown, MS 6.0 and Turbo C++ agree memmove insures
             no overlap
*/

/*
 *      $Id$
 *
 *      $Log$
 */

static const char rcsid[] =
        "$Id$";

/* "DCP" a uucp clone. Copyright Richard H. Lamb 1985,1986,1987 */

/* 7-window "g" ptotocol */

/*--------------------------------------------------------------------*/
/*    Thanks goes to John Gilmore for sending me a copy of Greg       */
/*    Chesson's UUCP protocol description -- Obviously invaluable.    */
/*    Thanks also go to Andrew Tannenbaum for the section on          */
/*    Siding window protocols with a program example in his           */
/*    "Computer Networks" book.                                       */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>

#ifdef __TURBOC__
#include <mem.h>
#include <alloc.h>
#else
#include <malloc.h>
#endif

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "dcp.h"
#include "dcpsys.h"
#include "dcpgpkt.h"
#include "hostable.h"
#include "security.h"
#include "ulib.h"
#include "modem.h"
#include "catcher.h"

/*--------------------------------------------------------------------*/
/*                           Local defines                            */
/*--------------------------------------------------------------------*/

#define PKTSIZE   MAXPACK
#define MINPKT    32

#define HDRSIZE   6
#define MAXTRY 4

#ifndef GDEBUG
#define GDEBUG 4
#endif

/*--------------------------------------------------------------------*/
/*    Control whether some buffers are placed outside the default     */
/*    data segment                                                    */
/*--------------------------------------------------------------------*/

#ifdef __TURBOC__
#define memavail  coreleft
#else
#define memavail  stackavail
#endif

/*--------------------------------------------------------------------*/
/*                     g-packet type definitions                      */
/*--------------------------------------------------------------------*/

#define DATA   0
#define CLOSE  1
#define NAK    2
#define SRJ    3
#define ACK    4
#define INITC  5
#define INITB  6
#define INITA  7

#define POK    -1

#define MAXWINDOW 7
#define NBUF   8              /* always SAME as MAXSEQ ? */
#define MAXSEQ 8

typedef enum {
      I_CALLEE,
      I_CALLER,
      I_COMPLETE,
      I_EMPTY,
      I_ERROR,
      I_GRPACK,
      I_INITA,
      I_INITA_SEND,
      I_INITB,
      I_INITB_SEND,
      I_INITC,
      I_INITC_SEND,
      I_RESTART }
      I_STATE;

#define between(a,b,c) ((a<=b && b<c) || \
                        (c<a && a<=b) || \
                        (b<c && c<a))

#define nextpkt(x)    ((x + 1) % MAXSEQ)
#define nextbuf(x)    ((x + 1) % (nwindows+1))

/*--------------------------------------------------------------------*/
/*              Global variables for packet definitions               */
/*--------------------------------------------------------------------*/

currentfile();

static int rwl, swl, swu, rwu, irec, lazynak;
static unsigned nbuffers;
static int rbl, sbl, sbu;
static INTEGER nerr;
static unsigned outlen[NBUF], inlen[NBUF], xmitlen[NBUF];
static boolean arrived[NBUF];
static size_t nwindows;
static char *outbuf[NBUF];
static char *inbuf[NBUF];
static time_t ftimer[NBUF];
static int timeouts, outsequence, naksin, naksout, screwups;
static int reinit, shifts, badhdr, resends;
static unsigned char *grpkt = NULL;
static boolean variablepacket;  /* "v" or in modem file              */

/*--------------------------------------------------------------------*/
/*                    Internal function prototypes                    */
/*--------------------------------------------------------------------*/

static int initialize(const boolean caller, const char protocol );

static int  gmachine(const int timeout);

static void gspack(int  type,
                   int  yyy,
                   int  xxx,
                   int  len,
                   unsigned xmit,
                   char  *data);

static int  grpack(int  *yyy,
                   int  *xxx,
                   int  *len,
                   char *data,
                   const int timeout);

static void gstats( void );

static unsigned int checksum(char *data, int len);

/****************** SUB SUB SUB PACKET HANDLER ************/

/*--------------------------------------------------------------------*/
/*    g o p e n p k                                                   */
/*                                                                    */
/*    Initialize processing for protocol                              */
/*--------------------------------------------------------------------*/

int Gopenpk(const boolean caller)
{
   return initialize(caller , 'G');
} /* Gopenpk */

/*--------------------------------------------------------------------*/
/*    v o p e n p k                                                   */
/*                                                                    */
/*    Initialize processing for protocol                              */
/*--------------------------------------------------------------------*/

int vopenpk(const boolean caller)
{
   return initialize(caller, 'v');
} /* vopenpk */

/*--------------------------------------------------------------------*/
/*    g o p e n p k                                                   */
/*                                                                    */
/*    Initialize processing for protocol                              */
/*--------------------------------------------------------------------*/

int gopenpk(const boolean caller)
{
   return initialize(caller, 'g');
} /* vopenpk */

/*--------------------------------------------------------------------*/
/*    i n i t i a l i z e                                             */
/*                                                                    */
/*    Initialize processing for protocol                              */
/*--------------------------------------------------------------------*/

static int initialize(const boolean caller, const char protocol )
{
   int i, xxx, yyy, len, maxwindows;
   boolean  sent_inita = FALSE, recv_inita = FALSE;
   boolean  sent_initb = FALSE, recv_initb = FALSE;
   boolean  sent_initc = FALSE, recv_initc = FALSE;
   I_STATE state;

/*--------------------------------------------------------------------*/
/* Read modem file values for the number of windows and packet sizes  */
/*--------------------------------------------------------------------*/

   pktsize = GetGPacket( MAXPACK, protocol );
   maxwindows = GetGWindow(
                     min( MAXWINDOW, RECV_BUF / (pktsize+HDRSIZE)),
                     protocol);

   variablepacket = bmodemflag[MODEM_VARIABLEPACKET] | (protocol == 'v');

   grpkt = malloc( pktsize + HDRSIZE );

/*--------------------------------------------------------------------*/
/*                     Initialize error counters                      */
/*--------------------------------------------------------------------*/

   timeouts = outsequence = naksin = naksout = screwups =
      shifts = badhdr = resends = reinit = 0;

/*--------------------------------------------------------------------*/
/*                    Initialize proto parameters                     */
/*--------------------------------------------------------------------*/

   nerr = nbuffers = 0;
   sbl = swl = swu = sbu = 1;
   rbl = rwl = 0;
   nwindows = maxwindows;
   rwu = nwindows - 1;

   for (i = 0; i < NBUF; i++)
   {
      ftimer[i] = 0;
      arrived[i] = FALSE;
   }

/*--------------------------------------------------------------------*/
/*                          3-way handshake                           */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    The three-way handshake should be independent of who            */
/*    initializes it, but it seems that some versions of uucico       */
/*    assume that the caller sends first and the callee responds.     */
/*    This only matters if we are the callee and the first packet     */
/*    is garbled.  If we send a packet, the other side will assume    */
/*    that we must have seen the packet they sent and will never      */
/*    time out and send it again.  Therefore, if we are the callee    */
/*    we don't send a packet the first time through the loop.         */
/*    This can still fail, but should usually work, and, after        */
/*    all, if the initialization packets are received correctly       */
/*    there will be no problem no matter what we do.                  */
/*                                                                    */
/*    (The above quoted verbatim from Ian Taylor)                     */
/*--------------------------------------------------------------------*/

   state = caller ? I_CALLER : I_CALLEE;

/*--------------------------------------------------------------------*/
/*    INIT sequence.                                                  */
/*--------------------------------------------------------------------*/

   while( state != I_COMPLETE )
   {
      switch( state )
      {

/*--------------------------------------------------------------------*/
/*                          Receive a packet                          */
/*--------------------------------------------------------------------*/

         case I_GRPACK:
            switch (grpack(&yyy, &xxx, &len, NULL, M_gPacketTimeout ))
            {

               case INITA:
                  printmsg(5, "**got INITA");
                  state = I_INITA;
                  break;

               case INITB:
                  printmsg(5, "**got INITB");
                  state = I_INITB;
                  break;

               case INITC:
                  printmsg(5, "**got INITC");
                  state = I_INITC;
                  break;

               case EMPTY:
                  printmsg(5, "**got EMPTY");
                  state = I_EMPTY;
                  break;

               case CLOSE:
                  printmsg(GDEBUG, "**got CLOSE");
                  gspack(CLOSE, 0, 0, 0, 0, NULL);
                  return FAILED;
                  break;

               default:
                  printmsg(GDEBUG, "**got SCREW UP");
                  state = I_ERROR;
                  break;
            }

            if (bmodemflag[MODEM_CD] && !CD())
            {
               printmsg(0,"gopenpk: Modem carrier lost");
               return FAILED;
            }
            break;

/*--------------------------------------------------------------------*/
/*                         Initialize states                          */
/*--------------------------------------------------------------------*/

         case I_CALLER:
            state = I_INITA_SEND;
            break;

         case I_CALLEE:
            state = I_GRPACK;
            break;

/*--------------------------------------------------------------------*/
/*                  Process received or sent packets                  */
/*--------------------------------------------------------------------*/

         case I_INITA:
            if (yyy < (int) nwindows)
            {
               nwindows = yyy;
               rwu = nwindows - 1;
            }
            recv_inita = TRUE;
            state = sent_inita ? I_INITB_SEND : I_INITA_SEND;
            break;

         case I_INITA_SEND:
            gspack(INITA, 0, 0, 0, pktsize, NULL);
            sent_inita = TRUE;
            state = I_GRPACK;
            break;

         case I_INITB:
            if ( recv_inita &&  sent_inita )
            {
               i = (int) 8 * (2 << (yyy+1));
               if (i < (int) pktsize)
                  pktsize = i;
               recv_initb = TRUE;
               state = sent_initb ? I_INITC_SEND : I_INITB_SEND;
            } /* if */
            else
               state = I_RESTART;
            break;

         case I_INITB_SEND:
            gspack(INITB, 0, 0, 0, pktsize, NULL);
                                       /* Data segment (packet) size    */
            sent_initb = TRUE;
            state = I_GRPACK;
            break;

         case I_INITC:
            if ( recv_initb &&  sent_initb )
            {
               if (yyy < (int) nwindows)
               {
                  nwindows = yyy;
                  rwu = nwindows - 1;
               }
               recv_initc = TRUE;
               state = sent_initc ? I_COMPLETE : I_INITC_SEND;
            }
            else
               state = I_RESTART;
            break;

         case I_INITC_SEND:
            gspack(INITC, 0, 0, 0, pktsize, NULL);
            sent_initc = TRUE;
            state = recv_initc ? I_COMPLETE : I_GRPACK;
            break;

/*--------------------------------------------------------------------*/
/*                            Error states                            */
/*--------------------------------------------------------------------*/

         case I_EMPTY:
            timeouts++;
            state = I_RESTART;
            break;

         case I_ERROR:
            screwups++;
            state = I_RESTART;
            break;

         case I_RESTART:
            printmsg(2,"gopenpk: Restarting initialize sequence");
            nerr++;
            sent_inita = recv_inita = sent_initb = recv_initb =
                         sent_initc = recv_initc = FALSE;
            state = I_INITA_SEND;
            break;

      } /* switch */

      if ( terminate_processing )
      {
         printmsg(0,"gopenpk: Terminated by user");
         return FAILED;
      }

      if (nerr >= M_MaxErr)
      {
         remote_stats.errors += nerr;
         nerr = 0;
         printmsg(0,
            "gopenpk: Consecutive error limit of %ld exceeded, "
                     "%ld total errors",
             (long) M_MaxErr, remote_stats.errors);
         return(FAILED);
      }
   } /* while */

/*--------------------------------------------------------------------*/
/*                    Allocate the needed buffers                     */
/*--------------------------------------------------------------------*/

   grpkt = realloc( grpkt, pktsize + HDRSIZE );

   i = 0;
   while( i <= (int) nwindows)
   {

      inbuf[i] = malloc( pktsize );
      if (inbuf[i] == (char *) NULL ) checkref( NULL );
                              /* Forces the regular error message */

      outbuf[i] = malloc( pktsize );
      checkref( outbuf[i] );

      i ++;

   } /* while */


   nerr = 0;
   lazynak = 0;

#ifdef WIN32
   printmsg(2,"Short packets %sabled, "
              "Window size %d, "
              "Packet size %d\n",
            variablepacket ? "en" : "dis", nwindows, pktsize );
#else
   printmsg(2,"Smart packets %sabled, "
              "Window size %d, "
              "Packet size %d, "
              "Memory avail %u",
            variablepacket ? "en" : "dis", nwindows, pktsize,
            memavail());
#endif

   return(OK); /* channel open */

} /*initialize*/

/*--------------------------------------------------------------------*/
/*    g f i l e p k t                                                 */
/*                                                                    */
/*    Begin a file transfer (not used by "g" protocol)                */
/*--------------------------------------------------------------------*/

int gfilepkt( void )
{

   return OK;

} /* gfilepkt */

/*--------------------------------------------------------------------*/
/*    g c l o s e p k                                                 */
/*                                                                    */
/*    Close packet machine                                            */
/*--------------------------------------------------------------------*/

int gclosepk()
{
   unsigned i;

   for (i = 0; i < MAXTRY; i++)
   {
      gspack(CLOSE, 0, 0, 0, 0, NULL);
      if (gmachine(M_gPacketTimeout) == CLOSE)
         break;
   } /* for (i = 0; i < MAXTRY; i++) */

/*--------------------------------------------------------------------*/
/*                        Release our buffers                         */
/*--------------------------------------------------------------------*/

   i = 0;

   while( i <= nwindows )
   {
      free( (void *)inbuf[i] );
      free( outbuf[i] );
      inbuf[i] = outbuf[i] = NULL;
      i++;
   } /* while( i < NBUF ) */

   free( grpkt );
   grpkt = NULL;

/*--------------------------------------------------------------------*/
/*                Report the results of our adventures                */
/*--------------------------------------------------------------------*/

   gstats();

/*--------------------------------------------------------------------*/
/*                          Return to caller                          */
/*--------------------------------------------------------------------*/

   return(0);

} /*gclosepk*/

/*--------------------------------------------------------------------*/
/*    g s t a t s                                                     */
/*                                                                    */
/*    Report summary of errors for processing                         */
/*--------------------------------------------------------------------*/

static void gstats( void )
{
   remote_stats.errors += nerr;
   nerr = 0;
   if ( remote_stats.errors || badhdr )
   {
      printmsg(0,
         "%d time outs, %d port reinits, %d out of seq pkts, "
         "%d NAKs rec, %d NAKs sent",
            timeouts, reinit, outsequence, naksin, naksout);
      printmsg(0,
         "%d invalid pkt types, %d re-syncs, %d bad pkt hdrs, %d pkts resent",
            screwups, shifts, badhdr, resends);
   } /* if ( remote_stats.errors || shifts || badhdr ) */
} /* gstats */

/*--------------------------------------------------------------------*/
/*    g g e t p k t                                                   */
/*                                                                    */
/*    Gets no more than a packet's worth of data from                 */
/*    the "packet I/O state machine".  May have to                    */
/*    periodically run the packet machine to get some packets.        */
/*                                                                    */
/*    on input:   don't care                                          */
/*    on return:  data+\0 and length in len.                          */
/*                                                                    */
/*    ret(0)   if all's well                                          */
/*    ret(-1) if problems (failed)                                    */
/*--------------------------------------------------------------------*/

int ggetpkt(char *data, int *len)
{
   int   retry = M_MaxErr;
   time_t start;
#ifdef _DEBUG
   int savedebug = debuglevel;
#endif

   irec = 1;
   checkref( data );

/*--------------------------------------------------------------------*/
/*                Loop to wait for the desired packet                 */
/*--------------------------------------------------------------------*/

   time( &start );
   while (!arrived[rbl] && retry)
   {
      if (gmachine(M_gPacketTimeout) != POK)
         return(-1);

      if (!arrived[rbl] )
      {
         time_t now;
         if (time( &now ) > (start + M_gPacketTimeout) )
         {
#ifdef _DEBUG
            if ( debuglevel < 6 )
               debuglevel = 6;
#endif
            printmsg(GDEBUG,
                     "ggetpkt: Timeout %d waiting for inbound packet %d",
                     M_MaxErr - --retry, remote_stats.packets + 1);
            timeouts++;
            start = now;
         } /* if (time( now ) > (start + M_gPacketTimeout) ) */
      } /* if (!arrived[rbl] ) */
   } /* while (!arrived[rbl] && i) */

#ifdef _DEBUG
   debuglevel = savedebug;
#endif

   if (!arrived[rbl])
   {
      printmsg(0,"ggetpkt: Remote host failed to respond after %ld seconds",
               (long) M_gPacketTimeout * M_MaxErr);
      gclosepk();
      return -1;
   }

/*--------------------------------------------------------------------*/
/*                           Got a packet!                            */
/*--------------------------------------------------------------------*/

   *len = inlen[rbl];
   memcpy(data, inbuf[rbl], *len);

   arrived[rbl] = FALSE;      /* Buffer is now emptied               */
   rwu = nextpkt(rwu);        /* bump receive window                 */

   return(0);

} /*ggetpkt*/


/*
   g s e n d p k t

   Put at most a packet's worth of data in the packet state
   machine for transmission.
   May have to run the packet machine a few times to get
   an available output slot.

   on input: data=*data; len=length of data in data.

   return:
    0 if all's well
   -1 if problems (failed)
*/

int gsendpkt(char *data, int len)
{
   int delta;
#ifdef _DEBUG
   int savedebug = debuglevel;
#endif

   checkref( data );
   irec = 0;
   /* WAIT FOR INPUT i.e. if weve sent SWINDOW pkts and none have been
      acked, wait for acks */
   while (nbuffers >= nwindows)
      if (gmachine(0) != POK)    /* Spin with no timeout             */
         return(-1);

/*--------------------------------------------------------------------*/
/*               Place packet in table and mark unacked               */
/*--------------------------------------------------------------------*/

   memcpy(outbuf[sbu], data, len);

/*--------------------------------------------------------------------*/
/*                       Handle short packets.                        */
/*--------------------------------------------------------------------*/

   xmitlen[sbu] = pktsize;
   if (variablepacket)
      while ( ((len * 2) < (int) xmitlen[sbu]) && (xmitlen[sbu] > MINPKT) )
         xmitlen[sbu] /= 2;

   if ( xmitlen[sbu] < MINPKT )
   {
      printmsg(0,"gsendpkt: Bad packet size %d, "
               "data length %d",
               xmitlen[sbu], len);
      xmitlen[sbu] = MINPKT;
   }

   delta = xmitlen[sbu] - len;
   if (delta > 127)
   {
      memmove(outbuf[sbu] + 2, outbuf[sbu], len);
      memset(outbuf[sbu]+len+2, 0, delta - 2);
                              /* Pad with nulls.  Ugh.               */
      outbuf[sbu][0] = (unsigned char) ((delta & 0x7f) | 0x80);
      outbuf[sbu][1] = (unsigned char) (delta >> 7);
   } /* if (delta > 127) */
   else if (delta > 0 )
   {
      memmove(outbuf[sbu] + 1, outbuf[sbu], len);
      outbuf[sbu][0] = (unsigned char) delta;
      memset(outbuf[sbu]+len+1, 0, delta - 1);
                              /* Pad with nulls.  Ugh.               */
   } /* else if (delta > 0 )  */

/*--------------------------------------------------------------------*/
/*                            Mark packet                             */
/*--------------------------------------------------------------------*/

   outlen[sbu] = len;
   ftimer[sbu] = time(nil(long));
   nbuffers++;

/*--------------------------------------------------------------------*/
/*                              send it                               */
/*--------------------------------------------------------------------*/

   gspack(DATA, rwl, swu, outlen[sbu], xmitlen[sbu], outbuf[sbu]);

   swu = nextpkt(swu);        /* Bump send window                    */
   sbu = nextbuf( sbu );      /* Bump to next send buffer            */

#ifdef _DEBUG
   debuglevel = savedebug;
#endif

   return(0);

} /*gsendpkt*/


/*--------------------------------------------------------------------*/
/*    g e o f p k t                                                   */
/*                                                                    */
/*    Transmit EOF to the other system                                */
/*--------------------------------------------------------------------*/

int geofpkt( void )
{
   if (gsendpkt("", 0))          /* Empty packet == EOF              */
      return FAILED;
   else
      return OK;
} /* geofpkt */

/*--------------------------------------------------------------------*/
/*    g w r m s g                                                     */
/*                                                                    */
/*    Send a message to remote system                                 */
/*--------------------------------------------------------------------*/

int gwrmsg( char *s )
{
   for(; strlen(s) >= pktsize; s += pktsize) {
      int result = gsendpkt(s, pktsize);
      if (result)
         return result;
   }

   return gsendpkt(s, strlen(s)+1);
} /* gwrmsg */

/*--------------------------------------------------------------------*/
/*    g r d m s g                                                     */
/*                                                                    */
/*    Read a message from the remote system                           */
/*--------------------------------------------------------------------*/

int grdmsg( char *s)
{
   for ( ;; )
   {
      int len;
      int result = ggetpkt( s, &len );
      if (result || (s[len-1] == '\0'))
         return result;
      s += len;
   } /* for */

} /* grdmsg */

/**********  Packet Machine  ********** RH Lamb 3/87 */

/*--------------------------------------------------------------------*/
/*    g m a c h i n e                                                 */
/*                                                                    */
/*    Ideally we would like to fork this process off in an            */
/*    infinite loop and send and receive packets through "inbuf"      */
/*    and "outbuf".  Can't do this in MS-DOS so we setup "getpkt"     */
/*    and "sendpkt" to call this routine often and return only        */
/*    when the input buffer is empty thus "blocking" the packet-      */
/*    machine task.                                                   */
/*--------------------------------------------------------------------*/

static int gmachine(const int timeout )
{
   static time_t idletimer = 0;

   boolean done   = FALSE;    /* True = drop out of machine loop  */
   boolean close  = FALSE;    /* True = terminate connection upon
                                        exit                      */
   boolean inseq  = TRUE;     /* True = Count next out of sequence
                                        packet as an error           */
   while ( !done )
   {
      boolean resend = FALSE;    /* True = resend data packets       */
      boolean donak  = FALSE;    /* True = NAK the other system      */
      unsigned long packet_no = remote_stats.packets;

      int pkttype, rack, rseq, rlen, rbuf, i1;
      time_t now;

#ifdef UDEBUG
      if ( debuglevel >= 7 )     /* Optimize processing a little bit */
      {

         printmsg(10, "* send %d %d < W < %d %d, "
                      "receive %d %d < W < %d, "
                      "error %d, packet %d",
            swl, sbl, swu, sbu, rwl, rbl, rwu, nerr,
            (long) remote_stats.packets);

/*--------------------------------------------------------------------*/
/*    Waiting for ACKs for swl to swu-1.  Next pkt to send=swu        */
/*    rwl=expected pkt                                                */
/*--------------------------------------------------------------------*/

      }
#endif

/*--------------------------------------------------------------------*/
/*             Attempt to retrieve a packet and handle it             */
/*--------------------------------------------------------------------*/

      pkttype = grpack(&rack, &rseq, &rlen, inbuf[nextbuf(rbl)], timeout);
      time(&now);
      switch (pkttype) {

         case CLOSE:
            remote_stats.packets++;
            printmsg(GDEBUG, "**got CLOSE");
            close = done = TRUE;
            break;

         case EMPTY:
            printmsg(timeout ? GDEBUG : 8, "**got EMPTY");

            if (bmodemflag[MODEM_CD] && !CD())
            {
               printmsg(0,"gmachine: Modem carrier lost");
               nerr++;
               close = TRUE;
            }

            if ( terminate_processing )
            {
               printmsg(0,"gmachine: User aborted processing");
               close = TRUE;
            }

            if (ftimer[sbl])
            {
#ifdef UDEBUG
               printmsg(6, "---> seq, elapst %d %ld", sbl,
                    ftimer[sbl] - now);
#endif
               if ( ftimer[sbl] <= (now - M_gPacketTimeout))
               {
                   printmsg(4, "*** timeout %d (%ld)",
                               sbl, (long) remote_stats.packets);
                       /* Since "g" is "go-back-N", when we time out we
                          must send the last N pkts in order.  The generalized
                          sliding window scheme relaxes this reqirment. */
                   nerr++;
                   timeouts++;
                   resend = TRUE;
               } /* if */
            } /* if */

            done = TRUE;
            break;

         case DATA:
            printmsg(5, "**got DATA %d %d", rack, rseq);
            i1 = nextpkt(rwl);   /* (R+1)%8 <-- -->(R+W)%8 */
            if (i1 == rseq) {
               lazynak--;
               remote_stats.packets++;
               idletimer = now;
               rwl = i1;
               rbl = nextbuf( rbl );
               inseq = arrived[rbl] = TRUE;
               inlen[rbl] = rlen;
               printmsg(5, "*** ACK d %d %d", rwl, rbl);
               gspack(ACK, rwl, 0, 0, 0, NULL);
               done = TRUE;   /* return to caller when finished      */
                              /* in a mtask system, unneccesary      */
            } else {
               if (inseq || ( now > (idletimer + M_gPacketTimeout)))
               {
                  donak = TRUE;  /* Only flag first out of sequence
                                    packet as error, since we know
                                    following ones also bad             */
                  outsequence++;
                  inseq = FALSE;
               }
               printmsg(GDEBUG, "*** unexpect %d ne %d (%d - %d)",
                                       rseq, i1, rwl, rwu);
            } /* else */

            if ( swl == swu )       /* We waiting for an ACK?     */
               break;               /* No --> Skip ACK processing */
            /* else Fall through to ACK case */

         case NAK:
         case ACK:
            if (pkttype == NAK)
            {
               nerr++;
               naksin++;
               printmsg(5, "**got NAK %d", rack);
               resend = TRUE;
            }
            else if (pkttype == ACK)
               printmsg(5, "**got ACK %d", rack);

            while(between(swl, rack, swu))
            {                             /* S<-- -->(S+W-1)%8 */
               remote_stats.packets++;
               printmsg(5, "*** ACK %d", swl);
               ftimer[sbl] = 0;
               idletimer = now;
               nbuffers--;
               done = TRUE;            /* Get more data for input */
               swl = nextpkt(swl);
               sbl = nextbuf(sbl);
            } /* while */

            if (!done && (pkttype == ACK)) /* Find packet?         */
            {
               printmsg(GDEBUG,"*** ACK for bad packet %d (%d - %d)",
                           rack, swl, swu);
            } /* if */
            break;

         case ERROR:
            printmsg(GDEBUG, "*** got BAD CHK");
            naksout++;
            donak = TRUE;
            lazynak = 0;               /* Always NAK bad checksum */
            break;

         default:
            screwups++;
            nerr++;
            printmsg(GDEBUG, "*** got SCREW UP");
            break;

      } /* switch */

/*--------------------------------------------------------------------*/
/*      If we received an NAK or timed out, resend data packets       */
/*--------------------------------------------------------------------*/

      if ( resend )
      for (rack = swl,
           rbuf = sbl;
           between(swl, rack, swu);
           rack = nextpkt(rack), rbuf = nextbuf( rbuf ))
      {                          /* resend rack->(swu-1)             */
         resends++;

         if (( outbuf[rbuf] == NULL ))
         {
            printmsg(0,"gmachine: Transmit of NULL packet (%d %d)",
                     rwl, rbuf);
            panic();
         }

         if (( xmitlen[rbuf] == 0 ))
         {
            printmsg(0,"gmachine: Transmit of 0 length packet (%d %d)",
                     rwl, rbuf);
            panic();
         }

         gspack(DATA, rwl, rack, outlen[rbuf], xmitlen[rbuf], outbuf[rbuf]);
         printmsg(5, "*** resent %d", rack);
         idletimer = ftimer[rbuf] = now;
      } /* for */

/*--------------------------------------------------------------------*/
/*  If we have an error and have not recently sent a NAK, do so now.  */
/*  We then reset our counter so we receive at least a window full of */
/*                 packets before sending another NAK                 */
/*--------------------------------------------------------------------*/

      if ( donak )
      {
         nerr++;
         if ( (lazynak < 1) || (now > (idletimer + M_gPacketTimeout)))
         {
            printmsg(5, "*** NAK d %d", rwl);
            gspack(NAK, rwl, 0, 0, 0, NULL);
            naksout++;
            idletimer = now;
            lazynak = nwindows + 1;
         } /* if ( lazynak < 1 ) */
      } /* if ( donak ) */

/*--------------------------------------------------------------------*/
/*                   Update error counter if needed                   */
/*--------------------------------------------------------------------*/

      if ((close || (packet_no != remote_stats.packets)) && (nerr > 0))
      {
         printmsg(GDEBUG,"gmachine: Packet %ld had %ld errors during transfer",
                     remote_stats.packets, (long) nerr);
         remote_stats.errors += nerr;
         nerr = 0;
      }

/*--------------------------------------------------------------------*/
/*    If we have an excessive number of errors, drop out of the       */
/*    loop                                                            */
/*--------------------------------------------------------------------*/

      if (nerr >= M_MaxErr)
      {
         printmsg(0,
            "gmachine: Consecutive error limit of %d exceeded, %d total errors",
            M_MaxErr, nerr + remote_stats.errors);
         done = close = TRUE;
         gstats();
      }
   } /* while */

/*--------------------------------------------------------------------*/
/*    Return to caller, gracefully terminating packet machine if      */
/*    requested                                                       */
/*--------------------------------------------------------------------*/

   if ( close )
   {
      gspack(CLOSE, 0, 0, 0, 0, NULL);
      return CLOSE;
   }
   else
      return POK;

} /*gmachine*/


/*************** FRAMING *****************************/

/*
   g s p a c k

   Send a packet

   type=type yyy=pkrec xxx=timesent len=length<=pktsize data=*data
   ret(0) always
*/

static void gspack(int type,
                   int yyy,
                   int xxx,
                   int len,
                   unsigned xmit,
                   char *data)
{
   unsigned int check, i;
   unsigned char header[HDRSIZE];

#ifdef   LINKTEST
   /***** Link Testing Mods *****/
   unsigned char  dpkerr[10];
   /***** End Link Testing Mods *****/
#endif   /* LINKTEST */

#ifdef   LINKTEST
   /***** Link Testing Mods - create artificial errors *****/
   printf("**n:normal,e:error,l:lost,p:partial,h:bad header,s:new seq--> ");
   gets(dpkerr);
   if (dpkerr[0] == 's')
      sscanf(&dpkerr[1], "%d", &xxx);
   /***** End Link Testing Mods *****/
#endif   /* LINKTEST */

   if ( debuglevel > 4 )
      printmsg(5, "send packet type %d, yyy=%d, xxx=%d, len=%d, buf = %d",
               type, yyy, xxx, len, xmit);

   header[0] = '\020';
   header[4] = (unsigned char) (type << 3);

   switch (type) {

      case CLOSE:
         break;   /* stop protocol */

      case NAK:
         header[4] += yyy;
         break;   /* reject */

      case SRJ:
         break;

      case ACK:
         header[4] += yyy;
         break;   /* ack */

      case INITA:
      case INITC:
         header[4] += nwindows;
         break;

      case INITB:
         i = MINPKT;
         while( i < xmit )
         {
            header[4] ++;
            i *= 2;
         }
         break;

      case DATA:
         header[4] = (unsigned char) (0x80 + (xxx << 3) + yyy);
         if (len < (int) xmit)      /* Short packet?              */
            header[4] |= 0x40;/* Count byte handled at higher level */

#ifdef UDEBUG
            printmsg(7, "data=|%.*s|", len, data);
#endif
         break;

      default:
         printmsg(0,"gspack: Invalid packet type %d",type);
         panic();
   } /* switch */

/*--------------------------------------------------------------------*/
/*    Now we finish up the header.  For data packets, determine       */
/*    the K number in header[1], which specifies the number of        */
/*    actual data bytes transmitted as a power of 2; we also          */
/*    compute a checksum on the data.                                 */
/*--------------------------------------------------------------------*/

   if (type == DATA)
   {
      header[1] = 1;
      i = MINPKT;
      while( i < xmit )
      {
         header[1] ++;
         i *= 2;
      }

      if ( i != xmit )        /* Did it come out exact power of 2?   */
      {
         printmsg(0,"Packet length error ... %d != %d for K = %d",
               i, xmit, (int) header[1]);
         panic();             /* No --> Well, we blew THAT math      */
      } /* if ( i != xmit ) */

/*--------------------------------------------------------------------*/
/*                        Compute the checksum                        */
/*--------------------------------------------------------------------*/

      check = checksum(data, xmit);
      i = header[4]; /* got to do this on PC for ex-or high bits */
      i &= 0xff;
      check = (check ^ i) & 0xffff;
      check = (0xaaaa - check) & 0xffff;
   }
   else {
      header[1] = 9;          /* Control packet size K number (9)    */
      check = (0xaaaa - header[4]) & 0xffff;
                              /* Simple checksum for control         */
   } /* else */

   header[2] = (unsigned char) (check & 0xff);
   header[3] = (unsigned char) ((check >> 8) & 0xff);
   header[5] = (unsigned char)
            ((header[1] ^ header[2] ^ header[3] ^ header[4]) & 0xff) ;

#ifdef   LINKTEST
   /***** More Link Testing Mods *****/
   switch(dpkerr[0]) {
   case 'e':
      data[10] = - data[10];
      break;
   case 'h':
      header[5] = - header[5];
      break;
   case 'l':
      return;
   case 'p':
      swrite((char *) header, HDRSIZE);
      if (header[1] != 9)
         swrite(data, xmit - 3);
      return;
   default:
      break;
   }
   /***** End Link Testing Mods *****/
#endif   /* LINKTEST */

   swrite((char *) header, HDRSIZE);      /* header is 6-bytes long */
   if (header[1] != 9)
      swrite(data, xmit);           /* data is always 64 bytes long */

} /*gspack*/


/*
   g r p a c k

   Read packet

   on return: yyy=pkrec xxx=pksent len=length<=PKTSIZE  data=*data

   ret(type)   ok
   ret(EMPTY)  input buf empty
   ret(ERROR)  bad header

   ret(EMPTY)  lost packet timeout
   ret(ERROR)  checksum error
   ret(-5)     packet size != 64

   NOTE (specifications for sread()):

   sread(buf, n, timeout)
      while(TRUE) {
         if (# of chars available >= n) (without dec internal counter)
            read n chars into buf (decrement internal char counter)
            break
    else
       if (time > timeout)
          break;
      }
      return(# of chars available)

*/

static int grpack(int *yyy,
                  int *xxx,
                  int *len,
                  char *data,
                  const int timeout)
{
   static int got_hdr  = FALSE;
   static int received = 0;     /* Bytes already read into buffer */
   int needed;

   unsigned int type, check, checkchk, i, total = 0;
   unsigned char c, c2;

   time_t start;

   if (got_hdr)
      goto get_data;

/*--------------------------------------------------------------------*/
/*   Spin up to timeout waiting for a Control-P, our sync character   */
/*--------------------------------------------------------------------*/

   start = 0;
   while (!got_hdr)
   {
      unsigned char *psync;

      needed = HDRSIZE - received;
      if ( needed > 0 )       /* Have enough bytes for header?       */
      {                       /* No --> Read as much as we need      */
         int wait;

         if ( start == 0 )    /* First pass through data?            */
         {                    /* Yes --> Set timers up               */
            start = time(nil(time_t));
            wait = timeout;
         } /* if ( start == 0 ) */
         else {
            wait = (int) (time(NULL) - start) - timeout;
            if (wait < 0)     /* Negative timeout?                   */
               wait = 0;      /* Make it no time out                 */
         } /* else */

         if (sread((char *) &grpkt[received], needed, wait ) < (unsigned) needed )
                              /* Did we get the needed data?         */
            return EMPTY;     /* No --> Return to caller             */

         received += needed;
      } /* if ( needed < received ) */

/*--------------------------------------------------------------------*/
/*            Search for sync character in newly read data            */
/*--------------------------------------------------------------------*/

#ifdef UDEBUG
      printmsg(10,"grpack: Have %d characters after reading %d",
               received, needed);
#endif

      psync = memchr( grpkt, '\020', received );
      if ( psync == NULL )    /* Did we find the sync character?     */
         received = 0;        /* No --> Reset to empty buffer        */
      else if ( psync != grpkt ) /* First character in buffer?       */
      {                       /* No --> Make it first character      */
         received -= psync - grpkt;
         shifts++;
         memmove( grpkt, psync, received );
                              /* Shift buffer over                   */
      } /* else */

/*--------------------------------------------------------------------*/
/*    If we have read an entire packet header, then perform a         */
/*    simple XOR checksum to determine if it is valid.  If we have    */
/*    a valid checksum, drop out of this search, else drop the        */
/*    sync character and restart the scan.                            */
/*--------------------------------------------------------------------*/

      if ( received >= HDRSIZE )
      {
         i = (unsigned) (grpkt[1] ^ grpkt[2] ^ grpkt[3] ^
                        grpkt[4] ^ grpkt[5]);
         i &= 0xff;
         printmsg(i ? 2 : 10, "prpkt %02x %02x %02x %02x %02x .. %02x ",
            grpkt[1], grpkt[2], grpkt[3], grpkt[4], grpkt[5], i);

         if (i == 0)          /* Good header?                        */
            got_hdr = TRUE;   /* Yes --> Drop out of loop            */
         else {               /* No  --> Flag it, continue loop      */
            badhdr++;
            printmsg(GDEBUG, "*** bad pkt header ***");
            memmove( grpkt, &grpkt[ 1 ], --received );
                              /* Begin scanning for sync character
                                 with next byte                      */
         } /* else */
      } /* if ( received > HDRSIZE ) */
   } /* while */

/*--------------------------------------------------------------------*/
/*                       Handle control packets                       */
/*--------------------------------------------------------------------*/

   if (grpkt[1] == 9)
   {
      if ( data != NULL )
         *data = '\0';
      *len = 0;
      c = grpkt[4];
      type = c >> 3;
      *yyy = c & 0x07;
      *xxx = 0;
      check = 0;
      checkchk = 0;
      got_hdr = FALSE;
   }

/*--------------------------------------------------------------------*/
/*                        Handle data packets                         */
/*--------------------------------------------------------------------*/
   else {
get_data:
      if ( data == NULL )
      {
         printmsg(0,"grpack: Unexpected data packet!");
         received = 0;
         return(ERROR);
      }

/*--------------------------------------------------------------------*/
/*             Compute the size of the data block desired             */
/*--------------------------------------------------------------------*/

      total = 8 * (2 << grpkt[1]);
      if (total > pktsize)  /* Within the defined limits?          */
      {                       /* No --> Other system has bad header,
                                 or the header got corrupted         */
         printmsg(0,"grpack: Invalid packet size %d (%d)",
            total, (int) grpkt[1]);
         received = 0;
         got_hdr = FALSE;
         return(ERROR);
      }

      needed = total + HDRSIZE - received;
                                 /* Compute byte required to fill
                                    data buffer                      */

/*--------------------------------------------------------------------*/
/*     If we don't have enough data in the buffer, read some more     */
/*--------------------------------------------------------------------*/

      if ((needed > 0) &&
          (sread((char *) &grpkt[HDRSIZE+total-needed], needed , timeout) < (unsigned)needed))
         return(EMPTY);

      got_hdr = FALSE;           /* Must re-process header next pass */

/*--------------------------------------------------------------------*/
/*              Break packet header into various values               */
/*--------------------------------------------------------------------*/

      type = 0;
      c2 = grpkt[4];
      c = (unsigned char) (c2 & 0x3f);
      *xxx = c >> 3;
      *yyy = c & 0x07;
      i = grpkt[3];
      i = (i << 8) & 0xff00;
      check = grpkt[2];
      check = i | (check & 0xff);
      checkchk = checksum( (char *) grpkt + HDRSIZE , total);
      i = grpkt[4] | 0x80;
      i &= 0xff;
      checkchk = 0xaaaa - (checkchk ^ i);
      checkchk &= 0xffff;
      if (checkchk != check)
      {
         printmsg(4, "*** checksum error ***");
         memmove( grpkt, grpkt + HDRSIZE, total );
                              /* Save data so we can scan for sync   */
         received = total;    /* Note the amount of the data in buf  */
         return(ERROR);       /* Return to caller with error         */
      }

/*--------------------------------------------------------------------*/
/*    The checksum is correct, now determine the length of the        */
/*    data to return.                                                 */
/*--------------------------------------------------------------------*/

      *len = total;

      if (c2 & 0x40)
      {
         int ii;
         if ( grpkt[HDRSIZE] & 0x80 )
         {
            ii = (grpkt[HDRSIZE] & 0x7f) + ((grpkt[HDRSIZE+1] & 0xff) << 7);
            *len -= ii;
            memcpy(data, grpkt + HDRSIZE + 2, *len);
         }
         else {
            ii = (grpkt[HDRSIZE] & 0xff);
            *len -= ii;
            memcpy(data, grpkt + HDRSIZE + 1, *len);
         } /* else */
      }
      else
         memcpy( data, grpkt + HDRSIZE, *len);
   } /* else */

/*--------------------------------------------------------------------*/
/*           Announce what we got and return to the caller            */
/*--------------------------------------------------------------------*/

   received = 0;              /* Returning entire buffer, reset count */
   printmsg(5, "receive packet type %d, yyy=%d, xxx=%d, len=%d",
      type, *yyy, *xxx, *len);

#ifdef UDEBUG
   printmsg(13, " checksum rec=%04x comp=%04x\ndata=|%.*s|",
      check, checkchk, total, grpkt + HDRSIZE);
#endif

   return(type);

} /*grpack*/


/*
   c h e c k s u m
*/

static unsigned int checksum(char *data, int len)
{
   int i, j;
   unsigned int tmp, chk1, chk2;
   chk1 = 0xffff;
   chk2 = 0;
   j = len;
   for (i = 0; i < len; i++) {
      if (chk1 & 0x8000) {
         chk1 <<= 1;
         chk1++;
      } else {
         chk1 <<= 1;
      }
      tmp = chk1;
      chk1 += (data[i] & 0xff);
      chk2 += chk1 ^ j;
      if ((chk1 & 0xffff) <= (tmp & 0xffff))
         chk1 ^= chk2;
      j--;
   }
   return(chk1 & 0xffff);

} /*checksum*/
