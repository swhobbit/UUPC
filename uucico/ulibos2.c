/*--------------------------------------------------------------------*/
/*    u l i b o s 2 . c                                               */
/*                                                                    */
/*    OS/2 serial port support for UUCICO                             */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989 by Andrew H. Derbyshire.             */
/*                                                                    */
/*    Changes Copyright (c) 1990-1992 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *       $Id: ULIBOS2.C 1.9 1993/04/05 04:32:19 ahd Exp $
 *       $Log: ULIBOS2.C $
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

#ifdef WIN32

#include <windows.h>
#include <limits.h>
#else

/*--------------------------------------------------------------------*/
/*                         OS/2 include files                         */
/*--------------------------------------------------------------------*/

#define INCL_DOSDEVIOCTL
#define INCL_BASE
#include <os2.h>
#include <limits.h>
#endif

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "ulib.h"
#include "ssleep.h"
#include "catcher.h"

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

currentfile();

boolean   port_active = FALSE;  /* TRUE = port handler handler active  */
static boolean   carrierdetect = FALSE;  /* Modem is not connected     */

#define LINELOG "LineData.Log"      /* log serial line data here */

static int log_handle;
static int logmode = 0;             /* Not yet logging            */
#define WRITING 1
#define READING 2
static FILE *log_stream;
static boolean hangup_needed = FALSE;
static boolean console = FALSE;

static current_baud = 0;

#define FAR_NULL ((PVOID) 0L)

/*--------------------------------------------------------------------*/
/*           Definitions of control structures for DOS API            */
/*--------------------------------------------------------------------*/

#ifdef WIN32

static HANDLE hCom;
static COMMTIMEOUTS CommTimeout;
static DCB dcb;

#else /* OS/2 */

static HFILE com_handle;
static struct _LINECONTROL com_attrib;
static struct _RXQUEUE com_queue;
static struct _MODEMSTATUS com_signals;
static struct _DCBINFO com_dcbinfo;

#endif


static BYTE com_status;
static USHORT com_error;
static USHORT usPrevPriority;

static void ShowError( const USHORT status );

#ifdef WIN32
static void ShowModem( const DWORD status );
#else /* OS/2 */
static void ShowModem( const BYTE status );
#endif

/*--------------------------------------------------------------------*/
/*    o p e n l i n e                                                 */
/*                                                                    */
/*    Open the serial port for I/O                                    */
/*--------------------------------------------------------------------*/

