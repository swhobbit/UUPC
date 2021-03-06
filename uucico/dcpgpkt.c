/*--------------------------------------------------------------------*/
/*       d c p g p k t . c                                            */
/*                                                                    */
/*       UUCP 'g' protocol module for UUPC/extended.  Supports 7      */
/*       window variable length packets of up to MAXPACK bytes in     */
/*       length.                                                      */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Stuart Lynne May/87                                          */
/*                                                                    */
/*       Copyright (c) Richard H. Lamb 1985, 1986, 1987               */
/*       Changes Copyright (c) Stuart Lynne 1987                      */
/*                                                                    */
/*    Changes Copyright (c) 1989-2002 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *      $Id: dcpgpkt.c 1.45 2001/03/12 13:54:49 ahd v1-13k $
 *
 *      $Log: dcpgpkt.c $
 *      Revision 1.45  2001/03/12 13:54:49  ahd
 *      Annual copyright update
 *
 *      Revision 1.44  2000/05/12 12:32:00  ahd
 *      Annual copyright update
 *
 *      Revision 1.43  1999/01/08 02:20:56  ahd
 *      Convert currentfile() to RCSID()
 *
 *      Revision 1.42  1999/01/04 03:53:30  ahd
 *      Annual copyright change
 *
 *      Revision 1.41  1998/03/01 01:39:26  ahd
 *      Annual Copyright Update
 *
 *      Revision 1.40  1997/04/24 01:33:32  ahd
 *      Annual Copyright Update
 *
 *      Revision 1.39  1996/01/01 21:20:39  ahd
 *      Annual Copyright Update
 *
 *      Revision 1.38  1995/01/29 14:08:40  ahd
 *      Use long counters to avoid overflow on really lousy phone lines
 *
 *      Revision 1.37  1995/01/07 16:38:17  ahd
 *      Change boolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *      Revision 1.36  1994/12/22 00:34:53  ahd
 *      Annual Copyright Update
 *
 *      Revision 1.35  1994/06/13 00:12:05  ahd
 *      Correct length of error variable in messages
 *
 *        Revision 1.34  1994/05/23  22:45:10  ahd
 *        Drop use of second variable for buffer tracking
 *
 *        Revision 1.33  1994/05/08  02:43:07  ahd
 *        Handle carrier detect option internal to CD()
 *
 *        Revision 1.32  1994/02/19  05:09:02  ahd
 *        Use standard first header
 *
 * Revision 1.31  1994/02/16  02:26:27  ahd
 * Delete unneeded allocation of gspkt
 * Move grpkt to FAR memory
 *
 * Revision 1.30  1994/02/13  13:51:22  rommel
 * Correct 32 bit ifdef
 *
 * Revision 1.29  1994/01/01  19:18:41  ahd
 * Annual Copyright Update
 *
 * Revision 1.28  1993/12/26  16:20:17  ahd
 * Suppress compile warning
 *
 * Revision 1.27  1993/12/24  05:12:54  ahd
 * Use far buffer for master communications buffer
 *
 * Revision 1.26  1993/12/23  03:16:03  rommel
 * OS/2 32 bit support for additional compilers
 *
 * Revision 1.25  1993/12/02  13:49:58  ahd
 * 'e' protocol support
 *
 * Revision 1.24  1993/12/02  03:59:37  dmwatt
 * 'e' protocol support
 *
 * Revision 1.23  1993/10/24  12:48:56  ahd
 * Correct timeouts to user specified, rather than double user specified
 *
 * Revision 1.22  1993/10/13  01:47:46  ahd
 * Don't NAK upon startup
 *
 * Revision 1.21  1993/10/12  01:32:08  ahd
 * Normalize comments to PL/I style
 *
 * Revision 1.20  1993/10/09  22:21:55  rhg
 * ANSIfy source
 *
 * Revision 1.18  1993/10/04  03:57:20  ahd
 * Drop new lines from start up message
 *
 * Revision 1.17  1993/10/02  22:56:59  ahd
 * Suppress compile warning
 *
 * Revision 1.16  1993/10/02  19:07:49  ahd
 * Treat Windows 3.x ala DOS in handling packets in far memory
 *
 * Revision 1.15  1993/09/20  04:41:54  ahd
 * OS/2 2.x support
 *
 * Revision 1.14  1993/07/22  23:22:27  ahd
 * First pass at changes for Robert Denny's Windows 3.1 support
 *
 * Revision 1.13  1993/05/30  00:01:47  ahd
 * Move UUFAR into header file
 *
 * Revision 1.12  1993/04/13  03:19:45  ahd
 * Only perform copy to gspkt if input to gsendpkt is non-null
 *
 * Revision 1.11  1993/04/13  03:00:05  ahd
 * Correct gspkt declare
 *
 * Revision 1.10  1993/04/13  02:26:30  ahd
 * Move buffers to FAR memory
 *
 * Revision 1.9  1993/04/05  04:32:19  ahd
 * Allow unique send and receive packet sizes
 *
 * Revision 1.8  1993/03/06  23:04:54  ahd
 * Make state names more descriptive
 *
 * Revision 1.7  1992/11/21  05:55:11  ahd
 * Use single bit field for gopenpk flag bits, add debugging info
 *
 * Revision 1.6  1992/11/20  12:38:39  ahd
 * Add additional flags to avoid prematurely ending init sequence
 *
 * Revision 1.5  1992/11/19  03:00:29  ahd
 * drop rcsid
 *
 * Revision 1.4  1992/11/17  13:45:37  ahd
 * Add comments from Ian Talyor
 *
 * Revision 1.3  1992/11/16  02:10:27  ahd
 * Rewrite protocol initialize to insure full exchange of packets
 *
 *          25Aug87 - Allow for up to 7 windows - Jal
 *
 *          01Nov87 - those strncpy's should really be memcpy's! -
 *                    Jal
 *
 *          11Sep89 - Raise TimeOut to 15 - ahd
 *
 *          30Apr90 - Add Jordon Brown's fix for short packet
 *                    retries. Reduce retry limit to 20
 *
 *          22Jul90 - Change error retry limit from per host to per
 *                    packet.
 *
 *          22Jul90 - Add error message for number of retries
 *                    exceeded
 *
 *          08Sep90 - Drop memmove to memcpy change supplied by
 *                    Jordan Brown, MS 6.0 and Turbo C++ agree
 *                    memmove insures no overlap
 */

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

