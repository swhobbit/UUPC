/*--------------------------------------------------------------------*/
/*       d c p e p k t . c                                            */
/*                                                                    */
/*       UUCP 'e' protocol support                                    */
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
 *    $Id: dcpepkt.c 1.21 2000/05/12 12:32:55 ahd v1-13g $
 *
 *    Revision history:
 *    $Log: dcpepkt.c $
 *    Revision 1.21  2000/05/12 12:32:55  ahd
 *    Annual copyright update
 *
 *    Revision 1.20  1999/01/08 02:20:56  ahd
 *    Convert currentfile() to RCSID()
 *
 *    Revision 1.19  1999/01/04 03:53:57  ahd
 *    Annual copyright change
 *
 *    Revision 1.18  1998/03/01 01:39:18  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.17  1997/04/24 01:33:18  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.16  1996/03/18 03:52:46  ahd
 *    Allow binary rmsg() processing without translation of CR/LF into \0
 *    Use enumerated list for synch types on rmsg() call
 *
 *    Revision 1.15  1996/01/01 21:20:01  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.14  1995/07/21 13:27:00  ahd
 *    If modem is unable to dial, be sure to resume suspended UUCICO if needed
 *
 *    Revision 1.13  1995/03/24 04:17:22  ahd
 *    Compiler warning message cleanup, optimize for low memory processing
 *
 *    Revision 1.12  1995/02/23 04:27:54  ahd
 *    Explicitly report timeouts, compiler warning cleanup
 *
 *    Revision 1.11  1995/01/07 16:38:02  ahd
 *    Change boolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.10  1994/12/22 00:34:37  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.9  1994/10/03 01:01:25  ahd
 *    Reflect read errors to caller rather than (stupidly) eating them
 *
 *        Revision 1.8  1994/05/04  23:36:34  ahd
 *        Count packets sent/received in command messages
 *
 * Revision 1.7  1994/02/19  05:06:41  ahd
 * Use standard first header
 *
 * Revision 1.6  1994/01/01  19:18:19  ahd
 * Annual Copyright Update
 *
 * Revision 1.5  1993/12/30  03:11:05  ahd
 * Use unique packet timeout variable for 'e' protocol
 *
 * Revision 1.4  1993/12/06  01:59:07  ahd
 * Delete unneeded pwinsock header
 *
 * Revision 1.3  1993/12/02  13:49:58  ahd
 * 'e' protocol support
 *
 * Revision 1.2  1993/12/02  03:59:37  dmwatt
 * 'e' protocol support
 *
 */

/*--------------------------------------------------------------------*/
/*       TCP/IP ("e") protocol.                                       */
/*                                                                    */
/*       Protocol for over reliable (TCP/IP) paths.                   */
/*                                                                    */
/*       'e' procotol is done by simply transmitting the length of    */
/*       the entire file as ASCII text, pads the length out to 20     */
/*       characters with NULLs, then sends the entire file with no    */
/*       further packet headers.                                      */
/*                                                                    */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"

#include "dcp.h"
#include "dcpepkt.h"
#include "dcpsys.h"
#include "hostable.h"
#include "security.h"
#include "ssleep.h"
#include "modem.h"
#include "commlib.h"

RCSID("$Id: dcpepkt.c 1.21 2000/05/12 12:32:55 ahd v1-13g $");

static unsigned long efilelength;
static unsigned long ebytesdone;

/*--------------------------------------------------------------------*/
/*    e o p e n p k                                                   */
/*                                                                    */
/*    Open "e" protocol to other system                               */
/*--------------------------------------------------------------------*/

#ifdef __TURBOC__
#pragma argsused
#endif

short eopenpk(const KWBoolean master)
{
   s_pktsize = r_pktsize = MAXPACK;    /* Fixed for 'e' procotol     */

   printmsg(5, "eopenpk:  called, master = %d", master);

   return DCP_OK;

} /* topenpk */

/*--------------------------------------------------------------------*/
/*    e g e t p k t                                                   */
/*                                                                    */
/*    Receive an "e" protocol packet of data from the other system    */
/*--------------------------------------------------------------------*/

