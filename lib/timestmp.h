#ifndef TIMESTMP_H
#define TIMESTMP_H

/*--------------------------------------------------------------------*/
/*       t i m e s t m p . h                                          */
/*                                                                    */
/*       Defines functions included in timestmp.c for UUPC/extended   */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-2001 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: timestmp.h 1.13 2000/05/12 12:40:50 ahd v1-13g $
 *
 *    Revision history:
 *    $Log: timestmp.h $
 *    Revision 1.13  2000/05/12 12:40:50  ahd
 *    Annual copyright update
 *
 *    Revision 1.12  1999/01/08 02:21:16  ahd
 *    Convert currentfile() to RCSID()
 *
 *    Revision 1.11  1999/01/04 03:55:03  ahd
 *    Annual copyright change
 *
 *    Revision 1.10  1998/03/01 01:27:48  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.9  1997/03/31 07:07:09  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.8  1996/01/01 20:57:39  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.7  1995/09/11 00:25:56  ahd
 *    Correct spelling error in comment
 *
 *    Revision 1.6  1995/01/02 05:04:57  ahd
 *    Pass 2 of integrating SYS file support from Mike McLagan
 *
 *    Revision 1.5  1994/12/22 00:13:57  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.4  1994/01/01 19:09:57  ahd
 *    Annual Copyright Update
 *
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

extern const char *compilev;            /* UUPC version number           */
extern const char *compilep;            /* Suite name (UUPC/extended)    */
extern const char *compilet;            /* UUPC compile time             */
extern const char *compiled;            /* UUPC compile date             */
extern char *compilen;            /* Current program name          */
extern char *compilew;            /* Current environment           */

void banner (char **argv);

#endif
