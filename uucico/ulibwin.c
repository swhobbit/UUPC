/*--------------------------------------------------------------------*/
/*       u l i b w i n . C                                            */
/*                                                                    */
/*       UUPC/extended serial port communications for Windows 3.1     */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes copyright (c) 1993, by Robert Denny                  */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1993 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: ulibwin.c 1.6 1993/10/01 02:36:45 dmwatt Exp $
 *
 *    Revision history:
 *    $Log: ulibwin.c $
 * Revision 1.6  1993/10/01  02:36:45  dmwatt
 * Suppress odd debugging messages in normal compile
 * Reformat source
 *
 * Revision 1.5  1993/09/29  13:18:06  ahd
 * Use new dummy setprty function
 *
 * Revision 1.4  1993/09/27  00:48:43  ahd
 * Add dummy set and reset priority functions
 *
 * Revision 1.3  1993/08/02  03:24:59  ahd
 * Further changes in support of Robert Denny's Windows 3.x support
 *
 * Revision 1.2  1993/07/31  16:27:49  ahd
 * Changes in support of Robert Denny's Windows support
 *
 * Revision 1.1  1993/07/22  23:24:23  ahd
 * Initial revision
 *
 */

/*--------------------------------------------------------------------*/
/*       Services provided by ulib.c:                                 */
/*                                                                    */
/*          - serial I/O                                              */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <io.h>
#include <time.h>

/*--------------------------------------------------------------------*/
/*                    MS Windows 3.1 include files                    */
/*--------------------------------------------------------------------*/

#include <windows.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "ulib.h"
#include "ssleep.h"
#include "commlib.h"

//
// Finally, Microsoft has documented a way to see the Modem Status
// Register bits for modem control lines. This was a real bizarre
// mess with SetCommEventMask() and GetCommEventMask(). The document
// is in the Developer's Knowledge Base:
//
// Title: INF: Accessing the Modem Status Register (MSR) in Windows
// Document Number: Q101417           Publ Date: 15-JUL-1993
// Product Name: Microsoft Windows Software Development Kit
// Product Version:  3.10
// Operating System: WINDOWS
//

#define COMM_MSRSHADOW 35          // Offset in DEB of MSR shadow
#define MSR_CTS              0x10  // absolute CTS state in MSR
#define MSR_DSR              0x20  // absolute DSR state in MSR
#define MSR_RI               0x40  // absolute RI state in MSR
#define MSR_RLSD             0x80  // absolute RLSD state in MSR

currentfile();

static boolean   carrierdetect = FALSE;  /* Modem is not connected     */

static boolean hangupNeeded = FALSE;
static UINT currentSpeed = 0;
static LPBYTE lpbModemBits;       // --> Modem Status Register bits

#define FAR_NULL ((PVOID) 0L)

#define IN_QUEUE_SIZE   2048
#define OUT_QUEUE_SIZE  2048
#define IN_XOFF_LIM     256
#define IN_XON_LIM      256

/*--------------------------------------------------------------------*/
/*         Definitions of control structures for Win 3.1 API          */
/*--------------------------------------------------------------------*/

static int nCid;
static DCB dcb;

/*--------------------------------------------------------------------*/
/*                        Internal prototypes                         */
/*--------------------------------------------------------------------*/

static void ShowModem( void );
static void ShowError( int status );

/*--------------------------------------------------------------------*/
/*    n o p e n l i n e                                               */
/*                                                                    */
/*    Open the serial port for I/O                                    */
/*--------------------------------------------------------------------*/

#ifdef __TURBOC__
#pragma argsused
#endif

