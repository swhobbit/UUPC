/*--------------------------------------------------------------------*/
/*    b u g o u t . c                                                 */
/*                                                                    */
/*    Support routines for UUPC/extended                              */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1995 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: bugout.c 1.11 1994/12/22 00:07:21 ahd v1-12n $
 *
 *    Revision history:
 *    $Log: bugout.c $
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

#ifdef _Windows
#include <windows.h>
#endif

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "catcher.h"
#include "timestmp.h"

#ifdef _Windows
#include "winutil.h"
#endif

/*--------------------------------------------------------------------*/
/*                           Global variables                         */
/*--------------------------------------------------------------------*/

int panic_rc = 69;                  /* Actually defined in catcher.h,
                                       but not all routines need the
                                       catcher.                      */

/*--------------------------------------------------------------------*/
/*    b u g o u t                                                     */
/*                                                                    */
/*    Perform a panic() exit from UUPC/extended                       */
/*--------------------------------------------------------------------*/

void bugout( const size_t lineno, const char *fname )
{

#ifdef _Windows
   char buf[BUFSIZ];
#endif

  printmsg(0,"%s aborting at line %d in file %s",
              compilen, lineno, fname );

#ifdef _Windows

   sprintf(buf,"%s aborting at line %d in file %s",
              compilen, lineno, fname );

   MessageBox( hOurWindow, buf, compilen, MB_ICONEXCLAMATION );

#else

   if ( ! bflag[F_SUPPRESSBEEP] )
      fputc('\a',stderr);

#endif

   exit(panic_rc);
} /*bugout */
