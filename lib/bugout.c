/*--------------------------------------------------------------------*/
/*    b u g o u t . c                                                 */
/*                                                                    */
/*    Support routines for UUPC/extended                              */
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
 *    $Id: bugout.c 1.7 1994/02/19 03:47:24 ahd Exp $
 *
 *    Revision history:
 *    $Log: bugout.c $
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

#ifdef _Windows

#include "uupcmoah.h"

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
   fputc('\a',stderr);
#endif

   exit(panic_rc);
} /*bugout */
