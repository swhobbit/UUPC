/*--------------------------------------------------------------------*/
/*    u l i b n t . c                                                 */
/*                                                                    */
/*    Windows NT serial port support for UUCICO                       */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) David M. Watt 1993, All Rights Reserved   */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1994 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *       $Id: ulibnt.c 1.16 1994/01/01 19:22:00 ahd Exp $
 *       $Log: ulibnt.c $
 * Revision 1.16  1994/01/01  19:22:00  ahd
 * Annual Copyright Update
 *
 * Revision 1.15  1993/12/29  03:34:37  dmwatt
 * Restructure to use overlapped I/O
 *
 * Revision 1.13  1993/12/06  02:29:32  ahd
 * Make bit twiddles of modem status bit AND and bit OR's, not logical!
 *
 * Revision 1.12  1993/12/06  01:59:07  ahd
 * Add missing handle retrieval function
 *
 * Revision 1.11  1993/11/30  04:13:30  dmwatt
 * Optimize port processing
 *
 * Revision 1.11  1993/11/30  04:13:30  dmwatt
 * Optimize port processing
 *
 * Revision 1.9  1993/11/06  17:57:09  rhg
 * Drive Drew nuts by submitting cosmetic changes mixed in with bug fixes
 *
 * Revision 1.8  1993/10/12  01:33:23  ahd
 * Normalize comments to PL/I style
 *
 * Revision 1.7  1993/10/07  22:56:45  ahd
 * Use dynamically allocated buffer
 *
 * Revision 1.6  1993/10/03  22:09:09  ahd
 * Use unsigned long to display speed
 *
 * Revision 1.5  1993/09/26  03:32:27  dmwatt
 * Use Standard Windows NT error message module
 *
 * Revision 1.4  1993/09/25  03:07:56  ahd
 * Add standard Windows NT error message calls
 *
 * Revision 1.3  1993/09/21  01:42:13  ahd
 * Use standard MAXPACK limit for save buffer size
 *
 * Revision 1.2  1993/09/20  04:50:57  ahd
 * Break out of ULIBOS2.C
 *
 * Revision 1.14  1993/05/30  15:25:50  ahd
 * Multiple driver support
 *
 * Revision 1.13  1993/05/30  00:08:03  ahd
 * Multiple communications driver support
 * Delete trace functions
 *
 * Revision 1.12  1993/05/09  03:41:47  ahd
 * Make swrite accept constant input strings
 *
 * Revision 1.11  1993/04/11  00:34:11  ahd
 * Global edits for year, TEXT, etc.
 *
 * Revision 1.10  1993/04/10  21:25:16  dmwatt
 * Add Windows NT support
 *
 * Revision 1.9  1993/04/05  04:32:19  ahd
 * Additional traps for modem dropping out
 *
 * Revision 1.8  1993/04/04  04:57:01  ahd
 * Add configurable OS/2 priority values
 *
 * Revision 1.7  1992/12/30  13:02:55  dmwatt
 * Dual path for Windows NT and OS/2
 *
 * Revision 1.6  1992/12/11  12:45:11  ahd
 * Correct RTS handshake
 *
 * Revision 1.5  1992/12/04  01:00:27  ahd
 * Add copyright message, reblock other comments
 *
 * Revision 1.4  1992/11/29  22:09:10  ahd
 * Add new define for BC++ OS/2 build
 *
 * Revision 1.3  1992/11/19  03:00:39  ahd
 * drop rcsid
 *
 * Revision 1.2  1992/11/15  20:11:48  ahd
 * Add English display of modem status and error bits
 *
 */

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
/*                      Windows NT include files                      */
/*--------------------------------------------------------------------*/

#include <windows.h>
#include <limits.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "ulib.h"
#include "ssleep.h"
#include "catcher.h"

#include "dcp.h"
#include "commlib.h"
#include "pnterr.h"
#include "suspend.h"

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

currentfile();

static boolean   carrierDetect = FALSE;  /* Modem is not connected    */

static boolean hangupNeeded = FALSE;

static currentSpeed = 0;

#define FAR_NULL ((PVOID) 0L)

/*--------------------------------------------------------------------*/
/*           Definitions of control structures for DOS API            */
/*--------------------------------------------------------------------*/

static HANDLE hCom = INVALID_HANDLE_VALUE;
static HANDLE hComEvent = INVALID_HANDLE_VALUE;
static COMMTIMEOUTS CommTimeout;
static DCB dcb;
static DCB save_dcb;

static BYTE com_status;
static USHORT com_error;