int openline(char *name, BPS baud, const boolean direct )
{
#ifdef WIN32
   DWORD dwError;
   DWORD Error;
   BOOL rc;
   HANDLE hProcess;
#else
   USHORT rc;
   USHORT action;
   USHORT priority = (E_priority == -99) ?
                           PRTYC_FOREGROUNDSERVER : (USHORT) E_priority;
   USHORT prioritydelta = (E_prioritydelta == -99) ?
                           0 : (USHORT) (E_prioritydelta + PRTYD_MINIMUM);
#endif

   if (port_active)              /* Was the port already active?     ahd   */
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

#ifdef WIN32

   hCom = CreateFile( name,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);

#else /* OS/2 */

   rc = DosOpen( name,
                 &com_handle,
                 &action,
                 0L,
                 0 ,
                 FILE_OPEN ,
                 OPEN_FLAGS_FAIL_ON_ERROR |
                 OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYREADWRITE, 0L );
#endif

/*--------------------------------------------------------------------*/
/*    Check the open worked.  We translation the common obvious       */
/*    error of file in use to english, for all other errors are we    */
/*    report the raw error code.                                      */
/*--------------------------------------------------------------------*/

#ifdef WIN32

   if (hCom == INVALID_HANDLE_VALUE) {
       dwError = GetLastError();
       printmsg(0, "openline: OpenFile Error %d on port %s",
          dwError, name);
       return TRUE;
   }

#else

   if ( rc == ERROR_SHARING_VIOLATION)
   {
      printmsg(0,"Port %s already in use", name);
      return TRUE;
   }
   else if ( rc != 0 )
   {
      printmsg(0,"openline: DosOpen error %d on port %s",
                  (int) rc, name );
      return TRUE;
   }
#endif

/*--------------------------------------------------------------------*/
/*                    Check for special test mode                     */
/*--------------------------------------------------------------------*/

   if ( equal(name,"CON"))
   {
      port_active = TRUE;     /* record status for error handler        */
      carrierdetect = FALSE;  /* Modem is not connected                 */
      console = TRUE;
      return 0;
   }

   console = FALSE;

/*--------------------------------------------------------------------*/
/*            Reset any errors on the communications port             */
/*--------------------------------------------------------------------*/

#ifdef WIN32

   rc = ClearCommError (hCom,
        &Error,
        NULL);

   if (!rc) {
      printmsg(0, "openline: Error in ClearCommError() call\n");
      printmsg(0, "Error returned was %d\n", Error);
   }

#else /* OS/2 */

   rc = DosDevIOCtl( &com_error, FAR_NULL, ASYNC_GETCOMMERROR ,
                     IOCTL_ASYNC, com_handle);
   if (rc)
   {
      printmsg(0,
            "openline: Unable to read errors for %s, error bits %x",
               name, (int) com_error );
      printmsg(0,"Return code from DosDevIOCtl was %#04x (%d)",
               (int) rc , (int) rc);
   } /*if */
   else if ( com_error )
      ShowError( com_error );

#endif

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

#ifdef WIN32

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

#else /* OS/2 */

   rc = DosDevIOCtl( &com_attrib, FAR_NULL, ASYNC_GETLINECTRL, IOCTL_ASYNC,
                com_handle);
                              /* Get old attributes from device      */
   if (rc)
   {
      printmsg(0,"openline: Unable to get line attributes for %s",name);
      printmsg(0,"Return code from DosDevIOCtl was %#04x (%d)",
               (int) rc , (int) rc);
      panic();
   } /*if */

   com_attrib.bDataBits = 0x08; /* Use eight bit path for data      */
   com_attrib.bParity   = 0x00; /* No parity                        */
   com_attrib.bStopBits = 0x00; /* 1 Stop Bit                       */

#endif

#ifdef UDEBUG
   printmsg(15,"openline: Setting attributes");
#endif

#ifdef WIN32

   rc = SetCommState(hCom, &dcb);
   if (!rc)

   {

      printmsg(0,"openline: Unable to set line attributes for %s",name);
      panic();

   }

#else /* OS/2 */

   rc = DosDevIOCtl( FAR_NULL, &com_attrib, ASYNC_SETLINECTRL,
                    IOCTL_ASYNC, com_handle);
   if (rc)
   {
      printmsg(0,"openline: Unable to set line attributes for %s",name);
      printmsg(0,"Return code from DosDevIOCtl was %#04x (%d)",
               (int) rc , (int) rc);
      panic();
   } /*if */

#endif

/*--------------------------------------------------------------------*/
/*                     Disable XON/XOFF flow control                  */
/*                     Enable CTS handling for flow control           */
/*--------------------------------------------------------------------*/

#ifdef UDEBUG
   printmsg(15,"openline: Getting flow control information");
#endif

#ifdef WIN32

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

#else /* OS/2 */

   rc = DosDevIOCtl( &com_dcbinfo, FAR_NULL, ASYNC_GETDCBINFO, IOCTL_ASYNC,
                 com_handle);
                              /* Get old attributes from device      */
   if (rc)
   {
      printmsg(0,"openline: Unable to get line attributes for %s",name);
      printmsg(0,"Return code from DosDevIOCtl was %#04x (%d)",
               (int) rc , (int) rc);
      panic();
   } /*if */

   com_dcbinfo.usWriteTimeout = 2999;  /* Write timeout 30 seconds   */
   com_dcbinfo.usReadTimeout = 24;     /* Read timeout .25 seconds   */
   com_dcbinfo.fbCtlHndShake = (BYTE)
                               (direct ? 0 : MODE_CTS_HANDSHAKE);
   com_dcbinfo.fbFlowReplace = 0;
                                       /* Unless rquested            */
   com_dcbinfo.fbTimeout = MODE_READ_TIMEOUT | MODE_NO_WRITE_TIMEOUT;

#ifdef UDEBUG
   printmsg(15,"openline: Setting dcb information");
#endif

   rc = DosDevIOCtl( FAR_NULL, &com_dcbinfo, ASYNC_SETDCBINFO,
                     IOCTL_ASYNC, com_handle);
   if ( rc )
   {
      printmsg(0,"openline: Unable to set flow control for %s",name);
      printmsg(0,"Return code from DosDevIOCtl was %#04x (%d)",
               (int) rc , (int) rc);
      panic();
   } /*if */

#endif

/*--------------------------------------------------------------------*/
/*                     Raise Data Terminal Ready                      */
/*--------------------------------------------------------------------*/

#ifdef WIN32
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

#else /* OS/2 */

   com_signals.fbModemOn = DTR_ON | RTS_ON ;
   com_signals.fbModemOff = 0xff;

#ifdef UDEBUG
   printmsg(15,"openline: Raising RTS/DTR");
#endif

   rc = DosDevIOCtl( &com_error, &com_signals, ASYNC_SETMODEMCTRL,
                     IOCTL_ASYNC, com_handle);
   if (rc)
   {
      printmsg(0,
            "openline: Unable to raise DTR/RTS for %s, error bits %#x",
                  name, (int) com_error );
      printmsg(0,"Return code from DosDevIOCtl was %#04x (%d)",
               (int) rc , (int) rc);
      panic();
   } /*if */

#endif

/*--------------------------------------------------------------------*/
/*        Log serial line data only if log file already exists        */
/*--------------------------------------------------------------------*/

   log_handle = open(LINELOG, O_WRONLY | O_TRUNC | O_BINARY);
   if (log_handle != -1) {

#ifdef UDEBUG
      printmsg(15, "openline: logging serial line data to %s", LINELOG);
#endif

      log_stream = fdopen(log_handle, "wb");
   }

   port_active = TRUE;     /* record status for error handler        */
   carrierdetect = FALSE;  /* Modem is not connected                 */

/*--------------------------------------------------------------------*/
/*                     Up our processing priority                     */
/*--------------------------------------------------------------------*/

#ifdef WIN32

   hProcess = GetCurrentProcess();
   rc = SetPriorityClass(hProcess, HIGH_PRIORITY_CLASS);

   if (!rc)
   {
      printmsg(0, "openline: unable to set priority for process");
      panic();
   }

#else /* OS/2 */

   rc = DosGetPrty(PRTYS_PROCESS, &usPrevPriority, 0);
   if (rc)
   {
      printmsg(0,"openline: Unable to get priority for task");
      printmsg(0,"Return code from DosGetPrty was %#04x (%d)",
               (int) rc , (int) rc);
      panic();
   } /*if */

   rc = DosSetPrty(PRTYS_PROCESS, priority, prioritydelta, 0);

   if (rc)
   {
      printmsg(0,"openline: Unable to set priority %u,%u for task",
                   priority, prioritydelta);

      printmsg(0,"Return code from DosSetPrty was %#04x (%d)",
               (int) rc , (int) rc);
   } /*if */

#endif

/*--------------------------------------------------------------------*/
/*                     Wait for port to stablize                      */
/*--------------------------------------------------------------------*/

   ddelay(500);            /* Allow port to stablize          */
   return 0;

} /*openline*/