#include "uupcmoah.h"

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

#include "dcp.h"
#include "dcpsys.h"
#include "dcpgpkt.h"
#include "hostable.h"
#include "security.h"
#include "commlib.h"
#include "modem.h"
#include "catcher.h"

/*--------------------------------------------------------------------*/
/*                           Local defines                            */
/*--------------------------------------------------------------------*/

#define PKTSIZE   MAXPACK
#define MINPKT    32

#define HDRSIZE   6
#define MAXTRY    4

#ifndef GDEBUG
#define GDEBUG 4
#endif

#ifdef __BORLANDC__
#pragma warn -sig
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
      I_EMPTY,
      I_ERROR,
      I_RESTART,
      I_CALLEE,
      I_CALLER,
      I_GRPACK,
      I_INITA_RECV,
      I_INITA_SEND,
      I_INITB_RECV,
      I_INITB_SEND,
      I_INITC_RECV,
      I_INITC_SEND,
      I_COMPLETE
      } I_STATE;

#define between(a,b,c) ((a<=b && b<c) || \
                        (c<a && a<=b) || \
                        (b<c && c<a))

#define nextpkt(x)    ((x + 1) % MAXSEQ)

/*--------------------------------------------------------------------*/
/*              Global variables for packet definitions               */
/*--------------------------------------------------------------------*/

RCSID("$Id: dcpgpkt.c 1.45 2001/03/12 13:54:49 ahd v1-13k $");

static short irec, lazynak;
static unsigned rwl, rwu, swl, swu;
static unsigned nbuffers;
static KEWSHORT nerr;
static unsigned short outlen[NBUF], inlen[NBUF], xmitlen[NBUF];
static KWBoolean arrived[NBUF];
static unsigned nwindows;

static char UUFAR outbuf[NBUF][MAXPACK];
static char UUFAR inbuf[NBUF][MAXPACK];
static time_t ftimer[NBUF];
static long timeouts, outsequence, naksin, naksout, screwups;
static long reinit, shifts, badhdr, resends;

static KWBoolean variablepacket;  /* "v" or in modem file             */

static time_t idletimer = 0;

/*--------------------------------------------------------------------*/
/*                    Internal function prototypes                    */
/*--------------------------------------------------------------------*/

static short initialize(const KWBoolean caller, const char protocol );

static short  gmachine(const short timeout);

static void gspack(short type,
                   unsigned yyy,
                   unsigned xxx,
                   unsigned len,
                   unsigned xmit,
                   char UUFAR *data);

static short  grpack(unsigned *yyy,
                     unsigned *xxx,
                     unsigned *len,
                     char UUFAR *data,
                     const short timeout);

static void gstats( void );

static unsigned checksum(const char UUFAR *data, unsigned len);

/****************** SUB SUB SUB PACKET HANDLER ************/

/*--------------------------------------------------------------------*/
/*    g o p e n p k                                                   */
/*                                                                    */
/*    Initialize processing for protocol                              */
/*--------------------------------------------------------------------*/

short Gopenpk(const KWBoolean caller)
{
   return initialize(caller , 'G');
} /* Gopenpk */

/*--------------------------------------------------------------------*/
/*    v o p e n p k                                                   */
/*                                                                    */
/*    Initialize processing for protocol                              */
/*--------------------------------------------------------------------*/

short vopenpk(const KWBoolean caller)
{
   return initialize(caller, 'v');
} /* vopenpk */

/*--------------------------------------------------------------------*/
/*    g o p e n p k                                                   */
/*                                                                    */
/*    Initialize processing for protocol                              */
/*--------------------------------------------------------------------*/

short gopenpk(const KWBoolean caller)
{
   return initialize(caller, 'g');
} /* vopenpk */

/*--------------------------------------------------------------------*/
/*    i n i t i a l i z e                                             */
/*                                                                    */
/*    Initialize processing for protocol                              */
/*--------------------------------------------------------------------*/

