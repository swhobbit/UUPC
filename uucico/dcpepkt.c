/*--------------------------------------------------------------------*/
/*       d c p e p k t . c                                            */
/*                                                                    */
/*       UUCP 'e' protocol support                                    */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1993 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: dcpepkt.c 1.2 1993/12/02 03:59:37 dmwatt Exp dmwatt $
 *
 *    Revision history:
 *    $Log: dcpepkt.c $
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

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "dcp.h"
#include "dcpepkt.h"
#include "dcpsys.h"
#include "hostable.h"
#include "security.h"
#include "ssleep.h"
#include "modem.h"
#include "commlib.h"

#ifdef _Windows
#include "pwinsock.h"
#endif

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

short eopenpk(const boolean master)
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
      recv = (short) min(efilelength - ebytesdone, r_pktsize);

   if ( sread( packet, recv, M_tPacketTimeout) < recv )
      printmsg(0,"tgetpkt: Data read failed for %d bytes", (int) recv);

   remote_stats.packets++;

   *bytes = recv;
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
   else if ( swrite( ip , len ) != len )
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

short efilepkt(const boolean xmit, const unsigned long bytes)
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

      if (swrite( startbuf, sizeof startbuf ) <
         sizeof startbuf)
      {
         printmsg(0, "efilepkt: Length write failed");
         return DCP_FAILED;
      }

   } else {

      if (sread( (char *) &startbuf, sizeof startbuf, M_tPacketTimeout) <
         sizeof startbuf)
      {
         printmsg(0,"efilepkt: Length read failed");
         return DCP_FAILED;
      }

      /* Read the file length */

      efilelength = strtol(startbuf, NULL, 10);
      printmsg(4, "efilepkt: received file length %lu", efilelength);
   }

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

   return(0);

} /* ewrmsg */

/*--------------------------------------------------------------------*/
/*    e r d m s g                                                     */
/*                                                                    */
/*    Read a message from the remote system                           */
/*--------------------------------------------------------------------*/

short erdmsg( char *s)
{
   if (rmsg( s, 4, M_tPacketTimeout, r_pktsize ))
      return(0);
   else
      return -1;
} /* erdmsg */