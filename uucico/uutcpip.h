#ifndef _UUTCPIP_H
#define _UUTCPIP_H

/*--------------------------------------------------------------------*/
/*       u u t c p i p . h                                            */
/*                                                                    */
/*       TCP/IP definitions for UUPC/extended                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1998 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: uutcpip.h 1.4 1997/11/28 23:13:35 ahd v1-12u $
 *
 *    Revision history:
 *    $Log: uutcpip.h $
 *    Revision 1.4  1997/11/28 23:13:35  ahd
 *    Additional auditing, including DNS support
 *
 *    Revision 1.3  1997/11/28 04:52:53  ahd
 *    Initial UUSMTPD OS/2 support
 *
 *    Revision 1.2  1997/11/21 18:11:31  ahd
 *    Add definition of UUCP port number
 *
 *    Revision 1.1  1997/06/03 03:26:38  ahd
 *    Initial revision
 *
 */

#if defined(__OS2__)
#define OS2
#define BSD_SELECT
#include <types.h>
#include <sys\select.h>
#include <sys\socket.h>
#include <netinet\in.h>
#include <netdb.h>
#include <utils.h>
#include <sys\time.h>
#include <nerrno.h>

#define WSAGetLastError() sock_errno()
#define closesocket(s) soclose(s)
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define SOCKADDR_IN struct sockaddr_in
#define SOCKADDR struct sockaddr
#define LPHOSTENT struct hostent *
#define LPSERVENT struct servent *
#define PSOCKADDR struct sockaddr *

#ifdef __OS2__
typedef int SOCKET;
#endif

#include "../uucico/psos2err.h"        /* Emulation of winsock error msgs */

#else /* DOS and Windows */
#include <windows.h>
#include "winsock.h"

#include "pwserr.h"           /* Windows sockets error messages       */

#ifdef WIN32
#include "pnterr.h"
#endif

#ifdef _Windows
#include "pwinsock.h"      /* definitions for 16 bit Winsock functions  */
#endif

#endif /* DOS and Windows */

#define UUCP_SERVICE "uucp"
#define UUCP_PORT    540

#define NETDEBUG 4

#endif /* _UUTCPIP_H */
