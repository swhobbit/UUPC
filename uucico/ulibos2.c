/*--------------------------------------------------------------------*/
/*    u l i b o s 2 . c                                               */
/*                                                                    */
/*    OS/2 serial port support for UUCICO                             */
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
 *       $Id: ulibos2.c 1.41 1994/05/06 03:55:50 ahd Exp $
 *       $Log: ulibos2.c $
 *        Revision 1.41  1994/05/06  03:55:50  ahd
 *        Hot login support
 *
 *        Revision 1.40  1994/04/27  00:02:15  ahd
 *        Pick one: Hot handles support, OS/2 TCP/IP support,
 *                  title bar support
 *
 * Revision 1.39  1994/03/09  04:17:41  ahd
 * Correct query of port speed under 32 bit API
 *
 * Revision 1.38  1994/02/23  04:17:23  ahd
 * Only go into extended character (buffering) mode if reading one
 * character AND over 2400 bps, not either!
 *
 * Revision 1.37  1994/02/19  05:12:53  ahd
 * Use standard first header
 *
 * Revision 1.36  1994/01/01  19:22:11  ahd
 * Annual Copyright Update
 *
 * Revision 1.35  1993/12/26  16:20:17  ahd
 * Don't always perform read ahead on slow links
 *
 * Revision 1.34  1993/12/24  05:12:54  ahd
 * Use far buffer for master communications buffer
 *
 * Revision 1.33  1993/12/06  01:59:07  ahd
 * Use bit AND, not logical AND, to determine modem logic
 *
 * Revision 1.32  1993/11/20  14:48:53  ahd
 * Add support for passing port name/port handle/port speed/user id to child
 *
 * Revision 1.32  1993/11/20  14:48:53  ahd
 * Add support for passing port name/port handle/port speed/user id to child
 *
 * Revision 1.31  1993/11/16  05:37:01  ahd
 * Double buffer I/O to reduce overruns
 *
 * Revision 1.30  1993/11/15  05:43:29  ahd
 * Save/restore port status for dain bramaged programs like TCP/IP
 * Normalize, shorten error messages
 *
 * Revision 1.29  1993/11/08  04:46:49  ahd
 * Drop DTR if program is killed
 *
 * Revision 1.28  1993/11/06  17:57:09  rhg
 * Drive Drew nuts by submitting cosmetic changes mixed in with bug fixes
 *
 * Revision 1.27  1993/11/06  13:04:13  ahd
 * Disable RTS handshaking ... OS/2 rejects it
 *
 * Revision 1.26  1993/11/06  12:19:26  rommel
 * Enable DTR and RTS for modem
 *
 * Revision 1.25  1993/10/12  01:33:23  ahd
 * Normalize comments to PL/I style
 *
 * Revision 1.24  1993/10/07  22:56:45  ahd
 * Use dynamically allocated buffer
 *
 * Revision 1.23  1993/10/03  22:09:09  ahd
 * Use unsigned long to display speed
 *
 * Revision 1.22  1993/10/01  01:17:44  ahd
 * Drop unneeded special case for port in use message
 *
 * Revision 1.21  1993/09/29  05:25:21  ahd
 * Don't die after duplicate port close
 *
 * Revision 1.20  1993/09/29  04:52:03  ahd
 * Use unique handler for port suspending
 *
 * Revision 1.19  1993/09/27  04:04:06  ahd
 * Normalize references to modem speed to avoid incorrect displays
 *
 * Revision 1.18  1993/09/25  03:07:56  ahd
 * Convert to standard OS/2 error message call
 *
 * Revision 1.17  1993/09/24  03:43:27  ahd
 * Use OS/2 error message routine
 *
 * Revision 1.16  1993/09/21  01:42:13  ahd
 * Use standard MAXPACK limit for save buffer size
 *
 * Revision 1.15  1993/09/20  04:46:34  ahd
 * OS/2 2.x support (BC++ 1.0 support)
 * TCP/IP support from Dave Watt
 * 't' protocol support
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

#include "uupcmoah.h"

#include <fcntl.h>
#include <io.h>

/*--------------------------------------------------------------------*/
/*                         OS/2 include files                         */
/*--------------------------------------------------------------------*/

#define INCL_DOSDEVIOCTL
#define INCL_BASE
#define INCL_NOPMAPI

#include <os2.h>
#include <limits.h>

#ifndef __OS2__
typedef USHORT APIRET ;  /* Define older API return type              */
#endif

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "ulib.h"
#include "ssleep.h"
#include "catcher.h"
#include "pos2err.h"

#include "commlib.h"
#include "usrcatch.h"

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