/*--------------------------------------------------------------------*/
/*    s r e a d                                                       */
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

unsigned int sread(char *output, unsigned int wanted, unsigned int timeout)
{
#ifdef WIN32
   static LPVOID psave;
   DWORD Error;
   BOOL rc;
#else /* OS/2 */
   USHORT rc;
#endif
   static char save[BUFSIZ];
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

#ifdef WIN32

   rc = ClearCommError (hCom,
        &Error,
        NULL);

   if (!rc) {
      printmsg(0, "sread:  Unable to read port errors\n");
      printmsg(0, "Error mask was set to %d\n", Error);
   }

#else /* OS/2 */

   rc = DosDevIOCtl( &com_error, FAR_NULL, ASYNC_GETCOMMERROR ,
                     IOCTL_ASYNC, com_handle);

   if (rc && ! console )
   {
      printmsg(0,"sread: Unable to read port errors");
      printmsg(0,"Return code from DosDevIOCtl was %#04x (%d)",
               (int) rc , (int) rc);
   } /*if */
   else if ( com_error )
      ShowError( com_error );

#endif

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
#ifdef WIN32
      DWORD received;
      DWORD needed = wanted - bufsize;
      DWORD port_timeout;
#else /* OS/2 */
      USHORT needed =  (USHORT) wanted - bufsize;

      USHORT port_timeout;

      USHORT received = 0;

#endif

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

#ifdef WIN32

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

#else /* OS/2 */

      if (( port_timeout != com_dcbinfo.usReadTimeout ) && ! console )
      {
         com_dcbinfo.usReadTimeout = port_timeout;
         rc = DosDevIOCtl(FAR_NULL, &com_dcbinfo, ASYNC_SETDCBINFO,
                          IOCTL_ASYNC, com_handle);
         if ( rc )
         {
            printmsg(0,"sread: Unable to set timeout for comm port");
            printmsg(0,"Return code from DosDevIOCtl was %#04x (%d)",
                     (int) rc , (int) rc);
            panic();
         } /* if */
      } /* if */

#endif

#ifdef UDEBUG
      printmsg(15,"sread: Port time out is %ud seconds/100",
               port_timeout);
#endif

/*--------------------------------------------------------------------*/
/*                 Read the data from the serial port                 */
/*--------------------------------------------------------------------*/

#ifdef WIN32

      rc = ReadFile (hCom, &save[bufsize], needed, &received, NULL);

      if (!rc) {
         printmsg(0,
            "sread: Read from comm port for %d bytes failed, received = %d.",
            needed, received);
         bufsize = 0;
         return 0;
      }

#else /* OS/2 */

      rc = DosRead( com_handle, &save[bufsize], needed, &received );

      if ( rc == ERROR_INTERRUPT)
      {
         printmsg(2,"Read Interrupted");
         return 0;
      }
      else if ( rc != 0 )
      {
         printmsg(0,"sread: Read from comm port for %d bytes failed.",
                  needed);
         printmsg(0,"Return code from DosRead was %#04x (%d)",
                  (int) rc , (int) rc);
         bufsize = 0;
         return 0;
      }

#endif

#ifdef UDEBUG
      printmsg(15,"sread: Want %d characters, received %d, total %d in buffer",
            (int) wanted, (int) received, (int) bufsize + received);
#endif


/*--------------------------------------------------------------------*/
/*                    Log the newly received data                     */
/*--------------------------------------------------------------------*/

      if (log_handle != -1)
      {
#ifdef VERBOSE
         size_t column;
#endif
         if (logmode != READING)
         {
            fputs("\nRead:  ", log_stream);
            logmode = READING;
         } /* if */
#ifdef VERBOSE
         for (column = 0; column < received; column++) {
            char s[18];
            itoa(0x100 | (unsigned) save[bufsize + column], s, 16);
                                          /* Make it printable hex   */
            fwrite(s, 1, 2, log_stream);  /* Write hex to the log    */
         } /* for */
#else
         fwrite(&save[bufsize], 1, received, log_stream);
#endif
      } /* if */

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

} /*sread*/


