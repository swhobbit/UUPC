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
 *    $Id: titlen.c 1.1 1994/04/24 20:07:58 dmwatt Exp $
 *
 *    Revision history:
 *    $Log: titlen.c $
 * Revision 1.1  1994/04/24  20:07:58  dmwatt
 * Initial revision
 *
 */

/*--------------------------------------------------------------------*/
/*                        System header files                         */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"
#include <windows.h>

#include "timestmp.h"

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

   sprintf(buf, "%s: ", compilen);
   vsprintf(buf + strlen(buf), fmt, arg_ptr);

   va_end( arg_ptr );

   SetConsoleTitle(buf);

} /* setTitle */
