/*--------------------------------------------------------------------*/
/*       p s o s 2 e r r . h                                          */
/*                                                                    */
/*       Report error message from OS/2 TCP/IP socket error messages  */
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
 *    $Id: psos2err.h 1.1 1994/01/01 19:23:58 ahd Exp $
 *
 *    Revision history:
 *    $Log: pwserr.h $
 * Revision 1.1  1994/01/01  19:23:58  ahd
 * Initial revision
 *
 */

void pSOS2Err(const size_t lineno,
             const char *fname,
             const char *prefix,
             int rc);

/*--------------------------------------------------------------------*/
/*                 Macro for generic error messages from DOS          */
/*--------------------------------------------------------------------*/

#define printWSerror( x, rc )  pSOS2Err( __LINE__, cfnptr, x, rc)

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define SOCKADDR_IN struct sockaddr_in
#define LPHOSTENT struct hostent *
#define LPSERVENT struct servent *
#define PSOCKADDR struct sockaddr *
