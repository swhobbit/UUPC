/*--------------------------------------------------------------------*/
/*       s s l e e p . c                                              */
/*                                                                    */
/*       Smart sleep routines for UUPC/extended                       */
/*                                                                    */
/*       DOS and Windows NT support by Dave Watt, Windows 3.x         */
/*       support by Robert Denny, and OS/2 support by Drew            */
/*       Derbyshire                                                   */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1998 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: ssleep.c 1.30 1997/03/31 07:06:48 ahd v1-12u $
 *
 *    Revision history:
 *    $Log: ssleep.c $
 *    Revision 1.30  1997/03/31 07:06:48  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.29  1996/01/01 20:52:02  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.28  1995/05/07 14:39:30  ahd
 *    Add comment check for user break, for use with Windows software
 *
 *    Revision 1.27  1995/02/22 12:14:24  ahd
 *    Correct 16 bit compiler warning errors
 *
 *    Revision 1.26  1995/01/29 16:43:03  ahd
 *    IBM C/Set compiler warnings
 *
 *    Revision 1.25  1995/01/07 16:14:45  ahd
 *    Change KWBoolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.24  1994/12/22 00:11:18  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.23  1994/12/09 03:42:09  ahd
 *    All suppressbeep support to allow NOT making any sound
 *
 *     Revision 1.22  1994/10/03  03:24:10  ahd
 *     Correct spelling of TimerId
 *
 *     Revision 1.21  1994/10/03  01:04:39  Software
 *     Trap in ability to set Windows timer
 *
 *     Revision 1.20  1994/02/19  04:46:54  ahd
 *     Use standard first header
 *
 *     Revision 1.19  1994/02/19  04:12:02  ahd
 *     Use standard first header
 *
 *     Revision 1.18  1994/02/19  03:58:42  ahd
 *     Use standard first header
 *
 *     Revision 1.17  1994/02/18  23:15:19  ahd
 *     Use standard first header
 *
 *     Revision 1.16  1994/01/01  19:05:54  ahd
 *     Annual Copyright Update
 *
 *     Revision 1.15  1993/12/29  03:39:15  dmwatt
 *     Do not include winutil under Windows NT
 *
 *     Revision 1.14  1993/12/26  16:20:17  ahd
 *     Reorganize to break out OS dependent functions
 *
 *     Revision 1.13  1993/10/12  01:20:43  ahd
 *     Normalize comments to PL/I style
 *
 *     Revision 1.12  1993/10/12  00:41:51  ahd
 *     Normalize comments
 *
 *     Revision 1.11  1993/10/09  15:47:51  rhg
 *     ANSIify the source
 *
 *     Revision 1.10  1993/09/27  02:42:11  ahd
 *     Use signed number for delay computations under DOS
 *
 *     Revision 1.9  1993/09/24  03:43:27  ahd
 *     Use OS/2 error messages
 *
 *     Revision 1.8  1993/09/20  04:39:51  ahd
 *     OS/2 2.x support
 *
 *     Revision 1.7  1993/08/02  03:24:59  ahd
 *     Further changes in support of Robert Denny's Windows 3.x support
 *
 *     Revision 1.6  1993/07/31  16:22:16  ahd
 *     Changes in support of Robert Denny's Windows 3.x support
 *
 *     Revision 1.5  1993/07/22  23:19:50  ahd
 *     First pass for Robert Denny's Windows 3.x support changes
 *
 *     Revision 1.4  1993/04/11  00:32:29  ahd
 *     Global edits for year, TEXT, etc.
 *
 * Revision 1.3  1992/12/12  16:12:13  ahd
 * Correct test for DesqView
 *
 * Revision 1.2  1992/12/07  02:43:20  ahd
 * Add DesqView support from David M. Watt
 *
 * Revision 1.1  1992/11/16  05:00:26  ahd
 * Initial revision
 *
 */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"

#include <limits.h>

#include <signal.h>

/*--------------------------------------------------------------------*/
/*               MS-DOS and OS/2 specific include files               */
/*--------------------------------------------------------------------*/

#if defined(FAMILYAPI) || defined(__OS2__)

#define INCL_NOPM
#define INCL_BASE
#include <os2.h>

