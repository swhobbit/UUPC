/*--------------------------------------------------------------------*/
/*       u l i b 1 4 . c                                              */
/*                                                                    */
/*       Serial port communications driver for ARTICOMM INT14         */
/*       driver                                                       */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Copyright (c) Richard H. Lamb 1985-1987                         */
/*                                                                    */
/*    Changes Copyright 1992 by Mark W. Schumann                      */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1993 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       This suite needs some tuning, but the Wonderworks lacks      */
/*       the time and more importantly the software to do so.         */
/*       Specific problems:                                           */
/*                                                                    */
/*          The INT14 interface is not checked to be ARTICOMM, and    */
/*          thus the extensions to the generic INT14 functions may    */
/*          fail with no warning to the user.                         */
/*                                                                    */
/*          Non-ARTICOMM programs are not supported.                  */
/*                                                                    */
/*          No error is returned if a write times out                 */
/*                                                                    */
/*          Input data is discarded if a read times out, which        */
/*          will confuse the 'g' packet input processor.              */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: ULIB.C 1.11 1993/05/30 00:08:03 ahd Exp $
 *
 *    $Log: ULIB.C $
 *
 *   30 Nov 92 - Adapt for use with INT14 drivers.
 *               Currently this is guaranteed to work only with
 *               Artisoft's ARTICOMM driver, but porting to others
 *               should be minor.  Mods by Mark W. Schumann
 *               <mark@whizbang.wariat.org>
 */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <dos.h>    /* Declares union REGS and int86(). */

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "hlib.h"
#include "ulib.h"
#include "comm.h"          // Modem status bits
#include "ssleep.h"
#include "catcher.h"

#include "fossil.h"        // Various INT14 definitions
#include "commlib.h"       // Trace functions, etc.

/*--------------------------------------------------------------------*/
/*                        Internal prototypes                         */
/*--------------------------------------------------------------------*/

static void modemControl( char mask, boolean on);
static void ShowModem( void );
static unsigned char bps_table(int);

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

static BPS currentBPS;
static char currentDirect;
static boolean carrierDetect;

currentfile();
static boolean hangupNeeded = TRUE;


/*--------------------------------------------------------------------*/
/*    b p s _ t a b l e                                               */
/*                                                                    */
/*    Look up the INT14 baud rate value corresponding to (bps).       */
/*--------------------------------------------------------------------*/

static unsigned char bps_table (int bps)
{

   static int tab[] = {110, 150, 300, 600, 1200, 2400, 4800, 9600, -1};
   unsigned char i;

   for (i = 0; tab[i] > 0; i++)
   {
      if (tab[i] == bps)
         return i;
   }

   return 255;

} /* bps_table */

/*--------------------------------------------------------------------*/
/*    i o p e n l i n e                                               */
/*                                                                    */
/*    Open the serial port for I/O                                    */
/*--------------------------------------------------------------------*/

int iopenline(char *name, BPS bps, const boolean direct)
{

   if (portActive)              /* Was the port already active?      */
      closeline();               /* Yes --> Shutdown it before open  */

   printmsg(15, "openline: %s, %d", name, bps);

   currentDirect = (char) (direct ? 'D' : 'M');

   if (sscanf(name, "COM%d", &portNum) != 1)
   {
      printmsg(0,"Communications port must be format COMx, was %s",
                name);
      panic();
   }

   norecovery = FALSE;     // Flag we need a graceful shutdown after
                           // Cntl-BREAK

/*--------------------------------------------------------------------*/
/*       With the INT14 setup, we don't worry about the lock file     */
/*       since our modem sharing software is supposed to deal with    */
/*       resource contention.                                         */
/*--------------------------------------------------------------------*/

   portNum--;              /* Change ordinal number to offset       */
   SIOSpeed( bps );        /* Open the port and set the speed       */
   flowcontrol( FALSE );   /* Enable hardware flow control          */

   ssleep(2);              /* Wait two seconds as required by V.24  */
   carrierDetect = FALSE;  /* No modem connected yet                */

   traceStart( name );

   portActive = TRUE;     /* record status for error handler */

   return 0;               // Return success to caller

} /* iopenline */


/*--------------------------------------------------------------------*/
/*    i s r e a d                                                     */
/*                                                                    */
/*    Read from the serial port                                       */
/*                                                                    */
/*    Non-blocking read essential to "g" protocol.  See               */
/*    "dcpgpkt.c" for description.  This all changes in a             */
/*    multi-tasking system.  Requests for I/O should get queued       */
/*    and an event flag given.  Then the requesting process (e.g.     */
/*    gmachine()) waits for the event flag to fire processing         */
/*    either a read or a write.  Could be implemented on VAX/VMS      */
/*    or DG but not MS-DOS.                                           */
/*--------------------------------------------------------------------*/

