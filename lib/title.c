/*--------------------------------------------------------------------*/
/*       t i t l e 2 . c                                              */
/*                                                                    */
/*       Set task List Entry under Windows and DOS (stub only)        */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1996 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: title.c 1.5 1995/02/20 00:40:12 ahd v1-12q $
 *
 *    Revision history:
 *    $Log: title.c $
 *    Revision 1.5  1995/02/20 00:40:12  ahd
 *    Correct C compiler warnings
 *
 *    Revision 1.4  1994/12/22 00:11:42  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.3  1994/05/31 00:08:11  ahd
 *    Add missing title.h header file
 *
 * Revision 1.2  1994/05/04  02:40:52  ahd
 * Add missing stdarg.h
 *
 * Revision 1.1  1994/04/24  20:35:08  ahd
 * Initial revision
 *
 */

/*--------------------------------------------------------------------*/
/*                        System header files                         */
/*--------------------------------------------------------------------*/

#include <stdarg.h>

#include "uupcmoah.h"
#include "title.h"
#include "timestmp.h"

#ifdef _Windows
#include <windows.h>
#include <winutil.h>
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
#else

#ifdef DOSTITLE
   va_list arg_ptr;
   static char escape = 0x1b;
   fprintf( stderr, "\n\n\n\n");    /* Insure three empty lines      */
   fprintf( stderr, "%c[3A", escape );
                                    /* Up three lines                */
   fprintf( stderr, "%c[s%c[1;1f", escape, escape );
                                    /* Save cursor pos, home cursor  */
   fprintf( stderr, "%c[0K", escape );
                                    /* Delete to end of line         */

   fprintf( stderr, "%s: ", compilen );

   va_start(arg_ptr,fmt);
   vfprintf( stderr, fmt, arg_ptr );
   va_end( arg_ptr );

   fprintf( stderr, "%c[u", escape );  /* restore cursor position    */
#endif

#endif

} /* setTitle */
