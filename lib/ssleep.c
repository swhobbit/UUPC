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
 *    $Header: E:\src\uupc\LIB\RCS\SSLEEP.C 1.2 1992/12/07 02:43:20 ahd Exp $
 *
 *    Revision history:
 *    $Log: SSLEEP.C $
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

#ifdef FAMILYAPI

#define INCL_NOPM
#define INCL_BASE
#include <os2.h>

#else

#include <dos.h>
#include <sys/timeb.h>

#endif

#ifdef WIN32
#include <windows.h>
#endif

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "ssleep.h"
#include "safeio.h"

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

currentfile();

/*--------------------------------------------------------------------*/
/*                     MS-DOS specific functions                      */
/*--------------------------------------------------------------------*/

#ifndef FAMILYAPI
#define MULTIPLEX 0x2F
#define DESQVIEW 0x15

#ifdef _Windows

#include <windows.h>

void WindowsDelay( int milliseconds );

/*--------------------------------------------------------------------*/
/*    W i n d o w s D e l a y                                         */
/*                                                                    */
/*    Delay processing under Windows                                  */
/*--------------------------------------------------------------------*/

void WindowsDelay( int milliseconds )
{
   WORD TimerId = SetTimer( NULL, 0,
            milliseconds ? (WORD) milliseconds : (WORD) 1,
            NULL );

   if ( TimerId == 0 )
   {
      printmsg(0, "WindowsDelay: Unable to set Windows Timer");
      return;
   } /* if */

   WaitMessage();

   if ( !KillTimer( NULL, TimerId ) )
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
   int irq;

   if (result != 2)           /* First call?                         */
      return result;          /* No --> Return saved result          */

   irq = MULTIPLEX;
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
   int irq = MULTIPLEX;

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
   time_t start = time((time_t *)NULL);
   time_t left = interval;

/*--------------------------------------------------------------------*/
/*            Break the spin into chunks ddelay can handle            */
/*--------------------------------------------------------------------*/

   while ( (left*1000L) > (long) INT_MAX )
   {
      ddelay( 5000 );         /* Five seconds per pass               */
      left = max(interval - (time( NULL ) - start),0);
   } /* while */

/*--------------------------------------------------------------------*/
/*                 Final delay for the time remaining                 */
/*--------------------------------------------------------------------*/

   ddelay( (int) (left * 1000L) );

} /*ssleep*/

/*--------------------------------------------------------------------*/
/*    d d e l a y                                                     */
/*                                                                    */
/*    Delay for an interval of milliseconds                           */
/*--------------------------------------------------------------------*/

void   ddelay   (int milliseconds)
{

#ifdef FAMILYAPI
   USHORT result;
#else
   struct timeb t;
   time_t seconds;
   unsigned last;
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
   Sleep(milliseconds);

/*--------------------------------------------------------------------*/
/*                             OS/2 wait                              */
/*--------------------------------------------------------------------*/

#elif FAMILYAPI
   result = DosSleep(milliseconds);
   if (result != 0)
      panic();
#else

/*--------------------------------------------------------------------*/
/*                            MS-DOS wait                             */
/*--------------------------------------------------------------------*/

#ifndef _Windows

#ifdef __TURBOC__
   enable();
#else
   _enable();
#endif

#endif

/*--------------------------------------------------------------------*/
/*       Handle the special case of 0 delay, which is simply a        */
/*                  request to give up our timeslice                  */
/*--------------------------------------------------------------------*/

   if (milliseconds == 0)     /* Make it compatible with DosSleep    */
   {

#ifdef _Windows
      WindowsDelay( milliseconds );
#else

      if (RunningUnderWindows())
         WinGiveUpTimeSlice( );
      else if (RunningUnderDesqview())
         DVGiveUpTimeSlice();

#endif
      return;
   } /* if */

   ftime(&t);                 /* Get a starting time                 */
   last = t.millitm;          /* Save milliseconds portion           */
   seconds = t.time;          /* Save seconds as well                */

   while( milliseconds > 0)   /* Begin the spin loop                 */
   {

#ifdef _Windows

      WindowsDelay( milliseconds );

#else
      if (RunningUnderWindows())
         WinGiveUpTimeSlice();
      else if (RunningUnderDesqview())
         DVGiveUpTimeSlice();
      else {

#ifdef __TURBOC__

         delay( milliseconds );

#else

         int volatile count;  /* Don't let compiler optimize this    */
         for ( count = 0; count < 2400; count ++);
                              /* We spin so that interrupts are
                                 enabled for most of the loop        */
#endif

      } /* else */
#endif /* _Windows */

      ftime(&t);              /* Take a new time check               */

      if (t.time == seconds)  /* Same second as last pass?           */
         milliseconds -= (t.millitm - last); /* Yes --> mSecond delta*/
      else
         milliseconds -= 1000 * (int) (t.time - seconds)
                              - (last - t.millitm);
                              /* No --> Handle wrap of mSeconds      */

      last = t.millitm;       /* Update last tick indicator          */
      seconds = t.time;       /* Update this as well; only needed if
                                 it changed (see above), but it
                                 kills time (which is our job)       */
   } /* while */

#endif /* FAMILYAPI */

} /* ddelay */
