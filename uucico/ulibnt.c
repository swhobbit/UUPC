/*--------------------------------------------------------------------*/
/*    u l i b o s 2 . c                                               */
/*                                                                    */
/*    OS/2 serial port support for UUCICO                             */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989 by Andrew H. Derbyshire.             */
/*                                                                    */
/*    Changes Copyright (c) 1990-1993 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *       $Id: ulibnt.c 1.2 1993/09/20 04:50:57 ahd Exp $
 *       $Log: ulibnt.c $
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
 * Add Windows/NT support
 *
 * Revision 1.9  1993/04/05  04:32:19  ahd
 * Additional traps for modem dropping out
 *
 * Revision 1.8  1993/04/04  04:57:01  ahd
 * Add configurable OS/2 priority values
 *
 * Revision 1.7  1992/12/30  13:02:55  dmwatt
 * Dual path for Windows/NT and OS/2
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
/*                      Windows/NT include files                      */
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

#include "commlib.h"

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

currentfile();

static boolean   carrierdetect = FALSE;  /* Modem is not connected     */

static boolean hangupNeeded = FALSE;
static boolean console = FALSE;

static currentSpeed = 0;

#define FAR_NULL ((PVOID) 0L)

/*--------------------------------------------------------------------*/
/*           Definitions of control structures for DOS API            */
/*--------------------------------------------------------------------*/

static HANDLE hCom;
static COMMTIMEOUTS CommTimeout;
static DCB dcb;

static BYTE com_status;
static USHORT com_error;

#ifdef __OS2__
static ULONG usPrevPriority;
#else
static USHORT usPrevPriority;
#endif

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
   DWORD Error;
   BOOL rc;
   HANDLE hProcess;

   if (portActive)              /* Was the port already active?     ahd   */
      closeline();               /* Yes --> Shutdown it before open  ahd   */

#ifdef UDEBUG
   printmsg(15, "openline: %s, %d", name, baud);
#endif

/*--------------------------------------------------------------------*/
/*                      Validate the port format                      */
/*--------------------------------------------------------------------*/

   if (!equal(name,"CON") && !equaln(name, "COM", 3 ))
   {
      printmsg(0,"openline: Communications port begin with COM, was %s",
         name);
      panic();
   }

/*--------------------------------------------------------------------*/
/*                          Perform the open                          */
/*--------------------------------------------------------------------*/

   hCom = CreateFile( name,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);

/*--------------------------------------------------------------------*/
/*    Check the open worked.  We translation the common obvious       */
/*    error of file in use to english, for all other errors are we    */
/*    report the raw error code.                                      */
/*--------------------------------------------------------------------*/

   if (hCom == INVALID_HANDLE_VALUE) {
       dwError = GetLastError();
       printmsg(0, "openline: OpenFile Error %d on port %s",
          dwError, name);
       return TRUE;
   }

/*--------------------------------------------------------------------*/
/*                    Check for special test mode                     */
/*--------------------------------------------------------------------*/

   if ( equal(name,"CON"))
   {
      portActive = TRUE;     /* record status for error handler        */
      carrierdetect = FALSE;  /* Modem is not connected                 */
      console = TRUE;
      return 0;
   }

   console = FALSE;

/*--------------------------------------------------------------------*/
/*            Reset any errors on the communications port             */
/*--------------------------------------------------------------------*/

   rc = ClearCommError (hCom,
        &Error,
        NULL);

   if (!rc) {
      printmsg(0, "openline: Error in ClearCommError() call\n");
      printmsg(0, "Error returned was %d\n", Error);
   }

/*--------------------------------------------------------------------*/
/*                           Set baud rate                            */
/*--------------------------------------------------------------------*/

   SIOSpeed(baud);

/*--------------------------------------------------------------------*/
/*                        Set line attributes                         */
/*--------------------------------------------------------------------*/

#ifdef UDEBUG
   printmsg(15,"openline: Getting attributes");
#endif

   rc = GetCommState(hCom, &dcb);
   if (!rc) {
      printmsg(0,"openline: Unable to get line attributes for %s",name);

      printmsg(0,"Return code from DosDevIOCtl was %#04x (%d)",

               (int) rc , (int) rc);

      panic();
   }

   dcb.StopBits = ONESTOPBIT;
   dcb.Parity = NOPARITY;
   dcb.ByteSize = 8;

#ifdef UDEBUG
   printmsg(15,"openline: Setting attributes");