int nopenline(char *name, BPS baud, const boolean direct )
{
   int rc;

   if (portActive)               /* Was the port already active?     ahd   */
      closeline();               /* Yes --> Shutdown it before open  ahd   */

#ifdef UDEBUG
   printmsg(15, "openline: %s, %ul", name, (unsigned long) baud);
#endif

   if (!equaln(name, "COM", 3 ))
   {
      printmsg(0,
         "openline: Communications port must be format COMx, was %s",
         name);
      return TRUE;
   }

   if((nCid = OpenComm(name, IN_QUEUE_SIZE, OUT_QUEUE_SIZE)) < 0)
   {
      printmsg(0, "openline: Failed to open port %s.", name);
      printmsg(0, "nopenline: %s: OpenComm returned %#04X (%d)",
                   name,
                   nCid,
                   nCid);
      return TRUE;
   }

/*--------------------------------------------------------------------*/
/*        Get the pointer to the MSR shadow in COMM.DRV's DEB         */
/*--------------------------------------------------------------------*/

   lpbModemBits = (LPBYTE)SetCommEventMask(nCid, 0) + COMM_MSRSHADOW;

/*--------------------------------------------------------------------*/
/*            Reset any errors on the communications port             */
/*--------------------------------------------------------------------*/

   if ((rc = GetCommError (nCid, NULL)) != 0)
   {
      printmsg(0, "openline: Error condition reset on port %s.", name);
      ShowError(rc);
   }

/*--------------------------------------------------------------------*/
/*                           Set baud rate                            */
/*--------------------------------------------------------------------*/

   nSIOSpeed(baud);

/*--------------------------------------------------------------------*/
/*                        Set line attributes                         */
/*--------------------------------------------------------------------*/

   printmsg(15,"openline: Getting attributes");
   if ((rc = GetCommState(nCid, &dcb)) != 0)
   {
      printmsg(0,"nopenline: %s: GetCommState was %#04x (%d)",
                  name,
                  rc,
                  rc);
      panic();
   }

   dcb.StopBits = ONESTOPBIT;
   dcb.Parity = NOPARITY;
   dcb.ByteSize = 8;

/*--------------------------------------------------------------------*/
/*                      Set up for Flow Control                       */
/*--------------------------------------------------------------------*/

   printmsg(15,"openline: Disabling XON/XOFF flow control");

   dcb.fOutX = 0;
   dcb.fInX = 0;
   if(!direct)                 // nodirect means RTS/CTS flow OK
   {
#ifdef UDEBUG
      printmsg(15, "openline: Enabling RTS/CTS flow control");
#endif
      dcb.fOutxCtsFlow = 1;
      dcb.fRtsflow = 1;
      dcb.XoffLim = IN_XOFF_LIM;
      dcb.XonLim = IN_XON_LIM;
   }
   else {
#ifdef UDEBUG
      printmsg(4, "openline: Disabling RTS/CTS flow control");
#endif
      dcb.fOutxCtsFlow = 0;
      dcb.fRtsflow = 0;
   }

/*--------------------------------------------------------------------*/
/*                Set up for Modem Control as needed                  */
/*--------------------------------------------------------------------*/

   dcb.fDtrDisable = 0;
   dcb.fRtsDisable = 0;

/*--------------------------------------------------------------------*/
/*              Modify the DCB with the new attributes                */
/*--------------------------------------------------------------------*/

#ifdef UDEBUG
   printmsg(15,"openline: Setting attributes");
#endif

   if ((rc = SetCommState(&dcb)) != 0)
   {
      printmsg(0,"nopenline: %s: return code from SetCommState was %#04X (%d)",
                 name,
                 rc,
                 rc);
      panic();
   }

/*--------------------------------------------------------------------*/
/*                 Assure RTS and DTR are asserted                    */
/*--------------------------------------------------------------------*/


#ifdef UDEBUG
   printmsg(15,"openline: Raising RTS/DTR");
#endif

   if (EscapeCommFunction(nCid, SETRTS) != 0)
   {
      printmsg(0, "openline: Failed to raise RTS for %s", name);
      panic();
   }
   if (EscapeCommFunction(nCid, SETDTR) != 0)
   {
      printmsg(0, "openline: Unable to raise DTR for %s", name);
      panic();
   }

   ShowModem();

/*--------------------------------------------------------------------*/
/*        Log serial line data only if log file already exists        */
/*--------------------------------------------------------------------*/

   traceStart( name );

   portActive = TRUE;     /* record status for error handler */

/*--------------------------------------------------------------------*/
/*                     Wait for port to stablize                      */
/*--------------------------------------------------------------------*/

   ssleep(2);              /* Allow port to stabilize per V.24  */

   return 0;

} /* nopenline */