currentfile();

static boolean hangupNeeded = FALSE;

static BPS currentSpeed = 0;
static BPS saveSpeed = 0;

#define FAR_NULL ((PVOID) 0L)

/*--------------------------------------------------------------------*/
/*           Definitions of control structures for DOS API            */
/*--------------------------------------------------------------------*/

static HFILE commHandle = -1;
static LINECONTROL com_attrib;
static MODEMSTATUS com_signals;
static DCBINFO com_dcbinfo;

static LINECONTROL save_com_attrib;
static DCBINFO save_com_dcbinfo;

static void ShowError( const USHORT status );

static void ShowModem( const BYTE status );

/*--------------------------------------------------------------------*/
/*    n o p e n l i n e                                               */
/*                                                                    */
/*    Open the serial port for I/O                                    */
/*--------------------------------------------------------------------*/

int nopenline(char *name, BPS portSpeed, const boolean direct )
{

   APIRET rc;
   USHORT com_error;

#ifdef __OS2__
   ULONG ParmLengthInOut;
   ULONG DataLengthInOut;

   ULONG action;

#else

   USHORT action;

#endif

   if (portActive)               /* Was the port already active?    */
      closeline();               /* Yes --> Shutdown it before open */

#ifdef UDEBUG
   printmsg(15, "nopenline: %s, %lu", name, portSpeed);
#endif

/*--------------------------------------------------------------------*/
/*                      Validate the port format                      */
/*--------------------------------------------------------------------*/

   if (!equaln(name, "COM", 3 ))
   {
      printmsg(0,"nopenline: Communications port begin with COM, was %s",
         name);
      panic();
   }

   if ( commHandle == -1 )
   {

/*--------------------------------------------------------------------*/
/*                          Perform the open                          */
/*--------------------------------------------------------------------*/

      rc = DosOpen( name,
                    &commHandle,
                    &action,
                    0L,
                    0 ,
                    FILE_OPEN ,
                    OPEN_FLAGS_FAIL_ON_ERROR |
                    OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYREADWRITE,
                    0L );

/*--------------------------------------------------------------------*/
/*                       Check the open worked.                       */
/*--------------------------------------------------------------------*/

      if ( rc )
      {
         printOS2error( name, rc );
         commHandle = -1;
         return TRUE;
      }

   } /* if ( commHandle == -1 ) */

/*--------------------------------------------------------------------*/
/*            Reset any errors on the communications port             */
/*--------------------------------------------------------------------*/

#ifdef __OS2__

   ParmLengthInOut = 0;
   DataLengthInOut = sizeof(com_error);
   rc = DosDevIOCtl( commHandle, IOCTL_ASYNC, ASYNC_GETCOMMERROR,
      NULL,0L,&ParmLengthInOut,(PVOID) &com_error,sizeof(com_error),
      &DataLengthInOut);

#else

   rc = DosDevIOCtl( &com_error, FAR_NULL, ASYNC_GETCOMMERROR ,
                     IOCTL_ASYNC, commHandle);

#endif

   if (rc)
   {
      ShowError( com_error );
      printOS2error( "ASYNC_GETCOMMERROR", rc );
   } /*if */
   else if ( com_error )
      ShowError( com_error );

/*--------------------------------------------------------------------*/
/*                           Set port speed                           */
/*--------------------------------------------------------------------*/

   saveSpeed = GetSpeed();    /* Save original speed                 */

   if ( portSpeed )           /* Don't set it we're a hot open       */
      SIOSpeed(portSpeed);

/*--------------------------------------------------------------------*/
/*                        Set line attributes                         */
/*--------------------------------------------------------------------*/

#ifdef UDEBUG
   printmsg(15,"nopenline: Getting attributes");
#endif

#ifdef __OS2__

   ParmLengthInOut = 0;
   DataLengthInOut = sizeof(com_attrib);
   rc = DosDevIOCtl( commHandle,
                     IOCTL_ASYNC,
                     ASYNC_GETLINECTRL,
                     NULL,
                     0L,
                     &ParmLengthInOut,
                     (PVOID) &com_attrib,
                     sizeof(com_attrib),
                     &DataLengthInOut); /* Get old attributes from device  */

#else

   rc = DosDevIOCtl( &com_attrib,
                     FAR_NULL,
                     ASYNC_GETLINECTRL,
                     IOCTL_ASYNC,
                     commHandle);   /* Get old attributes from device  */
#endif

   if (rc)
   {
      printOS2error( "ASYNC_GETLINECTRL", rc );
      panic();
   } /*if */

   memcpy( &save_com_attrib, &com_attrib, sizeof com_attrib );
                              /* Save the attributes                */

   com_attrib.bDataBits = 0x08; /* Use eight bit path for data      */
   com_attrib.bParity   = 0x00; /* No parity                        */
   com_attrib.bStopBits = 0x00; /* 1 Stop Bit                       */

#ifdef UDEBUG
   printmsg(15,"nopenline: Setting attributes");
#endif

#ifdef __OS2__

   ParmLengthInOut = sizeof(com_attrib);
   DataLengthInOut = 0;
   rc = DosDevIOCtl( commHandle,
                     IOCTL_ASYNC,
                     ASYNC_SETLINECTRL,
                     (PVOID) &com_attrib,
                     sizeof(com_attrib),
                     &ParmLengthInOut,
                     NULL,
                     0L,
                     &DataLengthInOut);

#else

   rc = DosDevIOCtl( FAR_NULL,
                     &com_attrib,
                     ASYNC_SETLINECTRL,
                     IOCTL_ASYNC,
                     commHandle);
#endif

   if (rc)
   {
      printOS2error( "ASYNC_SETLINECTRL", rc );
      panic();
   } /*if */

/*--------------------------------------------------------------------*/
/*       Disable software (XON/XOFF) flow control and enable          */
/*       hardware (CTS) for flow control                              */
/*--------------------------------------------------------------------*/

#ifdef UDEBUG
   printmsg(15,"nopenline: Getting flow control information");
#endif

#ifdef __OS2__

   ParmLengthInOut = 0;
   DataLengthInOut = sizeof(com_dcbinfo);
   rc = DosDevIOCtl( commHandle,
                     IOCTL_ASYNC,
                     ASYNC_GETDCBINFO,
                     NULL,
                     0L,
                     &ParmLengthInOut,
                     (PVOID) &com_dcbinfo,
                     sizeof(com_dcbinfo),
                     &DataLengthInOut);   /* Get old attributes from device  */

#else

   rc = DosDevIOCtl( &com_dcbinfo,
                     FAR_NULL,
                     ASYNC_GETDCBINFO,
                     IOCTL_ASYNC,
                     commHandle);    /* Get old attributes from device  */

#endif

   if (rc)
   {
      printOS2error( "ASYNC_GETDCBINFO", rc );
      panic();
   } /*if */

   memcpy( &save_com_dcbinfo, &com_dcbinfo, sizeof com_dcbinfo );
                              /* Save the DCB information            */

   com_dcbinfo.usWriteTimeout = 2999;  /* Write timeout 30 seconds   */
   com_dcbinfo.usReadTimeout = 24;     /* Read timeout .25 seconds   */
   com_dcbinfo.fbCtlHndShake = (BYTE) (MODE_DTR_CONTROL |
                                      (direct ? 0 : MODE_CTS_HANDSHAKE));
                                       /* Always drop DTR at port close,
                                          only use CTS handshaking on
                                          requested ports.            */

   com_dcbinfo.fbFlowReplace = MODE_RTS_HANDSHAKE;
                                       /* Handshake on output         */

   com_dcbinfo.fbTimeout = MODE_READ_TIMEOUT | MODE_NO_WRITE_TIMEOUT;

#ifdef UDEBUG
   printmsg(15,"nopenline: Setting dcb information");
#endif

#ifdef __OS2__

   ParmLengthInOut = sizeof(com_dcbinfo);
   DataLengthInOut = 0;
   rc = DosDevIOCtl( commHandle,
                     IOCTL_ASYNC,
                     ASYNC_SETDCBINFO,
                     (PVOID) &com_dcbinfo,
                     sizeof(com_dcbinfo),
                     &ParmLengthInOut,
                     NULL,
                     0L,
                     &DataLengthInOut);

#else

   rc = DosDevIOCtl( FAR_NULL,
                     &com_dcbinfo,
                     ASYNC_SETDCBINFO,
                     IOCTL_ASYNC,
                     commHandle);

#endif

   if ( rc )
   {
      printOS2error( "ASYNC_SETDCBINFO", rc );
      panic();
   } /*if */

/*--------------------------------------------------------------------*/
/*                     Raise Data Terminal Ready                      */
/*--------------------------------------------------------------------*/

   com_signals.fbModemOn = DTR_ON;
   com_signals.fbModemOff = 0xff;

#ifdef UDEBUG
   printmsg(15,"nopenline: Raising RTS/DTR");
#endif

#ifdef __OS2__

   ParmLengthInOut = sizeof(com_signals);
   DataLengthInOut = sizeof(com_error);

   rc = DosDevIOCtl( commHandle,
                     IOCTL_ASYNC,
                     ASYNC_SETMODEMCTRL,
                     (PVOID)&com_signals,
                     sizeof(com_signals),
                     &ParmLengthInOut,
                     (PVOID) &com_error,
                     sizeof(com_error),
                     &DataLengthInOut);

#else

   rc = DosDevIOCtl( &com_error,
                     &com_signals,
                     ASYNC_SETMODEMCTRL,
                     IOCTL_ASYNC,
                     commHandle);

#endif

   if (rc)
   {
      printOS2error( "ASYNC_SETMODEMCTRL", rc );
      ShowError( com_error );
      panic();
   } /*if */

   traceStart( name );     /* Enable logging                         */

   portActive = TRUE;      /* record status for error handler        */
   carrierDetect = FALSE;  /* Modem is not connected                 */

/*--------------------------------------------------------------------*/
/*                     Wait for port to stablize                      */
/*--------------------------------------------------------------------*/

   ddelay(500);            /* Allow port to stablize          */
   return 0;

} /* nopenline */

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