unsigned int isread(char *buffer, unsigned int wanted, unsigned int timeout)
{

   union REGS regs;          /* Scratch area for interrupt calls. */

   /* With INT14, don't bother with a counter; just set a timeout    */
   /*  value with INT14/AX=8009.  This is Artisoft-specific.         */

   int count = 0;

   ShowModem();                  // Report modem status

/*--------------------------------------------------------------------*/
/*             Now actually try to read a buffer of data              */
/*--------------------------------------------------------------------*/

   regs.x.ax = 0x8009;            /* Set timeouts */
   regs.x.bx = timeout * 91 / 5;  /* Send timeout in ticks */
   regs.x.cx = timeout * 91 / 5;  /* Receive timeout in ticks */
   regs.x.dx = portNum;           /* Port number */
   int86 (FS_INTERRUPT, &regs, &regs);

   for ( count = 0; count < (int) wanted; count++ )
   {
      unsigned short result = FossilCntl( FS_RECV1, 0);

      if (result & 0x8000)          // Did read time out?
      {
         printmsg (20, "Timeout in sread().");
         traceData( buffer, count + 1, FALSE );
         return count;
      }

      buffer[count] = (char) (result & 0x00ff);
   }

   traceData( buffer, count + 1, FALSE );
   return count + 1;

} /* isread */

/*--------------------------------------------------------------------*/
/*    i s w r i t e                                                   */
/*                                                                    */
/*    Write to the serial port                                        */
/*--------------------------------------------------------------------*/

int iswrite(char *data, unsigned int len)
{
   unsigned int i;

   ShowModem();

/*--------------------------------------------------------------------*/
/*       For an INT14 interface, just spit the data bytes out one     */
/*       at a time.                                                   */
/*--------------------------------------------------------------------*/

   for (i = 0; i < len; i++)
      FossilCntl( FS_XMIT1, data[i] );

   traceData( data, len, TRUE );

/*--------------------------------------------------------------------*/
/*              Return byte count transmitted to caller               */
/*--------------------------------------------------------------------*/

   return len;

} /* iswrite */

/*--------------------------------------------------------------------*/
/*    i s s e n d b r k                                               */
/*                                                                    */
/*    Send a break signal out the serial port                         */
/*--------------------------------------------------------------------*/

void issendbrk(unsigned int duration)
{
   printmsg(12, "ssendbrk: %d", duration);

/*--------------------------------------------------------------------*/
/*       With INT14, we drop DSR for a quarter of a second to         */
/*       indicate a BREAK sequence.                                   */
/*                                                                    */
/*       (Snuffles doubts this works properly)                        */
/*--------------------------------------------------------------------*/

   modemControl( 0x20, TRUE );   // Raise flag, which lowers DSR

   ddelay (250);                 // Wait a quarter second

   modemControl( 0x20, FALSE);   // Lower flag, which raises DSR

} /* issendbrk */


/*--------------------------------------------------------------------*/
/*    i c l o s e l i n e                                             */
/*                                                                    */
/*    Close the serial port down                                      */
/*--------------------------------------------------------------------*/

void icloseline(void)
{
   if (!portActive)
      panic();

   portActive = FALSE;     /* flag port closed for error handler  */

   modemControl( 0x20, FALSE );  // Lower DSR

   ddelay (500);

   traceStop();

} /* icloseline */


/*--------------------------------------------------------------------*/
/*    i h a n g u p                                                   */
/*                                                                    */
/*    Hangup the telephone by dropping DTR.                           */
/*--------------------------------------------------------------------*/

void ihangup( void )
{
   if (!hangupNeeded)
      return;
   hangupNeeded = FALSE;

   modemControl( 0x20, FALSE );  // Lower DSR

   ddelay (500);                 /* Pause half a second */

   modemControl( 0x20, TRUE );   // Restore DSR

   ddelay (2000);                /* Wait two seconds to recover */

   printmsg(3,"hangup: complete.");
   carrierDetect = FALSE;  /* No modem connected yet               */

} /* ihangup */

/*--------------------------------------------------------------------*/
/*    i S I O S p e e d                                               */
/*                                                                    */
/*    Re-specify the speed of an opened serial port                   */
/*--------------------------------------------------------------------*/

