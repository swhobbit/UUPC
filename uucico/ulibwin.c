/*--------------------------------------------------------------------*/
/*       u l i b w i n . C                                            */
/*                                                                    */
/*       UUPC/extended serial port communications for Windows 3.1     */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes copyright (c) 1993, by Robert Denny                  */
/*       Changes copyright (c) 1994-1995, by Miles Zarathustra        */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1994 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: ulibwin.c 1.17 1995/02/25 13:17:09 miles Exp $
 *
 *    Revision history:
 *    $Log: ulibwin.c $
 *    Revision 1.17  1995/02/25 13:17:09  miles
 *    Make module more robust
 *
 *      21nov94 getMSR() added to avoid GP fault
 *      it's probably only necessary if the port is invalid.
 *      CHATPRJ is#defined for chat, not for UUCICO.
 *
 *      #ifdef USEPRINTMSG ->#ifndef CHATPRJ
 *      because printmsg() uses va_list it's unkosher in a DLL.
 *
 *      mlz -- nopenline de-clawed : nCid is assumed to be valid
 *      showmodem de-clawed.  ... will fail in a DLL.
 *
 *        Revision 1.14  1994/05/08  22:46:32  ahd
 *        Correct compile error
 *
 *        Revision 1.13  1994/05/07  21:45:33  ahd
 *        Correct CD() processing to be sticky -- once it fails, it
 *        keeps failing until reset by close or hangup.
 *
 * Revision 1.12  1994/02/19  05:07:34  ahd
 * Use standard first header
 *
 * Revision 1.11  1994/02/13  04:46:01  ahd
 * Correct date of Microsoft document
 *
 * Revision 1.10  1994/01/01  19:22:22  ahd
 * Annual Copyright Update
 *
 * Revision 1.9  1993/12/24  05:12:54  ahd
 * Use far buffer for master communications buffer
 *
 * Revision 1.8  1993/10/12  01:33:23  ahd
 * Normalize comments to PL/I style
 *
 * Revision 1.8  1993/10/12  01:33:23  ahd
 * Normalize comments to PL/I style
 *
 * Revision 1.7  1993/10/03  22:09:09  ahd
 * Use unsigned long to display speed
 *
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

#include "uupcmoah.h"

#include <fcntl.h>
#include <dos.h>
#include <io.h>

/*--------------------------------------------------------------------*/
/*                    MS Windows 3.1 include files                    */
/*--------------------------------------------------------------------*/

#include <windows.h>

#ifdef CHATPRJ
#include "chat.h"
#else
  static void checkForBreak( void )
  {
     /* Source not supplied by Miles.  (Grrrrr).   */
  };
#endif

unsigned int windowsBuffer = 2048;
                                 /* Hardcoded, since rest of Mile's
                                    package is not available. Grrrr. */

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "ulib.h"
#include "ssleep.h"
#include "commlib.h"

currentfile();

static int portNum = 0;
static KWBoolean hangupNeeded = KWFalse;
static UINT currentSpeed = 0;

static unsigned int inQueueSize, outQueueSize;

/*--------------------------------------------------------------------*/
/*                                                                    */
/*    The q101417 algorithm referenced below, while it may be ugly,   */
/*    gives a GP fault if the port is invalid.                        */
/*                                                                    */
/*    getMSR has been substituted instead.                            */
/*                                                                    */
/* Finally, Microsoft has documented a way to see the Modem Status    */
/* Register bits for modem control lines. This was a real bizarre     */
/* mess with SetCommEventMask() and GetCommEventMask(). The document  */
/* is in the Developer's Knowledge Base:                              */
/*                                                                    */
/* Title: INF: Accessing the Modem Status Register (MSR) in Windows   */
/* Document Number: Q101417           Publ Date: 15-JUL-1993          */
/* Product Name: Microsoft Windows Software Development Kit           */
/* Product Version:  3.10                                             */
/* Operating System: WINDOWS                                          */
/*                                                                    */
/*--------------------------------------------------------------------*/

