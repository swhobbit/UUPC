/*--------------------------------------------------------------------*/
/*       d c p f p k t . c                                            */
/*                                                                    */
/*       UUCP 'f' protocol support                                    */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-2001 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: dcpfpkt.c 1.28 2000/05/12 12:32:55 ahd v1-13g $
 *
 *    Revision history:
 *    $Log: dcpfpkt.c $
 *    Revision 1.28  2000/05/12 12:32:55  ahd
 *    Annual copyright update
 *
 *    Revision 1.27  1999/01/08 02:20:56  ahd
 *    Convert currentfile() to RCSID()
 *
 *    Revision 1.26  1999/01/04 03:53:57  ahd
 *    Annual copyright change
 *
 *    Revision 1.25  1998/03/01 01:39:22  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.24  1997/05/11 04:28:26  ahd
 *    SMTP client support for RMAIL/UUXQT
 *
 *    Revision 1.23  1997/04/24 01:33:24  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.22  1996/01/01 21:20:24  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.21  1995/01/07 16:38:08  ahd
 *    Change boolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.20  1994/12/22 00:34:46  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.19  1994/02/19 05:07:00  ahd
 *    Use standard first header
 *
 * Revision 1.18  1994/01/01  19:18:29  ahd
 * Annual Copyright Update
 *
 * Revision 1.17  1993/12/29  03:54:30  dmwatt
 * Suppress compiler warnings for unused args
 *
 * Revision 1.16  1993/12/02  13:49:58  ahd
 * 'e' protocol support
 *
 * Revision 1.15  1993/12/02  03:59:37  dmwatt
 * 'e' protocol support
 *
 * Revision 1.14  1993/11/06  17:57:09  rhg
 * Drive Drew nuts by submitting cosmetic changes mixed in with bug fixes
 *
 * Revision 1.13  1993/10/09  22:21:55  rhg
 * ANSIfy source
 *
 * Revision 1.12  1993/10/02  19:07:49  ahd
 * Suppress compiler warning
 *
 * Revision 1.11  1993/09/20  04:41:54  ahd
 * OS/2 2.x support
 *
 * Revision 1.10  1993/07/31  16:27:49  ahd
 * Changes in support of Robert Denny's Windows support
 *
 * Revision 1.9  1993/07/22  23:22:27  ahd
 * First pass at changes for Robert Denny's Windows 3.1 support
 *
 * Revision 1.8  1993/05/30  00:01:47  ahd
 * Multiple communications driver support
 *
 * Revision 1.7  1993/04/05  12:26:01  ahd
 * Correct prototypes to match gpkt
 *
 * Revision 1.6  1993/04/05  04:35:40  ahd
 * Allow unique send/receive packet sizes
 *
 * Revision 1.3  1992/11/19  02:36:29  ahd
 * Revision 1.2  1992/11/15  20:10:47  ahd
 * Clean up modem file support for different protocols
 */

/*--------------------------------------------------------------------*/
/*    Flow control ("f") protocol.                                    */
/*                                                                    */
/*    This protocol relies on flow control of the data stream.  It    */
/*    is meant for working over links that can (almost) be            */
/*    guaranteed to be errorfree, specifically X.25/PAD links.  A     */
/*    sumcheck is carried out over a whole file only.  If a           */
/*    transport fails the receiver can request retransmission(s).     */
/*    This protocol uses a 7-bit datapath only, so it can be used     */
/*    on links that are not 8-bit transparent.                        */
/*                                                                    */
/*    When using this protocol with an X.25 PAD:  Although this       */
/*    protocol uses no control chars except CR, control chars NULL    */
/*    and ^P are used before this protocol is started; since ^P is    */
/*    the default char for accessing PAD X.28 command mode, be        */
/*    sure to disable that access (PAD par 1).  Also make sure        */
/*    both flow control pars (5 and 12) are set.  The CR used in      */
/*    this proto is meant to trigger packet transmission, hence       */
/*    par 3 should be set to 2; a good value for the Idle Timer       */
/*    (par 4) is 10.  All other pars should be set to 0.              */
/*                                                                    */
/*    Normally a calling site will take care of setting the local     */
/*    PAD pars via an X.28 command and those of the remote PAD via    */
/*    an X.29 command, unless the remote site has a special           */
/*    channel assigned for this protocol with the proper par          */
/*    settings.                                                       */
/*                                                                    */
/*    Author:  Piet Beertema, CWI, Amsterdam, Sep 1984                */
/*                                                                    */
/*    Adapted to uupc 3.0 and THINK C 4.0 by Dave Platt, Jul 1991     */
/*                                                                    */
/*    Adapted to UUPC/extended by Drew Derbyshire, 1992               */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"

