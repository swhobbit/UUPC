/*--------------------------------------------------------------------*/
/*       p o s 2 e r r . h                                            */
/*                                                                    */
/*       Report error message from OS/2 error message                 */
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
 *    $Id: pos2err.h 1.10 2001/03/12 13:50:09 ahd v1-13k $
 *
 *    Revision history:
 *    $Log: pos2err.h $
 *    Revision 1.10  2001/03/12 13:50:09  ahd
 *    Annual copyright update
 *
 *    Revision 1.9  2000/05/12 12:40:50  ahd
 *    Annual copyright update
 *
 *    Revision 1.8  1999/01/08 02:21:16  ahd
 *    Convert currentfile() to RCSID()
 *
 *    Revision 1.7  1999/01/04 03:55:03  ahd
 *    Annual copyright change
 *
 *    Revision 1.6  1998/03/01 01:27:05  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.5  1997/03/31 07:05:33  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.4  1996/01/01 20:57:52  ahd
 *    Annual Copyright Update
 *
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
             const char UUFAR *fname,
             const char *prefix,
             unsigned int rc);

/*--------------------------------------------------------------------*/
/*                 Macro for generic error messages from DOS          */
/*--------------------------------------------------------------------*/

#define printOS2error( x, rc )  pOS2Err( __LINE__, _rcsId, x, rc)
