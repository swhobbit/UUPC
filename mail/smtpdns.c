/*--------------------------------------------------------------------*/
/*       s m t p d n s . c                                            */
/*                                                                    */
/*       TCP/IP domain lookup for UUPC/extended                       */
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
 *    $Id: smtpdns.c 1.6 2000/05/12 12:35:45 ahd v1-13g $
 *
 *    $Log: smtpdns.c $
 *    Revision 1.6  2000/05/12 12:35:45  ahd
 *    Annual copyright update
 *
 *    Revision 1.5  1999/02/21 04:07:20  ahd
 *    Optimize processing to avoid DNS lookups on localhost,
 *    possible when disconnected from the net
 *
 *    Revision 1.4  1999/01/08 02:21:05  ahd
 *    Convert currentfile() to RCSID()
 *
 *    Revision 1.3  1999/01/04 03:54:27  ahd
 *    Annual copyright change
 *
 *    Revision 1.2  1998/03/01 01:32:38  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.1  1997/11/28 23:11:38  ahd
 *    Initial revision
 *
 */

/*--------------------------------------------------------------------*/
/*                           Include files                            */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"
#include "smtpdns.h"

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

RCSID("$Id: smtpdns.c 1.6 2000/05/12 12:35:45 ahd v1-13g $");

/*--------------------------------------------------------------------*/
/*       g e t H o s t N a m e F r o m S o c k e t                    */
/*                                                                    */
/*       Report host name of the connected socket                     */
/*--------------------------------------------------------------------*/

KWBoolean
getHostNameFromSocket( SMTPConnection *connection )
{
   static const char mName[] = "getHostNameFromSocket";
   SOCKADDR_IN peerSin;
   SOCKADDR_IN localSin;
   LPHOSTENT phe;
   int len = sizeof localSin;


   memset( &localSin, '\0', sizeof localSin );

   if ( getsockname( connection->handle,
                     (SOCKADDR *) &localSin,
                     &len ))
   {
      int wsErr = WSAGetLastError();
      printWSerror("getsockname", wsErr);
      return KWFalse;
   }

/*--------------------------------------------------------------------*/
/*             Get the IP address of the remote peer                  */
/*--------------------------------------------------------------------*/

   len = sizeof peerSin;
   memset( &peerSin, '\0', sizeof peerSin );

   if ( getpeername( connection->handle,
                     (SOCKADDR *) &peerSin,
                     &len ))
   {
      int wsErr = WSAGetLastError();
      printWSerror("getpeername", wsErr);
      return KWFalse;
   }

   sprintf( connection->hostAddr, "[%s]",inet_ntoa( peerSin.sin_addr ));

/*--------------------------------------------------------------------*/
/*            Short circuit connections from local system             */
/*--------------------------------------------------------------------*/

   /* Local loop back address? */
   if (ntohl(peerSin.sin_addr.s_addr) == 0x7f000001)
   {
      strcpy(connection->hostName, "localhost");
      connection->localhost = KWTrue;
      return KWTrue;
   }

   /* Local IP address (serial/Ethernet link?) */
   if (peerSin.sin_addr.s_addr == localSin.sin_addr.s_addr)
   {
      strcpy(connection->hostName, E_domain);
      connection->localhost = KWTrue;
      return KWTrue;
   }

/*--------------------------------------------------------------------*/
/*           Not special address, ask DNS for the full name           */
/*--------------------------------------------------------------------*/

   phe = gethostbyaddr( (char *) &peerSin.sin_addr.s_addr,
                        sizeof peerSin.sin_addr.s_addr,
                        AF_INET );

   if ( phe == NULL )
   {
      int wsErr = h_errno;
      printmsg(0, "%s: Cannot locate hostname for IP address %s",
                  mName,
                  connection->hostAddr );
      strcpy( connection->hostName, connection->hostAddr );
      connection->reverseLookup = KWFalse;
      printWSerror("gethostbyaddr", wsErr);
   } /* if ( phe == NULL )  */
   else {
      strncpy(connection->hostName,
              phe->h_name,
              sizeof connection->hostName );
      connection->reverseLookup = KWTrue;
      connection->hostName[ sizeof connection->hostName - 1 ] = '\0';
   }

   return KWTrue;

} /* getHostNameFromSocket */
