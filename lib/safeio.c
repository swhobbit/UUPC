/*--------------------------------------------------------------------*/
/*    s a f e i o . c                                                 */
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

#if defined( FAMILYAPI )
#if defined( WIN32 )
#include <windows.h>
#include <string.h>
#else
    #define INCL_NOCOMMON
    #define INCL_NOPM
    #define INCL_VIO
    #define INCL_KBD
    #include <os2.h>
    #include <string.h>
#endif
#else /* FAMILYAPI */
#include <conio.h>
    #include <dos.h>
    #include <bios.h>
#endif

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "safeio.h"

/*--------------------------------------------------------------------*/
/*                    Local Structures and defines                    */
/*--------------------------------------------------------------------*/

#ifdef FAMILYAPI
/* KBDKEYINFO structure, for KbdCharIn and KbdPeek */

#ifdef __GNUC__
typedef struct _KBDKEYINFO {  /* kbci */
   UCHAR chChar;
   UCHAR chScan;
   UCHAR fbStatus;
   UCHAR bNlsShift;
   USHORT   fsState;
   ULONG time;
}KBDKEYINFO;

#ifndef IO_WAIT
#define IO_WAIT      0
#endif

#ifndef FINAL_CHAR_IN
#define FINAL_CHAR_IN       0x40
#endif

#endif
#endif

/*--------------------------------------------------------------------*/
/*    s a f e i n                                                     */
/*                                                                    */
/*    Inputs a character using system level calls.  From MicroSoft    */
/*    Programmer's Workbench QuickHelp samples                        */
/*--------------------------------------------------------------------*/

int safein( void )
{
#ifdef _Windows
   return getchar( );
#else
#if defined( FAMILYAPI )
#if defined( WIN32 )
   CHAR ch;
   DWORD dwBytesRead;
   HANDLE hStdIn = GetStdHandle(STD_INPUT_HANDLE);

   ReadFile(hStdIn, &ch, 1, &dwBytesRead, NULL);      

   return ch;
#else /* WIN32 */
    KBDKEYINFO kki;

    KbdCharIn( &kki, IO_WAIT, 0 );
    return kki.chChar;
#endif /* WIN32 */
#else /* FAMILYAPI */

    int c = (_bios_keybrd( _KEYBRD_READ ) & 0xff );
    union REGS inregs, outregs;

    inregs.h.ah = 0x0e;
    inregs.h.al = (char) c;
    int86( 0x10, &inregs, &outregs );
    return c;

#endif
#endif
} /* safein */

/*--------------------------------------------------------------------*/
/*    s a f e p e e k                                                 */
/*                                                                    */
/*    Determine if a character is waiting at the keyboard for us.     */
/*    Written by ahd based on safein (above).                         */
/*--------------------------------------------------------------------*/

boolean safepeek( void )
{

/*--------------------------------------------------------------------*/
/*                         OS/2 keyboard peek                         */
/*--------------------------------------------------------------------*/
#ifdef _Windows
   return 0;
#else
#if defined( FAMILYAPI )
#ifdef WIN32
   HANDLE hStdIn = GetStdHandle(STD_INPUT_HANDLE);
   INPUT_RECORD Buffer;
   DWORD nEventsRead;

   PeekConsoleInput(hStdIn, &Buffer, 1, &nEventsRead);

   if (nEventsRead != 0 && Buffer.EventType == KEY_EVENT)
      return TRUE;
   return FALSE;
#else
    KBDKEYINFO kki;

    KbdPeek( &kki, 0 );
    return (kki.fbStatus & FINAL_CHAR_IN);
#endif /* WIN32 */
#else

/*--------------------------------------------------------------------*/
/*                         DOS Keyboard peek                          */
/*--------------------------------------------------------------------*/

    return (_bios_keybrd( _KEYBRD_READY ) & 0xff );
#endif
#endif

} /* safepeek */

/*--------------------------------------------------------------------*/
/*    s a f e f l u s h                                               */
/*                                                                    */
/*    Flush the keyboard look ahead buffer.                           */
/*    Written by ahd based on safein (above).                         */
/*--------------------------------------------------------------------*/

void safeflush( void )
{

#ifdef _Windows
   return;
#else

/*--------------------------------------------------------------------*/
/*                         OS/2 keyboard flush                        */
/*--------------------------------------------------------------------*/

#if defined( FAMILYAPI )
#ifdef WIN32
   HANDLE hStdIn = GetStdHandle(STD_INPUT_HANDLE);
   FlushConsoleInputBuffer(hStdIn);
#else
    KbdFlushBuffer( 0 );      /* That's all!  (Makes you love rich
                                 API's, doesn't it?)                 */

#endif /* WIN32 */
#else

/*--------------------------------------------------------------------*/
/*                         DOS Keyboard flush                         */
/*--------------------------------------------------------------------*/

   union REGS regs;

   regs.h.ah = 0x0C;       /* Flush buffer, read keyboard            */
   regs.h.al = 0x00;       /* Don't actually read keyboard           */
   intdos( &regs, &regs ); /* Make it happen                         */

#endif
#endif

} /* safeflush */