#endif

   rc = SetCommState(hCom, &dcb);
   if (!rc)
   {

      printmsg(0,"openline: Unable to set line attributes for %s",name);
      panic();

   }

/*--------------------------------------------------------------------*/
/*                     Disable XON/XOFF flow control                  */
/*                     Enable CTS handling for flow control           */
/*--------------------------------------------------------------------*/

#ifdef UDEBUG
   printmsg(15,"openline: Getting flow control information");
#endif

   GetCommState(hCom, &dcb);
   dcb.fOutX = 0;
   dcb.fInX = 0;
   dcb.fOutxCtsFlow = 1;
   rc = SetCommState(hCom, &dcb);
   if (!rc) {
       printmsg(0,"openline: Unable to set comm attributes for %s",name);

       panic();
   }

/* Get communications timeout information */

   rc = GetCommTimeouts(hCom, &CommTimeout);
   if (!rc) {
      Error = GetLastError();
      printmsg(0, "openline: error on GetCommTimeouts() on %s: error %d",
          name, Error);
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

            "openline: Unable to raise DTR/RTS for %s",
                  name);

      panic();
   }

   traceStart( name );     // Enable logging

   portActive = TRUE;     /* record status for error handler        */
   carrierdetect = FALSE;  /* Modem is not connected                 */

/*--------------------------------------------------------------------*/
/*                     Up our processing priority                     */
/*--------------------------------------------------------------------*/

   hProcess = GetCurrentProcess();
   rc = SetPriorityClass(hProcess, HIGH_PRIORITY_CLASS);

   if (!rc)
   {
      printmsg(0, "openline: unable to set priority for process");
      panic();
   }

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
   DWORD Error;
   BOOL rc;
   static char save[MAXPACK];
   static USHORT bufsize = 0;
   time_t stop_time ;
   time_t now ;

/*--------------------------------------------------------------------*/
/*           Determine if our internal buffer has the data            */
/*--------------------------------------------------------------------*/

   if (bufsize >= wanted)
   {
      memmove( output, save, wanted );
      bufsize -= wanted;
      if ( bufsize )          /* Any data left over?                 */
         memmove( save, &save[wanted], bufsize );  /* Yes --> Save it*/
      return wanted + bufsize;
   } /* if */

/*--------------------------------------------------------------------*/
/*            Reset any errors on the communications port             */
/*--------------------------------------------------------------------*/

   rc = ClearCommError (hCom,
        &Error,
        NULL);

   if (!rc) {
      printmsg(0, "sread:  Unable to read port errors\n");
      printmsg(0, "Error mask was set to %d\n", Error);
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
      DWORD needed = wanted - bufsize;
      DWORD port_timeout;

/*--------------------------------------------------------------------*/
/*                     Handle an aborted program                      */
/*--------------------------------------------------------------------*/

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

      if (stop_time > now )
      {
         port_timeout = (USHORT) (stop_time - now) / needed * 100;
         if (port_timeout < 100)
            port_timeout = 100;
      }
      else
         port_timeout = 0;

      if (!console)
      {
          port_timeout *= 10; /* OS/2 is in hundredths; NT in msec */
          CommTimeout.ReadTotalTimeoutConstant = 0;
          CommTimeout.WriteTotalTimeoutConstant = 0;
          CommTimeout.ReadIntervalTimeout = port_timeout;
          CommTimeout.ReadTotalTimeoutMultiplier = 1;
          CommTimeout.WriteTotalTimeoutMultiplier = 0;
          rc = SetCommTimeouts(hCom, &CommTimeout);

          if ( !rc )
          {
             LPVOID lpMessageBuffer;
             Error = GetLastError();
             FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                NULL, Error, LANG_USER_DEFAULT, &lpMessageBuffer, 0, NULL);

             printmsg(0, "sread: unable to set timeout for comm port");
             printmsg(0, "sread: %s", lpMessageBuffer);
             LocalFree((HLOCAL)lpMessageBuffer);
             panic();
          }
      }

#ifdef UDEBUG
      printmsg(15,"sread: Port time out is %ud seconds/100",
               port_timeout);
#endif

/*--------------------------------------------------------------------*/
/*                 Read the data from the serial port                 */
/*--------------------------------------------------------------------*/

      rc = ReadFile (hCom, &save[bufsize], needed, &received, NULL);

      if (!rc) {
         printmsg(0,
            "sread: Read from comm port for %d bytes failed, received = %d.",
            needed, received);
         bufsize = 0;
         return 0;
      }

