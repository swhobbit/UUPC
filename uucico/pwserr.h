#ifndef _PWSERR_H
#define _PWSERR_H

/*--------------------------------------------------------------------*/
/*       p w s e r r . h                                              */
/*                                                                    */
/*       Report error message from Windows sockets error message      */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-2002 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: pwserr.h 1.11 2001/03/12 13:51:16 ahd v1-13k $
 *
 *    Revision history:
 *    $Log: pwserr.h $
 *    Revision 1.11  2001/03/12 13:51:16  ahd
 *    Annual copyright update
 *
 *    Revision 1.10  2000/05/12 12:41:43  ahd
 *    Annual copyright update
 *
 *    Revision 1.9  1999/02/21 04:11:15  ahd
 *    Add flag for determination localhost is connecting socket
 *
 *    Revision 1.8  1999/01/08 02:21:16  ahd
 *    Convert currentfile() to RCSID()
 *
 *    Revision 1.7  1999/01/04 03:55:33  ahd
 *    Annual copyright change
 *
 *    Revision 1.6  1998/03/01 01:42:47  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.5  1997/04/24 01:37:45  ahd
 *    Annual Copyright Update
 *
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
             const char UUFAR *fname,
             const char *prefix,
             int rc);

/*--------------------------------------------------------------------*/
/*                 Macro for generic error messages from DOS          */
/*--------------------------------------------------------------------*/

#define printWSerror( x, rc )  pWSErr( __LINE__, _rcsId, x, rc)

#endif /* _PWSERR_H */