#include "dcp.h"
#include "dcpfpkt.h"
#include "dcpsys.h"
#include "hostable.h"
#include "security.h"
#include "ssleep.h"
#include "modem.h"
#include "commlib.h"

/*--------------------------------------------------------------------*/
/*                              Defines                               */
/*--------------------------------------------------------------------*/

#ifndef MAXMSGLEN
#define MAXMSGLEN BUFSIZ
#endif /* MAXMSGLEN */

RCSID("$Id: dcpfpkt.c 1.28 2000/05/12 12:32:55 ahd v1-13g $");

/*--------------------------------------------------------------------*/
/*                    Internal function prototypes                    */
/*--------------------------------------------------------------------*/

static short fsendresp(short state);

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

static short chksum;

/*--------------------------------------------------------------------*/
/*    f o p e n p k                                                   */
/*                                                                    */
/*    Open "f" protocol to other system                               */
/*--------------------------------------------------------------------*/

#ifdef __TURBOC__
#pragma argsused
#elif _MSC_VER >= 700
#pragma warning(disable:4100)   /* suppress unref'ed formal param. warnings */
#endif

short fopenpk(const KWBoolean master)
{
   flowcontrol(KWTrue);

   if ( M_fPacketSize > MAXPACK)
      M_fPacketSize = MAXPACK;

   r_pktsize = s_pktsize = M_fPacketSize;
   ssleep(2); /* Give peer time to perform corresponding port setup */
   return DCP_OK;
} /* fopenpk */

#if _MSC_VER >= 700
#pragma warning(default:4100)   /* restore unref'ed formal param. warnings */
#endif

/*--------------------------------------------------------------------*/
/*    f c l o s e p k                                                 */
/*                                                                    */
/*    Shutdown "f" procotol with other system                         */
/*--------------------------------------------------------------------*/

short fclosepk()
{
   flowcontrol(KWFalse);
   return DCP_OK;
} /* fclosepk */

/*--------------------------------------------------------------------*/
/*    f w r m s g                                                     */
/*                                                                    */
/*    Send a control message to remote system with "f" procotol       */
/*--------------------------------------------------------------------*/

short fwrmsg(char *str)
{
   char bufr[MAXMSGLEN];
   char *s = bufr;

   while (*str)
      *s++ = *str++;
   if (*(s-1) == '\n')
      s--;
   *s++ = '\r';
   if (swrite(bufr, (unsigned int) (s - bufr)) == (int)(s - bufr))
      return DCP_OK;
   else
      return DCP_FAILED;
} /* fwrmsg */

/*--------------------------------------------------------------------*/
/*    f r d m s g                                                     */
/*                                                                    */
/*    Read a control message from remote host with "f" protocol       */
/*--------------------------------------------------------------------*/

short frdmsg(char *str)
{
   char *smax;
   char *s = str;

   smax = s + MAXPACK - 1;
   for (;;) {
      if (sread(s, 1, M_fPacketTimeout) <= 0)
      {
         printmsg(0,"frdmsg: timeout reading message");
         *s++ = '\0';
         goto msgerr;
      }
      if (*s == '\r')
         break;
      if (*s < ' ')
         continue;
      if (s++ >= smax)
      {
         printmsg(0,"frdmsg: buffer overflow");
         *--s = '\0';
         goto msgerr;
      } /* if (s++ >= smax) */
   }
   *s = '\0';
   return DCP_OK;

msgerr:
   printmsg(0,"frdmsg: Message received \"%s\"", str);
   return DCP_FAILED;
} /* frdmsg */

/*--------------------------------------------------------------------*/
/*    f g e t p k t                                                   */
/*                                                                    */
/*    Receive an "f" protocol packet of data from the other system    */
/*--------------------------------------------------------------------*/

