/*
   ibmpc/ulibnt.c

   DCP Windows/NT system-dependent library

   Services provided by ulib.c:

   - UNIX commands simulation
   - serial I/O
   - set console mode to what's appropriate for UUPC/extended input

   Updated:

      14May89  - Added hangup() procedure                               ahd
      21Jan90  - Replaced code for rnews() from Wolfgang Tremmel
                 <tremmel@garf.ira.uka.de> to correct failure to
                 properly read compressed news.                         ahd
   6  Sep 90   - Change logging of line data to printable               ahd
      8 Sep 90 - Split ulib.c into dcplib.c and ulib.c                  ahd
      6 Apr 90 - Create libary for OS/2 from ulib.c                     ahd
      Apr 92   - Ported to NT, split off from ulibos2.c                 dmw
      20 Oct 92- added setstdinmode() to set input mode                 dmw
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
/*                         OS/2/Windows NT include files              */
/*--------------------------------------------------------------------*/

#ifdef WIN32
#include <windows.h>
#else
#define INCL_BASE
#include <os2.h>
#endif
/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "ulib.h"
#include "ssleep.h"

#include <limits.h>

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
static DCB dcb;
#else
#endif
/*--------------------------------------------------------------------*/
/*    o p e n l i n e                                                 */
/*                                                                    */
/*    Open the serial port for I/O                                    */
/*--------------------------------------------------------------------*/

