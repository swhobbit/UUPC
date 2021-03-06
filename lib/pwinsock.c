/*--------------------------------------------------------------------*/
/*       p w i n s o c k . c                                          */
/*                                                                    */
/*       WinSock support for Windows 3.1                              */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Copyright (c) David M. Watt 1993, All Right Reserved            */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-2002 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: pwinsock.c 1.20 2001/03/12 13:54:04 ahd v1-13k $
 *
 *    $Log: pwinsock.c $
 *    Revision 1.20  2001/03/12 13:54:04  ahd
 *    Annual Copyright update
 *
 *    Revision 1.19  2000/05/12 12:30:48  ahd
 *    Annual copyright update
 *
 *    Revision 1.18  1999/01/08 02:20:48  ahd
 *    Convert currentfile() to RCSID()
 *
 *    Revision 1.17  1999/01/04 03:52:55  ahd
 *    Annual copyright change
 *
 *    Revision 1.16  1998/03/01 01:24:55  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.15  1997/12/22 16:48:38  ahd
 *    Add support for 16 bit inet_ntoa for Borland C++ Windows UUCICO
 *
 *    Revision 1.14  1997/05/13 04:16:04  dmwatt
 *    Support setsockopt
 *
 *    Revision 1.13  1997/03/31 07:05:51  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.12  1996/01/01 20:50:29  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.11  1995/01/07 16:14:02  ahd
 *    Change boolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.10  1994/12/22 00:10:32  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.9  1994/02/19 04:45:42  ahd
 *    Use standard first header
 *
 * Revision 1.8  1994/02/19  04:10:40  ahd
 * Use standard first header
 *
 * Revision 1.7  1994/02/19  03:56:34  ahd
 * Use standard first header
 *
 * Revision 1.6  1994/02/18  23:13:37  ahd
 * Use standard first header
 *
 * Revision 1.5  1994/01/01  19:04:24  ahd
 * Annual Copyright Update
 *
 * Revision 1.4  1993/10/30  22:02:31  dmwatt
 * Correct host byte ordering errors
 *
 * Revision 1.3  1993/10/12  00:47:04  ahd
 * Normalize comments
 *
 * Revision 1.2  1993/09/27  00:45:20  ahd
 * Add missing def for shutdown()
 *
 * Revision 1.1  1993/09/20  04:39:51  ahd
 * Initial revision
 *
 */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"

#include <windows.h>

#include "winsock.h"

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "pwinsock.h"      /* definitions for 16 bit Winsock functions */

/*--------------------------------------------------------------------*/
/*                  Define pointers to the functions                  */
/*--------------------------------------------------------------------*/

int PASCAL FAR (*pWSAStartup)(WORD wVersionRequired, LPWSADATA lpWSAData);

int PASCAL FAR (*pWSACleanup)(void);

int PASCAL FAR (*pWSAGetLastError)(void);

BOOL PASCAL FAR (*pWSAIsBlocking)(void);

int PASCAL FAR (*pWSACancelBlockingCall)(void);

struct hostent FAR * PASCAL FAR (*pgethostbyname)(const char FAR * name);

struct servent FAR * PASCAL FAR (*pgetservbyname)(const char FAR * name,
                                                  const char FAR * proto);

unsigned long PASCAL FAR (*pinet_addr)(const char FAR * cp);

SOCKET PASCAL FAR (*psocket)(int af,
                             int type,
                             int protocol);

int PASCAL FAR (*pconnect)(SOCKET s,
                           const struct sockaddr FAR *name,
                           int namelen);

SOCKET PASCAL FAR (*paccept)(SOCKET s,
                             struct sockaddr FAR *addr,
                             int FAR *addrlen);

int PASCAL FAR (*plisten) (SOCKET s, int backlog);

int PASCAL FAR (*pbind)(SOCKET s,
                       const struct sockaddr FAR *addr,
                       int namelen);

int PASCAL FAR (*pselect)(int nfds,
                          fd_set FAR *readfds,
                          fd_set FAR *writefds,
                          fd_set FAR *exceptfds,
                          const struct timeval FAR *timeout);

int PASCAL FAR (*psend)(SOCKET s,
                        const char FAR * buf,
                        int len,
                        int flags);

int PASCAL FAR (*precv) (SOCKET s,
                         char FAR * buf,
                         int len,
                         int flags);

int PASCAL FAR (*pclosesocket)(SOCKET s);

u_short PASCAL FAR (*pntohs)(u_short netshort);

u_short PASCAL FAR (*phtons)(u_short hostshort);

u_long PASCAL FAR (*pntohl) (u_long netlong);

u_long PASCAL FAR (*phtonl) (u_long hostlong);

int PASCAL FAR (*psetsockopt) (SOCKET s,
                 int level,
                 int optname,
                 char FAR *optval,
                 int optlen);

int PASCAL FAR (*pshutdown) (SOCKET s, int how);

/*--------------------------------------------------------------------*/
/*                          Local variables                           */
/*--------------------------------------------------------------------*/

