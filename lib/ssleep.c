/*--------------------------------------------------------------------*/
/*    s s l e e p . c                                                 */
/*                                                                    */
/*    Smart sleep routines for UUPC/extended                          */
/*                                                                    */
/*    Written by Dave Watt, modified by Drew Derbyshire               */
/*                                                                    */
/*    Generates DOS specific code with Windows support by default,    */
/*    generates call to OS/2 family API if FAMILYAPI is defined       */
/*    generates calls to Windows/NT if WIN32 is defined               */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: ssleep.c 1.10 1993/09/27 02:42:11 ahd Exp $
 *
 *    Revision history:
 *    $Log: ssleep.c $
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

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>

/*--------------------------------------------------------------------*/
/*               MS-DOS and OS/2 specific include files               */
/*--------------------------------------------------------------------*/

#if defined(FAMILYAPI) || defined(__OS2__)

#define INCL_NOPM
#define INCL_BASE
#include <os2.h>

#else

#include <dos.h>
#include <sys/timeb.h>

#endif

#if defined(WIN32) || defined(_Windows)
#include <windows.h>
#endif

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "ssleep.h"
#include "safeio.h"
#include "catcher.h"

#if defined(_Windows)
#include "winutil.h"
#endif


#if defined(FAMILYAPI) || defined(__OS2__)
#include "pos2err.h"
#endif

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

#if defined(FAMILYAPI) || defined(__OS2__)
currentfile();
#endif


/*--------------------------------------------------------------------*/
/*                     MS-DOS specific functions                      */
/*--------------------------------------------------------------------*/

#if !defined(FAMILYAPI) && !defined(__OS2__)

currentfile();

#define MULTIPLEX 0x2F
#define DESQVIEW 0x15

#ifdef _Windows

static void WindowsDelay( const int milliseconds );

/*--------------------------------------------------------------------*/
/*    W i n d o w s D e l a y                                         */
/*                                                                    */
/*    Delay processing under Windows                                  */
/*                                                                    */
/*    NOTE: Minimum resolution is 54.925 ms.                          */
/*--------------------------------------------------------------------*/

static void WindowsDelay( const int milliseconds )
{
   MSG msg;
   WORD TimerId = 1;
   BOOL bTimerDone = FALSE;

   //
   //    A 0-delay call means give up control to Windows
   //

   if (milliseconds == 0)
   {
      while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
      {
         TranslateMessage(&msg);
         DispatchMessage(&msg);
      }
      return;
   }

   SetTimer( hOurWindow,
            TimerId,
            (milliseconds > 55) ? (WORD)milliseconds : (WORD)55 ,
            NULL );

   if ( TimerId == 0 )
   {
      printmsg(0, "WindowsDelay: Unable to set Windows Timer");
      panic();
      return;
   } /* if */

   //
   // LOCAL MESSAGE LOOP - Service Windows while waiting for
   // the timer message.
   //

   while(!bTimerDone && GetMessage(&msg, NULL, NULL, NULL))
   {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
      if (msg.message == WM_TIMER)
         bTimerDone = TRUE;
   }

   if (KillTimer( hOurWindow, TimerId ) == 0)
      printmsg(0, "WindowsDelay: Unable to kill Windows Timer %d",
                  (int) TimerId );

} /* WindowsDelay */

#else

#ifndef WIN32

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

   if (result != 2)           /* First call?                         */
      return result;          /* No --> Return saved result          */

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
/*    Returns TRUE if running under that OTHER DOS multi-tasker.      */
/*--------------------------------------------------------------------*/

static int RunningUnderDesqview(void)
{
   static int result = 2;
   union REGS inregs, outregs;

   if (result != 2)           /* First call?                         */
      return result;          /* No --> Return saved result          */

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

#endif /* _Windows */
#endif /* WIN32 */
#endif


/*--------------------------------------------------------------------*/
/*    ssleep() - wait n seconds                                       */
/*                                                                    */
/*    Simply delay until n seconds have passed.                       */
/*--------------------------------------------------------------------*/

void ssleep(time_t interval)
{
   time_t quit = time((time_t *)NULL) + interval;
   long left = (long) interval;

/*--------------------------------------------------------------------*/
/*            Break the spin into chunks ddelay can handle            */
/*--------------------------------------------------------------------*/

   while ( left > SHRT_MAX / 1000 )
   {
      ddelay( 5000 );         /* Five seconds per pass               */
      if ((left = (long) quit - (long) time( NULL )) <= 0)
         return;
   } /* while */

/*--------------------------------------------------------------------*/
/*                 Final delay for the time remaining                 */
/*--------------------------------------------------------------------*/

   ddelay( (KEWSHORT) ((short) left * 1000) );

} /*ssleep*/

/*--------------------------------------------------------------------*/
/*    d d e l a y                                                     */
/*                                                                    */
/*    Delay for an interval of milliseconds                           */
/*--------------------------------------------------------------------*/

void   ddelay   (KEWSHORT interval )
{

#if defined(FAMILYAPI) || defined(__OS2__)

   USHORT result;

#elif !defined(_Windows)

   struct timeb start;

#endif

#ifndef _Windows

/*--------------------------------------------------------------------*/
/*           Check for user aborts via the ESC (escape) key           */
/*--------------------------------------------------------------------*/

   if (bflag[F_ESCAPE])       /* Special Ctrl-C processing avail?    */
   {
      boolean beep = TRUE;

      while (safepeek())      /* Yes --> While character in buffer   */
      {
         if (safein() == '\033') /* Look for ESC                     */
            raise( SIGINT );     /* Yes --> eject via std exit       */
         else if ( beep )
         {
             putchar('\a');      /* No --> Complain to user          */
             beep = FALSE;       /* But be nice about it ...
                                    only once per pass through here  */
         } /* else if ( beep ) */
      } /* while */

   } /* if (bflag[F_ESCAPE]) */

#endif /* _Windows */

/*--------------------------------------------------------------------*/
/*                          Now do the wait                           */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          Windows/NT wait                           */
/*--------------------------------------------------------------------*/

#ifdef WIN32
   Sleep(interval);

/*--------------------------------------------------------------------*/
/*                           Windows wait                             */
/*--------------------------------------------------------------------*/

#elif defined(_Windows)
   WindowsDelay(interval);

/*--------------------------------------------------------------------*/
/*                             OS/2 wait                              */
/*--------------------------------------------------------------------*/

#elif defined(FAMILYAPI) || defined(__OS2__)

   result = DosSleep(interval);
   if (result)
      printOS2error( "DosSleep", result );
#else

/*--------------------------------------------------------------------*/
/*                            MS-DOS wait                             */
/*--------------------------------------------------------------------*/

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

   ftime(&start);             /* Get a starting time                 */

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

         int volatile count;  /* Don't let compiler optimize this    */
         for ( count = 0; count < 2400; count ++);
                              /* We spin so that interrupts are
                                 enabled for most of the loop        */
#endif

      } /* else */
   } /* while */

#endif /* FAMILYAPI */

} /* ddelay */