short fgetpkt(char *packet, short *bytes)
{
   char *op, c, *ip;
   short sum, len, left;
   char buf[5], tbuf[1];
   short i;
   static char special = 0;
   static KWBoolean eof = KWFalse;

/*--------------------------------------------------------------------*/
/*                    Handle EOF on previous call                     */
/*--------------------------------------------------------------------*/

   if ( eof )
   {
      eof = KWFalse;
      printmsg(2,"fgetpkt: EOF from other host");
      *bytes = 0;
      if (fsendresp(DCP_OK) == DCP_OK)
         return DCP_OK;
      else
         return DCP_FAILED;
   } /* if ( eof ) */

   left = (short) s_pktsize;
   op = packet;
   sum = chksum;

/*--------------------------------------------------------------------*/
/*                     Loop to fill up one packet                     */
/*--------------------------------------------------------------------*/

   do {
      ip = tbuf;
      len = (short) sread(ip, 1, M_fPacketTimeout); /* single-byte reads for now */
      if (len == 0) {
         printmsg(0,"fgetpkt: Timeout after %d seconds", M_fPacketTimeout);
         return DCP_FAILED;               /* Fail if timed out */
      }
      if ((*ip &= 0177) >= '\172') {
         if (special) {
            special = 0;
            if (*ip++ != '\176')
            {
               printmsg(0,"fgetpkt: Did not expect character ^%c (x%02x)",
                  (char) (*(ip-1) + 'A') , (short) *(ip-1));
               goto dcorr;
            }
            len = 0;

            while (len < 5) {
               i = (short) sread(&buf[len], 5 - len, M_fPacketTimeout);
               if (i == 0) {
                  printmsg(0,
                     "fgetpkt: Timeout reading %d chars after %d seconds",
                              5 - len, M_fPacketTimeout);
                  goto dcorr;
               }
               len += i;
            }

            printmsg(6, "fgetpkt: buf=|%.*s|", len , packet);
            if (buf[4] != '\r')
            {
               printmsg(0,
                  "fgetpkt: error: Expected carriage return, "
                   "not %s%c (x%02x)",
                  (buf[4] < ' ') ? "^" : "" ,
                  (char) (buf[4] + ((buf[4] < ' ') ? 'A' : 0)),
                  (short) buf[4]);
               goto dcorr;
            }
            sscanf(buf, "%4x", &chksum);
            *bytes = (short) (op - packet);
            if (chksum == sum) {
               eof = KWTrue;
               printmsg(6, "fgetpkt: data=|%.*s|", *bytes , packet);
               return DCP_OK;
            } else {
               printmsg(0, "fgetpkt: Checksum mismatch, told %04x, calc %04x",
                            chksum, sum);
               fsendresp(DCP_RETRY);
               return DCP_RETRY;
            }
         }
         special = *ip++;
      } else {
         if (*ip < '\040') {
            printmsg(0,"fgetpkt: error: got control character ^%c (%x)",
                  (char) (*ip + 'A') , (short) *ip);
            goto dcorr;
         }

         switch (special) {
            case 0:
               c = (char) (*ip++);
               break;
            case '\172':
               c = (char) (*ip++ - 0100);
               break;
            case '\173':
               c = (char) (*ip++ + 0100);
               break;
            case '\174':
               c = (char) (*ip++ + 0100);
               break;
            case '\175':
               c = (char) (*ip++ + 0200);
               break;
            case '\176':
               c = (char) (*ip++ + 0300);
               break;
            default:
               printmsg(0,"fgetpkt: Invalid special chracter 0x%2x",
                          (short) special );
               panic();
               c = '\0';
         }

         *op++ = c;
         left --;
         if (sum & 0x8000) {
            sum <<= 1;
            sum++;
         } else
            sum <<= 1;
         sum += (short) (c & 0377);
         sum &= 0xffff;
         special = 0;
      }
   } while (left > 0);

/*--------------------------------------------------------------------*/
/*            The packet is full of data, return to caller            */
/*--------------------------------------------------------------------*/

   *bytes = (short) s_pktsize;
   printmsg(6, "fgetpkt: data=|%.*s|", *bytes , packet);
   chksum = sum;
   return DCP_OK;

/*--------------------------------------------------------------------*/
/*            The data is corrupt; flush the incoming file            */
/*--------------------------------------------------------------------*/

dcorr:
   printmsg (0, "Data corrupted, skipping to EOF");

   len = 1;
   while (len)
      len = (short) sread(packet, 1, M_fPacketTimeout);

   fsendresp(DCP_RETRY);
   return DCP_RETRY;
} /* fgetpkt */