static USHORT usPrevPriority;

static void ShowError( const USHORT status );

static void ShowModem( const DWORD status );

/*--------------------------------------------------------------------*/
/*    n o p e n l i n e                                               */
/*                                                                    */
/*    Open the serial port for I/O                                    */
/*--------------------------------------------------------------------*/

int nopenline(char *name, BPS baud, const boolean direct )
{
   DWORD dwError;
   BOOL rc;

   if (portActive)              /* Was the port already active?     ahd  */
      closeline();               /* Yes --> Shutdown it before open  ahd  */

#ifdef UDEBUG
   printmsg(15, "nopenline: %s, %lu",
                 name,
                 (unsigned long) baud);
#endif

/*--------------------------------------------------------------------*/
/*                      Validate the port format                      */
/*--------------------------------------------------------------------*/

   if (!equal(name,"CON") && !equaln(name, "COM", 3 ))
   {
      printmsg(0,"nopenline: Communications port begin with COM, was %s",
         name);
      panic();
   }

/*--------------------------------------------------------------------*/
/*                          Perform the open                          */
/*--------------------------------------------------------------------*/
   hComEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
   if (hComEvent == INVALID_HANDLE_VALUE) {
      printNTerror("CreateEvent", GetLastError());
      panic();
   }

   hCom = CreateFile( name,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_OVERLAPPED,
        NULL);

/*--------------------------------------------------------------------*/
/*    Check the open worked.  We translation the common obvious       */
/*    error of file in use to english, for all other errors are we    */
/*    report the raw error code.                                      */
/*--------------------------------------------------------------------*/

   if (hCom == INVALID_HANDLE_VALUE) {
       dwError = GetLastError();
       printmsg(0, "nopenline: OpenFile error on port %s", name);
       printNTerror("nopenline", dwError);
       return TRUE;
   }

/*--------------------------------------------------------------------*/
/*            Reset any errors on the communications port             */
/*--------------------------------------------------------------------*/

   rc = ClearCommError (hCom,
        &dwError,
        NULL);

   if (!rc) {
      printmsg(0, "nopenline: Error in ClearCommError() call\n");
      printNTerror("nopenline", dwError);
   }

/*--------------------------------------------------------------------*/
/*          Get and save line attributes and baud rate                */
/*--------------------------------------------------------------------*/

#ifdef UDEBUG
   printmsg(15,"nopenline: Getting attributes");
#endif

   rc = GetCommState(hCom, &dcb);
   if (!rc) {
      dwError = GetLastError();

      printmsg(0,"nopenline: Unable to get line attributes for %s",name);
      printNTerror("nopenline", dwError);
      panic();
   }

   memcpy(&save_dcb, &dcb, sizeof(dcb));

/*--------------------------------------------------------------------*/
/*                           Set baud rate                            */
/*--------------------------------------------------------------------*/

   SIOSpeed(baud);

/*--------------------------------------------------------------------*/
/*                        Set line attributes                         */
/*--------------------------------------------------------------------*/

   dcb.StopBits = ONESTOPBIT;
   dcb.Parity = NOPARITY;
   dcb.ByteSize = 8;

#ifdef UDEBUG
   printmsg(15,"nopenline: Setting attributes");
#endif

   rc = SetCommState(hCom, &dcb);
   if (!rc)
   {
      dwError = GetLastError();

      printmsg(0,"nopenline: Unable to set line attributes for %s",name);
      printNTerror("nopenline", dwError);

      panic();

   }

/*--------------------------------------------------------------------*/
/*                     Disable XON/XOFF flow control                  */
/*                     Enable CTS handling for flow control           */
/*--------------------------------------------------------------------*/

#ifdef UDEBUG
   printmsg(15,"nopenline: Getting flow control information");
#endif

   GetCommState(hCom, &dcb);
   dcb.fOutX = 0;
   dcb.fInX = 0;
   dcb.fOutxCtsFlow = 1;
   rc = SetCommState(hCom, &dcb);
   if (!rc) {
      dwError = GetLastError();
      printmsg(0,"nopenline: Unable to set comm attributes for %s",name);
      printNTerror("nopenline", dwError);

      panic();
   }

/* Get communications timeout information */

   rc = GetCommTimeouts(hCom, &CommTimeout);
   if (!rc) {
      dwError = GetLastError();
      printmsg(0, "nopenline: error on GetCommTimeouts() on %s", name);
      printNTerror("nopenline", dwError);
      panic();
   }

/*--------------------------------------------------------------------*/
/*                     Raise Data Terminal Ready                      */
/*--------------------------------------------------------------------*/

   GetCommState(hCom, &dcb);
   dcb.fDtrControl = DTR_CONTROL_ENABLE;
   dcb.fRtsControl = RTS_CONTROL_ENABLE;

   rc = SetCommState(hCom, &dcb);
   if (!rc) {
      printmsg(0,

            "nopenline: Unable to raise DTR/RTS for %s",
                  name);

      panic();
   }

   traceStart( name );     /* Enable logging                          */

   portActive = TRUE;     /* record status for error handler        */
   carrierDetect = FALSE;  /* Modem is not connected                 */

/*--------------------------------------------------------------------*/
/*                     Wait for port to stablize                      */
/*--------------------------------------------------------------------*/

   ddelay(500);            /* Allow port to stablize          */
   return 0;

} /*openline*/

