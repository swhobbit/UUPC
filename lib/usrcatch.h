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
 *    $Id: catcher.c 1.2 1993/09/20 04:38:11 ahd Exp $
 *
 *    Revision history:
 *    $Log: catcher.c $
 */

extern int raised;

void
#ifdef __TURBOC__
__cdecl
#endif
usrhandler( int sig );
