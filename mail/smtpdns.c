/*--------------------------------------------------------------------*/
/*       s m t p d n s . c                                            */
/*                                                                    */
/*       TCP/IP domain lookup for UUPC/extended                       */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1999 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: smtpdns.c 1.3 1999/01/04 03:54:27 ahd Exp $
 *
 *    $Log: smtpdns.c $
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

RCSID("$Id: smtpdns.c 1.3 1999/01/04 03:54:27 ahd Exp $");

/*--------------------------------------------------------------------*/
/*       g e t H o s t N a m e F r o m S o c k e t                    */
/*                                                                    */
/*       Report host name of the connected socket                     */
/*--------------------------------------------------------------------*/

KWBoolean
getHostNameFromSocket( SMTPConnection *connection )
{
   static const char mName[] = "getHostNameFromSocket";
   SOCKADDR_IN sin;
   LPHOSTENT phe;
   int len = sizeof sin;

/*--------------------------------------------------------------------*/
/*         Stary by getting the IP address of the remote peer         */
/*--------------------------------------------------------------------*/

   memset( &sin, '\0', sizeof sin );

   if ( getpeername( connection->handle,
                     (SOCKADDR *) &sin,
                     &len ))
   {
      int wsErr = WSAGetLastError();
      printWSerror("getservbyname", wsErr);
      return KWFalse;
   }

   sprintf( connection->hostAddr, "[%s]",inet_ntoa( sin.sin_addr ));

   phe = gethostbyaddr( (char *) &sin.sin_addr.s_addr,
                        sizeof sin.sin_addr.s_addr,
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
