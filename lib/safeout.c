/*--------------------------------------------------------------------*/
/*    s a f e o u t . c                                               */
/*                                                                    */
/*    Console I/O functions for use during interrupt processing       */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Since C I/O functions are not safe inside signal routines,      */
/*    the code uses conditionals to use system-level DOS and OS/2     */
/*    services.  Another option is to set global flags and do any     */
/*    I/O operations outside the signal handler.                      */
/*--------------------------------------------------------------------*/

#define __MSC                 /* Make Borland C++ 2.0 act like MS C  */

#include <stdio.h>

#ifdef WIN32
    #include <windows.h>
    #include <string.h>
#else
#if defined( FAMILYAPI )
    #define INCL_NOCOMMON
    #define INCL_NOPM
    #define INCL_VIO
    #define INCL_KBD
    #include <os2.h>
    #include <string.h>
#else
    #include <dos.h>
    #include <bios.h>
    #include <conio.h>
#endif /* FAMILYAPI */
#endif /* WIN32 */

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "safeio.h"

/*--------------------------------------------------------------------*/
/*    s a f e o u t                                                   */
/*                                                                    */
/*    Outputs a string using system level calls. from MicroSoft       */
/*    Programmer's Workbench QuickHelp samples                        */
/*--------------------------------------------------------------------*/

void safeout( char *str )
{

#ifdef _Windows
   fputs( str , stdout );
   return;
#else
#if defined( WIN32 )
   HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
   DWORD dwBytesWritten;

   WriteFile(hStdOut, str, (DWORD)strlen(str), &dwBytesWritten, NULL);
   return;
   
#else
#if defined( FAMILYAPI )
   VioWrtTTY( str, strlen( str ), 0 );
#else
    union REGS inregs, outregs;

    inregs.h.ah = 0x0e;
    while( *str )
    {
        inregs.h.al = *str++;
        int86( 0x10, &inregs, &outregs );
    }

    safeflush();              /* Flush keyboard                      */

#endif /* FAMILYAPI */
#endif /* WIN32 */
#endif /* _Windows */
} /* safeout */