static short initialize(const KWBoolean caller, const char protocol )
{
   unsigned i, xxx, yyy, len, maxwindows;

#define B_SENT_INITA 0x01
#define B_SENT_INITB 0x02
#define B_SENT_INITC 0x04
#define B_RECV_INITA 0x10
#define B_RECV_INITB 0x20
#define B_RECV_INITC 0x40
#define B_INITA (B_SENT_INITA | B_RECV_INITA)
#define B_INITB (B_SENT_INITB | B_RECV_INITB)
#define B_INITC (B_SENT_INITC | B_RECV_INITC)

   unsigned flags = 0x00;   /* Init state flags, as defined above  */

   I_STATE state;

/*--------------------------------------------------------------------*/
/*    Read modem file values for the number of windows and packet     */
/*    sizes                                                           */
/*--------------------------------------------------------------------*/

   r_pktsize = s_pktsize = GetGPacket( MAXPACK, protocol );
   maxwindows = GetGWindow(
                     (KEWSHORT) min( MAXWINDOW, RECV_BUF / (s_pktsize+HDRSIZE)),
                     protocol);

   variablepacket = bmodemflag[MODEM_VARIABLEPACKET] || (protocol == 'v');

/*--------------------------------------------------------------------*/
/*                     Initialize error counters                      */
/*--------------------------------------------------------------------*/

   timeouts = outsequence = naksin = naksout = screwups =
      shifts = badhdr = resends = reinit = 0;

/*--------------------------------------------------------------------*/
/*                    Initialize proto parameters                     */
/*--------------------------------------------------------------------*/

   nerr = 0;
   nbuffers = 0;
   swl = swu = 1;
   rwl = 0;
   nwindows = maxwindows;
   rwu = nwindows - 1;

   for (i = 0; i < NBUF; i++)
   {
      ftimer[i] = 0;
      arrived[i] = KWFalse;
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
/*    Exchange initialization messages with the other system.         */
/*                                                                    */
/*    A problem:                                                      */
/*                                                                    */
/*    We send INITA; it gets received                                 */
/*    We receive INITA                                                */
/*    We send INITB; it gets garbled                                  */
/*    We receive INITB                                                */
/*                                                                    */
/*    We have seen and sent INITB, so we start to send INITC.  The    */
/*    other side as sent INITB but not seen it, so it times out       */
/*    and resends INITB.  We will continue sending INITC and the      */
/*    other side will continue sending INITB until both sides give    */
/*    up and start again with INITA.                                  */
/*                                                                    */
/*    It might seem as though if we are sending INITC and receive     */
/*    INITB, we should resend our INITB, but this could cause         */
/*    infinite echoing of INITB on a long-latency line.  Rather       */
/*    than risk that, I have implemented a fast drop-back             */
/*    procedure.  If we are sending INITB and receive INITC, the      */
/*    other side has gotten ahead of us.  We immediately fail and     */
/*    begin again with INITA.  For the other side, if we are          */
/*    sending INITC and see INITA, we also immediately fail back      */
/*    to INITA.                                                       */
/*                                                                    */
/*    Unfortunately, this doesn't work for the other case, in         */
/*    which we are sending INITB but the other side has not yet       */
/*    seen INITA.  As far as I can see, if this happens we just       */
/*    have to wait until we time out and resend INITA.                */
/*                                                                    */
/*    (The above also quoted verbatim from Ian Taylor; however, the   */
/*    code and associated bugs are all Drew's)                        */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    A note about the games with the variable "flags", which is      */
/*    bit twiddled alot below.  The statement below:                  */
/*                                                                    */
/*          flags = (flags & B_SENT_INITA) | B_RECV_INITA;            */
/*                                                                    */
/*    works to turn off all the bits in flags except B_SENT_INITA     */
/*    if it was already on, and then turns on flag B_RECV_INITA.      */
/*    We use statements like this to reset most of the flags at       */
/*    once, leaving one or two bits on; this in turn allows us to     */
/*    check in later states that the previous two states were the     */
/*    expected ones.                                                  */
/*                                                                    */
/*    Likewise, the following statement:                              */
/*                                                                    */
/*          state = (flags & B_SENT_INITA) ?                          */
/*                         I_INITB_SEND : I_INITA_SEND;               */
/*                                                                    */
/*    tests to see if B_SENT_INITA was already set, and return the    */
/*    "true" condition (I_INITB_SEND) otherwise return the "false"    */
/*    (I_INITA_SEND).                                                 */
/*--------------------------------------------------------------------*/

   while( state != I_COMPLETE )
   {
      printmsg(4, "gopenpk: I State = %2d, flag = 0x%02x",
               (int) state, (int) flags);

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
                  state = I_INITA_RECV;
                  break;

               case INITB:
                  printmsg(5, "**got INITB");
                  state = I_INITB_RECV;
                  break;

               case INITC:
                  printmsg(5, "**got INITC");
                  state = I_INITC_RECV;
                  break;

               case DCP_EMPTY:
                  printmsg(GDEBUG, "**got EMPTY");
                  state = I_EMPTY;

                  if (!CD())
                  {
                     printmsg(0,"gopenpk: Modem carrier lost");
                     return DCP_FAILED;
                  }
                  break;

               case CLOSE:
                  printmsg(GDEBUG, "**got CLOSE");
                  gspack(CLOSE, 0, 0, 0, 0, NULL);
                  return DCP_FAILED;

               default:
                  printmsg(GDEBUG, "**got SCREW UP");
                  state = I_ERROR;
                  break;
            }

            if ( !CD())
            {
               printmsg(0,"gopenpk: Modem carrier lost");
               return DCP_FAILED;
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

         case I_INITA_RECV:
            if (yyy < nwindows)
            {
               nwindows = yyy;
               rwu = nwindows - 1;
            }
            flags = (flags & B_SENT_INITA) | B_RECV_INITA;
            state = (flags & B_SENT_INITA) ? I_INITB_SEND : I_INITA_SEND;
            break;

         case I_INITA_SEND:
            gspack(INITA, 0, 0, 0, nwindows, NULL);
            flags = (flags & B_RECV_INITA) | B_SENT_INITA;
            state = I_GRPACK;
            break;

         case I_INITB_RECV:
            if ((flags & (B_RECV_INITA | B_SENT_INITA)) ==
                         (B_RECV_INITA | B_SENT_INITA))
            {
               i = 32 << yyy;
               if (i < s_pktsize)
                  s_pktsize = i;
               flags = (flags & B_SENT_INITB) | B_RECV_INITB;
               state = (flags & B_SENT_INITB) ? I_INITC_SEND : I_INITB_SEND;
            } /* if */
            else
               state = I_RESTART;
            break;

         case I_INITB_SEND:
            gspack(INITB, 0, 0, 0, r_pktsize, NULL);
                                       /* Data segment (packet) size   */
            flags = (flags & (B_INITA | B_RECV_INITB)) | B_SENT_INITB;
            state = I_GRPACK;
            break;

         case I_INITC_RECV:
            if ((flags & (B_RECV_INITB | B_SENT_INITB)) ==
                           (B_RECV_INITB | B_SENT_INITB))
            {
               if (yyy < nwindows)
               {
                  printmsg(0,"Unexpected INITC window size of %d",
                             nwindows );
                  nwindows = yyy;
                  rwu = nwindows - 1;
               }
               flags = (flags & B_SENT_INITC) | B_RECV_INITC;
               state = (flags & B_SENT_INITC) ? I_COMPLETE : I_INITC_SEND;
            }
            else
               state = I_RESTART;
            break;

         case I_INITC_SEND:
            gspack(INITC, 0, 0, 0, nwindows, NULL);
            flags = (flags & (B_INITB | B_RECV_INITC)) | B_SENT_INITC;
            state = (flags & B_RECV_INITC) ? I_COMPLETE : I_GRPACK;
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
            flags = 0x00;
            state = I_INITA_SEND;
            break;

      } /* switch */

      if ( terminate_processing )
      {
         printmsg(0,"gopenpk: Terminated by user");
         return DCP_FAILED;
      }

      if (nerr >= M_MaxErr)
      {
         remote_stats.errors += nerr;
         nerr = 0;
         printmsg(0,
            "gopenpk: Consecutive error limit of %ld exceeded, "
                     "%ld total errors",
             (long) M_MaxErr, remote_stats.errors);
         return(DCP_FAILED);
      }
   } /* while */

/*--------------------------------------------------------------------*/
/*                    Allocate the needed buffers                     */
/*--------------------------------------------------------------------*/

   nerr = 0;
   lazynak = 0;

#if defined(BIT32ENV) || defined(_Windows)
   printmsg(2,"%s packets, "
              "Window size %d, "
              "Receive packet %d, "
              "Send packet %d",
            variablepacket ? "Variable" : "Fixed",
            nwindows,
            r_pktsize,
            s_pktsize );
#else
   printmsg(2,"%s packets, "
              "Window size %d, "
              "Receive packet %d, "
              "Send packet %d, "
              "Memory avail %u",
            variablepacket ? "Variable" : "Fixed",
            nwindows,
            r_pktsize,
            s_pktsize,
            memavail());
#endif

   time( &idletimer );              /* Flag our link has been used   */

   return(DCP_OK);                  /* Channel open to caller        */

} /* initialize */

