/*--------------------------------------------------------------------*/
/*       t i t l e 2 . c                                              */
/*                                                                    */
/*       Set task List Entry under Windows and DOS (stub only)        */
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
 *    $Id: title.c 1.1 1994/04/24 20:35:08 ahd Exp $
 *
 *    Revision history:
 *    $Log: title.c $
 * Revision 1.1  1994/04/24  20:35:08  ahd
 * Initial revision
 *
 */

/*--------------------------------------------------------------------*/
/*                        System header files                         */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"

#ifdef _Windows
#include <windows.h>
#include <winutil.h>
#include <timestmp.h>
#include <stdarg.h>
#endif

/*--------------------------------------------------------------------*/
/*       s e t T i t l e                                              */
/*                                                                    */
/*       Set task list entry for program under Windows; also          */
/*       provides stub function for DOS                               */
/*--------------------------------------------------------------------*/

#ifdef __TURBOC__
#pragma argsused
#endif

void setTitle( const char *fmt, ... )
{

#ifdef _Windows
   va_list arg_ptr;
   char buf[BUFSIZ];

   va_start(arg_ptr,fmt);

   sprintf( buf, "%s: ", compilen );

   vsprintf(buf + strlen(buf),
            fmt,
            arg_ptr);

   va_end( arg_ptr );

   SetWindowText(hOurWindow, buf);
#endif

} /* setTitle */