void iSIOSpeed(BPS bps)
{
   union REGS regs;          /* Scratch area for interrupt calls. */

   regs.h.ah = 0x00;            /* Initialize modem */
   regs.x.ax = 0x0400;          /* Initialize port */
   regs.h.bh = 0x00;            /* No parity */
   regs.h.bl = 0x00;            /* One stop bit */
   regs.h.ch = 0x03;            /* Eight-bit words */
   regs.h.cl = bps_table(bps);  /* New baud rate */
   regs.x.dx = portNum;
   int86 (FS_INTERRUPT, &regs, &regs);

   ShowModem();
   currentBPS = bps;

} /* iSIOSpeed */

/*--------------------------------------------------------------------*/
/*    i f l o w c o n t r o l                                         */
/*                                                                    */
/*    Enable/Disable in band (XON/XOFF) flow control                  */
/*--------------------------------------------------------------------*/

void iflowcontrol( boolean flow )
{

   union REGS regs;          /* Scratch area for interrupt calls. */

/*
   With INT14, don't open and close the port; toggle it in place
   by using INT14/AX=800A, which is again Artisoft-specific.
*/
   printmsg (4, "flowcontrol: %abling in-band flow control",
                              (flow ? "en" : "dis"));
   regs.x.ax = 0x800A;
   regs.h.bl = (unsigned char) (flow ? 2 : 1);
                                 // 2 is hardware, 1 is XON/XOFF
   regs.x.dx = portNum;
   int86 (FS_INTERRUPT, &regs, &regs);

   ShowModem();

} /* iflowcontrol */

/*--------------------------------------------------------------------*/
/*    i G e t S p e e d                                               */
/*                                                                    */
/*    Report current speed of communications connection               */
/*--------------------------------------------------------------------*/

BPS iGetSpeed( void )
{
   return currentBPS;
} /* GetSpeed */

/*--------------------------------------------------------------------*/
/*    i C D                                                           */
/*                                                                    */
/*    Report if we have carrier detect and lost it                    */
/*--------------------------------------------------------------------*/

#define FS_dsr_high() (FSStatus() & MDM_DSR)
#define FS_cd_high()  (FSStatus() & MDM_CD )

boolean iCD( void )
{
   boolean online = carrierDetect;

   ShowModem();
   carrierDetect = FS_cd_high();

/*--------------------------------------------------------------------*/
/*    If we previously had carrier detect but have lost it, we        */
/*    report it was lost.  If we do not yet have carrier detect,      */
/*    we return success because we may not have connected yet.        */
/*--------------------------------------------------------------------*/

   if (online)
      return carrierDetect && FS_dsr_high();
   else
      return FS_dsr_high();

} /* iCD */

/*--------------------------------------------------------------------*/
/*    S h o w M o d e m                                               */
/*                                                                    */
/*    Report current modem status                                     */
/*--------------------------------------------------------------------*/

#define mannounce(flag, bits, text ) ((flag & bits) ? text : "" )

static void ShowModem( void )
{
   static int oldStatus = 0xDEAD;
   int status;

   if ( debuglevel < 4 )
      return;

   status = FSStatus();

   if (status == oldStatus)
      return;

   printmsg(0, "ShowModem: %#02x%s%s%s%s%s%s%s%s",
      status,
      mannounce(MDM_CD,   status, "\tCarrier Detect"),
      mannounce(MDM_RI,   status, "\tRing Indicator"),
      mannounce(MDM_DSR,  status, "\tData Set Ready"),
      mannounce(MDM_CTS,  status, "\tClear to Send"),
      mannounce(MDM_CDC,  status, "\tCD changed"),
      mannounce(MDM_TRI,  status, "\tRI went OFF"),
      mannounce(MDM_DSRC, status, "\tDSR changed"),
      mannounce(MDM_CTSC, status, "\tCTS changed"));

   oldStatus = status;

} /* ShowModem */

/*--------------------------------------------------------------------*/
/*       m o d e m C o n t r o l                                      */
/*                                                                    */
/*       Twiddle a modem control register bit on or off               */
/*--------------------------------------------------------------------*/

static void modemControl( char mask, boolean on)
{
   union REGS regs;              /* Scratch area for interrupt calls. */

   regs.x.ax = 0x0500;           /* Extended port control: get modem
                                    control register                  */
   regs.x.dx = portNum;
   int86(FS_INTERRUPT, &regs, &regs);

   regs.x.ax = 0x0501;           /* Set modem control register */
   regs.x.dx = portNum;
   if ( on )
      regs.h.bl |= mask;         /* Raise flag */
   else
      regs.h.bl &= ~ mask;       /* Lower flag */

   int86(FS_INTERRUPT, &regs, &regs);

} /* modemControl */
