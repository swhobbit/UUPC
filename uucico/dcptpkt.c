/*--------------------------------------------------------------------*/
/*       d c p t p k t . c                                            */
/*                                                                    */
/*       UUCP 't' protocol support                                    */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1995 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: dcptpkt.c 1.11 1994/12/22 04:13:38 ahd Exp $
 *
 *    Revision history:
 *    $Log: dcptpkt.c $
 *    Revision 1.11  1994/12/22 04:13:38  ahd
 *    Correct 't' protocol processing to use 512 messages with no header
 *
 *    Revision 1.10  1994/02/20 19:11:18  ahd
 *    IBM C/Set 2 Conversion, memory leak cleanup
 *
 * Revision 1.9  1994/02/19  05:07:45  ahd
 * Use standard first header
 *
 * Revision 1.8  1994/01/01  19:19:40  ahd
 * Annual Copyright Update
 *
 * Revision 1.7  1993/11/13  17:43:26  ahd
 * Correct debug level on sending empty packet message
 *
 * Revision 1.7  1993/11/13  17:43:26  ahd
 * Correct debug level on sending empty packet message
 *
 * Revision 1.6  1993/11/08  04:46:49  ahd
 * Correct bug which prevented proper EOF being handled
 *
 * Revision 1.5  1993/10/12  01:32:46  ahd
 * Normalize comments to PL/I style
 *
 * Revision 1.4  1993/09/24  03:43:27  ahd
 * Correct byte reordering functions
 *
 * Revision 1.4  1993/09/24  03:43:27  ahd
 * Correct byte reordering functions
 *
 * Revision 1.3  1993/09/21  01:42:13  ahd
 * Delete functions duplicated from dcpgpkt.c
 *
 * Revision 1.2  1993/09/20  04:48:25  ahd
 * TCP/IP support from Dave Watt
 * 't' protocol support
 * OS/2 2.x support (BC++ 1.0 for OS/2)
 *
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
/*                                                                    */
/*       Note:  Many of the functions (msg write, msg read, start     */
/*       of file, file eof) for this protocol are as the same as      */
/*       the functions for 'g' protocol, and we use the actual 'g'    */
/*       protocol copies as defined in dcpsys.c.                      */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"

#if defined(WIN32) || defined(_Windows)
#include "winsock.h"       /* Needed for byte ordering               */
#endif

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

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

#define TPACKETSIZE  512
#define TBUFSIZE     1024

#ifndef _WINSOCKAPI_

/*--------------------------------------------------------------------*/
/*     Network functions needed when no winsock functions available   */
/*--------------------------------------------------------------------*/

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
      p[3 - i] = (unsigned char) ((input >> (i*8)) & 0xff);

   printmsg(15,"htonl: %lx = %x %x %x %x",input, p[0], p[1], p[2], p[3] );

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

   printmsg(15,"ntonh: %x %x %x %x = %lx",p[0], p[1], p[2], p[3], result );

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

short topenpk(const KWBoolean master)
{
   s_pktsize = r_pktsize = TBUFSIZE;
                                    /* Fixed for 't' procotol         */

   printmsg(4, "topenpk: Timeout = %d sec, buffer size = %d bytes, "
               "packet size = %d bytes",
               M_tPacketTimeout,
               TBUFSIZE,
               TPACKETSIZE );

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
                  (int) r_pktsize,
                  (int) recv );
      return -1;
   }

   if ( ! recv )
      printmsg(4,"tgetpkt: Received empty packet");
   else if (sread( packet, recv, M_tPacketTimeout) < recv)
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

   if ( ! len )
      printmsg(4,"tsendpkt: Sending empty packet");
   else if ( swrite( ip , len ) != len )
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

/*--------------------------------------------------------------------*/
/*    t w r m s g                                                     */
/*                                                                    */
/*    Send a message to remote system                                 */
/*--------------------------------------------------------------------*/

short twrmsg( char *s )
{

   int len = strlen(s) + 1;

/*--------------------------------------------------------------------*/
/*                    Write the actual message out                    */
/*--------------------------------------------------------------------*/

   if (swrite( s,  len ) < len )
   {
      printmsg(0, "twrmsg: message write of %d bytes failed", len);
      return -1;
   }

/*--------------------------------------------------------------------*/
/*       Write out as bytes as needed to pad the total data length    */
/*       to TPACKETSIZE                                               */
/*--------------------------------------------------------------------*/

   len = TPACKETSIZE - ( len % TPACKETSIZE );
                                    /* Determine bytes needed to
                                       round out a full packet    */

   if ( len )
   {
      char buf[TPACKETSIZE];

      memset( buf, '\0', len );     /* Helps data compression     */

      if (swrite( buf,  len ) < len )
      {
         printmsg(0, "twrmsg: write of %d padding bytes failed", len);
         return -1;
      }

   } /* if ( len ) */

   remote_stats.packets++;
   return(0);

} /* ewrmsg */

/*--------------------------------------------------------------------*/
/*    t r d m s g                                                     */
/*                                                                    */
/*    Read a message from the remote system                           */
/*--------------------------------------------------------------------*/

short trdmsg( char *s)
{

   size_t bytes = 0;

   while (sread( s + bytes,
                 TPACKETSIZE,
                 M_tPacketTimeout) == TPACKETSIZE )
   {
      int column;

      remote_stats.packets++;

      for ( column = 0; column < TPACKETSIZE; column ++ )
      {
         if ( s[ bytes + column ] == '\0' )  /* End of the string?   */
            return 0;                        /* Yes --> Report success */
      }

      bytes += TPACKETSIZE;

   } /* while */

/*--------------------------------------------------------------------*/
/*    We didn't get the end of the string in time, report an error    */
/*--------------------------------------------------------------------*/


   return -1;

} /* trdmsg */
