#ifndef TIMESTMP_H
#define TIMESTMP_H

/*--------------------------------------------------------------------*/
/*       t i m e s t m p . h                                          */
/*                                                                    */
/*       Defines functions included in timestmp.c for UUPC/extended   */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1994 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: timestmp.h 1.3 1993/11/13 17:38:09 ahd Exp $
 *
 *    Revision history:
 *    $Log: timestmp.h $
 *     Revision 1.3  1993/11/13  17:38:09  ahd
 *     Alter comment for compilew
 *
 *     Revision 1.2  1993/07/31  16:28:59  ahd
 *     Changes in support of Robert Denny's Windows Support
 *
 *
 *       07 Mar 92   -  Add current program name
 *
 *       08 Sep 90   -  Create for banner function (moved from
 *                      host.c)  ahd
 */

extern char compilev[];           /* UUPC version number           */
extern char compilep[];           /* UUPC Program name             */
extern char compilet[];           /* UUPC compile time             */
extern char compiled[];           /* UUPC compile date             */
extern char *compilen;            /* Current program name          */
extern char *compilew;            /* Current environment           */

void banner (char **argv);

#endif
