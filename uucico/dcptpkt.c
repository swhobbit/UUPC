/*--------------------------------------------------------------------*/
/*       d c p t p k t . c                                            */
/*                                                                    */
/*       UUCP 't' protocol support                                    */
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
 *    $Id: dcptpkt.c 1.1 1993/09/18 19:47:24 ahd Exp ahd $
 *
 *    Revision history:
 *    $Log: dcptpkt.c $
 * Revision 1.1  1993/09/18  19:47:24  ahd
 * Initial revision
 *
 */

/*--------------------------------------------------------------------*/
/*       TCP/IP ("t") protocol.                                       */
/*                                                                    */
/*       Protocol for over reliable (TCP/IP) paths.                   */
/*                                                                    */
/*       't' procotol is done by simply transmitting the four byte    */
/*       length of the packet in network byte order (big-endian)      */
/*       followed by the packet data itself.  No padding is           */
/*       performed.                                                   */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <time.h>
#include <string.h>

#if defined(WIN32) || defined(_Windows)
#include "winsock.h"       // Needed for byte ordering
#endif

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "dcp.h"
#include "dcptpkt.h"
#include "dcpsys.h"
#include "hostable.h"
#include "security.h"
#include "ssleep.h"
#include "modem.h"
#include "commlib.h"

#ifdef _Windows
#include "pwinsock.h"
#endif

/*--------------------------------------------------------------------*/
/*     Network functions needed when no winsock functions available   */
/*--------------------------------------------------------------------*/

#ifndef _WINSOCKAPI_

static unsigned long htonl( const unsigned long input );
static unsigned long ntohl( const unsigned long input );

/*--------------------------------------------------------------------*/
/*       h t o n l                                                    */
/*                                                                    */
/*       Convert unsigned long from host to network byte order        */
/*--------------------------------------------------------------------*/

static unsigned long htonl( const unsigned long input )
{
   unsigned long result;
   unsigned char *p = (unsigned char *) &result;
   int i;

   for (i = 0 ; i < sizeof input; i++ )
      p[i] = (unsigned char) ((input >> (i*8)) & 0xff);

   return result;

} /* htonl */

/*--------------------------------------------------------------------*/
/*       n t o h l                                                    */
/*                                                                    */
/*       Convert unsigned long from network to host byte order        */
/*--------------------------------------------------------------------*/

static unsigned long ntohl( const unsigned long input )
{
   unsigned char *p = (unsigned char *) &input;
   unsigned long result = 0;
   int i;

   for (i = 0 ; i < sizeof input; i++ )
      result = (result << 8) + p[i];

   return result;

} /* ntohl */

#endif

/*--------------------------------------------------------------------*/
/*    t o p e n p k                                                   */
/*                                                                    */
/*    Open "t" protocol to other system                               */
/*--------------------------------------------------------------------*/

#ifdef __TURBOC__
#pragma argsused
#endif

short topenpk(const boolean master)
{
   s_pktsize = r_pktsize = 1024;

   return DCP_OK;

} /* topenpk */

/*--------------------------------------------------------------------*/
/*    t g e t p k t                                                   */
/*                                                                    */
/*    Receive an "t" protocol packet of data from the other system    */
/*--------------------------------------------------------------------*/

short tgetpkt(char *packet, short *bytes)
{
   unsigned short recv;
   unsigned long nrecv;

   if (sread( (char *) &nrecv, sizeof nrecv, M_tPacketTimeout) < sizeof nrecv)
   {
      printmsg(0,"tgetpkt: Length read failed");
      return -1;
   }

   recv = (short) ntohl( nrecv );

   if ( recv > r_pktsize )
   {
      printmsg(0,"tgetpkt: Buffer overrun!  Wanted %d bytes, %d queued",
                  (int) *bytes,
                  (int) recv );
      return -1;
   }

   if (sread( packet, recv, M_tPacketTimeout) < recv)
   {
      printmsg(0,"tgetpkt: Data read failed for %d bytes", (int) recv);
      return -1;
   }

   remote_stats.packets++;

   *bytes = recv;

   return 0;

} /* tgetpkt */

/*--------------------------------------------------------------------*/
/*    t s e n d p k t                                                 */
/*                                                                    */
/*    Send an "t" protocol packet to the other system                 */
/*--------------------------------------------------------------------*/

short tsendpkt(char *ip, short len)
{

   unsigned long nxmit = htonl((unsigned long) len);

   if ( swrite( (char *) &nxmit, sizeof nxmit ) != sizeof nxmit )
      return -1;

   if ( len && (swrite( ip , len ) != len ))
      return -1;

   remote_stats.packets++;

   return 0;

} /* tsendpkt */


/*--------------------------------------------------------------------*/
/*    t c l o s e p k                                                 */
/*                                                                    */
/*    Shutdown "t" procotol with other system                         */
/*--------------------------------------------------------------------*/

short tclosepk()
{
   return DCP_OK;
} /* tclosepk */

#ifdef EXTRA_STUFF

/*--------------------------------------------------------------------*/
/*    t w r m s g                                                     */
/*                                                                    */
/*    Send a control message to remote system with "t" procotol       */
/*--------------------------------------------------------------------*/

short twrmsg(char *s)
{
   for( ; strlen(s) >= s_pktsize; s += s_pktsize)
   {
      short result = tsendpkt(s, (short) s_pktsize);
      if (result)
         return result;
   }

   return tsendpkt(s, (short) (strlen(s) + 1));

} /* twrmsg */

/*--------------------------------------------------------------------*/
/*    t r d m s g                                                     */
/*                                                                    */
/*    Read a control message from remote host with "t" protocol       */
/*--------------------------------------------------------------------*/

short trdmsg(char *s)
{
   for ( ;; )
   {
      short len;
      short result = tgetpkt( s, &len );
      if (result || (s[len-1] == '\0'))
         return result;
      s += len;
   } /* for */

} /* trdmsg */


/*--------------------------------------------------------------------*/
/*    t f i l e p k t                                                 */
/*                                                                    */
/*    Prepare for processing an "t" procotol file transfer            */
/*--------------------------------------------------------------------*/

short tfilepkt( void)
{

   return DCP_OK;

} /* tfilepkt */

/*--------------------------------------------------------------------*/
/*    t e o f                                                         */
/*                                                                    */
/*    Transmit "t" protocol end of file to the other system           */
/*--------------------------------------------------------------------*/

short teofpkt( void )
{
   if (tsendpkt("", 0))          /* Empty packet == EOF              */
      return DCP_FAILED;
   else
      return DCP_OK;
} /* teofpkt */

#endif
