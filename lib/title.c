/*--------------------------------------------------------------------*/
/*       t i t l e 2 . c                                              */
/*                                                                    */
/*       Set task List Entry under Windows and DOS (stub only)        */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1999 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: title.c 1.10 1999/01/04 03:52:28 ahd Exp $
 *
 *    Revision history:
 *    $Log: title.c $
 *    Revision 1.10  1999/01/04 03:52:28  ahd
 *    Annual copyright change
 *
 *    Revision 1.9  1998/03/01 01:25:41  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.8  1997/03/31 07:07:12  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.7  1996/11/18 04:46:49  ahd
 *    Normalize arguments to bugout
 *    Reset title after exec of sub-modules
 *    Normalize host status names to use HS_ prefix
 *
 *    Revision 1.6  1996/01/01 20:52:47  ahd
 *    Annual Copyright Update
 *
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

RCSID("$Id$");

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
   static char buf[BUFSIZ];

   if ( fmt )
   {
      va_start(arg_ptr,fmt);

      sprintf( buf, "%s: ", compilen );

      vsprintf(buf + strlen(buf),
               fmt,
               arg_ptr);

      va_end( arg_ptr );
   }

   SetWindowText(hOurWindow, buf);
#else

#ifdef DOSTITLE
   va_list arg_ptr;
   static char escape = 0x1b;

   if ( fmt == NULL )               /* Use previous format string?   */
        return;                     /* Not saved in DOS, just return */

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