static HINSTANCE hWinsock = NULL;

RCSID("$Id: pwinsock.c 1.20 2001/03/12 13:54:04 ahd v1-13k $");

/*--------------------------------------------------------------------*/
/*       p W i n S o c k I n i t                                      */
/*                                                                    */
/*       Initialize winsock.dll for Windows 3.1                       */
/*--------------------------------------------------------------------*/

KWBoolean pWinSockInit( void )
{

/*--------------------------------------------------------------------*/
/*                          Load the library                          */
/*--------------------------------------------------------------------*/

   if (!hWinsock)
      hWinsock = LoadLibrary("WINSOCK.DLL");
   else {
      printmsg(0,"pWinSockInit: called twice with no termination");
      panic();
   }

   if (!hWinsock)
   {
      printmsg(0, "pWinSockInit: could not find Winsock.DLL");
      return KWFalse;
   }

/*--------------------------------------------------------------------*/
/*       Initialize pointers to functions with in the libraries       */
/*--------------------------------------------------------------------*/

#ifdef __TURBOC__
#pragma warn -sus
#endif

   paccept                = GetProcAddress(hWinsock, (LPSTR)MAKELONG(  1,0));
   pbind                  = GetProcAddress(hWinsock, (LPSTR)MAKELONG(  2,0));
   pclosesocket           = GetProcAddress(hWinsock, (LPSTR)MAKELONG(  3,0));
   pconnect               = GetProcAddress(hWinsock, (LPSTR)MAKELONG(  4,0));
   phtonl                 = GetProcAddress(hWinsock, (LPSTR)MAKELONG(  8,0));
   phtons                 = GetProcAddress(hWinsock, (LPSTR)MAKELONG(  9,0));
   pinet_addr             = GetProcAddress(hWinsock, (LPSTR)MAKELONG( 10,0));
   plisten                = GetProcAddress(hWinsock, (LPSTR)MAKELONG( 13,0));
   pntohl                 = GetProcAddress(hWinsock, (LPSTR)MAKELONG( 14,0));
   pntohs                 = GetProcAddress(hWinsock, (LPSTR)MAKELONG( 15,0));
   precv                  = GetProcAddress(hWinsock, (LPSTR)MAKELONG( 16,0));
   pselect                = GetProcAddress(hWinsock, (LPSTR)MAKELONG( 18,0));
   psend                  = GetProcAddress(hWinsock, (LPSTR)MAKELONG( 19,0));
   psetsockopt            = GetProcAddress(hWinsock, (LPSTR)MAKELONG( 21,0));
   pshutdown              = GetProcAddress(hWinsock, (LPSTR)MAKELONG( 22,0));
   psocket                = GetProcAddress(hWinsock, (LPSTR)MAKELONG( 23,0));
   pgethostbyname         = GetProcAddress(hWinsock, (LPSTR)MAKELONG( 52,0));
   pgetservbyname         = GetProcAddress(hWinsock, (LPSTR)MAKELONG( 55,0));
   pWSAGetLastError       = GetProcAddress(hWinsock, (LPSTR)MAKELONG(111,0));
   pWSACancelBlockingCall = GetProcAddress(hWinsock, (LPSTR)MAKELONG(113,0));
   pWSAIsBlocking         = GetProcAddress(hWinsock, (LPSTR)MAKELONG(114,0));
   pWSAStartup            = GetProcAddress(hWinsock, (LPSTR)MAKELONG(115,0));
   pWSACleanup            = GetProcAddress(hWinsock, (LPSTR)MAKELONG(116,0));

#ifdef __TURBOC__
#pragma warn .sus
#endif

   return KWTrue;

} /* pWinSockInit */

/*--------------------------------------------------------------------*/
/*       p W i n S o c k E x i t                                      */
/*                                                                    */
/*       Clean up Windows 3.x winsock.dll                             */
/*--------------------------------------------------------------------*/

void pWinSockExit( void )
{

   if (hWinsock)
   {
      FreeLibrary(hWinsock);
      hWinsock = (HINSTANCE) NULL;
   }

} /* pWinSockExit */

#ifdef __BORLANDC__

/*--------------------------------------------------------------------*/
/*       i n e t _ n t o a                                            */
/*                                                                    */
/*       Convert internet address to printable (normally in           */
/*       library, missing for WinSock for some reason)                */
/*--------------------------------------------------------------------*/

char FAR *
PASCAL FAR inet_ntoa (struct in_addr in)
{
   char buf[sizeof (unsigned long) * 4 + 1 ];
   int subscript;
   unsigned long addr = ntohl( in.S_un.S_addr );

   for ( subscript = 0; subscript < sizeof (unsigned long); subscript ++ )
   {
      if ( subscript )
         strcat( buf, "." );
      else
         buf[0] = '\0';

      sprintf( buf + strlen( buf ), "%.3ld", addr & 0x000000ff );
      addr >>= 8;
   }

   return buf;
}

#endif