/*--------------------------------------------------------------------*/
/*    n s r e a d                                                     */
/*                                                                    */
/*    Read from the serial port                                       */
/*                                                                    */
/*   Non-blocking read essential to "g" protocol.  See "dcpgpkt.c"    */
/*   for description.                                                 */
/*                                                                    */
/*   This all changes in a multi-tasking system.  Requests for I/O    */
/*   should get queued and an event flag given.  Then the             */
/*   requesting process (e.g. gmachine()) waits for the event flag    */
/*   to fire processing either a read or a write.  Could be           */
/*   implemented on VAX/VMS or DG but not MS-DOS.                     */
/*                                                                    */
/*    OS/2 we could multitask, but we just let the system provide     */
/*    a timeout for us with very little CPU usage.                    */
/*--------------------------------------------------------------------*/

unsigned int nsread(char *output, unsigned int wanted, unsigned int timeout)
{
   static LPVOID psave;
   DWORD dwError;
   BOOL rc;
   time_t stop_time ;
   time_t now ;
   OVERLAPPED overlap;

   boolean firstPass =  ( currentSpeed > 2400 ) || ( wanted == 1 );
                              /* Perform extended read only on high-
                                 speed modem links when looking for
                                 packet data                         */

/*--------------------------------------------------------------------*/
/*                           Validate input                           */
/*--------------------------------------------------------------------*/

   if ( wanted > commBufferLength )
   {
      printmsg(0,"nsread: Overlength read, wanted %u bytes into %u buffer!",
                     (unsigned int) wanted,
                     (unsigned int) commBufferLength );
      panic();
   }

/*--------------------------------------------------------------------*/
/*           Determine if our internal buffer has the data            */
/*--------------------------------------------------------------------*/

   if (commBufferUsed >= wanted)
   {
      memcpy( output, commBuffer, wanted );
      commBufferUsed -= wanted;
      if ( commBufferUsed )   /* Any data left over?                 */
         memmove( commBuffer, commBuffer + wanted, commBufferUsed );
                              /* Yes --> Save it                     */
      return wanted + commBufferUsed;
   } /* if */

/*--------------------------------------------------------------------*/
/*            Reset any errors on the communications port             */
/*--------------------------------------------------------------------*/

   rc = ClearCommError (hCom,
        &dwError,
        NULL);

   if (!rc) {
      printmsg(0, "sread:  Unable to read port errors");
      printNTerror("sread", dwError);
   }

/*--------------------------------------------------------------------*/
/*                 Determine when to stop processing                  */
/*--------------------------------------------------------------------*/

   if ( timeout == 0 )
   {
      stop_time = 0;
      now = 1;                /* Any number greater than stop time   */
   }
   else {
      time( & now );
      stop_time = now + timeout;
   }

/*--------------------------------------------------------------------*/
/*            Try to read any needed data into the buffer             */
/*--------------------------------------------------------------------*/

   do {
      DWORD received;
      DWORD needed = wanted - commBufferUsed;
      DWORD portTimeout;

/*--------------------------------------------------------------------*/
/*                     Handle an aborted program                      */
/*--------------------------------------------------------------------*/
      if ( suspend_processing )
         return 0;

      if ( terminate_processing )
      {
         static boolean recurse = FALSE;
         if ( ! recurse )
         {
            printmsg(2,"sread: User aborted processing");
            recurse = TRUE;
         }
         return 0;
      }

/*--------------------------------------------------------------------*/
/*           Compute a new timeout for the read, if needed            */
/*--------------------------------------------------------------------*/

      if ((stop_time <= now ) || firstPass )
      {
         portTimeout = 0;
         firstPass = FALSE;
      }
      else {
         portTimeout = (USHORT) (stop_time - now) / needed * 1000;
         if (portTimeout < 1000)
            portTimeout = 1000;
      } /* else */

      if (portTimeout == 0)
         portTimeout = 10;

      CommTimeout.ReadTotalTimeoutConstant = portTimeout;
      CommTimeout.WriteTotalTimeoutConstant = 0;
      CommTimeout.ReadIntervalTimeout = (portTimeout != 0) ?
         0 : MAXDWORD;
      CommTimeout.ReadTotalTimeoutMultiplier = 0;
      CommTimeout.WriteTotalTimeoutMultiplier = 0;
      rc = SetCommTimeouts(hCom, &CommTimeout);

      if ( !rc )
      {
         dwError = GetLastError();
         printmsg(0, "sread: unable to set timeout for comm port");
         printNTerror("sread", dwError);
         panic();
      }

#ifdef UDEBUG
      printmsg(15,"sread: Port time out is %ud seconds/100",
               portTimeout);
#endif

/*--------------------------------------------------------------------*/
/*             Reset the event and initialize the overlap struct      */
/*--------------------------------------------------------------------*/
      ResetEvent(hComEvent);
      memset(&overlap, 0, sizeof(OVERLAPPED));
      overlap.hEvent = hComEvent;

/*--------------------------------------------------------------------*/
/*                 Read the data from the serial port                 */
/*--------------------------------------------------------------------*/
      rc = ReadFile (hCom,
                     commBuffer + commBufferUsed,
                     portTimeout ? needed : commBufferLength - commBufferUsed,
                     &received,
                     &overlap);

      if (!rc)
      {
         dwError = GetLastError();

         if (dwError != ERROR_IO_PENDING)
         {
            printmsg(0,
               "sread: Read from comm port for %d bytes failed, received = %d.",
               needed, (int) received);
            printNTerror("ReadFile", dwError);
            commBufferUsed = 0;
            return 0;
         }
      }

      rc = GetOverlappedResult(hCom, &overlap, &received, TRUE);

      if (!rc)
      {
         dwError = GetLastError();
         if (dwError != ERROR_OPERATION_ABORTED)
            printNTerror("GetOverlappedResult", GetLastError());
      }

#ifdef UDEBUG
      printmsg(15,"sread: Want %d characters, received %d, total %d in buffer",
                  (int) wanted,
                  (int) received,
                  (int) (commBufferUsed + received));
#endif

/*--------------------------------------------------------------------*/
/*                    Log the newly received data                     */
/*--------------------------------------------------------------------*/

      traceData( commBuffer + commBufferUsed,
                 (unsigned) received,
                 FALSE );

/*--------------------------------------------------------------------*/
/*            If we got the data, return it to the caller             */
/*--------------------------------------------------------------------*/

      commBufferUsed += received;
      if ( commBufferUsed >= wanted )
      {
         memcpy( output, commBuffer, wanted );
         commBufferUsed -= wanted;
         if ( commBufferUsed )   /* Any data left over?              */
            memmove( commBuffer, commBuffer + wanted, commBufferUsed );

         return wanted;

      } /* if */

/*--------------------------------------------------------------------*/
/*                 Update the clock for the next pass                 */
/*--------------------------------------------------------------------*/

      if (stop_time > 0)
         time( &now );

   } while (stop_time > now);

/*--------------------------------------------------------------------*/
/*         We don't have enough data; report what we do have          */
/*--------------------------------------------------------------------*/

   return commBufferUsed;

} /*nsread*/

