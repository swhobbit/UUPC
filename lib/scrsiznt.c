/*
 *    $Id: SCRSIZNT.C 1.3 1992/12/30 13:09:25 dmwatt Exp $
 *
 *    $Log: SCRSIZNT.C $
 *     Revision 1.3  1992/12/30  13:09:25  dmwatt
 *     Correct boolean compare
 *
 */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include <stdio.h>

#include <windows.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "scrsize.h"

/*--------------------------------------------------------------------*/
/*    s c r s i z e                                                   */
/*                                                                    */
/*    Return screen size under Windows/NT                             */
/*--------------------------------------------------------------------*/

short scrsize( void )
{
   CONSOLE_SCREEN_BUFFER_INFO info;
   BOOL result;
   HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);

   result = GetConsoleScreenBufferInfo(hStdout, &info);

   if ( result != TRUE )
   {
      printmsg(0,"Windows/NT error code %d retrieving console information",
               GetLastError() );
      return PAGESIZE;
   }

   return info.dwSize.Y;
} /* scrsize */