/*--------------------------------------------------------------------*/
/*    g f i l e p k t                                                 */
/*                                                                    */
/*    Begin a file transfer (not used by "g" protocol)                */
/*--------------------------------------------------------------------*/

#ifdef __TURBOC__
#pragma argsused
#endif

short gfilepkt( const KWBoolean send, const unsigned long len)
{

   return DCP_OK;

} /* gfilepkt */

/*--------------------------------------------------------------------*/
/*    g c l o s e p k                                                 */
/*                                                                    */
/*    Close packet machine                                            */
/*--------------------------------------------------------------------*/

short gclosepk()
{
   unsigned i;

   for (i = 0; i < MAXTRY; i++)
   {
      gspack(CLOSE, 0, 0, 0, 0, NULL);
      if (gmachine(M_gPacketTimeout) == CLOSE)
         break;
   } /* for (i = 0; i < MAXTRY; i++) */

/*--------------------------------------------------------------------*/
/*                Report the results of our adventures                */
/*--------------------------------------------------------------------*/

   gstats();

/*--------------------------------------------------------------------*/
/*                          Return to caller                          */
/*--------------------------------------------------------------------*/

   return(0);

} /* gclosepk */

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
         "%ld time outs, %ld port reinits, %ld out of seq pkts, "
         "%ld NAKs rec, %ld NAKs sent",
            (long) timeouts,
            (long) reinit,
            (long) outsequence,
            (long) naksin,
            (long) naksout);
      printmsg(0,
         "%ld invalid pkt types, %ld re-syncs, %ld bad pkt hdrs, %ld pkts resent",
            (long) screwups,
            (long) shifts,
            (long) badhdr,
            (long) resends);
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

