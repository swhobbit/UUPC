/*--------------------------------------------------------------------*/
/*       p w s e r r . h                                              */
/*                                                                    */
/*       Report error message from Windows sockets error message      */
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
 *    $Id: pwserr.h 1.4 1996/01/01 21:23:59 ahd v1-12r $
 *
 *    Revision history:
 *    $Log: pwserr.h $
 *    Revision 1.4  1996/01/01 21:23:59  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.3  1994/12/22 00:40:50  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.2  1994/01/01 19:23:58  ahd
 *    Annual Copyright Update
 *
 * Revision 1.1  1993/10/02  23:13:51  dmwatt
 * Initial revision
 *
 * Revision 1.1  1993/10/02  23:13:51  dmwatt
 * Initial revision
 *
 * Revision 1.1  1993/09/24  03:46:07  dmwatt
 * Initial revision
 *
 *
 */

void pWSErr(const size_t lineno,
             const char *fname,
             const char *prefix,
             int rc);

/*--------------------------------------------------------------------*/
/*                 Macro for generic error messages from DOS          */
/*--------------------------------------------------------------------*/

#define printWSerror( x, rc )  pWSErr( __LINE__, cfnptr, x, rc)
