/*--------------------------------------------------------------------*/
/*       s m t p d n s . c                                            */
/*                                                                    */
/*       TCP/IP domain lookup for UUPC/extended                       */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1997 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: smtpnetw.c 1.5 1997/11/28 04:52:10 ahd Exp $
 *
 *    $Log: smtpnetw.c $
 */

/*--------------------------------------------------------------------*/
/*                           Include files                            */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"
#include "smtpdns.h"

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

RCSID("$Id$");

currentfile();

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