unsigned int nsread(char UUFAR *output, unsigned int wanted, unsigned int timeout)
{
   APIRET rc;
   time_t stop_time ;
   time_t now ;
   USHORT com_error;

   boolean firstPass = ( currentSpeed > 2400 ) && ( wanted == 1 );
                              /* Perform extended read only on high-
                                 speed modem links when looking for
                                 packet data                         */

#ifdef __OS2__
   ULONG ParmLengthInOut;
   ULONG DataLengthInOut;
#endif

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
      MEMCPY( output, commBuffer, wanted );
      commBufferUsed -= wanted;
      if ( commBufferUsed )   /* Any data left over?                 */
         MEMMOVE( commBuffer, commBuffer + wanted, commBufferUsed );
                              /* Yes --> Save it                     */
      return wanted + commBufferUsed;
   } /* if */

/*--------------------------------------------------------------------*/
/*            Reset any errors on the communications port             */
/*--------------------------------------------------------------------*/

#ifdef __OS2__

   ParmLengthInOut = 0;
   DataLengthInOut = sizeof(com_error);
   rc = DosDevIOCtl( commHandle,
                     IOCTL_ASYNC,
                     ASYNC_GETCOMMERROR,
                     NULL,
                     0L,
                     &ParmLengthInOut,
                     (PVOID) &com_error,
                     sizeof(com_error),
                     &DataLengthInOut);