/*--------------------------------------------------------------------*/
/*    s w r i t e                                                     */
/*                                                                    */
/*    Write to the serial port                                        */
/*--------------------------------------------------------------------*/

int swrite(char *data, unsigned int len)
{
#ifdef WIN32
   DWORD bytes;
   BOOL rc;
#else
   size_t bytes;

   USHORT rc;

#endif
   hangup_needed = TRUE;      /* Flag that the port is now dirty  */

/*--------------------------------------------------------------------*/
/*         Write the data out as the queue becomes available          */
/*--------------------------------------------------------------------*/

#ifdef WIN32

   rc = WriteFile (hCom, data, len, &bytes, NULL);

   if (!rc) {
      printmsg(0,"swrite: Write to communications port failed.");

      return bytes;
   }

#else /* OS/2 */

   rc = DosWrite( com_handle, data , len, &bytes);
   if (rc)
   {
      printmsg(0,"swrite: Write to communications port failed.");
      printmsg(0,"Return code from DosWrite was %#04x (%d)",
               (int) rc , (int) rc);
      return bytes;
   } /*if */

#endif

/*--------------------------------------------------------------------*/
/*                        Log the data written                        */
/*--------------------------------------------------------------------*/

   if (log_handle != -1) {
#ifdef VERBOSE
      char s[18];
#endif
      if (logmode != WRITING)
      {
         fputs("\nWrite: ", log_stream);
         logmode = WRITING;
      }
#ifdef VERBOSE
      for (bytes = 0; bytes < len; bytes++) {
         itoa(0x100 | (unsigned) *data++, s, 16);
                                        /* Make it printable hex  ahd */
         fputc(s[1], log_stream);       /* Put it in the log    */
         fputc(s[2], log_stream);       /* Put it in the log    */
      }
#else
      fwrite(data, 1, len, log_stream);
#endif
   }

/*--------------------------------------------------------------------*/
/*            Return bytes written to the port to the caller          */
/*--------------------------------------------------------------------*/

   return len;

} /*swrite*/


