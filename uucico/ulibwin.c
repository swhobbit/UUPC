/*--------------------------------------------------------------------*/
/*       u l i b w i n . C                                            */
/*                                                                    */
/*       UUPC/extended serial port communications for Windows 3.1     */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes copyright (c) 1993, by Robert Denny                  */
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



//      21nov94 getMSR() added to avoid GP fault
//      it's probably only necessary if the port is invalid.
//      CHATPRJ is #defined for chat, not for UUCICO.

//      #ifdef USEPRINTMSG -> #ifndef CHATPRJ
//      because printmsg() uses va_list it's unkosher in a DLL.

//      mlz -- nopenline de-clawed : nCid is assumed to be valid
//      showmodem de-clawed.  ... will fail in a DLL.

/*
 *    $Id: ulibwin.c 1.14 1994/05/08 22:46:32 ahd Exp $
 *
 *    Revision history:
 *    $Log: ulibwin.c $
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

  //  asymmetrical preprocessor - sorry! // mlz
  //
#ifdef CHATPRJ
  #include "chat.h"
#else
  checkForBreak();
#endif

extern long windowsBuffer; // lets the user set it with the -b option


#include "u.h"

#include "debug.h"//erase


/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "ulib.h"
#include "ssleep.h"
#include "commlib.h"

currentfile();

int portNum=NULL;
static boolean hangupNeeded = FALSE;
static UINT currentSpeed = 0;

/*
      The q101417 algorithm referenced below, while it may be ugly,
      gives a GP fault if the port is invalid.

      getMSR has been substituted instead.   // mlz
 */
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

//#define COMM_MSRSHADOW 35          /* Offset in DEB of MSR shadow     */
#define MSR_CTS              0x10  /* absolute CTS state in MSR        */
#define MSR_DSR              0x20  /* absolute DSR state in MSR        */
#define MSR_RI               0x40  /* absolute RI state in MSR        */
#define MSR_RLSD             0x80  /* absolute RLSD state in MSR      */


#ifdef _MSC_VER
  #define inportb _inp
#endif


//  modem status register.  The above MSR flags apply.
//
getMSR(int port) { // port is 1 thru 4
  uint far *bios=(uint far *)0x400000;  // == 40:0
  uint baseAddr=bios[port-1];
  if (baseAddr) return inportb(baseAddr + 6); // offset of 6=msr
  return 0; // invalid port!
}

#define FAR_NULL ((PVOID) 0L)

//#define IN_QUEUE_SIZE   32768 // was 2048  // mlz
//#define OUT_QUEUE_SIZE  32768 // was 2048
//#include "dcp.h" // dcpsys needs it
//#include "dcpsys.h" // for flds[]   // won't work in passive mode
                      // see flds comment below
UINT IN_QUEUE_SIZE,OUT_QUEUE_SIZE;
#define IN_XOFF_LIM     256
#define IN_XON_LIM      256

/*--------------------------------------------------------------------*/
/*         Definitions of control structures for Win 3.1 API          */
/*--------------------------------------------------------------------*/

int nCid; // mlz -- will be assigned in chat
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
//      return: 0=ok, "TRUE"=fail
/*--------------------------------------------------------------------*/


#ifdef __TURBOC__
#pragma argsused
#endif