#elif defined(WIN32) || defined(_Windows)

#include <windows.h>
#else

#include <dos.h>
#include <sys/timeb.h>

#endif

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "ssleep.h"

#if defined(_Windows)

#include "winutil.h"

#elif defined(FAMILYAPI) || defined(__OS2__)

#include "pos2err.h"

#else

#include "safeio.h"
#include "catcher.h"

#endif

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

currentfile();

#ifdef _Windows

#define CHECK_FOR_BREAK

/*--------------------------------------------------------------------*/
/*    d d e l a y                                                     */
/*                                                                    */
/*    Delay processing under Windows                                  */
/*                                                                    */
/*    NOTE: Minimum resolution is 54.925 ms.                          */
/*--------------------------------------------------------------------*/

void ddelay( const KEWSHORT milliseconds )
{
   MSG msg;
   WORD TimerId = 1;
   BOOL bTimerDone = KWFalse;

/*--------------------------------------------------------------------*/
/*          A 0-delay call means give up control to Windows           */
/*--------------------------------------------------------------------*/

   if (milliseconds == 0)
   {
      while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
      {
         TranslateMessage(&msg);

         DispatchMessage(&msg);
      }
      return;
   }

   TimerId = SetTimer( hOurWindow,
                       TimerId,
                       (milliseconds > 55) ? (WORD)milliseconds : (WORD)55 ,
                       NULL );

   if ( TimerId == 0 )
   {
      printmsg(0, "WindowsDelay: Unable to set Windows Timer");
      panic();
      return;
   } /* if */

/*--------------------------------------------------------------------*/
/*       LOCAL MESSAGE LOOP - Service Windows while waiting for       */
/*       the timer message.                                           */
/*--------------------------------------------------------------------*/

   while(!bTimerDone && GetMessage(&msg, NULL, NULL, NULL))
   {
      TranslateMessage(&msg);

      switch( msg.message )
      {
         case WM_TIMER:
            bTimerDone = KWTrue;
            /* Fall through and dispatch message   */

         default:
            DispatchMessage(&msg);
            break;

      } /* switch( msg.message ) */
   } /* while */

   if (KillTimer( hOurWindow, TimerId ) == 0)
      printmsg(0, "WindowsDelay: Unable to kill Windows Timer %d",
                  (int) TimerId );

} /* ddelay */

#elif defined(WIN32)

/*--------------------------------------------------------------------*/
/*       d d e l a y                                                  */
/*                                                                    */
/*       Delay function for Windows NT                                */
/*--------------------------------------------------------------------*/

void ddelay (const KEWSHORT interval )
{

   Sleep(interval);

} /* ddelay */

#elif defined(FAMILYAPI) || defined(__OS2__)

/*--------------------------------------------------------------------*/
/*       d d e l a y                                                  */
/*                                                                    */
/*       Delay function for OS/2                                      */
/*--------------------------------------------------------------------*/

void ddelay (const KEWSHORT interval )
{

   unsigned int result = DosSleep(interval);

   if (result)
      printOS2error( "DosSleep", result );

} /* ddelay */

#else

#define CHECK_FOR_BREAK

/*--------------------------------------------------------------------*/
/*       DOS wait functions.  Bizarre variations to cover DOS         */
/*       programs under Windows and both MS C and Borland C++         */
/*       compilers                                                    */
/*--------------------------------------------------------------------*/

#define MULTIPLEX 0x2F
#define DESQVIEW 0x15

/*--------------------------------------------------------------------*/
/*                      Local function declares                       */
/*--------------------------------------------------------------------*/

static void WinGiveUpTimeSlice(void);

static int RunningUnderWindows(void);

static int RunningUnderDesqview(void);

static void DVGiveUpTimeSlice(void);

