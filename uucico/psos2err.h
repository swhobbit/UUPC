/*--------------------------------------------------------------------*/
/*       p s o s 2 e r r . h                                          */
/*                                                                    */
/*       Report error message from OS/2 TCP/IP socket error messages  */
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
 *    $Id: psos2err.h 1.2 1994/05/23 23:19:29 dmwatt v1-12k $
 *
 *    Revision history:
 *    $Log: psos2err.h $
 *    Revision 1.2  1994/05/23 23:19:29  dmwatt
 *    Add reference for h_errno
 *
 * Revision 1.1  1994/04/26  03:34:33  ahd
 * Initial revision
 *
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

extern int h_errno;
