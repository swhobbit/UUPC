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
 *    $Id: dcpepkt.c 1.7 1993/11/13 17:43:26 ahd Exp $
 *
 *    Revision history:
 *    $Log: dcptpkt.c $
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
/*       TCP/IP ("e") protocol.                                       */
/*                                                                    */
/*       Protocol for over reliable (TCP/IP) paths.                   */
/*                                                                    */
/*       'e' procotol is done by simply transmitting the length of    */
/*       the entire file as ASCII text, pads the length out to 20     */
/*       characters with NULLs, then sends the entire file with no    */
/*       further packet headers.                                      */
/*                                                                    */
/*       Note:  Many of the functions (msg write, msg read, start     */
/*       of file, file eof) for this protocol are as the same as      */
/*       the functions for 'g' protocol, and we use the actual 'g'    */
/*       protocol copies as defined in dcpsys.c.                      */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#if defined(WIN32) || defined(_Windows)
#include "winsock.h"       /* Needed for byte ordering                 */
#endif

#ifdef WIN32
#include <io.h>
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

static unsigned long efilelength;
static unsigned long ebytesdone;
static boolean emaster;

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
   s_pktsize = r_pktsize = 1024;    /* Fixed for 'e' procotol         */

   emaster = master;
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

   if (ebytesdone > efilelength)
   {
      printmsg(0, "egetpkt:  received file larger than length!");
      return -1;
   }

   recv = min(efilelength - ebytesdone, r_pktsize);

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
      printmsg(4,"esendpkt: Sending empty packet");
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
} /* tclosepk */

short efilepkt(void)
{
   ebytesdone = 0;
   efilelength = 0;

   return DCP_OK;
}

short eeofpkt(void)
{
   printmsg(5, "eeofpkt: called");

   if (ebytesdone != efilelength)
   {
      printmsg(0, "eeofpkt:  I don't think we're done! "
         " done = %lu, length = %lu\n", ebytesdone, efilelength);

      return DCP_FAILED;
   }
   return DCP_OK;
}

boolean estartfile(FILE *xfer_stream)
{
   char startbuf[20];
   printmsg(5, "estartfile: called");

/* If we're the master, then send our file length, otherwise receive it */

   ebytesdone = 0;

   if (emaster) {
      efilelength = _filelength(xfer_stream->_file);

      printmsg(5, "estartpacket: sending file length is %lu", efilelength);

      memset(startbuf, 0, sizeof startbuf);
      sprintf(startbuf, "%ld", efilelength);
      if (swrite( (char *)&startbuf, sizeof startbuf ) <
         sizeof startbuf)
      {
         printmsg(0, "estartfile: Length write failed");
         return FALSE;
      }

   } else {

      if (sread( (char *) &startbuf, sizeof startbuf, M_tPacketTimeout) <
         sizeof startbuf)
      {
         printmsg(0,"estartfile: Length read failed");
         return FALSE;
      }

/* Read the file length */

      efilelength = strtol(startbuf, NULL, 10);
      printmsg(5, "estartpacket: received file length %lu", efilelength);
   }

   return TRUE;
}