#define MSR_CTS              0x10  /* absolute CTS state in MSR       */
#define MSR_DSR              0x20  /* absolute DSR state in MSR       */
#define MSR_RI               0x40  /* absolute RI state in MSR        */
#define MSR_RLSD             0x80  /* absolute RLSD state in MSR      */

#ifdef SUICIDE

#ifdef _MSC_VER
#define inportb _inp
#endif

#else

#define COMM_MSRSHADOW 35          /* Offset in DEB of MSR shadow     */

#endif

#define FAR_NULL ((PVOID) 0L)

#define IN_XOFF_LIM     256
#define IN_XON_LIM      256

/*--------------------------------------------------------------------*/
/*         Definitions of control structures for Win 3.1 API          */
/*--------------------------------------------------------------------*/

int nCid;                           /* will be assigned in chat      */
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
/*    return: KWFalse = ok, KWTrue = fail                                 */
/*--------------------------------------------------------------------*/

int nopenline(char *name, BPS baud, const KWBoolean direct )
{

#ifndef CHATPRJ

   int rc;

   if (portActive)               /* Was the port already active?     */
      closeline();               /* Yes --> Shutdown it before open  */

#ifdef UDEBUG
   printmsg(15, "openline: %s, %ul", name, (unsigned long) baud);
#endif

   if (!equaln(name, "COM", 3 ) || name[3]<'1' || name[3]>'4')
   {
      printmsg(0,
               "openline: \"Device = \" must be in format COMx, "
               " 1 <= x <= 4; was %s", name);
      return KWTrue;
   }

   portNum = name[3] - '0';

   windowsBuffer = max(windowsBuffer,2048);

   inQueueSize = windowsBuffer;
   outQueueSize = min(windowsBuffer, 8192);

   if ((nCid = OpenComm(name, inQueueSize, outQueueSize)) < 0)
   {
      printmsg(0, "openline: Failed to open port %s.", name);
      printmsg(0, "nopenline: %s: OpenComm returned %#04X (%d)",
           name,
           nCid,
           nCid);
      return KWTrue;
   }

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

#ifdef UDEBUG
   printmsg(15,"openline: Getting attributes");
#endif

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

#ifdef UDEBUG
   printmsg(15,"openline: Disabling XON/XOFF flow control");
#endif

   dcb.fOutX = 0;
   dcb.fInX = 0;

   if (!direct)                 /* nodirect means RTS/CTS flow OK      */
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
      printmsg(0,"nopenline: %s: SetCommState status is %#04X (%d)",
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

   portActive = KWTrue;    /* record status for error handler */

/*--------------------------------------------------------------------*/
/*                     Wait for port to stablize                      */
/*--------------------------------------------------------------------*/

   ssleep(2);              /* Allow port to stabilize per V.24  */

#endif  /* CHATPRJ */

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
/*      while (KWTrue)                                                */
/*         if# of chars available >= n (w/o dec internal counter)     */
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

unsigned int nsread(char UUFAR *output,
            unsigned int wanted,
            unsigned int timeout)
{

   int received;
   time_t stop_time, now;
   COMSTAT stat;

/*--------------------------------------------------------------------*/
/*            This catches a fencepost condition later...             */
/*--------------------------------------------------------------------*/

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

   for( ;; )
   {

#ifdef CHATPRJ
      if (beenPressed())
         return 0;
#else
      checkForBreak();              /* uucico                        */
#endif

/*--------------------------------------------------------------------*/
/*       Check & clear the comm port.  This gets the number chars in  */
/*       the receive queue as well, in the COMSTAT structure.         */
/*--------------------------------------------------------------------*/

      GetCommError(nCid, &stat );

/*--------------------------------------------------------------------*/
/*       If wanted number of bytes are available, break out and       */
/*       read 'em.                                                    */
/*--------------------------------------------------------------------*/

      if (stat.cbInQue >= wanted)
         break;

      ddelay(0);                    /* Be friendly to Windows'
                                       cooperative multitasking...   */

      if (stop_time == 0)           /* Immediate timeout?            */
      {                             /* We're out of here             */
         return(stat.cbInQue);
      }

      time( &now );

      if (stop_time <= now)         /* Timeout on request*/
      {
#ifdef UDEBUG
         printmsg(15, "nsread: timeout(%d) - %d chars avail",
             timeout, stat.cbInQue);
#endif
         return(stat.cbInQue);
      }

   } /* for( ;; ) */

/*--------------------------------------------------------------------*/
/*       We have enough in the RX queue.  Grab 'em right into the     */
/*       caller's buffer.                                             */
/*--------------------------------------------------------------------*/

   received = ReadComm(nCid, output, wanted);

   printmsg(15, "nsread: Got %d characters, %d still in RX queue.",
          (int)received, (int)(stat.cbInQue - received));

/*--------------------------------------------------------------------*/
/*                    Log the newly received data                     */
/*--------------------------------------------------------------------*/

   traceData( output, wanted, KWFalse );
   return(received);

} /* nsread */

/*--------------------------------------------------------------------*/
/*       o u t Q u e u e F r e e                                      */
/*                                                                    */
/*       Determine count of characters free in output queue           */
/*--------------------------------------------------------------------*/

static unsigned int outQueueFree( void )
{
   COMSTAT stat;

   GetCommError(nCid, &stat );

   return outQueueSize - stat.cbOutQue;
}

/*--------------------------------------------------------------------*/
/*    n s w r i t e                                                   */
/*                                                                    */
/*    Write to the serial port                                        */
/*--------------------------------------------------------------------*/

int nswrite(const char UUFAR *data, unsigned int len)
{
   int bytes;
   int rc;

   hangupNeeded = KWTrue;     /* Flag that the port is now dirty  */

#ifdef UDEBUG
   printmsg(15,"nswrite: Writing %u bytes to port", len );
#endif

/*--------------------------------------------------------------------*/
/*                      Report our modem status                       */
/*--------------------------------------------------------------------*/

   ShowModem();

/*--------------------------------------------------------------------*/
/*       Introduce a little flow control - Actual line pacing is      */
/*       handled at a lower level.  This should actually be           */
/*       interrupt driven under Windows, but this quick hack from     */
/*       the DOS ULIB.C doesn't bother.  So sue me.                   */
/*--------------------------------------------------------------------*/

   if ( outQueueFree() < len )
   {
      int spin = 0;
      static int const max_spin = 20;  /* Should be configured, is not  */

      int currentQueueFree = outQueueFree();

      if ( len > outQueueSize )
      {
         printmsg(0,"nswrite: Transmit buffer overflow; buffer size %d, "
                    "needed %d",
                     outQueueSize,
                     len);
         panic();
      }

      while( (len > currentQueueFree) && (spin < max_spin) )
      {
         int wait;
         int needed;
         unsigned int newQueueFree;

         needed = max(outQueueSize / 4, len - currentQueueFree);
                              /* Minimize thrashing by requiring
                                 big chunks */

         wait = (int) ((long) needed * 10000L / (long) currentSpeed);
                              /* Compute time in milliseconds
                                 assuming 10 bits per byte           */

         printmsg(4,"nswrite: Waiting %d ms for %d bytes in queue"
                     ", pass %d",
                     wait, needed, spin);

         ddelay( (KEWSHORT) wait ); /* Actually perform the wait     */

         newQueueFree = outQueueFree();

         if ( newQueueFree == currentQueueFree )
            spin++;           /* No progress, consider timing out    */
         else
            currentQueueFree = newQueueFree;
                              /* Update our progress                 */

      } /* while( (len > currentQueueFree) && spin ) */

      if ( currentQueueFree < len )
      {
         printmsg(0,"nswrite: Buffer overflow, needed %d bytes"
                     " from queue of %d",
                     len, outQueueSize);

         return 0;

      } /* if ( currentQueueFree < len ) */

   } /* if ( outQueueFree() < len ) */

/*--------------------------------------------------------------------*/
/*         Write the data out as the queue becomes available          */
/*--------------------------------------------------------------------*/

   bytes = WriteComm(nCid, data, len);

   rc = GetCommError(nCid, NULL);

   if (rc)
   {
      printmsg(0,"nswrite: WriteComm failed, "
                 "return code from GetCommError was %#04x (%d)",
                  rc ,
                  rc);

      ShowError(rc);
      return bytes;
   }

/*--------------------------------------------------------------------*/
/*                        Log the data written                        */
/*--------------------------------------------------------------------*/

   traceData( data, len, KWTrue );

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
   printmsg(12, "nssendbrk: %d", duration);
#endif

   SetCommBreak(nCid);
   ddelay(duration == 0 ? 200 : duration);
   ClearCommBreak(nCid);

} /*ssendbrk*/

#ifdef SUICIDE                      /* Crashes on kendra, alas       */

/*--------------------------------------------------------------------*/
/*    g e t M S R                                                     */
/*                                                                    */
/*    Get modem statrus register bits via by asking hardware          */
/*    directly.                                                       */
/*--------------------------------------------------------------------*/

static getMSR(const int port)
{                                   /* port is 1 thru 4              */
  static unsigned short far *bios = (unsigned short far *) 0x400000;
                                    /* == 0040:0000                  */
  unsigned short baseAddr = bios[port-1];

  if ( ! port )                     /* Reset lookaside info?         */
    return 0;                       /* Yes --> No op in this version */

  if (baseAddr)
     return inportb(baseAddr + 6);  /* offset of 6 = msr             */

  return 0;                         /* invalid port!                 */

} /* getMSR */

#else

/*--------------------------------------------------------------------*/
/*    g e t M S R                                                     */
/*                                                                    */
/*    Get modem statrus register bits via by Windows                  */
/*    directly.                                                       */
/*--------------------------------------------------------------------*/

static int getMSR(const int port)
{
   static LPBYTE lpbModemBits = 0;  /* --> Modem Status Register bits */


   if ( ! port )                    /* Reset lookaside info?         */
   {
      lpbModemBits = 0;             /* yes --> reset it              */
      return 0;                     /* return gracefully             */
   }
   else if ( ! lpbModemBits )
      lpbModemBits = (LPBYTE)SetCommEventMask(nCid, 0) + COMM_MSRSHADOW;

   if ( lpbModemBits )
      return (int) *lpbModemBits;
   else
      return 0;

} /* getMSR */

#endif

/*--------------------------------------------------------------------*/
/*    n c l o s e l i n e                                             */
/*                                                                    */
/*    Close the serial port down                                      */
/*--------------------------------------------------------------------*/

void ncloseline(void)
{
   if ( ! portActive )
      return;

   portActive = KWFalse;    /* flag port closed for error handler  */
   hangupNeeded = KWFalse;  /* Don't fiddle with port any more    */

/*--------------------------------------------------------------------*/
/*                             Lower DTR                              */
/*--------------------------------------------------------------------*/

    if (EscapeCommFunction(nCid, CLRDTR | CLRRTS) != 0)
    printmsg(0,"closeline: Unable to lower DTR/RTS");

/*--------------------------------------------------------------------*/
/*                      Actually close the port                       */
/*--------------------------------------------------------------------*/

   if (CloseComm(nCid) != 0)
      printmsg(0, "closeline: close of serial port failed");

/*--------------------------------------------------------------------*/
/*                   Stop logging the data to disk                    */
/*--------------------------------------------------------------------*/

   traceStop();

#ifdef UDEBUG
   printmsg(3,"Serial port closed");
#endif

   getMSR( 0 );                     /* reset MSR lookaside buffer    */

} /* ncloseline */

/*--------------------------------------------------------------------*/
/*    n h a n g u p                                                   */
/*                                                                    */
/*    Hangup the telephone by dropping DTR.  Works with HAYES and     */
/*    many compatibles.                                               */
/*--------------------------------------------------------------------*/

void nhangup( void )
{
   hangupNeeded = KWFalse;
   carrierDetect = KWFalse;

/*--------------------------------------------------------------------*/
/*                              Drop DTR                              */
/*--------------------------------------------------------------------*/

   if (EscapeCommFunction(nCid, CLRDTR) != 0)
   {
      printmsg(0, "hangup: Unable to lower DTR for comm port");
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
   }

   ddelay(2000);         /* Now wait for the poor thing to recover    */

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
   printmsg(15,"nSIOSpeed: Setting baud rate to %lu",
           (unsigned long) currentSpeed);

#ifdef UDEBUG
   ShowModem();
#endif

   GetCommState (nCid, &dcb);

   dcb.BaudRate = currentSpeed;
   rc = SetCommState (&dcb);

   if (rc)
   {
      printmsg(0,"nSIOSPeed: Unable to set baud rate for port to %lu",
         (unsigned long) currentSpeed);
      panic();
   }

} /* nSIOSpeed */

/*--------------------------------------------------------------------*/
/*    n f l o w c o n t r o l                                         */
/*                                                                    */
/*    Enable/Disable in band (XON/XOFF) flow control                  */
/*--------------------------------------------------------------------*/

void nflowcontrol( KWBoolean flow )
{
   int rc;
   DCB dcb;

   GetCommState(nCid, &dcb);

   if (flow)
   {
      dcb.fOutX = KWTrue;
      dcb.fInX = KWTrue;
      dcb.fRtsflow = KWFalse;
      dcb.fOutxCtsFlow = KWFalse;
   }
   else {
      dcb.fOutX = KWFalse;
      dcb.fInX = KWFalse;
      dcb.fRtsflow = KWTrue;
      dcb.fOutxCtsFlow = KWTrue;
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
/*                                                                    */
/*    If we previously had carrier detect but have lost it, we        */
/*    report it was lost.  If we do not yet have carrier detect,      */
/*    we return success because we may not have connected yet.        */
/*    If DSR is not present, we always report no carrier, as there    */
/*    is either no modem at all(!) or it's not turned on.             */
/*--------------------------------------------------------------------*/

KWBoolean nCD( void )
{
   KWBoolean newCarrierDetect;

   if ((getMSR(portNum) & MSR_RLSD) != 0)
      newCarrierDetect = KWTrue;
   else
      newCarrierDetect = KWFalse;

   if ( newCarrierDetect )
      carrierDetect = newCarrierDetect;

   if ( carrierDetect )
      return newCarrierDetect;
   else if ((getMSR(portNum) & MSR_DSR) != 0)
      return KWTrue;
   else
      return KWFalse;

} /* nCD */

#ifdef CHATPRJ                   /*  don't call printmsg from the DLL. */

static void ShowModem( void )
{
}

#else

/*--------------------------------------------------------------------*/
/*    S h o w M o d e m                                               */
/*                                                                    */
/*    Report current modem status when changed from last call         */
/*--------------------------------------------------------------------*/

#define mannounce(flag, bits, text ) (((flag & bits) != 0) ? text : "" )

static void ShowModem( void )
{

   BYTE modem_bits = getMSR(portNum);
   static BYTE old_bits = 0xFF;

   if ( debuglevel < 4 )
      return;

   if ( (debuglevel < 4) ||            /* Silent at lower debuglevels */
      (modem_bits == old_bits))        /* Show only changes in modem signals */
      return;

   printmsg(0, "ShowModem: %#02x %s %s %s",
        modem_bits,
        mannounce(MSR_RLSD,  modem_bits, "DCD"),
        mannounce(MSR_DSR,   modem_bits, "DSR"),
        mannounce(MSR_CTS,   modem_bits, "CTS"));
   old_bits = modem_bits;

} /* ShowModem */

#endif /* CHATPRJ */

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
