#ifndef WINSOCK_H
#define WINSOCK_H

/*--------------------------------------------------------------------*/
/*          p w i n s o c k . h                                       */
/*                                                                    */
/*          Windows Sockets API definitions for Windows 3.x           */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Copyright (c) David M. Watt 1993, All Right Reserved            */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1994 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: pwinsock.h 1.3 1993/10/30 22:02:31 dmwatt Exp $
 *
 *    Revision history:
 *    $Log: pwinsock.h $
 * Revision 1.3  1993/10/30  22:02:31  dmwatt
 * Correct host byte ordering errors
 *
 * Revision 1.2  1993/09/27  00:50:57  ahd
 * Add missing shutdown definition
 *
 * Revision 1.1  1993/09/20  04:51:31  ahd
 * Initial revision
 *
 * Revision 1.1  1993/09/20  04:51:31  ahd
 * Initial revision
 *
 */

#define WSAStartup pWSAStartup
#define WSACleanup pWSACleanup
#define WSAGetLastError pWSAGetLastError
#define WSAIsBlocking pWSAIsBlocking
#define WSACancelBlockingCall pWSACancelBlockingCall
#define gethostbyname pgethostbyname
#define getservbyname pgetservbyname
#define inet_addr pinet_addr
#define socket psocket
#define connect pconnect
#define accept paccept
#define listen plisten
#define bind pbind
#define select pselect
#define send psend
#define recv precv
#define closesocket pclosesocket
#define ntohs pntohs
#define htons pntohs
#define ntohl pntohl
#define htonl phtonl
#define shutdown pshutdown

extern int PASCAL FAR (*pWSAStartup)(WORD wVersionRequired, LPWSADATA lpWSAData);
extern int PASCAL FAR (*pWSACleanup)(void);
extern int PASCAL FAR (*pWSAGetLastError)(void);
extern BOOL PASCAL FAR (*pWSAIsBlocking)(void);
extern int PASCAL FAR (*pWSACancelBlockingCall)(void);
extern struct hostent FAR * PASCAL FAR (*pgethostbyname)(const char FAR * name);
extern struct servent FAR * PASCAL FAR (*pgetservbyname)(const char FAR * name,
        const char FAR * proto);
extern unsigned long PASCAL FAR (*pinet_addr)(const char FAR * cp);
extern SOCKET PASCAL FAR (*psocket)(int af, int type, int protocol);
extern int PASCAL FAR (*pconnect)(SOCKET s, const struct sockaddr FAR *name, int namelen);
extern SOCKET PASCAL FAR (*paccept)(SOCKET s, struct sockaddr FAR *addr,
        int FAR *addrlen);
extern int PASCAL FAR (*plisten) (SOCKET s, int backlog);
extern int PASCAL FAR (*bind)(SOCKET s, const struct sockaddr FAR *addr, int namelen);
extern int PASCAL FAR (*pselect)(int nfds, fd_set FAR *readfds, fd_set FAR *writefds,
        fd_set FAR *exceptfds, const struct timeval FAR *timeout);
extern int PASCAL FAR (*psend)(SOCKET s, const char FAR * buf, int len, int flags);
extern int PASCAL FAR (*precv) (SOCKET s, char FAR * buf, int len, int flags);
extern int PASCAL FAR (*pclosesocket)(SOCKET s);

extern u_short PASCAL FAR (*pntohs)(u_short netshort);

extern u_short PASCAL FAR (*phtons)(u_short hostshort);

extern u_long PASCAL FAR (*phtonl) (u_long hostlong);

extern u_long PASCAL FAR (*pntohl) (u_long netlong);

extern int PASCAL FAR (*pshutdown) (SOCKET s, int how);

/*--------------------------------------------------------------------*/
/*     Our one "real" prototype to initialize the other functions     */
/*--------------------------------------------------------------------*/

boolean pWinSockInit( void );

void pWinSockExit( void );

#endif
