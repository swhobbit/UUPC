/*--------------------------------------------------------------------*/
/*       p o s 2 e r r . h                                            */
/*                                                                    */
/*       Report error message from OS/2 error message                 */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1996 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: pos2err.h 1.3 1994/12/22 00:13:28 ahd v1-12q $
 *
 *    Revision history:
 *    $Log: pos2err.h $
 *    Revision 1.3  1994/12/22 00:13:28  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.2  1994/01/01 19:09:22  ahd
 *    Annual Copyright Update
 *
 * Revision 1.1  1993/09/24  03:46:07  ahd
 * Initial revision
 *
 *
 */

void pOS2Err(const size_t lineno,
             const char *fname,
             const char *prefix,
             unsigned int rc);

/*--------------------------------------------------------------------*/
/*                 Macro for generic error messages from DOS          */
/*--------------------------------------------------------------------*/

#define printOS2error( x, rc )  pOS2Err( __LINE__, cfnptr, x, rc)
