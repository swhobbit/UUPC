/*--------------------------------------------------------------------*/
/*       t i t l e 2 . c                                              */
/*                                                                    */
/*       Set task List Entry under OS/2; based on code from Kai       */
/*       Uwe Rommel                                                   */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-2002 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: title2.c 1.16 2001/03/12 13:52:56 ahd v1-13k $
 *
 *    Revision history:
 *    $Log: title2.c $
 *    Revision 1.16  2001/03/12 13:52:56  ahd
 *    Annual copyright update
 *
 *    Revision 1.15  2000/05/12 12:29:45  ahd
 *    Annual copyright update
 *
 *    Revision 1.14  1999/01/08 02:20:43  ahd
 *    Convert currentfile() to RCSID()
 *
 *    Revision 1.13  1999/01/04 03:52:28  ahd
 *    Annual copyright change
 *
 *    Revision 1.12  1998/11/04 01:59:55  ahd
 *    Prevent buffer overflows
 *
 *    Revision 1.11  1998/03/01  01:25:44  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.10  1997/03/31 07:07:18  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.9  1996/01/01 20:55:06  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.8  1995/01/29 16:43:03  ahd
 *    IBM C/Set compiler warnings
 *
 *    Revision 1.7  1995/01/29 14:07:59  ahd
 *    Clean up most IBM C/Set Compiler Warnings
 *
 *    Revision 1.6  1995/01/07 16:14:56  ahd
 *    Change KWBoolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.5  1994/12/22 00:11:48  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.4  1994/05/31 00:08:11  ahd
 *    Add missing title.h header file
 *
 * Revision 1.3  1994/05/30  02:21:23  ahd
 * Properly restore original title on exit
 *
 * Revision 1.2  1994/05/04  02:02:19  ahd
 * Blank out title upon program exit
 *
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
#include "title.h"

RCSID("$Id: title2.c 1.16 2001/03/12 13:52:56 ahd v1-13k $");

void restoreOriginalTitle( void );

/*--------------------------------------------------------------------*/
/*       s e t T i t l e                                              */
/*                                                                    */
/*       Set task list entry for program under OS/2                   */
/*--------------------------------------------------------------------*/

void setTitle( const char *fmt, ... )
{
   HSWITCH hSwitch;
   SWCNTRL swctl;
   static SWCNTRL swctlSave;
   int pid = getpid();
   va_list arg_ptr;

   static KWBoolean firstPass = KWTrue;

   hSwitch = WinQuerySwitchHandle(NULL, (unsigned long) pid );
   WinQuerySwitchEntry(hSwitch, &swctl);

   if ( firstPass )
   {
      firstPass = KWFalse;
      atexit( restoreOriginalTitle );
      memcpy( &swctlSave, &swctl, sizeof swctl );
   }

   if ( fmt )
   {
      char buffer[KW_BUFSIZ];

      /* Clear old title         */
      memset( buffer,
              ' ',
              sizeof swctl.szSwtitle );

      va_start(arg_ptr,fmt);

      sprintf( buffer, "%s: ", compilen );

      vsprintf(buffer + strlen(buffer),
               fmt,
               arg_ptr);

      va_end( arg_ptr );
      memcpy(swctl.szSwtitle, buffer, sizeof swctl.szSwtitle );

   }
   else
      memcpy( &swctl, &swctlSave, sizeof swctl );

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
