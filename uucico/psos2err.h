/*--------------------------------------------------------------------*/
/*       p s o s 2 e r r . h                                          */
/*                                                                    */
/*       Report error message from OS/2 TCP/IP socket error messages  */
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
 *    $Id: psos2err.h 1.5 1997/04/24 01:37:40 ahd v1-12u $
 *
 *    Revision history:
 *    $Log: psos2err.h $
 *    Revision 1.5  1997/04/24 01:37:40  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.4  1996/01/01 21:23:45  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.3  1994/12/22 00:40:40  ahd
 *    Annual Copyright Update
 *
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