#ifdef UDEBUG
      printmsg(15,"sread: Want %d characters, received %d, total %d in buffer",
            (int) wanted, (int) received, (int) bufsize + received);
#endif

/*--------------------------------------------------------------------*/
/*                    Log the newly received data                     */
/*--------------------------------------------------------------------*/

      traceData( &save[bufsize], received, FALSE );

/*--------------------------------------------------------------------*/
/*            If we got the data, return it to the caller             */
/*--------------------------------------------------------------------*/

      bufsize += received;
      if ( bufsize == wanted )
      {
         memmove( output, save, bufsize);
         bufsize = 0;

         if (debuglevel > 14)
            fwrite(output,1,bufsize,stdout);

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

   return bufsize;

} /*nsread*/

/*--------------------------------------------------------------------*/
/*    n s w r i t e                                                   */
/*                                                                    */
/*    Write to the serial port                                        */
/*--------------------------------------------------------------------*/

int nswrite(const char *input, unsigned int len)
{

   char *data = (char *) input;

   DWORD bytes;
   BOOL rc;
   hangupNeeded = TRUE;      /* Flag that the port is now dirty  */

/*--------------------------------------------------------------------*/
/*         Write the data out as the queue becomes available          */
/*--------------------------------------------------------------------*/

   rc = WriteFile (hCom, data, len, &bytes, NULL);

   if (!rc) {
      printmsg(0,"swrite: Write to communications port failed.");

      return bytes;
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
   USHORT rc;
   HANDLE hProcess;

   if ( ! portActive )
      panic();

   portActive = FALSE; /* flag port closed for error handler  */
   hangupNeeded = FALSE;  /* Don't fiddle with port any more  */

/*--------------------------------------------------------------------*/
/*                           Lower priority                           */
/*--------------------------------------------------------------------*/

   hProcess = GetCurrentProcess();
   rc = SetPriorityClass(hProcess, NORMAL_PRIORITY_CLASS);

   if (!rc)
   {
      printmsg(0, "closeline:  Unable to lower priority for task");
      panic();
   }

/*--------------------------------------------------------------------*/
/*                             Lower DTR                              */
/*--------------------------------------------------------------------*/

   if (!EscapeCommFunction(hCom, CLRDTR | CLRRTS))
   {
      printmsg(0,"closeline: Unable to lower DTR/RTS");
   }

/*--------------------------------------------------------------------*/
/*                      Actually close the port                       */
/*--------------------------------------------------------------------*/

   if(!CloseHandle(hCom))
   {
      printmsg(0, "closeline: close of serial port failed, reason %d",
         GetLastError());
   }

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

   if ( console )
      return;

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
   carrierdetect = FALSE;  /* Modem is not connected                 */
   ddelay(500);            /* Really only need 250 milliseconds         */

/*--------------------------------------------------------------------*/
/*                          Bring DTR back up                         */
/*--------------------------------------------------------------------*/

   if (!EscapeCommFunction(hCom, SETDTR))
   {
      printmsg(0, "hangup: Unable to raise DTR for comm port");
      panic();
   }

   ddelay(2000);           /* Now wait for the poor thing to recover    */

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
   printmsg(15,"SIOSpeed: Setting baud rate to %u", (unsigned int) baud);
#endif

   GetCommState (hCom, &dcb);
   dcb.BaudRate = baud;
   rc = SetCommState (hCom, &dcb);
   if (!rc && !console) {
      printmsg(0,"SIOSpeed: Unable to set baud rate for port to %d",baud);
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

   if ( console )
      return;

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

      printmsg(0,"flowcontrol: Unable to set flow control");

      printmsg(0,"Return code from SetCommState() was %d",
               GetLastError());

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
   boolean previous_carrierdetect = carrierdetect;
   USHORT rc;

   DWORD status;
   static DWORD oldstatus = (DWORD) 0xDEADBEEF;

   if ( console )
      return feof( stdin ) == 0;

   rc = GetCommModemStatus(hCom, &status);
   if ( !rc )
   {
      printmsg(0,"CD: Unable to get modem status");
      printmsg(0,"Return code from GetCommModemStatus() was %d",
               GetLastError());
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

   carrierdetect = status && MS_RLSD_ON;

   if (previous_carrierdetect)
      return (status && (MS_RLSD_ON || MS_DSR_ON)) ==
                        (MS_RLSD_ON || MS_DSR_ON);
   else
      return (status && MS_DSR_ON);

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