/*--------------------------------------------------------------------*/
/*    s r e a d                                                       */
/*                                                                    */
/*    Read from the serial port                                       */
/*                                                                    */
/*   Non-blocking read essential to "g" protocol. The rather cryptic  */
/*   "description" in dcpgpkt.c is:                                   */
/*                                                                    */
/*   sread(buf, n, timeout)                                           */
/*      while(TRUE)                                                   */
/*         if # of chars available >= n (w/o dec internal counter)    */
/*            read n chars into buf (dec internal counter)            */
/*            break                                                   */
/*         else                                                       */
/*            if time > timeout                                       */
/*               break                                                */
/*                                                                    */
/*   NOTE: Timeout of 0 returns right away, indicating the number of  */
/*         bytes in our local receive buffer. There's GOTTA be a      */
/*         better way...                                              */
/*                                                                    */
/*   This all changes in a multi-tasking system.  Requests for I/O    */
/*   should get queued and an event flag given.  Then the             */
/*   requesting process (e.g. gmachine()) waits for the event flag    */
/*   to fire processing either a read or a write.  Could be           */
/*   implemented on VAX/VMS or DG but not MS-DOS.                     */
/*                                                                    */
/*--------------------------------------------------------------------*/

unsigned int nsread(char *output, unsigned int wanted, unsigned int timeout)

{
   int rc, received;
   time_t stop_time;
   time_t now;
   COMSTAT stat;

   //
   // This catches a fencepost condition later...
   //

   if (wanted == 0)
   {
      ddelay(0);
      return(0);
   }

/*--------------------------------------------------------------------*/
/*                      Report our modem status                       */
/*--------------------------------------------------------------------*/

  ShowModem();

/*--------------------------------------------------------------------*/
/*                 Determine when to stop processing                  */
/*--------------------------------------------------------------------*/

   if ( timeout == 0 )        /* 0 = don't wait around.               */
   {
      stop_time = 0;
      now = 1;                /* Any number greater than stop time    */
   }
   else {
      time( & now );
      stop_time = now + timeout;
   }

/*--------------------------------------------------------------------*/
/*       Watch RX Queue till wanted bytes available or timeout        */
/*--------------------------------------------------------------------*/

   while(TRUE)
   {

      //
      // Check & clear the comm port. This gets the #chars in the
      // receive queue as well, in the COMSTAT structure.
      //

      if ((rc = GetCommError(nCid, &stat)) != 0)
      {
         printmsg(0,"sread: Read of %d bytes failed.", wanted);
         printmsg(0,"       return code from GetCommError was %#04x (%d)",
                                                rc , rc);
         ShowError(rc);
         return 0;
      }

      //
      // If wanted # bytes are available, break out and read 'em.
      //

      if (stat.cbInQue >= wanted)
         break;               // We have enough, break out!

      //
      // Be friendly to Windows' cooperative multitasking...
      //

      ddelay(0);

      //
      // If timeout is zero, return immediately.
      //

      if (stop_time == 0)
         return(stat.cbInQue);

      //
      // Check for timeout. If timed out, return.
      //

      time( &now );
      if(stop_time <= now)
      {
         printmsg(15, "sread: timeout(%d) - %d chars avail",
                         timeout, stat.cbInQue);
         return(stat.cbInQue);
      }

   } // end of while(TRUE)

   //
   // We have enough in the RX queue. Grab 'em right into the
   // caller's buffer.
   //

   received = ReadComm(nCid, output, wanted);

   printmsg(15, "sread: Got %d characters, %d still in RX queue.",
                   (int)received, (int)(stat.cbInQue - received));

/*--------------------------------------------------------------------*/
/*                    Log the newly received data                     */
/*--------------------------------------------------------------------*/

   traceData( output, wanted, FALSE );

   return(received);

} /* nsread */

/*--------------------------------------------------------------------*/
/*    n s w r i t e                                                   */
/*                                                                    */
/*    Write to the serial port                                        */
/*--------------------------------------------------------------------*/

int nswrite(const char *data, unsigned int len)
{
   int bytes;
   int rc;

   hangupNeeded = TRUE;      /* Flag that the port is now dirty  */

/*--------------------------------------------------------------------*/
/*                      Report our modem status                       */
/*--------------------------------------------------------------------*/

   ShowModem();

/*--------------------------------------------------------------------*/
/*         Write the data out as the queue becomes available          */
/*--------------------------------------------------------------------*/

   bytes = WriteComm(nCid, data, len);

   rc = GetCommError(nCid, NULL);
   if (rc)
   {
      printmsg(0,"nswrite: WriteComm failed, "
                 "return code from GetCommError was %#04x (%d)",
                        rc , rc);
      ShowError(rc);
      return bytes;
   }

/*--------------------------------------------------------------------*/
/*                        Log the data written                        */
/*--------------------------------------------------------------------*/

   traceData( data, len, TRUE );

/*--------------------------------------------------------------------*/
/*            Return bytes written to the port to the caller          */
/*--------------------------------------------------------------------*/

   return len;

} /* nswrite */

