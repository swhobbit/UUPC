/*--------------------------------------------------------------------*/
/*    b u g o u t . c                                                 */
/*                                                                    */
/*    Support routines for UUPC/extended                              */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-2000 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: bugout.c 1.22 1999/01/08 02:20:52 ahd Exp $
 *
 *    Revision history:
 *    $Log: bugout.c $
 *    Revision 1.22  1999/01/08 02:20:52  ahd
 *    Convert currentfile() to RCSID()
 *
 *    Revision 1.21  1999/01/04 03:52:28  ahd
 *    Annual copyright change
 *
 *    Revision 1.20  1998/11/24 03:08:50  ahd
 *    Use BREAKPOINT, not divide by 0, to enter debugger
 *
 *    Revision 1.19  1998/05/11 01:20:48  ahd
 *    Spin off log file before aborting
 *
 *    Revision 1.18  1998/04/24 03:30:13  ahd
 *    Don't abort via memory error if running GUI
 *
 *    Revision 1.17  1998/04/08 11:32:07  ahd
 *    Force a crash, not non-zero exit, when debugging
 *
 *    Revision 1.16  1998/03/01 01:23:05  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.15  1997/03/31 06:58:10  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.14  1996/11/18 04:46:49  ahd
 *    Normalize arguments to bugout
 *    Reset title after exec of sub-modules
 *    Normalize host status names to use HS_ prefix
 *
 *    Revision 1.13  1996/01/01 20:49:48  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.12  1995/02/24 00:37:28  ahd
 *    Move panic_rc from catcher.c into bugout.c, keeps catcher from
 *    being included into every module.
 *
 *    Revision 1.11  1994/12/22 00:07:21  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.10  1994/12/09 03:42:09  ahd
 *    All suppressbeep support to allow NOT making any sound
 *
 *     Revision 1.9  1994/02/20  19:05:02  ahd
 *     IBM C/Set 2 Conversion, memory leak cleanup
 *
 *     Revision 1.8  1994/02/19  04:39:11  ahd
 *     Use standard first header
 *
 *     Revision 1.7  1994/02/19  03:47:24  ahd
 *     Use standard first header
 *
 *     Revision 1.7  1994/02/19  03:47:24  ahd
 *     Use standard first header
 *
 *     Revision 1.6  1994/02/18  23:07:33  ahd
 *     Use standard first header
 *
 *     Revision 1.5  1994/01/01  19:00:29  ahd
 *     Annual Copyright Update
 *
 *     Revision 1.4  1993/10/12  00:49:39  ahd
 *     Normalize comments
 *
 */

#include "uupcmoah.h"

#if defined(_Windows) || defined(WIN32)
#include <windows.h>
#endif

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "catcher.h"
#include "timestmp.h"
#include "logger.h"

#ifdef _Windows
#include "winutil.h"
#endif

/*--------------------------------------------------------------------*/
/*                           Global variables                         */
/*--------------------------------------------------------------------*/

int panic_rc = 69;                  /* Actually defined in catcher.h,
                                       but not all routines need the
                                       catcher.                      */

RCSID("$Id: bugout.c 1.22 1999/01/08 02:20:52 ahd Exp $");

/*--------------------------------------------------------------------*/
/*    b u g o u t                                                     */
/*                                                                    */
/*    Perform a panic() exit from UUPC/extended                       */
/*--------------------------------------------------------------------*/

void bugout(const char UUFAR *fname, const size_t lineno )
{

#ifdef _Windows
   char buf[BUFSIZ];
#endif

  printmsg(0,"%s aborting at line %d in file %s",
              compilen, lineno, localFName(fname));
  copylog();

#ifdef _Windows

   sprintf(buf,"%s aborting at line %d in file %s",
              compilen, lineno, localFName(fname));

   MessageBox( hOurWindow, buf, compilen, MB_ICONEXCLAMATION );

#else

   if ( ! bflag[F_SUPPRESSBEEP] )
      fputc('\a',stderr);

#endif

#if defined(UDEBUG32) && !defined(UUGUI)

/*--------------------------------------------------------------------*/
/*                   Crash the program if debugging                   */
/*--------------------------------------------------------------------*/

   BREAKPOINT
#endif

   exit(panic_rc);

} /*bugout */