int nopenline(char *name, BPS baud, const boolean direct ) {

#ifndef CHATPRJ
   int rc;

   if (portActive)               /* Was the port already active?     ahd  */
      closeline();               /* Yes --> Shutdown it before open  ahd  */

   #ifdef UDEBUG
   printmsg(15, "openline: %s, %ul", name, (unsigned long) baud);
   #endif

   if (!equaln(name, "COM", 3 ) || name[3]<'1' || name[3]>'4') {
      printmsg(0,
     "openline: \"Device=\" must be in format COMx, "
     " 1<=x<=4; was %s", name);
      return TRUE;
   }

   // The above test guarantees the format of "name"
   // ifdef CHATPRJ, it's set in dialChat() -- mlz
   //
   portNum=name[3]-'0';

   //strlwr(flds[FLD_PROTO]);
   //   flds will not yet be set in passive polling mode:
   //   it needs to see what system it is before it sets the protocol
   //
   //if (strchr(flds[FLD_PROTO],'v')) {
   /*
   if (1) {
      IN_QUEUE_SIZE=32768;
      OUT_QUEUE_SIZE=32768;
   }
   else {
     IN_QUEUE_SIZE=2048;
     OUT_QUEUE_SIZE=2048;
   }
   */
   windowsBuffer=max(windowsBuffer,2048);
   IN_QUEUE_SIZE=(OUT_QUEUE_SIZE=windowsBuffer);
   if ((nCid = OpenComm(name, IN_QUEUE_SIZE, OUT_QUEUE_SIZE)) < 0)
   {
      printmsg(0, "openline: Failed to open port %s.", name);
      printmsg(0, "nopenline: %s: OpenComm returned %#04X (%d)",
           name,
           nCid,
           nCid);
      return TRUE;
   }

/*--------------------------------------------------------------------*/
/*            Reset any errors on the communications port             */
/*--------------------------------------------------------------------*/

   if ((rc = GetCommError (nCid, NULL)) != 0) {
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
   if(!direct)                 /* nodirect means RTS/CTS flow OK      */
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

#endif//CHATPRJ

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

unsigned int nsread(char UUFAR *output,
            unsigned int wanted,
            unsigned int timeout) {

   int rc, received; time_t stop_time, now; COMSTAT stat;

   // This catches a fencepost condition later...
   //
   if (wanted == 0) { ddelay(0); return(0);
   }

/*--------------------------------------------------------------------*/
/*                      Report our modem status                       */
/*--------------------------------------------------------------------*/

  #ifndef CHATPRJ
  ShowModem();
  #endif

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

   while(TRUE) {
      #ifdef CHATPRJ
      if (beenPressed()) return 0;
      #else
      checkForBreak(); // uucico
      #endif

      // Check & clear the comm port. This gets the #chars in the
      // receive queue as well, in the COMSTAT structure.
      //
      if ((rc = GetCommError(nCid, &stat)) != 0) {
    /*
        Just plow right on through.
        Thing is, some modems give a single framing error
        every time they connect.  Hardly a reason to quit.

        It needs to do a GetCommEventMask first, anyway.

     #ifndef CHATPRJ
     printmsg(0,"sread: Read of %d bytes failed.", wanted);
     printmsg(0,"       return code from GetCommError was %#04x (%d)",
                        rc , rc);
     #endif
     ShowError(rc);
     return 0;

     */
      }

      //
      // If wanted # bytes are available, break out and read 'em.
      //
      if (stat.cbInQue >= wanted) break;

      // Be friendly to Windows' cooperative multitasking...
      //
      ddelay(0);

      // If timeout is zero, return immediately.
      //
      if (stop_time == 0) { return(stat.cbInQue);
      }

      // Check for timeout. If timed out, return.
      //
      time( &now );
      if(stop_time <= now) {
     #ifndef CHATPRJ
     printmsg(15, "sread: timeout(%d) - %d chars avail",
             timeout, stat.cbInQue);
     #endif
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

int nswrite(const char UUFAR *data, unsigned int len)
{
   int bytes;
   int rc;

   hangupNeeded = TRUE;      /* Flag that the port is now dirty  */

/*--------------------------------------------------------------------*/
/*                      Report our modem status                       */
/*--------------------------------------------------------------------*/

   #ifndef CHATPRJ
   ShowModem();
   #endif

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
showSpot;

   if ( ! portActive ) return;
      // panic();
      // since ncloseline gets called when exit()-ing,
      // it's a really bad idea to call exit again inside it

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
showSpot;

} /* ncloseline */

/*--------------------------------------------------------------------*/
/*    n h a n g u p                                                   */
/*                                                                    */
/*    Hangup the telephone by dropping DTR.  Works with HAYES and     */
/*    many compatibles.                                               */
/*--------------------------------------------------------------------*/

void nhangup( void )
{
   hangupNeeded = FALSE;
   carrierDetect = FALSE;

/*--------------------------------------------------------------------*/
/*                              Drop DTR                              */
/*--------------------------------------------------------------------*/

   if (EscapeCommFunction(nCid, CLRDTR) != 0)
   {
      printmsg(0, "hangup: Unable to lower DTR for comm port");
      //panic();  // really! no need to cop out here.
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
      //panic();
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
   printmsg(15,"SIOSpeed: Setting baud rate to %lu",
           (unsigned long) currentSpeed);

   #ifndef CHATPRJ
   ShowModem();
   #endif
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
/*                                                                    */
/*    If we previously had carrier detect but have lost it, we        */
/*    report it was lost.  If we do not yet have carrier detect,      */
/*    we return success because we may not have connected yet.        */
/*    If DSR is not present, we always report no carrier, as there    */
/*    is either no modem at all(!) or it's not turned on.             */
/*--------------------------------------------------------------------*/


boolean nCD( void ) {
   boolean newCarrierDetect = ((getMSR(portNum) & MSR_RLSD) != 0);
   if ( newCarrierDetect ) carrierDetect = newCarrierDetect;
   if ( carrierDetect ) return newCarrierDetect;
   else return ((getMSR(portNum) & MSR_DSR) != 0) ? TRUE : FALSE;

} /* nCD */

/*--------------------------------------------------------------------*/
/*    S h o w M o d e m                                               */
/*                                                                    */
/*    Report current modem status when changed from last call         */
/*--------------------------------------------------------------------*/

#define mannounce(flag, bits, text ) (((flag & bits) != 0) ? text : "" )

static void ShowModem( void ) {
   #ifndef CHATPRJ   //  don't call printmsg from the DLL.
   BYTE modem_bits = getMSR(portNum);
   static BYTE old_bits = 0xFF;

   if ( debuglevel < 4 )
      return;

   if ( (debuglevel < 4) ||            /* Silent at lower debuglevels  */
      (modem_bits == old_bits))        /* Show only changes in modem signals  */
      return;

   printmsg(0, "ShowModem: %#02x %s %s %s",
        modem_bits,
        mannounce(MSR_RLSD,  modem_bits, "DCD"),
        mannounce(MSR_DSR,   modem_bits, "DSR"),
        mannounce(MSR_CTS,   modem_bits, "CTS"));
   old_bits = modem_bits;
   #endif//mlz
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
