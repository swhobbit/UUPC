/*--------------------------------------------------------------------*/
/*    s a f e o u t . c                                               */
/*                                                                    */
/*    Console I/O functions for use during interrupt processing       */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1999 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: safeout.c 1.17 1998/03/01 01:25:09 ahd v1-13f ahd $
 *
 *    Revision history:
 *    $Log: safeout.c $
 *    Revision 1.17  1998/03/01 01:25:09  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.16  1997/03/31 07:06:13  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.15  1996/01/01 20:53:30  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.14  1995/01/29 14:07:59  ahd
 *    Clean up most IBM C/Set Compiler Warnings
 *
 *    Revision 1.13  1994/12/22 00:10:51  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.12  1994/02/20 19:05:02  ahd
 *    IBM C/Set 2 Conversion, memory leak cleanup
 *
 *     Revision 1.11  1994/02/19  04:46:09  ahd
 *     Use standard first header
 *
 *     Revision 1.10  1994/02/19  04:11:15  ahd
 *     Use standard first header
 *
 *     Revision 1.9  1994/02/19  03:57:28  ahd
 *     Use standard first header
 *
 *     Revision 1.8  1994/02/18  23:14:21  ahd
 *     Use standard first header
 *
 *     Revision 1.7  1994/01/01  19:04:55  ahd
 *     Annual Copyright Update
 *
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

#include "uupcmoah.h"

/*--------------------------------------------------------------------*/
/*    Since C I/O functions are not safe inside signal routines,      */
/*    the code uses conditionals to use system-level DOS and OS/2     */
/*    services.  Another option is to set global flags and do any     */
/*    I/O operations outside the signal handler.                      */
/*--------------------------------------------------------------------*/

#ifdef WIN32

    #include <windows.h>

#elif defined( FAMILYAPI ) || defined(__OS2__)

    #define INCL_NOCOMMON
    #define INCL_NOPM
    #define INCL_VIO
    #define INCL_KBD
    #include <os2.h>

#else

    #include <dos.h>
    #include <bios.h>
    #include <conio.h>

#endif /* FAMILYAPI */

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

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

   VioWrtTTY( str, (unsigned short) strlen( str ), 0 );

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
