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
 *    $Id: title2.c 1.1 1994/04/24 20:35:08 ahd Exp $
 *
 *    Revision history:
 *    $Log: title2.c $
 * Revision 1.1  1994/04/24  20:35:08  ahd
 * Initial revision
 *
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

void restoreOriginalTitle( void );

/*--------------------------------------------------------------------*/
/*                          Local variables                           */
/*--------------------------------------------------------------------*/


currentfile();

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

   static boolean firstPass = TRUE;

   if ( firstPass )
   {
      firstPass = FALSE;
      atexit( restoreOriginalTitle );
   }

   hSwitch = WinQuerySwitchHandle(NULL, pid );
   WinQuerySwitchEntry(hSwitch, &swctl);

   if ( fmt )
   {
      va_start(arg_ptr,fmt);

      sprintf( swctl.szSwtitle, "%s: ", compilen );

      vsprintf(swctl.szSwtitle + strlen(swctl.szSwtitle),
               fmt,
               arg_ptr);

      va_end( arg_ptr );
   }
   else
      *(swctl.szSwtitle) = '\0';

   memset( swctl.szSwtitle + strlen(swctl.szSwtitle),
           ' ',
           MAXNAMEL - strlen(swctl.szSwtitle));
                                       /* Clear old title         */

   WinChangeSwitchEntry(hSwitch, &swctl);

} /* setTitle */

/*--------------------------------------------------------------------*/
/*       r e s t o r e O r i g i n a l T i t l e                      */
/*                                                                    */
/*       Restore title saved on first call to setTitle                */
/*--------------------------------------------------------------------*/

void restoreOriginalTitle( void )
{

   setTitle( NULL );
}
