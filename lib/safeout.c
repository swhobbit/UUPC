/*--------------------------------------------------------------------*/
/*    s a f e o u t . c                                               */
/*                                                                    */
/*    Console I/O functions for use during interrupt processing       */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1994 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: safeout.c 1.6 1993/10/12 00:49:39 ahd Exp $
 *
 *    Revision history:
 *    $Log: safeout.c $
 *     Revision 1.6  1993/10/12  00:49:39  ahd
 *     Normalize comments
 *
 *     Revision 1.5  1993/10/03  00:05:32  ahd
 *     Only define currentfile() under Windows NT
 *
 *     Revision 1.4  1993/09/20  04:39:51  ahd
 *     OS/2 2.x support
 *
 *     Revision 1.3  1993/07/20  21:42:43  dmwatt
 *     Don't rely on standard I/O under Windows/NT
 *
 */

/*--------------------------------------------------------------------*/
/*    Since C I/O functions are not safe inside signal routines,      */
/*    the code uses conditionals to use system-level DOS and OS/2     */
/*    services.  Another option is to set global flags and do any     */
/*    I/O operations outside the signal handler.                      */
/*--------------------------------------------------------------------*/

#define __MSC                 /* Make Borland C++ 2.0 act like MS C   */

#include <stdio.h>

#ifdef WIN32
    #include <windows.h>
    #include <string.h>

#elif defined( FAMILYAPI ) || defined(__OS2__)

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

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "safeio.h"

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

#if defined(WIN32)
currentfile();
#endif

#if defined(WIN32)
static HANDLE hConsoleOut = INVALID_HANDLE_VALUE;

void InitConsoleOutputHandle(void)
{
   hConsoleOut = CreateFile("CONOUT$", GENERIC_READ | GENERIC_WRITE,
      FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
      FILE_ATTRIBUTE_NORMAL, 0);

   if (hConsoleOut == INVALID_HANDLE_VALUE) {
      printmsg(0, "InitConsoleHandles:  could not open console handles!");
      panic();
   }
}
#endif

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

#elif defined( WIN32 )

   DWORD dwBytesWritten;

   if (hConsoleOut == INVALID_HANDLE_VALUE)
      InitConsoleOutputHandle();

   WriteFile(hConsoleOut, str, (DWORD)strlen(str), &dwBytesWritten, NULL);
   return;

#elif defined( FAMILYAPI ) || defined(__OS2__)

   VioWrtTTY( str, strlen( str ), 0 );

#else
    union REGS inregs, outregs;

    inregs.h.ah = 0x0e;
    while( *str )
    {
        inregs.h.al = *str++;
        int86( 0x10, &inregs, &outregs );
    }

    safeflush();              /* Flush keyboard                       */

#endif /* _Windows */
} /* safeout */