/*--------------------------------------------------------------------*/
/*    n s s e n d b r k                                               */
/*                                                                    */
/*    send a break signal out the serial port                         */
/*--------------------------------------------------------------------*/

void nssendbrk(unsigned int duration)
{


#ifdef UDEBUG
   printmsg(12, "ssendbrk: %d", duration);
#endif

   SetCommBreak(nCid);
   ddelay(duration == 0 ? 200 : duration);
   ClearCommBreak(nCid);

} /*ssendbrk*/

/*--------------------------------------------------------------------*/
/*    n c l o s e l i n e                                             */
/*                                                                    */
/*    Close the serial port down                                      */
/*--------------------------------------------------------------------*/

void ncloseline(void)
{

   if ( ! portActive )
      panic();

   portActive = FALSE;     /* flag port closed for error handler  */
   hangupNeeded = FALSE;  /* Don't fiddle with port any more     */

/*--------------------------------------------------------------------*/
/*                             Lower DTR                              */
/*--------------------------------------------------------------------*/

    if (EscapeCommFunction(nCid, CLRDTR | CLRRTS) != 0)
        printmsg(0,"closeline: Unable to lower DTR/RTS");

/*--------------------------------------------------------------------*/
/*                      Actually close the port                       */
/*--------------------------------------------------------------------*/

   if(CloseComm(nCid) != 0)
      printmsg(0, "closeline: close of serial port failed");

/*--------------------------------------------------------------------*/
/*                   Stop logging the data to disk                    */
/*--------------------------------------------------------------------*/

   traceStop();

   printmsg(3,"Serial port closed");

} /* ncloseline */

/*--------------------------------------------------------------------*/
/*    n h a n g u p                                                   */
/*                                                                    */
/*    Hangup the telephone by dropping DTR.  Works with HAYES and     */
/*    many compatibles.                                               */
/*    14 May 89 Drew Derbyshire                                       */
/*--------------------------------------------------------------------*/

void nhangup( void )
{
   hangupNeeded = FALSE;

/*--------------------------------------------------------------------*/
/*                              Drop DTR                              */
/*--------------------------------------------------------------------*/

   if (EscapeCommFunction(nCid, CLRDTR) != 0)
   {
      printmsg(0, "hangup: Unable to lower DTR for comm port");
      panic();
   }

/*--------------------------------------------------------------------*/
/*                  Wait for the telephone to hangup                  */
/*--------------------------------------------------------------------*/

   printmsg(3,"hangup: Dropped DTR");
   ddelay(1000);            /* Really only need 250 milliseconds (HA) */

/*--------------------------------------------------------------------*/
/*                          Bring DTR backup                          */
/*--------------------------------------------------------------------*/

   if (EscapeCommFunction(nCid, SETDTR) != 0)
   {
      printmsg(0, "hangup: Unable to raise DTR for comm port");
      panic();
   }
   ddelay(500);         /* Now wait for the poor thing to recover    */

} /* nhangup */

/*--------------------------------------------------------------------*/
/*       S I O S p e e d                                              */
/*                                                                    */
/*       Re-specify the speed of an opened serial port                */
/*--------------------------------------------------------------------*/

void nSIOSpeed(BPS baud)
{
   WORD rc;

   currentSpeed = (UINT) baud;
   printmsg(15,"SIOSpeed: Setting baud rate to %lu",
               (unsigned long) currentSpeed);

   ShowModem();
   GetCommState (nCid, &dcb);

   dcb.BaudRate = currentSpeed;
   rc = SetCommState (&dcb);

   if (rc)
   {
      printmsg(0,"SIOSPeed: Unable to set baud rate for port to %lu",
                 (unsigned long) currentSpeed);
      panic();
   }

} /* nSIOSpeed */

/*--------------------------------------------------------------------*/
/*    n f l o w c o n t r o l                                         */
/*                                                                    */
/*    Enable/Disable in band (XON/XOFF) flow control                  */
/*--------------------------------------------------------------------*/