/*--------------------------------------------------------------------*/
/*              Use this first to see if the rest are OK              */
/*                                                                    */
/*                  MOV AX,1600h   ; Check for win386/win3.0          */
/*                                   present                          */
/*                  INT 2Fh                                           */
/* Return AL = 0 -> No Windows, AL = 80 -> No WIn386 mode             */
/*        AL = 1 or AL = FFh -> Win386 2.xx running                   */
/*   else AL = Major version (3), AH = Minor version                  */
/*--------------------------------------------------------------------*/
/* --------------- Release time slice                                 */
/*                  MOV AX,1680h   ; **** Release time slice          */
/*                  INT 2Fh        ; Let someone else run             */
/* Return code is AL = 80H -> service not installed, AL = 0 -> all    */
/*                                                              OK    */
/*--------------------------------------------------------------------*/
/* --------------- Enter critical section (disable task switch)       */
/*                  MOV AX,1681H   ; Don't tread on me!               */
/*                  INT 2Fh                                           */
/*--------------------------------------------------------------------*/
/* --------------- End critical section (Permit task switching)       */
/*                  MOV AX,1682h                                      */
/*                  INT 2Fh                                           */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    R u n n i n g U n d e r W i n d o w s                           */
/*                                                                    */
/*    Determines if we are running under MS-Windows 386 or            */
/*    MS-Windows 3.  We save the result, to avoid excessively         */
/*    disabling interrupts when in a spin loop.                       */
/*--------------------------------------------------------------------*/

static int RunningUnderWindows(void)
{

   static int result = 2;
   union REGS inregs, outregs;
   static const int irq = MULTIPLEX;

   if (result != 2)           /* First call?                          */
      return result;          /* No --> Return saved result           */

   inregs.x.ax = 0x1600;
   int86(irq, &inregs, &outregs);

   if ( (outregs.h.al & 0x7f) == 0)
      result = 0;
   else
      result = 1;

   return result;

} /* RunningUnderWindows */

/*--------------------------------------------------------------------*/
/*    W i n G i v e U p T i m e S l i c e                             */
/*                                                                    */
/*    Surrender our time slice when executing under Windows/386       */
/*    or Windows release 3.                                           */
/*--------------------------------------------------------------------*/

static void WinGiveUpTimeSlice(void)
{
   union REGS inregs, outregs;
   static const int irq = MULTIPLEX;

   inregs.x.ax = 0x1680;
   int86(irq, &inregs, &outregs);

   if (outregs.h.al != 0) {
      printmsg(0,"Problem giving up timeslice:  %u\n", outregs.h.al);
      panic();
   }

} /* WinGiveUpTimeSlice */

/*--------------------------------------------------------------------*/
/*    R u n n i n g U n d e r D e s q v i e w                         */
/*                                                                    */
/*    Returns KWTrue if running under that OTHER DOS multi-tasker.     */
/*--------------------------------------------------------------------*/

static int RunningUnderDesqview(void)
{
   static int result = 2;
   union REGS inregs, outregs;

   if (result != 2)           /* First call?                          */
      return result;          /* No --> Return saved result           */

   inregs.x.ax = 0x2B01;      /* Dos Set Date function */
   inregs.x.cx = 0x4445;      /* CX DX = 'DESQ' */
   inregs.x.dx = 0x5351;

   intdos(&inregs, &outregs);

   if (outregs.h.al == 0xff) {
      result = 0;
   } else {
      printmsg(4, "RunningUnderDesqview:  Running under DesqView (AX=0x%x)",
               (int) outregs.x.ax);
      result = 1;
   }

   return result;

} /* RunningUnderDesqview */

/*--------------------------------------------------------------------*/
/*    D V G i v e U p T i m e S l i c e                               */
/*                                                                    */
/*    Surrender the CPU under DesqView                                */
/*--------------------------------------------------------------------*/