/*--------------------------------------------------------------------*/
/*    s s e n d b r k                                                 */
/*                                                                    */
/*    send a break signal out the serial port                         */
/*--------------------------------------------------------------------*/

void ssendbrk(unsigned int duration)
{

#ifdef UDEBUG
   printmsg(12, "ssendbrk: %d", duration);
#endif

#ifdef WIN32

   SetCommBreak(hCom);

#else /* OS/2 */

   DosDevIOCtl( &com_error, FAR_NULL, ASYNC_SETBREAKON, IOCTL_ASYNC,
                com_handle);

   if ( com_error )
      ShowError( com_error );

#endif

   ddelay( duration == 0 ? 200 : duration);

#ifdef WIN32

   ClearCommBreak(hCom);

#else /* OS/2 */

   DosDevIOCtl( &com_error, FAR_NULL, ASYNC_SETBREAKOFF, IOCTL_ASYNC,
                com_handle);

   if ( com_error )
      ShowError( com_error );

#endif

} /*ssendbrk*/


/*--------------------------------------------------------------------*/
/*    c l o s e l i n e                                               */
/*                                                                    */
/*    Close the serial port down                                      */
/*--------------------------------------------------------------------*/

void closeline(void)
{
   USHORT rc;
#ifdef WIN32
   HANDLE hProcess;
#endif

   if ( ! port_active )
      panic();

   port_active = FALSE; /* flag port closed for error handler  */
   hangup_needed = FALSE;  /* Don't fiddle with port any more  */

/*--------------------------------------------------------------------*/
/*                           Lower priority                           */
/*--------------------------------------------------------------------*/

#ifdef WIN32

   hProcess = GetCurrentProcess();
   rc = SetPriorityClass(hProcess, NORMAL_PRIORITY_CLASS);

   if (!rc)
   {
      printmsg(0, "closeline:  Unable to lower priority for task");
      panic();
   }

#else /* OS/2 */

   rc = DosSetPrty(PRTYS_PROCESS,
                   usPrevPriority >> 8 ,
                   usPrevPriority & 0xff, 0);
   if (rc)
   {
      printmsg(0,"closeline: Unable to set priority for task");
      printmsg(0,"Return code from DosSetPrty was %#04x (%d)",
               (int) rc , (int) rc);
   } /*if */

#endif

/*--------------------------------------------------------------------*/
/*                             Lower DTR                              */
/*--------------------------------------------------------------------*/

#ifdef WIN32

   if (!EscapeCommFunction(hCom, CLRDTR | CLRRTS))
   {
      printmsg(0,"closeline: Unable to lower DTR/RTS");
   }

#else /* OS/2 */

   com_signals.fbModemOn  = 0x00;
   com_signals.fbModemOff = DTR_OFF | RTS_OFF;


   if (DosDevIOCtl( &com_error, &com_signals, ASYNC_SETMODEMCTRL,
                    IOCTL_ASYNC, com_handle))
   {
      printmsg(0,"closeine: Unable to lower DTR/RTS for port");
   } /*if */
   else if ( com_error )
         ShowError( com_error );

#endif

/*--------------------------------------------------------------------*/
/*                      Actually close the port                       */
/*--------------------------------------------------------------------*/

#ifdef WIN32

   if(!CloseHandle(hCom))
   {
      printmsg(0, "closeline: close of serial port failed, reason %d",
         GetLastError());
   }

#else /* OS/2 */

   rc = DosClose( com_handle );

   if ( rc != 0 )
      printmsg( 0,"Close of serial port failed, reason %d", (int) rc);

#endif

/*--------------------------------------------------------------------*/
/*                   Stop logging the data to disk                    */
/*--------------------------------------------------------------------*/

   if (log_handle != -1) {    /* close serial line log file */
      fclose(log_stream);
      close(log_handle);
   };

   printmsg(3,"Serial port closed");

} /* closeline */