/*--------------------------------------------------------------------*/
/*    n s w r i t e                                                   */
/*                                                                    */
/*    Write to the serial port                                        */
/*--------------------------------------------------------------------*/

int nswrite(const char *input, unsigned int len)
{
   OVERLAPPED overlap;

   char *data = (char *) input;

   DWORD bytes;
   DWORD dwError;
   BOOL rc;
   hangupNeeded = TRUE;      /* Flag that the port is now dirty  */

/*--------------------------------------------------------------------*/
/*         Write the data out as the queue becomes available          */
/*--------------------------------------------------------------------*/
   rc = ResetEvent(hComEvent);

   if (!rc)
   {
      printNTerror("ResetEvent", GetLastError());
   }

   memset(&overlap, 0, sizeof(OVERLAPPED));
   overlap.hEvent = hComEvent;

   rc = WriteFile (hCom, data, len, &bytes, &overlap);

   if (!rc) {
      dwError = GetLastError();
      if (dwError != ERROR_IO_PENDING)
      {
         printmsg(0,"nswrite: Write to communications port failed.");
         printNTerror("WriteFile", dwError);
         return bytes;
      }
   }

   rc = GetOverlappedResult(hCom, &overlap, &bytes, TRUE);

   if (!rc)
   {
      dwError = GetLastError();
      if (dwError != ERROR_OPERATION_ABORTED)
         printNTerror("GetOverlappedResult", GetLastError());
   }

/*--------------------------------------------------------------------*/
/*                        Log the data written                        */
/*--------------------------------------------------------------------*/

   traceData( data, len, TRUE);

/*--------------------------------------------------------------------*/
/*            Return bytes written to the port to the caller          */
/*--------------------------------------------------------------------*/

   return len;

} /*nswrite*/

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

   SetCommBreak(hCom);

   ddelay( duration == 0 ? 200 : duration);

   ClearCommBreak(hCom);

} /*nssendbrk*/

