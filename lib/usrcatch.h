#ifndef USRCATCH_H
#define USRCATCH_H

/*--------------------------------------------------------------------*/
/*       u s r c a t c h                                              */
/*                                                                    */
/*       SIGUSR handler for UUPC/extended                             */
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
 *    $Id: usrcatch.h 1.10 2001/03/12 13:50:09 ahd v1-13k $
 *
 *    Revision history:
 *    $Log: usrcatch.h $
 *    Revision 1.10  2001/03/12 13:50:09  ahd
 *    Annual copyright update
 *
 *    Revision 1.9  2000/05/12 12:40:50  ahd
 *    Annual copyright update
 *
 *    Revision 1.8  1999/01/04 03:55:03  ahd
 *    Annual copyright change
 *
 *    Revision 1.7  1998/03/01 01:28:05  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.6  1997/03/31 07:07:42  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.5  1996/01/01 20:59:15  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.4  1994/12/22 00:14:20  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.3  1994/01/01 19:10:09  ahd
 *    Annual Copyright Update
 *
 * Revision 1.2  1993/09/29  13:20:49  ahd
 * Provide default values for user signals
 *
 * Revision 1.1  1993/09/29  04:56:11  ahd
 * Initial revision
 *
 */

extern unsigned long raised;

void
#ifdef __TURBOC__
__cdecl
#endif
usrhandler( int sig );

#ifndef SIGUSR1
#define SIGUSR1  16     /* User-defined signal 1 */
#endif

#ifndef SIGUSR1
#define SIGUSR2  17     /* User-defined signal 2 */
#endif

#endif