short ggetpkt(char *data, short *len)
{
   short   retry = M_MaxErr;
   time_t start;
#ifdef _DEBUG
   short savedebug = debuglevel;
#endif

   irec = 1;
   checkref( data );

/*--------------------------------------------------------------------*/
/*                Loop to wait for the desired packet                 */
/*--------------------------------------------------------------------*/

   time( &start );
   while (!arrived[rwl] && retry)
   {
      if (gmachine(M_gPacketTimeout) != POK)
         return(-1);

      if (!arrived[rwl] )
      {
         time_t now;

         if (time( &now ) >= (start + M_gPacketTimeout) )
         {
#ifdef _DEBUG
            if ( debuglevel < 6 )
               debuglevel = 6;
#endif
            printmsg(GDEBUG,
                     "ggetpkt: Timeout %ld waiting for inbound packet %ld",
                     (long) M_MaxErr - --retry,
                     (long) remote_stats.packets + 1);
            timeouts++;
            start = now;
         } /* if (time( now ) > (start + M_gPacketTimeout) ) */

      } /* if (!arrived[rwl] ) */

   } /* while (!arrived[rwl] && retry) */

#ifdef _DEBUG
   debuglevel = savedebug;
#endif

   if (!arrived[rwl])
   {
      printmsg(0,"ggetpkt: Remote host failed to respond after %ld seconds",
               (long) M_gPacketTimeout * M_MaxErr);
      gclosepk();
      return -1;
   }

/*--------------------------------------------------------------------*/
/*                           Got a packet!                            */
/*--------------------------------------------------------------------*/

   *len = inlen[rwl];
   MEMCPY(data, inbuf[rwl], *len);

   arrived[rwl] = KWFalse;     /* Buffer is now emptied               */
   rwu = nextpkt(rwu);        /* bump receive window                 */

   return(0);

} /* ggetpkt */

/*--------------------------------------------------------------------*/
/*       g s e n d p k t                                              */
/*                                                                    */
/*       Put at most a packet's worth of data in the packet state     */
/*       machine for transmission.  May have to run the packet        */
/*       machine a few times to get an available output slot.         */
/*                                                                    */
/*       on input: data=*data; len=length of data in data.            */
/*                                                                    */
/*       return:                                                      */
/*        0 if all's well                                             */
/*       -1 if problems (failed)                                      */
/*--------------------------------------------------------------------*/

short gsendpkt(char *data, short len)
{
   int delta;
#ifdef _DEBUG
   short savedebug = debuglevel;
#endif

   checkref( data );
   irec = 0;

#ifdef UDEBUG
   printmsg(5,"Sending %d bytes from %p", (int) len, data);
#endif

/*--------------------------------------------------------------------*/
/*       WAIT FOR INPUT i.e. if we have sent SWINDOW pkts and none    */
/*       have been acked, wait for acks.  Note that we always go      */
/*       through the machine at least once to keep caught up with     */
/*       ACK's sent by remote machine.                                */
/*--------------------------------------------------------------------*/

   if ( nbuffers )
   do {
      if (gmachine(0) != POK)    /* Spin with no timeout             */
         return(-1);
   } while (nbuffers >= nwindows);

/*--------------------------------------------------------------------*/
/*               Place packet in table and mark unacked               */
/*--------------------------------------------------------------------*/

   MEMCPY(outbuf[swu], data, len);

/*--------------------------------------------------------------------*/
/*                       Handle short packets.                        */
/*--------------------------------------------------------------------*/

   xmitlen[swu] = (unsigned short) s_pktsize;
   if (variablepacket)
      while ( ((len * 2) < (short) xmitlen[swu]) && (xmitlen[swu] > MINPKT) )
         xmitlen[swu] /= 2;

   if ( xmitlen[swu] < MINPKT )
   {
      printmsg(0,"gsendpkt: Bad packet size %d, "
               "data length %d",
               xmitlen[swu], len);
      xmitlen[swu] = MINPKT;
   }

   delta = xmitlen[swu] - len;
   if (delta > 127)
   {
      MEMMOVE(outbuf[swu] + 2, outbuf[swu], len);
      MEMSET(outbuf[swu]+len+2, 0, delta - 2);
                              /* Pad with nulls.  Ugh.               */
      outbuf[swu][0] = (unsigned char) ((delta & 0x7f) | 0x80);
      outbuf[swu][1] = (unsigned char) (delta >> 7);
   } /* if (delta > 127) */
   else if (delta > 0 )
   {
      MEMMOVE(outbuf[swu] + 1, outbuf[swu], len);
      outbuf[swu][0] = (unsigned char) delta;
      MEMSET(outbuf[swu]+len+1, 0, delta - 1);
                              /* Pad with nulls.  Ugh.               */
   } /* else if (delta > 0 )  */

/*--------------------------------------------------------------------*/
/*                            Mark packet                             */
/*--------------------------------------------------------------------*/

   outlen[swu] = len;
   ftimer[swu] = time(nil(time_t));
   nbuffers++;

/*--------------------------------------------------------------------*/
/*                              send it                               */
/*--------------------------------------------------------------------*/

   gspack(DATA, rwl, swu, outlen[swu], xmitlen[swu], outbuf[swu]);

   swu = nextpkt(swu);        /* Bump send window                    */

#ifdef _DEBUG
   debuglevel = savedebug;
#endif

   return(0);

} /* gsendpkt */

/*--------------------------------------------------------------------*/
/*    g e o f p k t                                                   */
/*                                                                    */
/*    Transmit EOF to the other system                                */
/*--------------------------------------------------------------------*/

short geofpkt( void )
{
   if ((*sendpkt)("", 0))          /* Empty packet == EOF             */
      return DCP_FAILED;
   else
      return DCP_OK;
} /* geofpkt */

/*--------------------------------------------------------------------*/
/*    g w r m s g                                                     */
/*                                                                    */
/*    Send a message to remote system                                 */
/*--------------------------------------------------------------------*/

short gwrmsg( char *s )
{
   for( ; strlen(s) >= s_pktsize; s += s_pktsize)
   {
      short result = (*sendpkt)(s, (short) s_pktsize);
      if (result)
         return result;
   }

   return (*sendpkt)(s, (short) (strlen(s) + 1));

} /* gwrmsg */