/*--------------------------------------------------------------------*/
/*    n c l o s e l i n e                                             */
/*                                                                    */
/*    Close the serial port down                                      */
/*--------------------------------------------------------------------*/

void ncloseline(void)
{
   DWORD dwError;
   BOOL rc;

   if ( ! portActive )
      panic();

   portActive = FALSE; /* flag port closed for error handler  */
   hangupNeeded = FALSE;  /* Don't fiddle with port any more  */

/*--------------------------------------------------------------------*/
/*                             Lower DTR                              */
/*--------------------------------------------------------------------*/

   if (!EscapeCommFunction(hCom, CLRDTR | CLRRTS))
   {
      printmsg(0,"ncloseline: Unable to lower DTR/RTS");
   }

/*--------------------------------------------------------------------*/
/*                   Restore the original port settings               */
/*--------------------------------------------------------------------*/

   rc = SetCommState(hCom, &save_dcb);
   if (!rc) {
      dwError = GetLastError();

      printmsg(0,"ncloseline: Unable to restore line attributes");
      printNTerror("ncloseline", dwError);
   }

/*--------------------------------------------------------------------*/
/*                      Actually close the port                       */
/*--------------------------------------------------------------------*/

   if(!CloseHandle(hCom))
   {
      dwError = GetLastError();
      printmsg(0, "ncloseline: close of serial port failed");
      printNTerror("ncloseline", dwError);
   }

   if(!CloseHandle(hComEvent))
   {
      dwError = GetLastError();
      printmsg(0, "ncloseline: close of event failed");
      printNTerror("ncloseline", dwError);
   }

   hCom = INVALID_HANDLE_VALUE;
   hComEvent = INVALID_HANDLE_VALUE;

/*--------------------------------------------------------------------*/
/*                   Stop logging the data to disk                    */
/*--------------------------------------------------------------------*/

   traceStop();

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
   if (!hangupNeeded)
      return;

   hangupNeeded = FALSE;

/*--------------------------------------------------------------------*/
/*                              Drop DTR                              */
/*--------------------------------------------------------------------*/

   if (!EscapeCommFunction(hCom, CLRDTR))
   {
      printmsg(0, "hangup: Unable to lower DTR for comm port");
      panic();
   }

/*--------------------------------------------------------------------*/
/*                  Wait for the telephone to hangup                  */
/*--------------------------------------------------------------------*/

   printmsg(3,"hangup: Dropped DTR");
   carrierDetect = FALSE;  /* Modem is not connected                 */
   ddelay(500);            /* Really only need 250 milliseconds        */

/*--------------------------------------------------------------------*/
/*                          Bring DTR back up                         */
/*--------------------------------------------------------------------*/

   if (!EscapeCommFunction(hCom, SETDTR))
   {
      printmsg(0, "hangup: Unable to raise DTR for comm port");
      panic();
   }

   ddelay(2000);           /* Now wait for the poor thing to recover   */

} /* nhangup */

/*--------------------------------------------------------------------*/
/*    n S I O S p e e d                                               */
/*                                                                    */
/*    Re-specify the speed of an opened serial port                   */
/*                                                                    */
/*    Dropped the DTR off/on calls because this makes a Hayes drop    */
/*    the line if configured properly, and we don't want the modem    */
/*    to drop the phone on the floor if we are performing             */
/*    autobaud.                                                       */
/*                                                                    */
/*    (Configured properly = standard method of making a Hayes        */
/*    hang up the telephone, especially when you can't get it into    */
/*    command state because it is at the wrong speed or whatever.)    */
/*--------------------------------------------------------------------*/