int openline(char *name, BPS baud, const boolean direct )
{
   int value;

#ifdef WIN32
   DWORD dwError;
   DWORD Error;
   BOOL rc;
   HANDLE hProcess;
#else
#endif
   if (port_active)              /* Was the port already active?     ahd   */
      closeline();               /* Yes --> Shutdown it before open  ahd   */
   if ( port_active )
      panic();

   printmsg(15, "openline: %s, %d", name, baud);

   if (!equal(name,"CON") && sscanf(name, "COM%d", &value) != 1)
   {
      printmsg(0,"openline: Communications port must be format COMx, was %s",
         name);
      panic();
   }

#ifdef WIN32
   hCom = CreateFile( name,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);

   if (hCom == (HANDLE) 0xFFFFFFFF) {
       dwError = GetLastError();
       panic();
   }


#else
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

/*--------------------------------------------------------------------*/
/*            Reset any errors on the communications port             */
/*--------------------------------------------------------------------*/

#ifdef WIN32
   rc = ClearCommError (hCom,
        &Error,
        NULL);

   if (!rc) {
      printmsg(0, "openline: Error in ClearCommError() call\n");
      printmsg(0, "Error returned was %ld\n", Error);
   }
#else
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
      printmsg(0,"openline: Reset errors for %s, error bits were %#04x",
               name, (int) com_error );
#endif
/*--------------------------------------------------------------------*/
/*                           Set baud rate                            */
/*--------------------------------------------------------------------*/

   SIOSpeed(baud);

/*--------------------------------------------------------------------*/
/*                        Set line attributes                         */
/*--------------------------------------------------------------------*/

    printmsg(15,"openline: Getting attributes");

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

#else
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

   printmsg(15,"openline: Setting attributes");

#ifdef WIN32
   rc = SetCommState(hCom, &dcb);
   if (!rc)
   {
      printmsg(0,"openline: Unable to set line attributes for %s",name);
      panic();
   }
#else
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
/*                        Disable flow control                        */
/*--------------------------------------------------------------------*/

   printmsg(15,"openline: Getting flow control information");
#ifdef WIN32
   GetCommState(hCom, &dcb);
   dcb.fOutX = 0;
   dcb.fInX = 0;
   rc = SetCommState(hCom, &dcb);
   if (!rc) {
       printmsg(0,"openline: Unable to get line attributes for %s",name);
       panic();
   }
#else
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
   com_dcbinfo.fbCtlHndShake = 0;      /* No control/handshake by OS */
   com_dcbinfo.fbFlowReplace = 0;      /* No flow control, either    */
   com_dcbinfo.fbTimeout = MODE_READ_TIMEOUT | MODE_NO_WRITE_TIMEOUT;
#endif
   printmsg(15,"openline: Setting dcb information");
#ifdef WIN32
#else
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

#else
   com_signals.fbModemOn = DTR_ON | RTS_ON;
   com_signals.fbModemOff = 0xff;
#endif


   printmsg(15,"openline: Raising RTS/DTR");

#ifdef WIN32
#else
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
      printmsg(15, "openline: logging serial line data to %s", LINELOG);
      log_stream = fdopen(log_handle, "wb");
   }

   port_active = TRUE;     /* record status for error handler */

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
#else
   rc = DosGetPrty(PRTYS_PROCESS, &usPrevPriority, 0);
   if (rc)
   {
      printmsg(0,"openline: Unable to get priority for task");
      printmsg(0,"Return code from DosGetPrty was %#04x (%d)",
               (int) rc , (int) rc);
      panic();
   } /*if */

   rc = DosSetPrty(PRTYS_PROCESS, PRTYC_TIMECRITICAL, 0, 0);
   if (rc)
   {
      printmsg(0,"openline: Unable to set priority for task");
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
/*--------------------------------------------------------------------*/

unsigned int sread(char *output, unsigned int wanted, unsigned int timeout)
{
#ifdef WIN32
   static LPVOID psave;
   DWORD Error;
#endif
   static char save[BUFSIZ];
   static USHORT bufsize = 0;
   USHORT rc;
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
   printmsg(3, "sread: Calling ClearCommError\n");
   rc = ClearCommError (hCom,
        &Error,
        NULL);

   if (!rc) {
      printmsg(0, "sread:  Unable to read port errors\n");
      printmsg(0, "Error mask was set to %ld\n", Error);
   }

#else

   rc = DosDevIOCtl( &com_error, FAR_NULL, ASYNC_GETCOMMERROR ,
                     IOCTL_ASYNC, com_handle);
   if (rc)
   {
      printmsg(0,"sread: Unable to read port errors");
      printmsg(0,"Return code from DosDevIOCtl was %#04x (%d)",
               (int) rc , (int) rc);
   } /*if */
   else if ( com_error )
      printmsg(0,"sread: Reset port error, error bits were %#04x",
               (int) com_error );
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
      COMMTIMEOUTS CommTimeout;
      DWORD port_timeout;
#else
      USHORT needed =  (USHORT) wanted - bufsize;
      USHORT port_timeout;
      USHORT received = 0;
#endif
/*--------------------------------------------------------------------*/
/*           Compute a new timeout for the read, if needed            */
/*--------------------------------------------------------------------*/


      if (stop_time > now )
      {
         port_timeout = (USHORT) (stop_time - now) / needed * 100;
         if (port_timeout < 100)
            port_timeout = 100;
      }
      else if (! console) 
#ifdef WIN32
      {
          port_timeout = 0xffffffff;
          CommTimeout.ReadTotalTimeoutConstant = 0;
          CommTimeout.WriteTotalTimeoutConstant = 0;
          CommTimeout.ReadIntervalTimeout = port_timeout;
          CommTimeout.ReadTotalTimeoutMultiplier = 1;
          CommTimeout.WriteTotalTimeoutMultiplier = 1;
          SetCommTimeouts(hCom, &CommTimeout);
      }
#else
         port_timeout = 0;
#endif



#ifdef WIN32

       if ( port_timeout != CommTimeout.ReadIntervalTimeout ) {

          CommTimeout.ReadTotalTimeoutConstant = port_timeout;
          CommTimeout.WriteTotalTimeoutConstant = port_timeout;
          CommTimeout.ReadIntervalTimeout = port_timeout;
          CommTimeout.ReadTotalTimeoutMultiplier = 1;
          CommTimeout.WriteTotalTimeoutMultiplier = 1;
          rc = SetCommTimeouts(hCom, &CommTimeout);

          if (!rc) {

            rc = SetCommTimeouts(hCom, &CommTimeout);
             /*
             printmsg(0,"sread: Unable to set timeout for comm port");
             */
             /* panic(); */
          }
        }
#else
      if ( port_timeout != com_dcbinfo.usReadTimeout ){
         com_dcbinfo.usReadTimeout = port_timeout;
         rc = DosDevIOCtl(FAR_NULL, &com_dcbinfo, ASYNC_SETDCBINFO,
                          IOCTL_ASYNC, com_handle);
         if ( rc )
         {
            printmsg(0,"sread: Unable to set timeout for comm port");
            printmsg(0,"Return code from DosDevIOCtl was %#04x (%d)",
                     (int) rc , (int) rc);
            panic();
         } /*if */
         printmsg(15,"sread: new port time out is %d seconds/100",
                  port_timeout);
      } /* if */

#endif
/*--------------------------------------------------------------------*/
/*                 Read the data from the serial port                 */
/*--------------------------------------------------------------------*/

#ifdef WIN32

      rc = ReadFile (hCom, &save[bufsize], needed, &received, NULL);

      if (!rc) {
         printmsg(0,"sread: Read from comm port for %d bytes failed, received = %d.",
            needed,received);
         bufsize = 0;
         return 0;
      }
#else
      rc = DosRead( com_handle, &save[bufsize], needed, &received );

      if ( rc != 0 )
      {
         printmsg(0,"sread: Read from comm port for %d bytes failed.",
                  needed);
         printmsg(0,"Return code from DosRead was %#04x (%d)",
                  (int) rc , (int) rc);
         bufsize = 0;
         return 0;
      }
#endif
      printmsg(15,"sread: Want %d characters, received %d, total %d in buffer",
            (int) wanted, (int) received, (int) bufsize + received);

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
#else
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

   printmsg(12, "ssendbrk: %d", duration);

#ifdef WIN32
   SetCommBreak(hCom);
#else
   DosDevIOCtl( &com_error, FAR_NULL, ASYNC_SETBREAKON, IOCTL_ASYNC,
                com_handle);
#endif
   ddelay( duration == 0 ? 200 : duration);

#ifdef WIN32
   ClearCommBreak(hCom);
#else
   DosDevIOCtl( &com_error, FAR_NULL, ASYNC_SETBREAKOFF, IOCTL_ASYNC,
                com_handle);
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
   HANDLE hProcess;

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
#else
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

#else
   com_signals.fbModemOn  = 0x00;
   com_signals.fbModemOff = DTR_OFF | RTS_OFF;


   if (DosDevIOCtl( &com_error, &com_signals, ASYNC_SETMODEMCTRL,
                    IOCTL_ASYNC, com_handle))
   {
      printmsg(0,"closeline: Unable to lower DTR/RTS for %s",name);
   } /*if */

#endif
/*--------------------------------------------------------------------*/
/*                      Actually close the port                       */
/*--------------------------------------------------------------------*/

#ifdef WIN32
   if(!CloseHandle(hCom))
   {
      printmsg(0, "closeline: close of serial port failed");
   }
#else
   rc = DosClose( com_handle );

   if (rc != 0)
      printmsg(0, "closeline: close of serial port failed, reason %d", (int) rc);
#endif

/*--------------------------------------------------------------------*/
/*                   Stop logging the data to disk                    */
/*--------------------------------------------------------------------*/

   if (log_handle != -1) {    /* close serial line log file */
      fclose(log_stream);
      close(log_handle);
   };

   printmsg(3,"Serial port closed");

} /*closeline*/


/*--------------------------------------------------------------------*/
/*    H a n g u p                                                     */
/*                                                                    */
/*    Hangup the telephone by dropping DTR.  Works with HAYES and     */
/*    many compatibles.                                               */
/*    14 May 89 Drew Derbyshire                                       */
/*--------------------------------------------------------------------*/

void hangup( void )
{
   if (!hangup_needed || console)
      return;
   hangup_needed = FALSE;

/*--------------------------------------------------------------------*/
/*                              Drop DTR                              */
/*--------------------------------------------------------------------*/

#ifdef WIN32
   if (!EscapeCommFunction(hCom, CLRDTR))
   {
      printmsg(0, "hangup: Unable to lower DTR for comm port");
      panic();
   }
#else
   com_signals.fbModemOn  = 0x00;
   com_signals.fbModemOff = DTR_OFF;

   if (DosDevIOCtl( &com_error, &com_signals, ASYNC_SETMODEMCTRL,
                     IOCTL_ASYNC, com_handle))
   {
      printmsg(0,"hangup: Unable to lower DTR for comm port");
      panic();
   } /*if */
#endif
/*--------------------------------------------------------------------*/
/*                  Wait for the telephone to hangup                  */
/*--------------------------------------------------------------------*/

   printmsg(3,"hangup: Dropped DTR");
   ddelay(500);            /* Really only need 250 milliseconds         */


/*--------------------------------------------------------------------*/
/*                          Bring DTR backup                          */
/*--------------------------------------------------------------------*/

#ifdef WIN32
   if (!EscapeCommFunction(hCom, SETDTR))
   {
      printmsg(0, "hangup: Unable to raise DTR for comm port");
      panic();
   }
#else
   com_signals.fbModemOn = DTR_ON;
   com_signals.fbModemOff = 0xff;

   if (DosDevIOCtl( &com_error, &com_signals, ASYNC_SETMODEMCTRL,
                     IOCTL_ASYNC, com_handle))
   {
      printmsg(0,"hangup: Unable to raise DTR for comm port");
      panic();
   } /*if */

#endif
   ddelay(500);            /* Now wait for the poor thing to recover    */

}


/*--------------------------------------------------------------------*/
/* S I O S p e e d                                                    */
/*                                                                    */
/* Re-specify the speed of an opened serial port                      */
/*                                                                    */
/* Dropped the DTR off/on calls because this makes a Hayes drop the   */
/* line if configured properly, and we don't want the modem to drop   */
/* the phone on the floor if we are performing autobaud.              */
/*                                                                    */
/* (Configured properly = standard method of making a Hayes hang up   */
/* the telephone, especially when you can't get it into command state */
/* because it is at the wrong speed or whatever.)                     */
/*--------------------------------------------------------------------*/

void SIOSpeed(BPS baud)
{
   USHORT rc;

   printmsg(15,"SIOSpeed: Setting baud rate to %d", (int) baud);
#ifdef WIN32

   GetCommState (hCom, &dcb);
   dcb.BaudRate = baud;
   rc = SetCommState (hCom, &dcb);
   if (!rc && !console) {
      printmsg(0,"SIOSPeed: Unable to set baud rate for port to %d",baud);
      panic();
   }

#else
   rc = DosDevIOCtl( FAR_NULL, &baud,
                     ASYNC_SETBAUDRATE, IOCTL_ASYNC, com_handle);
   if (rc)
   {
      printmsg(0,"SIOSPeed: Unable to set baud rate for port to %d",
               baud);
      printmsg(0,"Return code from DosDevIOCtl was %#04x (%d)",
               (int) rc , (int) rc);
      panic();
   } /*if */
#endif
   current_baud = baud;

} /*SIOSpeed*/

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

   if (console)
      return;

   GetCommState(hCom, &dcb);

   if (flow)
   {
      dcb.fOutX = TRUE;
      dcb.fInX = TRUE;
      dcb.fRtsControl = RTS_CONTROL_ENABLE;
      dcb.fOutxCtsFlow = FALSE;
      rc = SetCommState(hCom, &dcb);
   } else {
      dcb.fOutX = FALSE;
      dcb.fInX = FALSE;
      dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;
      dcb.fOutxCtsFlow = TRUE;
      rc = SetCommState(hCom, &dcb);
   }

   if ( rc )
   {
      printmsg(0,"flowcontrol: Unable to set flow control");
      printmsg(0,"Return code from DosDevIOCtl was %#04x (%d)",
               (int) rc , (int) rc);
      panic();
   } /*if */
#else
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
      printmsg(0,"flowcontrol: Unable to set flow control for %s",name);
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
	DWORD dwModemStatus;

   boolean online = carrierdetect;
   BYTE status;
   USHORT rc;

   if ( console )
      return feof( stdin ) == 0;

   rc = GetCommModemStatus(hCom, &dwModemStatus);
   if ( rc )
   {
      printmsg(0,"CD: Unable to get modem status");
      printmsg(0,"Return code from GetCommModemStatus() was %#04x (%d)",
               (int) rc , (int) rc);
      panic();
   } /*if */

/*--------------------------------------------------------------------*/
/*    If we previously had carrier detect but have lost it, we        */
/*    report it was lost.  If we do not yet have carrier detect,      */
/*    we return success because we may not have connected yet.        */
/*--------------------------------------------------------------------*/

   carrierdetect = dwModemStatus && MS_RLSD_ON;

   if (online)
      return (status && (MS_RLSD_ON || MS_DSR_ON));
   else
      return (status && MS_DSR_ON);

} /* CD */

void setstdinmode(void)
{
   HANDLE hStdIn = GetStdHandle(STD_INPUT_HANDLE);
   DWORD mode;
   BOOL bSuccess;

   bSuccess = GetConsoleMode(hStdIn, &mode);

/* Disable mouse events so that later Peeks() only get characters */
   mode &= ~ENABLE_WINDOW_INPUT;
   mode &= ~ENABLE_MOUSE_INPUT;
   mode &= ~ENABLE_LINE_INPUT;
   mode |= ENABLE_PROCESSED_INPUT;

   SetConsoleMode(hStdIn, mode);
}