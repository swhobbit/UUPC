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
/*--------------------------------------------------------------------*/

/*
 *    $Id: DCPFPKT.C 1.7 1993/04/05 12:26:01 ahd Exp $
 *
 *    $Log: DCPFPKT.C $
 * Revision 1.7  1993/04/05  12:26:01  ahd
 * Correct prototypes to match gpkt
 *
 * Revision 1.6  1993/04/05  04:35:40  ahd
 * Allow unique send/receive packet sizes
 *
 * Revision 1.5  1992/11/20  12:38:26  ahd
 * Drop rcsid
 *
 * Revision 1.4  1992/11/19  03:01:13  ahd
 * drop rcsid
 *
 * Revision 1.3  1992/11/19  02:36:29  ahd
 * Revision 1.2  1992/11/15  20:10:47  ahd
 * Clean up modem file support for different protocols
 *
 */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <time.h>
#include <string.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
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
#endif MAXMSGLEN

currentfile();

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

short fopenpk(const boolean master)
{
   flowcontrol(TRUE);

   if ( M_fPacketSize > MAXPACK)
      M_fPacketSize = MAXPACK;

   r_pktsize = s_pktsize = M_fPacketSize;
   ssleep(2); /* Give peer time to perform corresponding port setup */
   return OK;
} /* fopenpk */

/*--------------------------------------------------------------------*/
/*    f c l o s e p k                                                 */
/*                                                                    */
/*    Shutdown "f" procotol with other system                         */
/*--------------------------------------------------------------------*/

short fclosepk()
{
   flowcontrol(FALSE);
   return OK;
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
   if (swrite(bufr, s - bufr) == (s - bufr))
      return OK;
   else
      return FAILED;
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
   return OK;

msgerr:
   printmsg(0,"frdmsg: Message received \"%s\"", str);
   return FAILED;
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
   static boolean eof = FALSE;

/*--------------------------------------------------------------------*/
/*                    Handle EOF on previous call                     */
/*--------------------------------------------------------------------*/

   if ( eof )
   {
      eof = FALSE;
      printmsg(0,"fgetpkt: EOF from other host");
      *bytes = 0;
      if (fsendresp(OK) == OK)
         return OK;
      else
         return FAILED;
   } /* if ( eof ) */

   left = s_pktsize;
   op = packet;
   sum = chksum;

/*--------------------------------------------------------------------*/
/*                     Loop to fill up one packet                     */
/*--------------------------------------------------------------------*/

   do {
      ip = tbuf;
      len = sread(ip, 1, M_fPacketTimeout); /* single-byte reads for now */
      if (len == 0) {
         printmsg(0,"fgetpkt: Timeout after %d seconds", M_fPacketTimeout);
         return FAILED;               /* Fail if timed out */
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
               i = sread(&buf[len], 5 - len, M_fPacketTimeout);
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
            *bytes = op - packet;
            if (chksum == sum) {
               eof = TRUE;
               printmsg(6, "fgetpkt: data=|%.*s|", *bytes , packet);
               return OK;
            } else {
               printmsg(0, "fgetpkt: Checksum mismatch, told %04x, calc %04x",
                            chksum, sum);
               fsendresp(RETRY);
               return RETRY;
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
         sum += c & 0377;
         sum &= 0xffff;
         special = 0;
      }
   } while (left > 0);

/*--------------------------------------------------------------------*/
/*            The packet is full of data, return to caller            */
/*--------------------------------------------------------------------*/

   *bytes = s_pktsize;
   printmsg(6, "fgetpkt: data=|%.*s|", *bytes , packet);
   chksum = sum;
   return OK;

/*--------------------------------------------------------------------*/
/*            The data is corrupt; flush the incoming file            */
/*--------------------------------------------------------------------*/

dcorr:
   printmsg (0, "Data corrupted, skipping to EOF");

   len = 1;
   while (len)
      len = sread(packet, 1, M_fPacketTimeout);

   fsendresp(RETRY);
   return RETRY;
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
      return FAILED;
   }
   do {
      if (sum & 0x8000) {
         sum <<= 1;
         sum++;
      } else
         sum <<= 1;
      sum += *ip & 0377;
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
   ret = swrite(obuf, nl);
   if ( ret == nl )
      return OK;
   else
      return FAILED;
} /* fsendpkt */

/*--------------------------------------------------------------------*/
/*    f f i l e p k t                                                 */
/*                                                                    */
/*    Prepare for processing an "f" procotol file transfer            */
/*--------------------------------------------------------------------*/

short ffilepkt( void)
{
   chksum = 0xffff;
   printmsg(3,"ffilepkt: Checksum reset");
   return OK;
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

   printmsg(0,"feofpkt: sending EOF");
   sprintf(ibuf, "\176\176%04x", chksum);
   printmsg(2,"--> %s", ibuf);
   fwrmsg(ibuf);

/*--------------------------------------------------------------------*/
/*                 Now get the response and report it                 */
/*--------------------------------------------------------------------*/

   if (frdmsg(ibuf) == FAILED)
      return FAILED;

   printmsg(2,"<-- %s",ibuf);

/*--------------------------------------------------------------------*/
/*              Determine our next step from the result               */
/*--------------------------------------------------------------------*/

   switch(*ibuf)
   {
      case 'R':
         return RETRY;

      case 'G':
         return OK;

      default:
         return FAILED;

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
      case OK:
         s = "G";
         break;

      case RETRY:
         s = "R";
         break;

      default:
         s = "Q";
         break;
   }

   printmsg(2,"--> %s", s);
   return fwrmsg(s);

} /* fsendresp */
