/*--------------------------------------------------------------------*/
/*       u l i b 1 4 . c                                              */
/*                                                                    */
/*       Serial port communications driver for INT14 NET interface    */
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
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: ulib14.c 1.3 1993/10/03 22:09:09 ahd Exp $
 *
 *    $Log: ulib14.c $
 * Revision 1.3  1993/10/03  22:09:09  ahd
 * Use unsigned long to display speed
 *
 * Revision 1.2  1993/05/30  15:25:50  ahd
 * Multiple driver support
 *
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
#include <errno.h>

#include <dos.h>    /* Declares union REGS and int86(). */

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "hlib.h"
#include "ulib.h"
#include "comm.h"          /*  Modem status bits */
#include "ssleep.h"
#include "catcher.h"

#include "fossil.h"        /* Various INT14 definitions */
#include "commlib.h"       /* Trace functions, etc. */

#include "dcp.h"           /* defines RECV_BUF */

/*--------------------------------------------------------------------*/
/*                        Internal prototypes                         */
/*--------------------------------------------------------------------*/

unsigned int isread(char *buffer, unsigned int wanted, unsigned int timeout);
static void modemControl( char mask, boolean on);
static void ShowModem( void );
static unsigned char bps_table(int);

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

static BPS currentBPS;
static char currentDirect;
static boolean carrierDetect;
static unsigned int rcv_pending = 0;
static char *rcv_buffer = NULL;
#define IMPOSSIBLE_SREAD (RECV_BUF + 1)

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

   printmsg(15, "iopenline: %s, %ul", name, bps);

   currentDirect = (char) (direct ? 'D' : 'M');

   if (sscanf(name, "COM%d", &portNum) != 1)
   {
      printmsg(0,"Communications port must be format COMx, was %s",
                name);
      panic();
   }

   norecovery = FALSE; /* Flag we need a graceful shutdown after Cntl-BREAK */

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

   rcv_pending = 0;
   if (rcv_buffer == NULL)
      if ((rcv_buffer = malloc(RECV_BUF)) == NULL)
         return ENOMEM;
   isread(NULL, IMPOSSIBLE_SREAD, 2);   /* do some read-ahead */
   rcv_pending = 0;                     /* and discard what we got */

   return 0;               /* Return success to caller */

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
   union REGS rcvregs, outregs;
   time_t quit = time( NULL ) + timeout;

#ifdef ARTICOMM_INT14   /* Richard H. Gumpertz (RHG@CPS.COM), 28 Sep 1993 */
   union REGS timregs;          /* Scratch area for interrupt calls. */

   timregs.x.ax = 0x8009;               /* Set timeouts */
   timregs.x.cx = timeout * 91 / 5;     /* Receive timeout in ticks */
   timregs.x.bx = 0x7FFF/*???*/;        /* Send timeout in ticks */
   timregs.x.dx = portNum;              /* Port number */
   int86(0x14, &timregs, &outregs);
#endif /* ARTICOMM_INT14 */

   rcvregs.h.ah = FS_RECV1;
   rcvregs.h.al = 0;
   rcvregs.x.dx = portNum;              /* Port number */
   rcvregs.x.bx = 0;

   while (rcv_pending < wanted)
   {
      if ( terminate_processing )
      {
         static boolean recurse = FALSE;
         if ( ! recurse )
         {
            printmsg(2,"isread: User aborted processing");
            recurse = TRUE;
         }
         rcv_pending = 0;
         return rcv_pending;
      }

      if (rcv_pending >= RECV_BUF)      /* rcv_buffer full? */
         return rcv_pending;

      int86(0x14, &rcvregs, &outregs);
      if (!(outregs.h.ah & 0x80))
         rcv_buffer[rcv_pending++] = (char) outregs.h.al;
      else
      {                                 /* the read timed out */
         time_t now;

         if (timeout == 0)              /* If not interested in waiting */
            return rcv_pending;         /* then get out of here fast */

         ShowModem();                   /* Report modem status */

         if ( (now = time(NULL)) >= quit )
         {
            printmsg(20, "isread: Timeout (timeout=%u, want=%u, have=%u)",
                          timeout, wanted, rcv_pending);
            return rcv_pending;
         }

#ifdef ARTICOMM_INT14 /* Richard H. Gumpertz (RHG@CPS.COM), 28 Sep 1993 */
         timregs.x.cx = (unsigned short)(quit - now) * 91 / 5; /* Receive timeout in ticks */
         int86(0x14, &timregs, &outregs);
#endif /* ARTICOMM_INT14 */
      }
   }

   memcpy(buffer, rcv_buffer, wanted);
   rcv_pending -= wanted;
   if (rcv_pending)
      memmove(rcv_buffer, rcv_buffer + wanted, rcv_pending);
   traceData( buffer, wanted, FALSE );
   return wanted + rcv_pending;
} /* isread */

