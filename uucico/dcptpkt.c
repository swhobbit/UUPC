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
 *    $Id: lib.h 1.11 1993/08/08 17:39:55 ahd Exp $
 *
 *    Revision history:
 *    $Log: dcpfpkt.c $
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

#ifdef defined(WIN32) || defined(_Windows)
#include <winsock.h>       // Needed for byte ordering
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
   unsigned char *p = (char *) &result;
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
   unsigned char *p = (char *) &input;
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
/*    t c l o s e p k                                                 */
/*                                                                    */
/*    Shutdown "t" procotol with other system                         */
/*--------------------------------------------------------------------*/

short tclosepk()
{
   return DCP_OK;
} /* tclosepk */

/*--------------------------------------------------------------------*/
/*    t w r m s g                                                     */
/*                                                                    */
/*    Send a control message to remote system with "t" procotol       */
/*--------------------------------------------------------------------*/

short twrmsg(char *str)
{
   for(; strlen(s) >= s_pktsize; s += s_pktsize)
   {
      short result = tsendpkt(s, s_pktsize);
      if (result)
         return result;
   }

   return tsendpkt(s, strlen(s)+1);

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
      short result = ggetpkt( s, &len );
      if (result || (s[len-1] == '\0'))
         return result;
      s += len;
   } /* for */

} /* trdmsg */

/*--------------------------------------------------------------------*/
/*    t g e t p k t                                                   */
/*                                                                    */
/*    Receive an "t" protocol packet of data from the other system    */
/*--------------------------------------------------------------------*/

short tgetpkt(char *packet, short *bytes)
{
   unsigned long recv;
   unsigned long nrecv;

   if ( sread( &nrecv, sizeof nrecv, E_tPacketTimeout) < E_tPacketTimeout)
   {
      printmsg(0,"tgetpkt: Length read failed");
      return DCP_FAILED:
   }

   recv = ntohl( nrecv );

   if ( sread( packet, bytes, E_tPacketTimeout) < E_tPacketTimeout)
   {
      printmsg(0,"tgetpkt: Data read failed");
      return DCP_FAILED:
   }

   remote_stats.packets++;

} /* tgetpkt */

/*--------------------------------------------------------------------*/
/*    t s e n d p k t                                                 */
/*                                                                    */
/*    Send an "t" protocol packet to the other system                 */
/*--------------------------------------------------------------------*/

short tsendpkt(char *ip, short len)
{

   unsigned long nxmit = htol((unsigned long) len);

   if ( swrite( &nxmit, sizeof nxmit )) != sizeof nxmit )
      return DCP_FAILED;

   if ( xmit && (swrite( ip , len ) != len ))
      return DCP_FAILED;

   remote_stats.packets++;

   return DCP_OK;

} /* tsendpkt */

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
   return tsendpkt( NULL, 0 );

} /* teofpkt */