/*--------------------------------------------------------------------*/
/*    H a n g u p                                                     */
/*                                                                    */
/*    Hangup the telephone by dropping DTR.  Works with HAYES and     */
/*    many compatibles.                                               */
/*    14 May 89 Drew Derbyshire                                       */
/*--------------------------------------------------------------------*/

void hangup( void )
{
   if (!hangup_needed)
      return;
   hangup_needed = FALSE;

   if ( console )
      return;

/*--------------------------------------------------------------------*/
/*                              Drop DTR                              */
/*--------------------------------------------------------------------*/

#ifdef WIN32

   if (!EscapeCommFunction(hCom, CLRDTR))
   {
      printmsg(0, "hangup: Unable to lower DTR for comm port");
      panic();
   }

#else /* OS/2 */

   com_signals.fbModemOn  = 0x00;
   com_signals.fbModemOff = DTR_OFF;

   if (DosDevIOCtl( &com_error, &com_signals, ASYNC_SETMODEMCTRL,
                     IOCTL_ASYNC, com_handle))
   {
      printmsg(0,"hangup: Unable to lower DTR for comm port");
      panic();
   } /*if */
   else if ( com_error )
         ShowError( com_error );

#endif

/*--------------------------------------------------------------------*/
/*                  Wait for the telephone to hangup                  */
/*--------------------------------------------------------------------*/

   printmsg(3,"hangup: Dropped DTR");
   carrierdetect = FALSE;  /* Modem is not connected                 */
   ddelay(500);            /* Really only need 250 milliseconds         */

/*--------------------------------------------------------------------*/
/*                          Bring DTR back up                         */
/*--------------------------------------------------------------------*/

#ifdef WIN32

   if (!EscapeCommFunction(hCom, SETDTR))
   {
      printmsg(0, "hangup: Unable to raise DTR for comm port");
      panic();
   }

#else /* OS/2 */

   com_signals.fbModemOn = DTR_ON;
   com_signals.fbModemOff = 0xff;

   if (DosDevIOCtl( &com_error, &com_signals, ASYNC_SETMODEMCTRL,
                     IOCTL_ASYNC, com_handle))
   {
      printmsg(0,"hangup: Unable to raise DTR for comm port");
      panic();
   } /*if */
   else if ( com_error )
         ShowError( com_error );

#endif
   ddelay(2000);           /* Now wait for the poor thing to recover    */

} /* hangup */


/*--------------------------------------------------------------------*/
/*    S I O S p e e d                                                 */
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

void SIOSpeed(BPS baud)
{
   USHORT rc;

#ifdef UDEBUG
   printmsg(15,"SIOSpeed: Setting baud rate to %u", (unsigned int) baud);
#endif

#ifdef WIN32

   GetCommState (hCom, &dcb);
   dcb.BaudRate = baud;
   rc = SetCommState (hCom, &dcb);
   if (!rc && !console) {
      printmsg(0,"SIOSpeed: Unable to set baud rate for port to %d",baud);
      panic();

   }

#else /* OS/2 */

   rc = DosDevIOCtl( FAR_NULL, &baud,
                     ASYNC_SETBAUDRATE, IOCTL_ASYNC, com_handle);

   if (rc && ! console )
   {
      printmsg(0,"SIOSPeed: Unable to set baud rate for port to %d",
               baud);
      printmsg(0,"Return code from DosDevIOCtl was %#04x (%d)",
               (int) rc , (int) rc);
      panic();
   } /*if */

#endif

   current_baud = baud;

} /* SIOSpeed */


/*--------------------------------------------------------------------*/
/*    f l o w c o n t r o l                                           */
/*                                                                    */
/*    Enable/Disable in band (XON/XOFF) flow control                  */
/*--------------------------------------------------------------------*/