static void DVGiveUpTimeSlice(void)
{
#ifdef __TURBOC__
   asm {
#else
   _asm \
      {
#endif
      push ax
      mov ax, 101AH
                              /* Switch over to Desqview's stack  */
      int     15H
      mov ax, 1000H
                              /* Give up the timeslice            */
      int     15H
      mov ax, 1025H
                              /* Switch stack back to application */
      int     15H
      pop ax
      }

} /* DVGiveUpTimeSlice */

/*--------------------------------------------------------------------*/
/*    d d e l a y                                                     */
/*                                                                    */
/*    Delay for an interval of milliseconds under DOS                 */
/*--------------------------------------------------------------------*/

void ddelay (const KEWSHORT interval )
{

   struct timeb start;

/*--------------------------------------------------------------------*/
/*           Check for user aborts via the ESC (escape) key           */
/*--------------------------------------------------------------------*/

  if (bflag[F_ESCAPE])       /* Special Ctrl-C processing avail?     */
     checkForBreak();

#ifdef __TURBOC__
   enable();
#else
   _enable();
#endif

/*--------------------------------------------------------------------*/
/*       Handle the special case of 0 delay, which is simply a        */
/*                  request to give up our timeslice                  */
/*--------------------------------------------------------------------*/

   if (interval == 0)     /* Make it compatible with DosSleep    */
   {

      if (RunningUnderWindows())
         WinGiveUpTimeSlice( );
      else if (RunningUnderDesqview())
         DVGiveUpTimeSlice();

      return;
   } /* if */

/*--------------------------------------------------------------------*/
/*                 Actual spin loop to perform the delay              */
/*--------------------------------------------------------------------*/

   ftime(&start);             /* Get a starting time                  */

   for( ; ; )
   {
      struct timeb now;
      long elapsed;

      ftime(&now);           /* See how much time has elapsed */
      elapsed = ((long) (now.time - start.time) - 1) * 1000L
                + (now.millitm + 1000 - start.millitm);

      if (elapsed > (long) interval)
         return;

      if (RunningUnderWindows())
         WinGiveUpTimeSlice();
      else if (RunningUnderDesqview())
         DVGiveUpTimeSlice();
      else {

#ifdef __TURBOC__

         delay( (short) ((long) interval - elapsed) );

#else

         int volatile count;  /* Don't let compiler optimize this     */
         for ( count = 0; count < 2400; count ++);
                              /* We spin so that interrupts are
                                 enabled for most of the loop         */
#endif

      } /* else */
   } /* while */

} /* ddelay */

#endif

/*--------------------------------------------------------------------*/
/*    ssleep() - wait n seconds                                       */
/*                                                                    */
/*    Common routine to delay until n seconds have passed.            */
/*--------------------------------------------------------------------*/

void ssleep(const time_t interval)
{
   time_t quit = time((time_t *)NULL) + interval;
   long left = (long) interval;

/*--------------------------------------------------------------------*/
/*            Break the spin into chunks ddelay can handle            */
/*--------------------------------------------------------------------*/

   while ( left > SHRT_MAX / 1000 )
   {
      ddelay( 5000 );         /* Five seconds per pass                */
      if ((left = (long) quit - (long) time( NULL )) <= 0)
         return;
   } /* while */

/*--------------------------------------------------------------------*/
/*                 Final delay for the time remaining                 */
/*--------------------------------------------------------------------*/

   ddelay( (KEWSHORT) ((short) left * 1000) );

} /* ssleep */

#ifdef CHECK_FOR_BREAK              /* Windows 3.x or DOS only       */

/*--------------------------------------------------------------------*/
/*       c h e c k F o r B r e a k                                    */
/*                                                                    */
/*       Determine if we have a user requested abort                  */
/*--------------------------------------------------------------------*/

void
checkForBreak( void )
{
   KWBoolean beep = KWFalse;

/*--------------------------------------------------------------------*/
/*                   Process all queued characters                    */
/*--------------------------------------------------------------------*/

   while (safepeek())
   {

      int input = safein();

      if (!input)
      {
        safein();
        return;
      }

      switch( input )
      {
         case ('C' - '@'):          /* Control-C                     */
         case ('X' - '@'):          /* Control-X                     */
         case ('[' - '@'):          /* Escape                        */

#ifdef _WINDOWS
           ctrlchandler(0);         /* No handler under Windows      */
#else
           raise( SIGINT );         /* Use C RTL to exit             */
#endif
           break;

        default:                    /* Unknown character, ignore     */
           if ( beep )
           {
              putchar('\a');        /* No --> Complain to user       */
              beep = KWFalse;       /* But be nice about it ...
                                       only once per call            */
           } /* if ( beep ) */

           break;

      } /* switch( input ) */

   } /* while (safepeek()) */

}  /* checkForBreak */

#endif /* CHECK_FOR_BREAK */
