#ifndef TIMESTMP_H
#define TIMESTMP_H

/*--------------------------------------------------------------------*/
/*       t i m e s t m p . h                                          */
/*                                                                    */
/*       Defines functions included in timestmp.c for UUPC/extended   */
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
 *    $Id: lib.h 1.10 1993/07/22 23:26:19 ahd Exp $
 *
 *    Revision history:
 *    $Log: lib.h $
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
extern char *compilew;            /* Current Windows version       */

void banner (char **argv);

#endif