short egetpkt(char *packet, short *bytes)
{
   unsigned short recv;
   printmsg(5, "egetpkt: called");

   if (ebytesdone == efilelength )
   {
      *bytes = 0;                /* Report EOF to caller          */
      printmsg(4,"egetpkt: File complete");
      return 0;                  /* Return success to caller      */
   }
   if (ebytesdone > efilelength)
   {
      printmsg(0, "egetpkt:  received more bytes than file length!");
      printmsg(0, "egetpkt:  done = %lu, length = %lu", ebytesdone,
         efilelength);
      return -1;
   }
   else
      recv = (unsigned short) min(efilelength - ebytesdone, r_pktsize);

   if ( sread( packet, recv, M_ePacketTimeout) < (int) recv )
   {
      printmsg(0,"egetpkt: Data read failed for %d bytes", (int) recv);
      return -1;
   }

   remote_stats.packets++;

   *bytes = (short) recv;
   ebytesdone += recv;

   return 0;

} /* egetpkt */

/*--------------------------------------------------------------------*/
/*    e s e n d p k t                                                 */
/*                                                                    */
/*    Send an "e" protocol packet to the other system                 */
/*--------------------------------------------------------------------*/

short esendpkt(char *ip, short len)
{
   printmsg(5, "egetpkt: called");

   if ( ! len )
      printmsg(4,"esendpkt: EOF reached");
   else if ( swrite( ip , (size_t) len ) != len )
      return -1;

   remote_stats.packets++;
   ebytesdone += len;

   return 0;

} /* esendpkt */

/*--------------------------------------------------------------------*/
/*    e c l o s e p k                                                 */
/*                                                                    */
/*    Shutdown "e" procotol with other system                         */
/*--------------------------------------------------------------------*/

short eclosepk()
{
   printmsg(5, "eclosepk: called");

   return DCP_OK;
} /* eclosepk */

/*--------------------------------------------------------------------*/
/*    e f i l e p k t                                                 */
/*                                                                    */
/*    Transmit/receive file length                                    */
/*--------------------------------------------------------------------*/

short efilepkt(const KWBoolean xmit, const unsigned long bytes)
{
   char startbuf[20];

   ebytesdone = 0;
   efilelength = 0;

   printmsg(5, "efilepkt: called");

/*--------------------------------------------------------------------*/
/*       If we're the master, then send our file length, otherwise    */
/*       receive it                                                   */
/*--------------------------------------------------------------------*/

   if (xmit) {
      efilelength = bytes;

      memset(startbuf, 0, sizeof startbuf);
      sprintf(startbuf, "%ld", efilelength);

      printmsg(4, "efilepkt: File length is %s", startbuf);

      if (swrite( startbuf, sizeof startbuf ) < (int) sizeof startbuf)
      {
         printmsg(0, "efilepkt: Length write failed");
         return DCP_FAILED;
      }

   } else {

      if (sread( startbuf, sizeof startbuf, M_ePacketTimeout) <
          (int) sizeof startbuf)
      {
         printmsg(0,"efilepkt: Length read failed");
         return DCP_FAILED;
      }

      /* Read the file length */

      efilelength = (unsigned long) strtol(startbuf, NULL, 10);
      printmsg(4, "efilepkt: received file length %lu", efilelength);
   }

   remote_stats.packets++;
   return DCP_OK;

} /* efilepkt */

/*--------------------------------------------------------------------*/
/*       e e o f p k t                                                */
/*                                                                    */
/*       Handle end-of-file for processed file                        */
/*--------------------------------------------------------------------*/

short eeofpkt(void)
{
   printmsg(5, "eeofpkt: called");

   if (ebytesdone != efilelength)
   {
      printmsg(0, "eeofpkt:  I don't think we're done! "
         " done = %lu, length = %lu", ebytesdone, efilelength);

      return DCP_FAILED;
   }

   return DCP_OK;

} /* eeofpkt */

/*--------------------------------------------------------------------*/
/*    e w r m s g                                                     */
/*                                                                    */
/*    Send a message to remote system                                 */
/*--------------------------------------------------------------------*/

short ewrmsg( char *s )
{

   if (swrite( s, strlen(s) + 1 ) < (int) strlen(s) + 1 )
   {
      printmsg(0, "ewrmsg: message write failed");
      return -1;
   }

   remote_stats.packets++;
   return(0);

} /* ewrmsg */

/*--------------------------------------------------------------------*/
/*    e r d m s g                                                     */
/*                                                                    */
/*    Read a message from the remote system                           */
/*--------------------------------------------------------------------*/

short erdmsg( char *s)
{

   if (rmsg( s, SYNCH_BINARY, M_ePacketTimeout, (int) r_pktsize ))
   {
      remote_stats.packets++;
      return(0);
   }

/*--------------------------------------------------------------------*/
/*    We didn't get the end of the string in time, report an error    */
/*--------------------------------------------------------------------*/

   printmsg(0, "erdmsg: Command read failed" );

   return -1;

} /* erdmsg */
