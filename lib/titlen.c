/*--------------------------------------------------------------------*/
/*       t i t l e n . c                                              */
/*                                                                    */
/*       Set window title entry under Windows NT                      */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1998 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: TITLEN.C 1.8 1998/03/01 01:25:47 ahd v1-12v $
 *
 *    Revision history:
 *    $Log: TITLEN.C $
 *    Revision 1.8  1998/03/01 01:25:47  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.7  1997/03/31 07:07:22  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.6  1996/11/18 04:46:49  ahd
 *    Normalize arguments to bugout
 *    Reset title after exec of sub-modules
 *    Normalize host status names to use HS_ prefix
 *
 *    Revision 1.5  1996/01/01 20:54:43  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.4  1994/12/22 00:11:58  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.3  1994/05/31 00:08:11  ahd
 *    Add missing title.h header file
 *
 * Revision 1.2  1994/05/29  16:29:08  dmwatt
 * Add program name as prefix to user supplied title
 *
 * Revision 1.1  1994/04/24  20:07:58  dmwatt
 * Initial revision
 *
 */

/*--------------------------------------------------------------------*/
/*                        System header files                         */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"
#include <windows.h>

#include "title.h"
#include "timestmp.h"

/*--------------------------------------------------------------------*/
/*       s e t T i t l e                                              */
/*                                                                    */
/*       Set console window title under Windows NT                    */
/*--------------------------------------------------------------------*/

#ifdef TAPI_SUPPORT
#else
void setTitle( const char *fmt, ... )
{
   va_list arg_ptr;
   static char buf[BUFSIZ];

/*--------------------------------------------------------------------*/
/*       If a new title was supplied, format it.  (Otherwise, we use  */
/*       the previous contents of the title buffer.)                  */
/*--------------------------------------------------------------------*/

   if ( fmt )
   {
      va_start(arg_ptr,fmt);

      sprintf(buf, "%s: ", compilen);
      vsprintf(buf + strlen(buf), fmt, arg_ptr);

      va_end( arg_ptr );
   }

#if defined(WIN32) && defined(UUGUI)
   win_setTitle(buf);
#else
   SetConsoleTitle(buf);
#endif

} /* setTitle */