/*--------------------------------------------------------------------*/
/*    f s e n d p k t                                                 */
/*                                                                    */
/*    Send an "f" protocol packet to the other system                 */
/*--------------------------------------------------------------------*/

short fsendpkt(char *ip, short len)
{
   char *op;
   short sum, nl;
   short ret;
   char obuf[MAXPACK * 2];
   op = obuf;
   nl = 0;
   sum = chksum;
   if (len == 0)
   {
      printmsg(0,"fsendpkt: Internal error: zero length for packet");
      return DCP_FAILED;
   }
   do {
      if (sum & 0x8000) {
         sum <<= 1;
         sum++;
      } else
         sum <<= 1;
      sum += (short) (*ip & 0377);
      sum &= 0xffff;
      if (*ip & 0200) {
         *ip &= 0177;
         if (*ip < 040) {
            *op++ = '\174';
            *op++ = (char) (*ip++ + 0100);
         } else
         if (*ip <= 0171) {
            *op++ = '\175';
            *op++ = *ip++;
         }
         else {
            *op++ = '\176';
            *op++ = (char) (*ip++ - 0100);
         }
         nl += 2;
      } else {
         if (*ip < 040) {
            *op++ = '\172';
            *op++ = (char) (*ip++ + 0100);
            nl += 2;
         } else
         if (*ip <= 0171) {
            *op++ = *ip++;
            nl++;
         } else {
            *op++ = '\173';
            *op++ = (char) (*ip++ - 0100);
            nl += 2;
         }
      }
   } while (--len > 0);
   chksum = sum;
   ret = (short) swrite(obuf, nl);
   if ( ret == nl )
      return DCP_OK;
   else
      return DCP_FAILED;
} /* fsendpkt */

/*--------------------------------------------------------------------*/
/*    f f i l e p k t                                                 */
/*                                                                    */
/*    Prepare for processing an "f" procotol file transfer            */
/*--------------------------------------------------------------------*/

#ifdef __TURBOC__
#pragma argsused
#endif

short ffilepkt( const KWBoolean transmit, const unsigned long bytes )
{
   chksum = (short) 0xffff;
   printmsg(3,"ffilepkt: Checksum reset");
   return DCP_OK;
} /* ffilepkt */

/*--------------------------------------------------------------------*/
/*    f e o f                                                         */
/*                                                                    */
/*    Transmit "f" protocol end of file to the other system           */
/*--------------------------------------------------------------------*/

short feofpkt( void )
{
   char ibuf[MAXMSGLEN];

/*--------------------------------------------------------------------*/
/*               Transmit EOF with an attached checksum               */
/*--------------------------------------------------------------------*/

   printmsg(2,"feofpkt: sending EOF");
   sprintf(ibuf, "\176\176%04x", chksum);
   printmsg(2,"--> %s", ibuf);
   fwrmsg(ibuf);

/*--------------------------------------------------------------------*/
/*                 Now get the response and report it                 */
/*--------------------------------------------------------------------*/

   if (frdmsg(ibuf) == DCP_FAILED)
      return DCP_FAILED;

   printmsg(2,"<-- %s",ibuf);

/*--------------------------------------------------------------------*/
/*              Determine our next step from the result               */
/*--------------------------------------------------------------------*/

   switch(*ibuf)
   {
      case 'R':
         return DCP_RETRY;

      case 'G':
         return DCP_OK;

      default:
         return DCP_FAILED;

   } /* switch */

} /* feofpkt */

/*--------------------------------------------------------------------*/
/*    f s e n d r e s p                                               */
/*                                                                    */
/*    Send result to a file transfer to other host                    */
/*--------------------------------------------------------------------*/

static short fsendresp(short state)
{
   char *s;
   switch (state)
   {
      case DCP_OK:
         s = "G";
         break;

      case DCP_RETRY:
         s = "R";
         break;

      default:
         s = "Q";
         break;
   }

   printmsg(2,"--> %s", s);
   return fwrmsg(s);

} /* fsendresp */