void nSIOSpeed(BPS baud)
{
   USHORT rc;

#ifdef UDEBUG
   printmsg(15,"SIOSpeed: Setting baud rate to %lu",
               (unsigned long) baud);
#endif

   GetCommState (hCom, &dcb);
   dcb.BaudRate = baud;
   rc = SetCommState (hCom, &dcb);
   if (!rc) {
      printmsg(0,"SIOSpeed: Unable to set baud rate for port to %lu",
                  (unsigned long) baud);
      panic();

   }

   currentSpeed = baud;

} /* nSIOSpeed */

/*--------------------------------------------------------------------*/
/*    n f l o w c o n t r o l                                         */
/*                                                                    */
/*    Enable/Disable in band (XON/XOFF) flow control                  */
/*--------------------------------------------------------------------*/

void nflowcontrol( boolean flow )
{
   USHORT rc;
   DCB dcb;
   DWORD dwError;

   GetCommState(hCom, &dcb);
   if (flow)
   {
      dcb.fOutX = TRUE;
      dcb.fInX = TRUE;
      dcb.fRtsControl = RTS_CONTROL_ENABLE;
      dcb.fOutxCtsFlow = FALSE;
   } else {
      dcb.fOutX = FALSE;
      dcb.fInX = FALSE;
      dcb.fRtsControl = RTS_CONTROL_ENABLE;
      dcb.fOutxCtsFlow = TRUE;
   }
   rc = SetCommState(hCom, &dcb);

   if ( !rc )
   {
      dwError = GetLastError();

      printmsg(0,"flowcontrol: Unable to set flow control");
      printNTerror("nflowcontrol", dwError);

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
} /* nGetSpeed */

/*--------------------------------------------------------------------*/
/*   n C D                                                            */
/*                                                                    */
/*   Return status of carrier detect                                  */
/*--------------------------------------------------------------------*/

boolean nCD( void )
{
   boolean previousCarrierDetect = carrierDetect;
   USHORT rc;

   DWORD status;
   static DWORD oldstatus = (DWORD) 0xDEADBEEF;
   DWORD dwError;

   rc = GetCommModemStatus(hCom, &status);
   if ( !rc )
   {
      dwError = GetLastError();
      printmsg(0,"nCD: Unable to get modem status");
      printNTerror("nCD", dwError);
      panic();
   } /*if */

   if ( status != oldstatus )
   {
      ShowModem( status );
      oldstatus = status;
   }

/*--------------------------------------------------------------------*/
/*    If we previously had carrier detect but have lost it, we        */
/*    report it was lost.  If we do not yet have carrier detect,      */
/*    we return success because we may not have connected yet.        */
/*--------------------------------------------------------------------*/

   carrierDetect = status & MS_RLSD_ON ? TRUE : FALSE;

   if (previousCarrierDetect)
      return carrierDetect;
   else
      return (status & MS_DSR_ON) ? TRUE : FALSE;

} /* nCD */

/*--------------------------------------------------------------------*/
/*    S h o w M o d e m                                               */
/*                                                                    */
/*    Report current modem status                                     */
/*--------------------------------------------------------------------*/

#define mannounce(flag, bits, text ) ((flag & bits) ? text : "" )

static void ShowModem( const DWORD status )
{
   if ( debuglevel < 4 )
      return;

   printmsg(0, "ShowModem: %#02x%s%s%s%s",
      status,
      mannounce(MS_RLSD_ON,  status, "  Carrier Detect"),
      mannounce(MS_RING_ON,  status, "  Ring Indicator"),
      mannounce(MS_DSR_ON,   status, "  Data Set Ready"),
      mannounce(MS_CTS_ON,   status, "  Clear to Send"));

} /* ShowModem */

/*--------------------------------------------------------------------*/
/*          n G e t C o m H a n d l e                                 */
/*                                                                    */
/*          Return handle to open port                                */
/*--------------------------------------------------------------------*/

int nGetComHandle( void )
{

   return (int) hCom;

}  /* nGetComHandle */

BOOL AbortComm(void)
{
   BOOL retval = FALSE;

   if (hCom != INVALID_HANDLE_VALUE)
      retval = PurgeComm(hCom, PURGE_TXABORT | PURGE_RXABORT);

   if (terminate_processing) {
      printmsg(9, "Setting hComEvent");
      if (hComEvent != INVALID_HANDLE_VALUE)
         SetEvent(hComEvent);
   }

   return retval;
}