/*--------------------------------------------------------------------*/
/*    i s w r i t e                                                   */
/*                                                                    */
/*    Write to the serial port                                        */
/*--------------------------------------------------------------------*/

int iswrite(char *data, unsigned int len)
{
   unsigned int i;
   union REGS xmtregs;

   ShowModem();

/*--------------------------------------------------------------------*/
/*       For an INT14 interface, just spit the data bytes out one     */
/*       at a time.                                                   */
/*--------------------------------------------------------------------*/

   xmtregs.h.ah = FS_XMIT1;
   xmtregs.x.dx = portNum;              /* Port number */
   xmtregs.x.bx = 0;

   for (i = 0; i < len; i++)
   {
      union REGS outregs;

      xmtregs.h.al = (unsigned char) data[i];

      int86(0x14, &xmtregs, &outregs);

#if 1 /* Richard H. Gumpertz (RHG@CPS.COM), 29 September 1993 */
      if (((outregs.h.ah & 0x61) == 0x01)
          && (rcv_pending < RECV_BUF))
         isread(NULL, IMPOSSIBLE_SREAD, 0); /* do some read ahead */
#endif /* RHG */
   }

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
   printmsg(12, "issendbrk: %d", duration);

/*--------------------------------------------------------------------*/
/*       With INT14, we drop DSR for a quarter of a second to         */
/*       indicate a BREAK sequence.                                   */
/*                                                                    */
/*       (Snuffles doubts this works properly)                        */
/*--------------------------------------------------------------------*/

   modemControl( 0x20, TRUE );   /* Raise flag, which lowers DSR */

   ddelay (250);                 /* Wait a quarter second */

   modemControl( 0x20, FALSE);   /* Lower flag, which raises DSR */

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

   modemControl( 0x20, FALSE );  /* Lower DSR */

   ddelay (500);

   traceStop();

   if (rcv_buffer != NULL)
   {
      free(rcv_buffer);
      rcv_buffer = NULL;
   }

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

   modemControl( 0x20, FALSE );  /* Lower DSR */

   ddelay (500);                 /* Pause half a second */

   modemControl( 0x20, TRUE );   /* Restore DSR */

   ddelay (2000);                /* Wait two seconds to recover */

   printmsg(3,"ihangup: complete.");
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
   regs.h.cl = bps_table((int) bps);  /* New baud rate */
   regs.x.dx = portNum;
   int86(0x14, &regs, &regs);

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

#ifdef ARTICOMM_INT14 /* Richard H. Gumpertz (RHG@CPS.COM), 28 Sep 1993 */
   union REGS regs;          /* Scratch area for interrupt calls. */

/*
   With INT14, don't open and close the port; toggle it in place
   by using INT14/AX=800A, which is again Artisoft-specific.
*/
   printmsg(4, "iflowcontrol: %sabling in-band flow control",
                               (flow ? "en" : "dis"));
   regs.x.ax = 0x800A;
   regs.h.bl = (unsigned char) (flow ? 2 : 1);
                                 /* 2 is hardware, 1 is XON/XOFF */
   regs.x.dx = portNum;
   int86(0x14, &regs, &regs);
#else /* ARTICOMM_INT14 */
   if (flow)
   {
      printmsg(4, "iflowcontrol: in-band flow control not supported");
   }
#endif /* ARTICOMM_INT14 */

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
   int86(0x14, &regs, &regs);

   regs.x.ax = 0x0501;           /* Set modem control register */
   regs.x.dx = portNum;
   if ( on )
      regs.h.bl |= mask;         /* Raise flag */
   else
      regs.h.bl &= ~ mask;       /* Lower flag */

   int86(0x14, &regs, &regs);

} /* modemControl */