void flowcontrol( boolean flow )
{
   USHORT rc;
#ifdef WIN32
   DCB dcb;
#endif

   if ( console )
      return;

#ifdef WIN32

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


#else /* OS/2 */

   if ( flow )
       com_dcbinfo.fbFlowReplace = (char)
            (com_dcbinfo.fbFlowReplace |
            (MODE_AUTO_TRANSMIT | MODE_AUTO_RECEIVE));
   else
      com_dcbinfo.fbFlowReplace = (char)
            (com_dcbinfo.fbFlowReplace &
            (0xff - MODE_AUTO_TRANSMIT - MODE_AUTO_RECEIVE));

   rc = DosDevIOCtl( FAR_NULL, &com_dcbinfo, ASYNC_SETDCBINFO,
                     IOCTL_ASYNC, com_handle);

   if ( rc )
   {
      printmsg(0,"flowcontrol: Unable to set flow control");
      printmsg(0,"Return code from DosDevIOCtl was %#04x (%d)",
               (int) rc , (int) rc);
      panic();
   } /*if */

#endif

} /*flowcontrol*/

/*--------------------------------------------------------------------*/
/*    G e t S p e e d                                                 */
/*                                                                    */
/*    Report current speed of communications connection               */
/*--------------------------------------------------------------------*/

BPS GetSpeed( void )
{
   return current_baud;
} /* GetSpeed */

/*--------------------------------------------------------------------*/
/*   C D                                                              */
/*                                                                    */
/*   Return status of carrier detect                                  */
/*--------------------------------------------------------------------*/

boolean CD( void )
{
   boolean previous_carrierdetect = carrierdetect;
   USHORT rc;

#ifdef WIN32
   DWORD status;
   static DWORD oldstatus = (DWORD) 0xDEADBEEF;
#else /* OS/2 */
   BYTE status;
   static BYTE oldstatus = (BYTE) 0xDEAD;
#endif

   if ( console )
      return feof( stdin ) == 0;

#ifdef WIN32

   rc = GetCommModemStatus(hCom, &status);
   if ( !rc )
   {
      printmsg(0,"CD: Unable to get modem status");
      printmsg(0,"Return code from GetCommModemStatus() was %d",
               GetLastError());
      panic();
   } /*if */

#else /* OS/2 */

   rc = DosDevIOCtl( &status, 0L, ASYNC_GETMODEMINPUT, IOCTL_ASYNC,
         com_handle );

   if ( rc )
   {
      printmsg(0,"CD: Unable to get modem status");
      printmsg(0,"Return code from DosDevIOCtl was %#04x (%d)",
               (int) rc , (int) rc);
      panic();
   } /*if */

#endif

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

#ifdef WIN32

   carrierdetect = status && MS_RLSD_ON;

   if (previous_carrierdetect)
      return (status && (MS_RLSD_ON || MS_DSR_ON)) ==
                        (MS_RLSD_ON || MS_DSR_ON);
   else
      return (status && MS_DSR_ON);

#else /* OS/2 */

   carrierdetect = status && DCD_ON;

   if (previous_carrierdetect)
      return carrierdetect;
   else
      return (status && DSR_ON);

#endif

} /* CD */

/*--------------------------------------------------------------------*/
/*    S h o w M o d e m                                               */
/*                                                                    */
/*    Report current modem status                                     */
/*--------------------------------------------------------------------*/

#define mannounce(flag, bits, text ) ((flag & bits) ? text : "" )

#ifdef WIN32
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

#else /* OS/2 */

static void ShowModem( const BYTE status )
{
   if ( debuglevel < 4 )
      return;

   printmsg(0, "ShowModem: %#02x%s%s%s%s",
      (int) status,
      mannounce(DCD_ON,   status, "  Carrier Detect"),
      mannounce(RI_ON,    status, "  Ring Indicator"),
      mannounce(DSR_ON,   status, "  Data Set Ready"),
      mannounce(CTS_ON,   status, "  Clear to Send"));

} /* ShowModem */

#endif

#ifdef WIN32
#else
/*--------------------------------------------------------------------*/
/*    S h o w E r r o r                                               */
/*                                                                    */
/*    Report modem error bits in English (more or less)               */
/*--------------------------------------------------------------------*/

static void ShowError( const USHORT status )
{
   printmsg(2, "Port Error: %#04x%s%s%s%s",
      (int) status,
      mannounce(RX_QUE_OVERRUN,      status, "  Queue Overrrun"),
      mannounce(RX_HARDWARE_OVERRUN, status, "  Hardware Overrun"),
      mannounce(PARITY_ERROR,        status, "  Parity Error"),
      mannounce(FRAMING_ERROR,       status, "  Framing Error"));

} /* ShowError */
#endif
