#ifndef USRCATCH_H
#define USRCATCH_H

/*--------------------------------------------------------------------*/
/*       u s r c a t c h                                              */
/*                                                                    */
/*       SIGUSR handler for UUPC/extended                             */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1993 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: usrcatch.h 1.1 1993/09/29 04:56:11 ahd Exp $
 *
 *    Revision history:
 *    $Log: usrcatch.h $
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
