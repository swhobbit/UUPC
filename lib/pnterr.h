/*--------------------------------------------------------------------*/
/*       p n t e r r . h                                              */
/*                                                                    */
/*       Report error message from OS/2 error message                 */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) David M. Watt 1993, All Rights Reserved   */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1999 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: pnterr.h 1.8 1999/01/04 03:55:03 ahd Exp $
 *
 *    Revision history:
 *    $Log: pnterr.h $
 *    Revision 1.8  1999/01/04 03:55:03  ahd
 *    Annual copyright change
 *
 *    Revision 1.7  1998/03/01 01:27:02  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.6  1997/03/31 07:05:26  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.5  1996/01/01 21:00:30  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.4  1994/12/22 00:13:23  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.3  1994/01/01 19:09:17  ahd
 *    Annual Copyright Update
 *
 * Revision 1.2  1993/09/26  03:34:48  dmwatt
 * Update copyright
 *
 * Revision 1.1  1993/09/25  03:02:55  ahd
 * Initial revision
 *
 * Revision 1.1  1993/09/24  03:46:07  ahd
 * Initial revision
 */

void pNTErr(const size_t lineno,
             const char UUFAR *fname,
             const char *prefix,
             DWORD rc);

/*--------------------------------------------------------------------*/
/*                 Macro for generic error messages from DOS          */
/*--------------------------------------------------------------------*/

#define printNTerror( x, rc )  pNTErr( __LINE__, _rcsId, x, rc)
