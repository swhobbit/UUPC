/*--------------------------------------------------------------------*/
/*       t i t l e 2 . c                                              */
/*                                                                    */
/*       Set task List Entry under OS/2; based on code from Kai       */
/*       Uwe Rommel                                                   */
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
#define  INCL_PM
#include <os2.h>
#include <process.h>
#include <stdarg.h>

#include "timestmp.h"

/*--------------------------------------------------------------------*/
/*       s e t T i t l e                                              */
/*                                                                    */
/*       Set task list entry for program under OS/2                   */
/*--------------------------------------------------------------------*/

void setTitle( const char *fmt, ... )
{
   HSWITCH hSwitch;
   SWCNTRL swctl;
   int pid = getpid();
   va_list arg_ptr;

   hSwitch = WinQuerySwitchHandle(NULL, pid );
   WinQuerySwitchEntry(hSwitch, &swctl);

   va_start(arg_ptr,fmt);

   sprintf( swctl.szSwtitle, "%s: ", compilen );

   vsprintf(swctl.szSwtitle + strlen(swctl.szSwtitle),
            fmt,
            arg_ptr);

   va_end( arg_ptr );

   memset( swctl.szSwtitle + strlen(swctl.szSwtitle),
           ' ',
           MAXNAMEL - strlen(swctl.szSwtitle));
                                       /* Clear old title         */

   WinChangeSwitchEntry(hSwitch, &swctl);

} /* setTitle */