void nflowcontrol( boolean flow )
{
   int rc;
   DCB dcb;

   GetCommState(nCid, &dcb);

   if (flow)
   {
      dcb.fOutX = TRUE;
      dcb.fInX = TRUE;
      dcb.fRtsflow = FALSE;
      dcb.fOutxCtsFlow = FALSE;
   }
   else {
      dcb.fOutX = FALSE;
      dcb.fInX = FALSE;
      dcb.fRtsflow = TRUE;
      dcb.fOutxCtsFlow = TRUE;
   }

   if ((rc = SetCommState(&dcb)) != 0)
   {
      printmsg(0,"flowcontrol: Unable to set flow control");
      printmsg(0,"Return code fromSetCommState was %#04x (%d)",
                  (int) rc,
                  (int) rc);
      panic();
   } /*if */

} /* nflowcontrol */

/*--------------------------------------------------------------------*/
/*    n G e t S p e e d                                               */
/*                                                                    */
/*    Report current speed of communications connection               */
/*--------------------------------------------------------------------*/

BPS nGetSpeed( void )
{
   return currentSpeed;
} /* GetSpeed */

/*--------------------------------------------------------------------*/
/*   n C D                                                            */
/*                                                                    */
/*   Return status of carrier detect                                  */
/*--------------------------------------------------------------------*/

boolean nCD( void )
{
   boolean online = carrierdetect;
   boolean modem_present;

   carrierdetect = ((*lpbModemBits & MSR_RLSD) != 0);
   modem_present = ((*lpbModemBits & MSR_DSR) != 0);

/*--------------------------------------------------------------------*/
/*    If we previously had carrier detect but have lost it, we        */
/*    report it was lost.  If we do not yet have carrier detect,      */
/*    we return success because we may not have connected yet.        */
/*    If DSR is not present, we always report no carrier, as there    */
/*    is either no modem at all(!) or it's not turned on.             */
/*--------------------------------------------------------------------*/

   if (online)
      return (modem_present && carrierdetect);
   else
      return (modem_present);

} /* nCD */

/*--------------------------------------------------------------------*/
/*    S h o w M o d e m                                               */
/*                                                                    */
/*    Report current modem status when changed from last call         */
/*--------------------------------------------------------------------*/

#define mannounce(flag, bits, text ) (((flag & bits) != 0) ? text : "" )

static void ShowModem( void )
{
   BYTE modem_bits = *lpbModemBits;
   static BYTE old_bits = 0xFF;

   if ( debuglevel < 4 )
      return;

   if ( (debuglevel < 4) ||            // Silent at lower debuglevels
      (modem_bits == old_bits))        // Show only changes in modem signals
      return;

   printmsg(0, "ShowModem: %#02x %s %s %s",
                modem_bits,
                mannounce(MSR_RLSD,  modem_bits, "DCD"),
                mannounce(MSR_DSR,   modem_bits, "DSR"),
                mannounce(MSR_CTS,   modem_bits, "CTS"));
   old_bits = modem_bits;

} /* ShowModem */

/*--------------------------------------------------------------------*/
/*    S h o w E r r o r                                               */
/*                                                                    */
/*    Report modem error bits in English (more or less)               */
/*--------------------------------------------------------------------*/

static void ShowError( int status )
{
   printmsg(2, "Port Error: %#04x%s%s%s%s%s",
      status,
      mannounce(CE_RXOVER,   status, " Recv Queue Ovfl"),
      mannounce(CE_OVERRUN,  status, " Hardware Overrun"),
      mannounce(CE_RXPARITY, status, " Parity Error"),
                mannounce(CE_FRAME,    status, " Framing Error"),
                mannounce(CE_TXFULL,   status, " Xmit Queue Full"));

} /* ShowError */

/*--------------------------------------------------------------------*/
/*       s e t P r t y                                                */
/*                                                                    */
/*       No operation under Windows                                   */
/*--------------------------------------------------------------------*/

#ifdef __TURBOC__
#pragma argsused
#endif

void setPrty( const KEWSHORT priorityIn, const KEWSHORT prioritydeltaIn )
{

} /* setPrty */

/*--------------------------------------------------------------------*/
/*       r e s e t P r t y                                            */
/*                                                                    */
/*       No operation under Windows                                   */
/*--------------------------------------------------------------------*/

void resetPrty( void )
{
} /* resetPrty */
