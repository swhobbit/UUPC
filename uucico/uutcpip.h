#ifndef _UUTCPIP_H
#define _UUTCPIP_H

/*--------------------------------------------------------------------*/
/*       u u t c p i p . h                                            */
/*                                                                    */
/*       TCP/IP definitions for UUPC/extended                         */
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
 *    $Id: lib.h 1.39 1997/05/11 04:28:53 ahd v1-12s $
 *
 *    Revision history:
 *    $Log: lib.h $
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
#define LPHOSTENT struct hostent *
#define LPSERVENT struct servent *
#define PSOCKADDR struct sockaddr *

#ifdef __OS2__
typedef int SOCKET;
#endif

#include "psos2err.h"        /* Emulation of winsock error msgs */

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

#endif /* _UUTCPIP_H */
