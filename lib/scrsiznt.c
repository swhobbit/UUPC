Here's the SCRSIZE.C for NT that you wanted.

(It assumes that "scrsize.h" defines PAGESIZE, which seemed reasonable,
given the context.)

- Dave

--- Begin SCRSIZEN.C ---
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

   if ( result != 0 )
   {
      printmsg(0,"Windows/NT error code %d retrieving console information",
               GetLastError() );
      return PAGESIZE;
   }

   return info.dwSize.Y;
} /* scrsize */
--- End SCRSIZEN.C ---
-- 
Dave Watt (N1HMB)                              Home: 617-391-6119
dmwatt@smersh.cambridge.ma.us (But I live in Medford)