#else

   rc = DosDevIOCtl( &com_error,
                     FAR_NULL,
                     ASYNC_GETCOMMERROR ,
                     IOCTL_ASYNC,
                     commHandle);

#endif

   if (rc )
   {
      printOS2error( "ASYNC_GETCOMMERROR", rc );
   } /*if */
   else if ( com_error )
      ShowError( com_error );

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
      USHORT needed =  (USHORT) wanted - commBufferUsed;

      USHORT portTimeout;

#ifdef __OS2__
      ULONG received = 0;
#else
      USHORT received = 0;
#endif

/*--------------------------------------------------------------------*/
/*                     Handle an aborted program                      */
/*--------------------------------------------------------------------*/

      if ( raised )
         return 0;

      if ( terminate_processing )
      {
         static boolean recurse = FALSE;
         if ( ! recurse )
         {
            printmsg(2,"nsread: User aborted processing");
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
         portTimeout = (USHORT) (stop_time - now) / needed * 100;
         if (portTimeout < 100)
            portTimeout = 100;
      } /* else */

      if ( portTimeout != com_dcbinfo.usReadTimeout )
      {
         com_dcbinfo.usReadTimeout = portTimeout;

#ifdef __OS2__

         ParmLengthInOut = sizeof(com_dcbinfo);
         DataLengthInOut = 0;
         rc = DosDevIOCtl( commHandle,
                           IOCTL_ASYNC,
                           ASYNC_SETDCBINFO,
                           (PVOID) &com_dcbinfo,
                           sizeof(com_dcbinfo),
                           &ParmLengthInOut,
                           NULL,
                           0L,
                           &DataLengthInOut);

#else

         rc = DosDevIOCtl(FAR_NULL,
                          &com_dcbinfo,
                          ASYNC_SETDCBINFO,
                          IOCTL_ASYNC,
                          commHandle);

#endif
         if ( rc )
         {
            printOS2error( "ASYNC_SETDCBINFO", rc );
            panic();
         } /* if */
      } /* if */

#ifdef UDEBUG
      printmsg(15,"nsread: Port time out is %ud seconds/100",
               portTimeout);
#endif

/*--------------------------------------------------------------------*/
/*                 Read the data from the serial port                 */
/*--------------------------------------------------------------------*/

      rc = DosRead( commHandle,
                    commBuffer + commBufferUsed,
                    portTimeout ? needed : commBufferLength - commBufferUsed,
                    &received );

      if ( rc == ERROR_INTERRUPT)
      {
         printmsg(2,"Read Interrupted");
         return 0;
      }
      else if ( rc != 0 )
      {
         printmsg(0,"nsread: Read from comm port for %d bytes failed.",
                  needed);
         printOS2error( "DosRead", rc );
         commBufferUsed = 0;
         return 0;
      }

#ifdef UDEBUG
      printmsg(15,"nsread: Want %d characters, received %d, total %d in buffer",
                  (int) wanted,
                  (int) received,
                  (int) commBufferUsed + received);
#endif

/*--------------------------------------------------------------------*/
/*                    Log the newly received data                     */
/*--------------------------------------------------------------------*/

      traceData( commBuffer + commBufferUsed, (unsigned) received, FALSE );

/*--------------------------------------------------------------------*/
/*            If we got the data, return it to the caller             */
/*--------------------------------------------------------------------*/

      commBufferUsed += received;
      if ( commBufferUsed >= wanted )
      {
         MEMCPY( output, commBuffer, wanted );
         commBufferUsed -= wanted;
         if ( commBufferUsed )   /* Any data left over?              */
            MEMMOVE( commBuffer, commBuffer + wanted, commBufferUsed );

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

int nswrite(const char UUFAR *input, unsigned int len)
{

   char UUFAR *data = (char UUFAR *) input;

#ifdef __OS2__
    ULONG bytes;
#else
   size_t bytes;
#endif

   APIRET rc;

   hangupNeeded = TRUE;      /* Flag that the port is now dirty  */

/*--------------------------------------------------------------------*/
/*         Write the data out as the queue becomes available          */
/*--------------------------------------------------------------------*/

   rc = DosWrite( commHandle, data , len, &bytes);
   if (rc)
   {
      printOS2error( "DosWrite", rc );
      return bytes;
   } /*if */

/*--------------------------------------------------------------------*/
/*                        Log the data written                        */
/*--------------------------------------------------------------------*/

   traceData( data, len, TRUE);

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

#ifdef __OS2__
   ULONG ParmLengthInOut;
   ULONG DataLengthInOut;
#endif

   USHORT com_error;

#ifdef UDEBUG
   printmsg(12, "ssendbrk: %d", duration);
#endif

#ifdef __OS2__

   ParmLengthInOut = 0;
   DataLengthInOut = sizeof(com_error);
   DosDevIOCtl( commHandle,
                IOCTL_ASYNC,
                ASYNC_SETBREAKON,
                NULL,
                0L,
                &ParmLengthInOut,
                (PVOID) &com_error,
                sizeof(com_error),
                &DataLengthInOut);

#else

   DosDevIOCtl( &com_error,
                FAR_NULL,
                ASYNC_SETBREAKON,
                IOCTL_ASYNC,
                commHandle);

#endif

   if ( com_error )
      ShowError( com_error );

   ddelay( duration == 0 ? 200 : duration);

#ifdef __OS2__
   ParmLengthInOut = 0;
   DataLengthInOut = sizeof(com_error);
   DosDevIOCtl( commHandle,
                IOCTL_ASYNC,
                ASYNC_SETBREAKOFF,
                NULL,
                0L,
                &ParmLengthInOut,
                (PVOID) &com_error,
                sizeof(com_error),
                &DataLengthInOut);

#else
   DosDevIOCtl( &com_error,
                FAR_NULL,
                ASYNC_SETBREAKOFF,
                IOCTL_ASYNC,
                commHandle);
#endif

   if ( com_error )
      ShowError( com_error );

} /*nssendbrk*/

/*--------------------------------------------------------------------*/
/*    n c l o s e l i n e                                             */
/*                                                                    */
/*    Close the serial port down                                      */
/*--------------------------------------------------------------------*/

void ncloseline(void)
{
   APIRET rc;
   USHORT com_error;

#ifdef __OS2__
   ULONG ParmLengthInOut;
   ULONG DataLengthInOut;
#endif

   if ( ! portActive )
   {
      printmsg(0,"ncloseline: Internal error, port already closed");
      return;
   }

   portActive = FALSE; /* flag port closed for error handler  */
   hangupNeeded = FALSE;  /* Don't fiddle with port any more  */

/*--------------------------------------------------------------------*/
/*                             Lower DTR                              */
/*--------------------------------------------------------------------*/

   com_signals.fbModemOn  = 0x00;
   com_signals.fbModemOff = DTR_OFF;

   printmsg(2,"Restoring port attributes and speed %lu",
               (unsigned long) saveSpeed );

#ifdef __OS2__

   ParmLengthInOut = sizeof(com_signals);
   DataLengthInOut = sizeof(com_error);

   rc = DosDevIOCtl( commHandle,
                     IOCTL_ASYNC,
                     ASYNC_SETMODEMCTRL,
                     (PVOID)&com_signals,
                     sizeof(com_signals),
                     &ParmLengthInOut,
                     (PVOID) &com_error,
                     sizeof(com_error),
                     &DataLengthInOut);

#else

   rc = DosDevIOCtl( &com_error,
                     &com_signals,
                     ASYNC_SETMODEMCTRL,
                     IOCTL_ASYNC,
                     commHandle);

#endif

   if ( rc )
      printOS2error( "ASYNC_SETMODEMCTRL", rc );
   else if ( com_error )
      ShowError( com_error );

#ifdef __OS2__

   ParmLengthInOut = sizeof(save_com_attrib);
   DataLengthInOut = 0;
   rc = DosDevIOCtl( commHandle,
                     IOCTL_ASYNC,
                     ASYNC_SETLINECTRL,
                     (PVOID) &save_com_attrib,
                     sizeof(save_com_attrib),
                     &ParmLengthInOut,
                     NULL,
                     0L,
                     &DataLengthInOut);

#else

   rc = DosDevIOCtl( FAR_NULL,
                     &save_com_attrib,
                     ASYNC_SETLINECTRL,
                     IOCTL_ASYNC,
                     commHandle);
#endif

   if (rc)
      printOS2error( "ASYNC_SETLINECTRL", rc );

/*--------------------------------------------------------------------*/
/*               Restore original modem DCB information               */
/*--------------------------------------------------------------------*/

#ifdef __OS2__

   ParmLengthInOut = sizeof(save_com_dcbinfo);
   DataLengthInOut = 0;
   rc = DosDevIOCtl( commHandle,
                     IOCTL_ASYNC,
                     ASYNC_SETDCBINFO,
                     (PVOID) &save_com_dcbinfo,
                     sizeof(save_com_dcbinfo),
                     &ParmLengthInOut,
                     NULL,
                     0L,
                     &DataLengthInOut);

#else

   rc = DosDevIOCtl( FAR_NULL,
                     &save_com_dcbinfo,
                     ASYNC_SETDCBINFO,
                     IOCTL_ASYNC,
                     commHandle);

#endif

   if ( rc )
      printOS2error( "ASYNC_SETDCBINFO", rc );

/*--------------------------------------------------------------------*/
/*                    Restore original port speed                     */
/*--------------------------------------------------------------------*/

   SIOSpeed( saveSpeed );

/*--------------------------------------------------------------------*/
/*                      Actually close the port                       */
/*--------------------------------------------------------------------*/

   rc = DosClose( commHandle );
   commHandle = -1;

   if ( rc != 0 )
      printOS2error( "DosClose", rc );

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
/*--------------------------------------------------------------------*/

void nhangup( void )
{

#ifdef __OS2__
   ULONG ParmLengthInOut;
   ULONG DataLengthInOut;
#endif

   USHORT com_error;
   APIRET rc;

   if (!hangupNeeded)
      return;

   hangupNeeded = FALSE;
   carrierDetect = FALSE;  /* Modem is not connected                 */

/*--------------------------------------------------------------------*/
/*                              Drop DTR                              */
/*--------------------------------------------------------------------*/

   com_signals.fbModemOn  = 0x00;
   com_signals.fbModemOff = DTR_OFF;

#ifdef __OS2__

   ParmLengthInOut = sizeof(com_signals);
   DataLengthInOut = sizeof(com_error);

   rc = DosDevIOCtl( commHandle,
                     IOCTL_ASYNC,
                     ASYNC_SETMODEMCTRL,
                     (PVOID)&com_signals,
                     sizeof(com_signals),
                     &ParmLengthInOut,
                     (PVOID) &com_error,
                     sizeof(com_error),
                     &DataLengthInOut);

#else

   rc = DosDevIOCtl( &com_error,
                     &com_signals,
                     ASYNC_SETMODEMCTRL,
                     IOCTL_ASYNC,
                     commHandle);

#endif

   if ( rc )
   {
      printmsg(0,"hangup: Unable to lower DTR for comm port");
      printOS2error( "ASYNC_SETMODEMCTRL", rc );
   } /*if */
   else if ( com_error )
         ShowError( com_error );

/*--------------------------------------------------------------------*/
/*                  Wait for the telephone to hangup                  */
/*--------------------------------------------------------------------*/

   printmsg(3,"hangup: Dropped DTR");
   ddelay(500);            /* Really only need 250 milliseconds        */

/*--------------------------------------------------------------------*/
/*                          Bring DTR back up                         */
/*--------------------------------------------------------------------*/

   com_signals.fbModemOn = DTR_ON;
   com_signals.fbModemOff = 0xff;

#ifdef __OS2__

   ParmLengthInOut = sizeof(com_signals);
   DataLengthInOut = sizeof(com_error);

   rc = DosDevIOCtl( commHandle,
                     IOCTL_ASYNC, ASYNC_SETMODEMCTRL,
                     (PVOID)&com_signals,
                     sizeof(com_signals),
                     &ParmLengthInOut,
                     (PVOID) &com_error,
                     sizeof(com_error),
                     &DataLengthInOut);

#else

   rc = DosDevIOCtl( &com_error,
                     &com_signals,
                     ASYNC_SETMODEMCTRL,
                     IOCTL_ASYNC,
                     commHandle);

#endif

   if ( rc )
   {
      printmsg(0,"hangup: Unable to raise DTR for comm port");
      printOS2error( "ASYNC_SETMODEMCTRL", rc );
   } /*if */
   else if ( com_error )
      ShowError( com_error );

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

void nSIOSpeed(BPS portSpeed)
{
   APIRET rc;

#ifdef __OS2__

   ULONG ParmLengthInOut;
   ULONG DataLengthInOut;

   struct
   {
      ULONG portSpeed;  /* this structure is needed to set the extended  */
      BYTE fraction;    /* port speed using function 41h DosDevIOCtl  */
   } comPortSpeed;

#else

   USHORT speed = (USHORT) portSpeed;

#endif

#ifdef UDEBUG
   printmsg(15,"SIOSpeed: Setting port speed to %lu",
               (unsigned long) portSpeed);
#endif

#ifdef __OS2__

   comPortSpeed.portSpeed = portSpeed;
   comPortSpeed.fraction = 0;
   ParmLengthInOut = sizeof(comPortSpeed);
   DataLengthInOut = 0;

   rc = DosDevIOCtl( commHandle,
                    IOCTL_ASYNC,
                    0x43,           /* Not defined in header file!   */
                    (PVOID) &comPortSpeed,
                    sizeof(comPortSpeed),
                    &ParmLengthInOut,
                    NULL,
                    0L,
                    &DataLengthInOut);

#else
   rc = DosDevIOCtl( FAR_NULL,
                     &speed,
                     ASYNC_SETBAUDRATE,
                     IOCTL_ASYNC,
                     commHandle);
#endif

   if (rc)
   {
      printmsg(0,"SIOSPeed: Unable to set port speed for port to %lu",
               portSpeed);
      printOS2error( "ASYNC_SETBAUDRATE", rc );
      panic();
   } /*if */

   currentSpeed = portSpeed;

} /* nSIOSpeed */

/*--------------------------------------------------------------------*/
/*    n f l o w c o n t r o l                                         */
/*                                                                    */
/*    Enable/Disable in band (XON/XOFF) flow control                  */
/*--------------------------------------------------------------------*/

void nflowcontrol( boolean flow )
{
   APIRET rc;

#ifdef __OS2__
   ULONG ParmLengthInOut;
   ULONG DataLengthInOut;
#endif

   if ( flow )
       com_dcbinfo.fbFlowReplace = (char)
            (com_dcbinfo.fbFlowReplace |
            (MODE_AUTO_TRANSMIT | MODE_AUTO_RECEIVE));
   else
      com_dcbinfo.fbFlowReplace = (char)
            (com_dcbinfo.fbFlowReplace &
            (0xff - MODE_AUTO_TRANSMIT - MODE_AUTO_RECEIVE));

#ifdef __OS2__

   ParmLengthInOut = sizeof(com_dcbinfo);
   DataLengthInOut = 0;
   rc = DosDevIOCtl( commHandle,
                     IOCTL_ASYNC,
                     ASYNC_SETDCBINFO,
                     (PVOID) &com_dcbinfo,
                     sizeof(com_dcbinfo),
                     &ParmLengthInOut,
                     NULL,
                     0L,
                     &DataLengthInOut);

#else

   rc = DosDevIOCtl( FAR_NULL,
                     &com_dcbinfo,
                     ASYNC_SETDCBINFO,
                     IOCTL_ASYNC,
                     commHandle);

#endif

   if ( rc )
   {
      printOS2error( "ASYNC_SETDCBINFO", rc );
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
   APIRET rc;

#ifdef __OS2__

   ULONG ParmLengthInOut;
   ULONG DataLengthInOut;

/*--------------------------------------------------------------------*/
/*        Structure for returning full port speed information         */
/*--------------------------------------------------------------------*/

   typedef struct _EXT_PORT_SPEED
   {
      ULONG portSpeed;
      BYTE  fraction;
      ULONG minPortSpeed;
      BYTE  minFraction;
      ULONG maxPortspeed;
      ULONG maxFraction;
   } EXT_PORT_SPEED;

   EXT_PORT_SPEED comPortSpeed;

   BPS speed;

#else

   USHORT speed;

#endif

/*--------------------------------------------------------------------*/
/*                      Save original port speed                      */
/*--------------------------------------------------------------------*/

#ifdef __OS2__

/*--------------------------------------------------------------------*/
/*       OS/2 2.x Format of call for DosDevIOCtl accepts portSpeed    */
/*       rates greater than 19200.                                    */
/*--------------------------------------------------------------------*/

   DataLengthInOut = sizeof(comPortSpeed);
   ParmLengthInOut = 0;

   rc = DosDevIOCtl( commHandle,
                    IOCTL_ASYNC,
                    0x63,           /* Not defined in header file!   */
                    NULL,
                    0L,
                    &ParmLengthInOut,
                    (PVOID) &comPortSpeed,
                    sizeof(comPortSpeed),
                    &DataLengthInOut);

   speed = comPortSpeed.portSpeed;

#else
   rc = DosDevIOCtl( &speed,
                     FAR_NULL,
                     ASYNC_GETBAUDRATE,
                     IOCTL_ASYNC,
                     commHandle);

#endif

   if (rc)
   {
      printOS2error( "ASYNC_GETBAUDRATE", rc );
      panic();
   } /*if */

/*--------------------------------------------------------------------*/
/*            Return speed of port to caller upon success             */
/*--------------------------------------------------------------------*/

   return speed;

} /* nGetSpeed */

/*--------------------------------------------------------------------*/
/*   n C D                                                            */
/*                                                                    */
/*   Return status of carrier detect                                  */
/*--------------------------------------------------------------------*/

boolean nCD( void )
{
   boolean newCarrierDetect;
   APIRET rc;

#ifdef __OS2__
   ULONG ParmLengthInOut;
   ULONG DataLengthInOut;
#endif

   BYTE status;
   static BYTE oldstatus = (BYTE) 0xDEAD;

#ifdef __OS2__
   ParmLengthInOut = 0;
   DataLengthInOut = sizeof(status);
   rc = DosDevIOCtl( commHandle,
                     IOCTL_ASYNC,
                     ASYNC_GETMODEMINPUT,
                     NULL,
                     0L,
                     &ParmLengthInOut,
                     (PVOID) &status,
                     sizeof(status),
                     &DataLengthInOut);
#else

   rc = DosDevIOCtl( &status,
                     0L,
                     ASYNC_GETMODEMINPUT,
                     IOCTL_ASYNC,
                     commHandle );

#endif

   if ( rc )
   {
      printOS2error( "ASYNC_GETMODEMINPUT", rc );
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

   newCarrierDetect = (status & DCD_ON) ? TRUE : FALSE;

   if ( newCarrierDetect )
      carrierDetect = newCarrierDetect;

   if (carrierDetect)
      return newCarrierDetect;
   else
      return (status & DSR_ON) ? TRUE : FALSE;

} /* nCD */

/*--------------------------------------------------------------------*/
/*    S h o w M o d e m                                               */
/*                                                                    */
/*    Report current modem status                                     */
/*--------------------------------------------------------------------*/

#define mannounce(flag, bits, text ) ((flag & bits) ? text : "" )

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

/*--------------------------------------------------------------------*/
/*          n G e t C o m H a n d l e                                 */
/*                                                                    */
/*          Return handle to open port                                */
/*--------------------------------------------------------------------*/

int nGetComHandle( void )
{

   return (int) commHandle;

}  /* nGetComHandle */

/*--------------------------------------------------------------------*/
/*       n S e t C o m m H a n d l e                                  */
/*                                                                    */
/*       Set current port handle                                      */
/*--------------------------------------------------------------------*/

void nSetComHandle( const int handle )
{
   commHandle = (HFILE) handle;
}