/*--------------------------------------------------------------------*/
/*    g r d m s g                                                     */
/*                                                                    */
/*    Read a message from the remote system                           */
/*--------------------------------------------------------------------*/

short grdmsg( char *s)
{
   for ( ;; )
   {
      short len;
      short result = (*getpkt)( s, &len );
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

static short gmachine(const short timeout )
{

   KWBoolean done  = KWFalse;   /* True = drop out of machine loop  */
   KWBoolean close  = KWFalse;  /* True = terminate connection upon
                                        exit                      */
   KWBoolean inseq  = KWTrue;   /* True = Count next out of sequence
                                        packet as an error           */
   while ( !done )
   {
      KWBoolean resend = KWFalse;  /* True = resend data packets       */
      KWBoolean donak  = KWFalse;  /* True = NAK the other system      */
      unsigned long packet_no = remote_stats.packets;

      short pkttype;
      unsigned rack, rseq, rlen, i1;
      time_t now;

      if ( debuglevel >= 10 )    /* Optimize processing a little bit */
      {

         printmsg(10, "* send %d < W < %d, "
                      "receive %d < W < %d, "
                      "error %d, packet %ld",
                      swl,
                      swu,
                      rwl,
                      rwu,
                      nerr,
                      (long) remote_stats.packets);

/*--------------------------------------------------------------------*/
/*    Waiting for ACKs for swl to swu-1.  Next pkt to send=swu        */
/*    rwl=expected pkt                                                */
/*--------------------------------------------------------------------*/

      }

/*--------------------------------------------------------------------*/
/*             Attempt to retrieve a packet and handle it             */
/*--------------------------------------------------------------------*/

      pkttype = grpack(&rack, &rseq, &rlen, inbuf[nextpkt(rwl)], timeout);
      time(&now);
      switch (pkttype)
      {

         case CLOSE:
            remote_stats.packets++;
            printmsg(GDEBUG, "**got CLOSE");
            close = done = KWTrue;
            break;

         case DCP_EMPTY:
            printmsg(timeout ? GDEBUG : 8, "**got EMPTY");

            if (!CD())
            {
               printmsg(0,"gmachine: Modem carrier lost");
               nerr++;
               close = KWTrue;
            }

            if ( terminate_processing )
            {
               printmsg(0,"gmachine: User aborted processing");
               close = KWTrue;
            }

            if (ftimer[swl])
            {
#ifdef UDEBUG
               printmsg(6, "---> seq, elapst %d %ld", swl,
                    ftimer[swl] - now);
#endif
               if ( ftimer[swl] <= (now - M_gPacketTimeout))
               {
                   printmsg(4, "*** timeout %d (%ld)",
                               swl, (long) remote_stats.packets);
                       /* Since "g" is "go-back-N", when we time out we
                          must send the last N pkts in order.  The generalized
                          sliding window scheme relaxes this reqirment. */
                   nerr++;
                   timeouts++;
                   resend = KWTrue;
               } /* if */
            } /* if */
            else if ( now >= (idletimer + M_gPacketTimeout))
               donak = KWTrue;

            done = KWTrue;
            break;

         case DATA:
            printmsg(5, "**got DATA %d %d", rack, rseq);
            i1 = nextpkt(rwl);   /* (R+1)%8 <-- -->(R+W)%8 */

            if (i1 == rseq)
            {
               lazynak--;
               remote_stats.packets++;
               idletimer = now;
               rwl = i1;
               inseq = arrived[rwl] = KWTrue;
               inlen[rwl] = (unsigned short) rlen;
               printmsg(5, "*** ACK d %d", rwl);
               gspack(ACK, rwl, 0, 0, 0, NULL);
               done = KWTrue;  /* return to caller when finished      */
                              /* in a mtask system, unneccesary      */
            }
            else {
               if (inseq || ( now >= (idletimer + M_gPacketTimeout)))
               {
                  donak = KWTrue;  /* Only flag first out of sequence
                                    packet as error, since we know
                                    following ones also bad            */
                  outsequence++;
                  inseq = KWFalse;
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
               resend = KWTrue;
            }
            else if (pkttype == ACK)
               printmsg(5, "**got ACK %d", rack);

            while(between(swl, rack, swu))
            {                             /* S<-- -->(S+W-1)%8 */
               remote_stats.packets++;
               printmsg(5, "*** ACK %d", swl);
               ftimer[swl] = 0;
               idletimer = now;
               nbuffers--;
               done = KWTrue;           /* Get more data for input */
               swl = nextpkt(swl);
            } /* while */

            if (!done && (pkttype == ACK)) /* Find packet?         */
            {
               printmsg(GDEBUG,"*** ACK for bad packet %d (%d - %d)",
                           rack, swl, swu);
            } /* if */
            break;

         case DCP_ERROR:
            printmsg(GDEBUG, "*** got BAD CHK");
            naksout++;
            donak = KWTrue;
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
      for (rack = swl;
           between(swl, rack, swu);
           rack = nextpkt(rack) )
      {                          /* resend rack->(swu-1)             */
         resends++;

         if ( outbuf[rack] == NULL )
         {
            printmsg(0,"gmachine: Transmit of NULL packet (%d %d)",
                     rwl, rack);
            panic();
         }

         if ( xmitlen[rack] == 0 )
         {
            printmsg(0,"gmachine: Transmit of 0 length packet (%d %d)",
                     rwl, rack);
            panic();
         }

         gspack(DATA, rwl, rack, outlen[rack], xmitlen[rack], outbuf[rack]);
         printmsg(5, "*** resent %d", rack);
         idletimer = ftimer[rack] = now;

      } /* for */

/*--------------------------------------------------------------------*/
/*       If we have an error and have not recently sent a NAK, do     */
/*       so now.  We then reset our counter so we receive at least    */
/*       a window full of packets before sending another NAK          */
/*--------------------------------------------------------------------*/

      if ( donak )
      {
         nerr++;
         if ( (lazynak < 1) || (now >= (idletimer + M_gPacketTimeout)))
         {
            printmsg(5, "*** NAK d %d", rwl);
            gspack(NAK, rwl, 0, 0, 0, NULL);
            naksout++;
            idletimer = now;
            lazynak = (short) (nwindows + 1);
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
         done = close = KWTrue;
         gstats();
      }
   } /* while */

/*--------------------------------------------------------------------*/
/*    Return to caller, gracefully terminating packet machine if      */
/*    requested                                                       */
/*--------------------------------------------------------------------*/

#ifdef UDEBUG
   printmsg(5,"gmachine: Exit %s", close ? "closed" : "open");
#endif

   if ( close )
   {
      gspack(CLOSE, 0, 0, 0, 0, NULL);
      return CLOSE;
   }
   else
      return POK;

} /* gmachine */

/*************** FRAMING *****************************/

/*
   g s p a c k

   Send a packet

   type=type yyy=pkrec xxx=timesent len=length<=s_pktsize data=*data
   ret(0) always
*/

static void gspack(short type,
                   unsigned yyy,
                   unsigned xxx,
                   unsigned len,
                   unsigned xmit,
                   char UUFAR *data)
{
   unsigned check, i;
   unsigned char header[HDRSIZE];

#ifdef   LINKTEST
   /***** Link Testing Mods *****/
   unsigned char dpkerr[10];
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

   switch (type)
   {

      case CLOSE:
         break;   /* stop protocol */

      case NAK:
         header[4] += (unsigned char) yyy;
         break;   /* reject */

      case SRJ:
         break;

      case ACK:
         header[4] += (unsigned char) yyy;
         break;   /* ack */

      case INITA:
      case INITC:
         header[4] += (unsigned char) xmit;
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
         if (len < xmit)              /* Short packet?              */
            header[4] |= 0x40;/* Count byte handled at higher level */

#ifdef UDEBUG
#ifdef BIT32ENV
            printmsg(7, "data=|%.*s|", len, data);
#else
            printmsg(7, "data=|%.*Fs|", len, data);
#endif
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
      i = header[4] & 0xff; /* got to do this on PC for ex-or high bits */
      check = (0xaaaa - ((check ^ i) & 0xffff)) & 0xffff;
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
   switch(dpkerr[0])
   {
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
      swrite(data, xmit);

} /* gspack */

/*--------------------------------------------------------------------*/
/*       g r p a c k                                                  */
/*                                                                    */
/*       Read packet                                                  */
/*                                                                    */
/*       on return:  yyy=pkrec                                        */
/*                   xxx=pksent                                       */
/*                   len=length<=PKTSIZE                              */
/*                   data=*data                                       */
/*                                                                    */
/*       ret(type)       ok                                           */
/*       ret(DCP_EMPTY)  input buf empty                              */
/*       ret(DCP_ERROR)  bad header                                   */
/*                                                                    */
/*       ret(DCP_EMPTY)  lost packet timeout                          */
/*       ret(DCP_ERROR)  checksum error                               */
/*                                                                    */
/*       NOTE (specifications for sread()):                           */
/*                                                                    */
/*          sread(buf, n, timeout)                                    */
/*          while(KWTrue)                                              */
/*          {                                                         */
/*             if (# of chars available >= n)                         */
/*                read n chars into buf                               */
/*                break                                               */
/*            else                                                    */
/*               if (time > timeout)                                  */
/*                  break;                                            */
/*          }                                                         */
/*          return(# of chars available)                              */
/*--------------------------------------------------------------------*/

static short grpack(unsigned *yyy,
                    unsigned *xxx,
                    unsigned *len,
                    char UUFAR *data,
                    const short timeout)
{
   static unsigned char UUFAR grpkt[MAXPACK+HDRSIZE];

   static short got_hdr  = KWFalse;
   static int received = 0;       /* Bytes already read into buffer */
   int needed;

   unsigned short type;
   unsigned check, checkchk;
   unsigned char i;
   unsigned total = 0;
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
      unsigned char UUFAR *psync;

      needed = HDRSIZE - received;
      if ( needed > 0 )       /* Have enough bytes for header?       */
      {                       /* No --> Read as much as we need      */
         short wait;

         if ( start == 0 )    /* First pass through data?            */
         {                    /* Yes --> Set timers up               */
            start = time(nil(time_t));
            wait = timeout;
         } /* if ( start == 0 ) */
         else {
            wait = (short) (timeout - (short) (time(NULL) - start));

            if (wait < 0)     /* Negative timeout?                   */
               wait = 0;      /* Make it no time out                 */
         } /* else */

         if (sread((char UUFAR *) grpkt + received, needed, wait ) <
             (unsigned short) needed )
                              /* Did we get the needed data?         */
            return DCP_EMPTY; /* No --> Return to caller             */

         received += needed;
      } /* if ( needed < received ) */

/*--------------------------------------------------------------------*/
/*            Search for sync character in newly read data            */
/*--------------------------------------------------------------------*/

#ifdef UDEBUG
      printmsg(10,"grpack: Have %d characters after reading %d",
               received, needed);
#endif

      psync = MEMCHR( grpkt, '\020', received );

      if ( psync == NULL )    /* Did we find the sync character?     */
         received = 0;        /* No --> Reset to empty buffer        */
      else if ( psync != grpkt ) /* First character in buffer?       */
      {                       /* No --> Make it first character      */
         received -= psync - grpkt;
         shifts++;
         MEMMOVE( grpkt, psync, received );
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
         i = (unsigned char) ((grpkt[1] ^ grpkt[2] ^ grpkt[3] ^
                               grpkt[4] ^ grpkt[5]) & 0xff);
         printmsg(i ? 2 : 10, "prpkt %02x %02x %02x %02x %02x .. %02x ",
            grpkt[1], grpkt[2], grpkt[3], grpkt[4], grpkt[5], i);

         if (i == 0)          /* Good header?                        */
            got_hdr = KWTrue;  /* Yes --> Drop out of loop            */
         else {               /* No  --> Flag it, continue loop      */
            badhdr++;
            printmsg(GDEBUG, "*** bad pkt header ***");
            MEMMOVE( grpkt, grpkt + 1, --received );
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
      type = (unsigned short) (c >> 3);
      *yyy = c & 0x07;
      *xxx = 0;
      check = 0;
      checkchk = 0;
      got_hdr = KWFalse;
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
         return(DCP_ERROR);
      }

/*--------------------------------------------------------------------*/
/*             Compute the size of the data block desired             */
/*--------------------------------------------------------------------*/

      total = 16 << grpkt[1];
      if (total > r_pktsize)  /* Within the defined limits?          */
      {                       /* No --> Other system has bad header,
                                 or the header got corrupted         */
         printmsg(0,"grpack: Invalid packet size %d (%d)",
            total, (int) grpkt[1]);
         received = 0;
         got_hdr = KWFalse;
         return(DCP_ERROR);
      }

      needed = total + HDRSIZE - received;
                                 /* Compute byte required to fill
                                    data buffer                      */

/*--------------------------------------------------------------------*/
/*     If we don't have enough data in the buffer, read some more     */
/*--------------------------------------------------------------------*/

      if ((needed > 0) &&
          (sread((char UUFAR *) grpkt + HDRSIZE + total - needed,
                  needed,
                  timeout) < (unsigned short)needed))
         return(DCP_EMPTY);

      got_hdr = KWFalse;          /* Must re-process header next pass */

/*--------------------------------------------------------------------*/
/*              Break packet header into various values               */
/*--------------------------------------------------------------------*/

      type = 0;
      c2 = grpkt[4];
      c = (unsigned char) (c2 & 0x3f);
      *xxx = c >> 3;
      *yyy = c & 0x07;
      check = ((grpkt[3] & 0xff) << 8) | (grpkt[2] & 0xff);
      checkchk = checksum( (char UUFAR *) grpkt + HDRSIZE , total);
      i = (unsigned char) ((grpkt[4] | 0x80) & 0xff);
      checkchk = (0xaaaa - (checkchk ^ i)) & 0xffff;
      if (checkchk != check)
      {
         printmsg(4, "*** checksum error ***");
         MEMMOVE( grpkt, grpkt + HDRSIZE, total );
                              /* Save data so we can scan for sync   */
         received = total;    /* Note the amount of the data in buf  */
         return(DCP_ERROR);   /* Return to caller with error         */
      }

/*--------------------------------------------------------------------*/
/*    The checksum is correct, now determine the length of the        */
/*    data to return.                                                 */
/*--------------------------------------------------------------------*/

      *len = total;

      if (c2 & 0x40)
      {
         unsigned ii;
         if ( grpkt[HDRSIZE] & 0x80 )
         {
            ii = (grpkt[HDRSIZE] & 0x7f) + ((grpkt[HDRSIZE+1] & 0xff) << 7);
            *len -= ii;
            MEMCPY(data, grpkt + HDRSIZE + 2, *len);
         }
         else {
            ii = (grpkt[HDRSIZE] & 0xff);
            *len -= ii;
            MEMCPY(data, grpkt + HDRSIZE + 1, *len);
         } /* else */
      }
      else
         MEMCPY( data, grpkt + HDRSIZE, *len);
   } /* else */

/*--------------------------------------------------------------------*/
/*           Announce what we got and return to the caller            */
/*--------------------------------------------------------------------*/

   received = 0;              /* Returning entire buffer, reset count */
   printmsg(5, "receive packet type %d, yyy=%d, xxx=%d, len=%d",
      type, *yyy, *xxx, *len);

#ifdef UDEBUG
#ifdef BIT32ENV
   printmsg(13, " checksum rec=%04x comp=%04x\ndata=|%.*s|",
#else
   printmsg(13, " checksum rec=%04x comp=%04x\ndata=|%.*Fs|",
#endif
               check, checkchk, total, grpkt + HDRSIZE);
#endif

   return(type);

} /* grpack */

/*--------------------------------------------------------------------*/
/*       c h e c k s u m                                              */
/*                                                                    */
/*       Compute 16 bit checksum on buffer                            */
/*--------------------------------------------------------------------*/

static unsigned checksum(const char UUFAR *data, unsigned len)
{
   unsigned i, j;
   unsigned tmp, chk1, chk2;
   chk1 = 0xffff;
   chk2 = 0;
   j = len;
   for (i = 0; i < len; i++)
   {
      if (chk1 & 0x8000)
      {
         chk1 <<= 1;
         chk1++;
      }
      else {
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

} /* checksum */
