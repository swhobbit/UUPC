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
/*    Changes Copyright (c) 1989-1995 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: ulib14.c 1.14 1995/01/07 16:40:16 ahd Exp $
 *
 *    $Log: ulib14.c $
 *    Revision 1.14  1995/01/07 16:40:16  ahd
 *    Change boolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.13  1994/12/22 00:36:31  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.12  1994/05/08 22:46:32  ahd
 *    Correct declare in icd()
 *
 *        Revision 1.11  1994/05/07  21:45:33  ahd
 *        Correct CD() processing to be sticky -- once it fails, it
 *        keeps failing until reset by close or hangup.
 *
 * Revision 1.10  1994/02/19  05:11:10  ahd
 * Use standard first header
 *
 * Revision 1.9  1994/01/01  19:21:25  ahd
 * Annual Copyright Update
 *
 * Revision 1.8  1993/12/26  16:20:17  ahd
 * Use common buffering
 *
 * Revision 1.7  1993/12/24  05:12:54  ahd
 * Use far buffer in 16 bit compilers
 *
 * Revision 1.6  1993/11/21  02:45:50  ahd
 * Add missing header files to suppress missing structure def warnings
 *
 * Revision 1.6  1993/11/21  02:45:50  ahd
 * Add missing header files to suppress missing structure def warnings
 *
 * Revision 1.5  1993/11/06  17:56:09  rhg
 * Drive Drew nuts by submitting cosmetic changes mixed in with bug fixes
 *
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

#include "uupcmoah.h"

#include <fcntl.h>
#include <io.h>
#include <errno.h>

#include <dos.h>    /* Declares union REGS and int86(). */

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "ulib14.h"
#include "comm.h"          /*  Modem status bits */
#include "ssleep.h"
#include "catcher.h"

#include "fossil.h"        /* Various INT14 definitions */
#include "commlib.h"       /* Trace functions, etc. */

#include "hostable.h"      /* Needed for structures in dcp.h   */
#include "usertabl.h"      /* Needed for structures in dcp.h   */
#include "security.h"      /* Needed for structures in dcp.h   */

/*--------------------------------------------------------------------*/
/*                        Internal prototypes                         */
/*--------------------------------------------------------------------*/

static void modemControl( char mask, KWBoolean on);
static void ShowModem( void );
static unsigned char bps_table(int);

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

static BPS currentBPS;
static char currentDirect;

currentfile();
static KWBoolean hangupNeeded = KWTrue;

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

int iopenline(char *name, BPS bps, const KWBoolean direct)
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

   norecovery = KWFalse; /* Flag we need a graceful shutdown after Cntl-BREAK */

/*--------------------------------------------------------------------*/
/*       With the INT14 setup, we don't worry about the lock file     */
/*       since our modem sharing software is supposed to deal with    */
/*       resource contention.                                         */
/*--------------------------------------------------------------------*/

   portNum--;              /* Change ordinal number to offset       */
   SIOSpeed( bps );        /* Open the port and set the speed       */
   flowcontrol( KWFalse );  /* Enable hardware flow control          */

   ssleep(2);              /* Wait two seconds as required by V.24  */
   carrierDetect = KWFalse;  /* No modem connected yet               */

   traceStart( name );

   portActive = KWTrue;    /* record status for error handler */

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

unsigned int isread(char UUFAR *buffer,
                    unsigned int wanted,
                    unsigned int timeout)
{
   union REGS rcvregs, outregs;
   time_t quit = time( NULL ) + timeout;

   size_t commBufferCached = commBufferUsed;

   if ( wanted > commBufferLength )
   {
      printmsg(0,"nsread: Overlength read, wanted %u bytes into %u buffer!",
                     (unsigned int) wanted,
                     (unsigned int) commBufferLength );
      panic();
   }

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

   while (commBufferUsed < wanted)
   {
      if ( terminate_processing )
      {
         static KWBoolean recurse = KWFalse;
         if ( ! recurse )
         {
            printmsg(2,"isread: User aborted processing");
            recurse = KWTrue;
         }
         commBufferUsed = 0;
         return commBufferUsed;
      }

      int86(0x14, &rcvregs, &outregs);
      if (!(outregs.h.ah & 0x80))
         commBuffer[commBufferUsed++] = (char) outregs.h.al;
      else {                                 /* the read timed out */
         if (timeout == 0)              /* If not interested in waiting */
            return commBufferUsed;      /* then get out of here fast */

         ShowModem();                   /* Report modem status */

         if ( time(NULL) >= quit )
         {
            printmsg(20, "isread: Timeout (timeout=%u, want=%u, have=%u)",
                          timeout, wanted, commBufferUsed);
            return commBufferUsed;
         }

#ifdef ARTICOMM_INT14 /* Richard H. Gumpertz (RHG@CPS.COM), 28 Sep 1993 */
         timregs.x.cx = (unsigned short)(quit - now) * 91 / 5; /* Receive timeout in ticks */
         int86(0x14, &timregs, &outregs);
#endif /* ARTICOMM_INT14 */

      } /* else */
   } /* while (commBufferUsed < wanted) */

   traceData( commBuffer + commBufferCached,
              commBufferUsed - commBufferCached,
              KWFalse );

   if ( buffer != NULL )
   {
      MEMCPY(buffer, commBuffer, wanted);
      commBufferUsed -= wanted;
      if (commBufferUsed)
         MEMMOVE(commBuffer, commBuffer + wanted, commBufferUsed);
   }

   return wanted + commBufferUsed;

} /* isread */

/*--------------------------------------------------------------------*/
/*    i s w r i t e                                                   */
/*                                                                    */
/*    Write to the serial port                                        */
/*--------------------------------------------------------------------*/

int iswrite(const char UUFAR *data, unsigned int len)
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
          && (commBufferUsed < commBufferLength))
         isread(NULL, commBufferLength, 0); /* do some read ahead */
#endif /* RHG */
   }

   traceData( data, len, KWTrue );

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

   modemControl( 0x20, KWTrue );  /* Raise flag, which lowers DSR */

   ddelay (250);                 /* Wait a quarter second */

   modemControl( 0x20, KWFalse);  /* Lower flag, which raises DSR */

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

   portActive = KWFalse;    /* flag port closed for error handler  */

   modemControl( 0x20, KWFalse );  /* Lower DSR */

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
   hangupNeeded = KWFalse;

   modemControl( 0x20, KWFalse );  /* Lower DSR */

   ddelay (500);                 /* Pause half a second */

   modemControl( 0x20, KWTrue );  /* Restore DSR */

   ddelay (2000);                /* Wait two seconds to recover */

   printmsg(3,"ihangup: complete.");
   carrierDetect = KWFalse;  /* No modem connected yet              */

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

void iflowcontrol( KWBoolean flow )
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

KWBoolean iCD( void )
{

   KWBoolean newCarrierDetect = FS_cd_high();

   ShowModem();

   if ( newCarrierDetect )
      carrierDetect = newCarrierDetect;

/*--------------------------------------------------------------------*/
/*    If we previously had carrier detect but have lost it, we        */
/*    report it was lost.  If we do not yet have carrier detect,      */
/*    we return success because we may not have connected yet.        */
/*--------------------------------------------------------------------*/

   if (carrierDetect)
      return newCarrierDetect && FS_dsr_high();
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

static void modemControl( char mask, KWBoolean on)
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
