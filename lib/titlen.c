/*--------------------------------------------------------------------*/
/*       t i t l e n . c                                              */
/*                                                                    */
/*       Set window title entry under Windows NT                      */
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
 *    $Id: lib.h 1.20 1994/02/20 19:16:21 ahd Exp $
 *
 *    Revision history:
 *    $Log: lib.h $
 */

/*--------------------------------------------------------------------*/
/*                        System header files                         */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"
#include <windows.h>

/*--------------------------------------------------------------------*/
/*       s e t T i t l e                                              */
/*                                                                    */
/*       Set console window title under Windows NT                    */
/*--------------------------------------------------------------------*/

void setTitle( const char *fmt, ... )
{
   va_list arg_ptr;
   char buf[BUFSIZ];

   va_start(arg_ptr,fmt);

   vsprintf(buf, fmt, arg_ptr);

   va_end( arg_ptr );

   SetConsoleTitle(buf);

} /* setTitle */
