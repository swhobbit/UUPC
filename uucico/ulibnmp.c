/*--------------------------------------------------------------------*/
/*    u l i b n m p . c                                               */
/*                                                                    */
/*    OS/2 named pipe support for UUCICO                              */
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
 *       $Id: ulibos2.c 1.16 1993/09/21 01:42:13 ahd Exp $
 *       $Log: ulibos2.c $
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
/*                         OS/2 include files                         */
/*--------------------------------------------------------------------*/

#define INCL_DOSDEVIOCTL
#define INCL_BASE
#include <os2.h>
#include <limits.h>

#ifndef __OS2__
typedef USHORT APIRET ;  // Define older API return type
#endif

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

static boolean   carrierDetect = FALSE;  /* Modem is not connected     */

static boolean hangupNeeded = FALSE;

static currentSpeed = 0;

#define FAR_NULL ((PVOID) 0L)

/*--------------------------------------------------------------------*/
/*           Definitions of control structures for DOS API            */
/*--------------------------------------------------------------------*/

static HFILE com_handle;

#ifdef __OS2__
static ULONG usPrevPriority;
#else
static USHORT usPrevPriority;
#endif

/*--------------------------------------------------------------------*/
/*    p o p e n l i n e                                               */
/*                                                                    */
/*    Open the serial port for I/O                                    */
/*--------------------------------------------------------------------*/

int popenline(char *name, BPS baud, const boolean direct )
{

   APIRET rc;
   USHORT com_error;

#ifdef __OS2__
   ULONG ParmLengthInOut;
   ULONG DataLengthInOut;

   PTIB ptib;
   PPIB ppib;

   ULONG action;

#else
   USHORT action;
#endif


   USHORT priority = (E_priority == -99) ?
                           PRTYC_FOREGROUNDSERVER : (USHORT) E_priority;
   USHORT prioritydelta = (E_prioritydelta == -99) ?
                           0 : (USHORT) (E_prioritydelta + PRTYD_MINIMUM);

   if (portActive)                  /* Was the port already active?     */
      closeline();                  /* Yes --> Shutdown it before open */

#ifdef UDEBUG
   printmsg(15, "popenline: %s, %d", name, baud);
#endif

/*--------------------------------------------------------------------*/
/*                          Perform the open                          */
/*--------------------------------------------------------------------*/

   rc = DosOpen( name,
                 &com_handle,
                 &action,
                 0L,
                 0 ,
                 FILE_OPEN ,
                 OPEN_FLAGS_FAIL_ON_ERROR |
                 OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYREADWRITE, 0L );

/*--------------------------------------------------------------------*/
/*    Check the open worked.  We translation the common obvious       */
/*    error of file in use to english, for all other errors are we    */
/*    report the raw error code.                                      */
/*--------------------------------------------------------------------*/

   if ( rc == ERROR_SHARING_VIOLATION)
   {
      printmsg(0,"Port %s already in use", name);
      return TRUE;
   }
   else if ( rc != 0 )
   {
      printmsg(0,"popenline: DosOpen error %d on pipe %s",
                  (int) rc, name );
      return TRUE;
   }

/*--------------------------------------------------------------------*/
/*            Reset any errors on the communications port             */
/*--------------------------------------------------------------------*/

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

/*--------------------------------------------------------------------*/
/*                           Set baud rate                            */
/*--------------------------------------------------------------------*/

   SIOSpeed(57600);

   traceStart( name );     // Enable logging

   portActive = TRUE;      /* Record status for error handler        */
   carrierDetect = FALSE;  /* Modem is not connected                 */

/*--------------------------------------------------------------------*/
/*                     Up our processing priority                     */
/*--------------------------------------------------------------------*/

#ifdef __OS2__
   rc = DosGetInfoBlocks( &ptib, &ppib);
   if ( !rc ) usPrevPriority = (ptib->tib_ptib2)->tib2_ulpri;
#else
   rc = DosGetPrty(PRTYS_PROCESS, &usPrevPriority, 0);
#endif

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

/*--------------------------------------------------------------------*/
/*                     Wait for port to stablize                      */
/*--------------------------------------------------------------------*/

   ddelay(500);            /* Allow port to stablize          */
   return 0;

} /*openline*/

/*--------------------------------------------------------------------*/
/*    p s r e a d                                                     */
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

unsigned int psread(char *output, unsigned int wanted, unsigned int timeout)
{
   APIRET rc;
   static char save[MAXPACK];
   static USHORT bufsize = 0;
   time_t stop_time ;
   time_t now ;
   boolean firstPass = TRUE;

#ifdef __OS2__
   ULONG ParmLengthInOut;
   ULONG DataLengthInOut;
#endif

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
      USHORT needed =  (USHORT) wanted - bufsize;
      ULONG status;

#ifdef __OS2__
      ULONG received = 0;
      AVAILDATA available;
#else
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

      if ( firstPass )
         firstPass = FALSE;
      else
         ddelay(0);

/*--------------------------------------------------------------------*/
/*                   See if we have the needed data                   */
/*--------------------------------------------------------------------*/

      rc = DosPeekNmPipe( com_handle,
                          &save[bufsize],
                          needed,
                          &received,
                          &available,
                          &status );

      if ( status != NP_STATE_CONNECTED )
      {
         carrierDetect = FALSE;
         printmsg(0,"psread: Pipe lost");
         return 0;
      }

/*--------------------------------------------------------------------*/
/*         If not enough data, look through waiting for more          */
/*--------------------------------------------------------------------*/

      if ( received == 0 )
         continue;

/*--------------------------------------------------------------------*/
/*                 Read the data from the serial port                 */
/*--------------------------------------------------------------------*/

      rc = DosRead( com_handle, &save[bufsize], needed, &received );

      if ( rc == ERROR_INTERRUPT)
      {
         printmsg(2,"Read Interrupted");
         return 0;
      }
      else if ( rc != 0 )
      {
         printmsg(0,"psread: Read from comm port for %d bytes failed.",
                  needed);
         printmsg(0,"Return code from DosRead was %#04x (%d)",
                  (int) rc , (int) rc);
         bufsize = 0;
         return 0;
      }

#ifdef UDEBUG
      printmsg(15,"psread: Want %d characters, received %d, total %d in buffer",
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
/*    p s w r i t e                                                   */
/*                                                                    */
/*    Write to the named pipe                                         */
/*--------------------------------------------------------------------*/

int pswrite(const char *input, unsigned int len)
{

   char *data = (char *) input;

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

   rc = DosWrite( com_handle, data , len, &bytes);
   if (rc)
   {
      printmsg(0,"nswrite: Write to communications port failed.");
      printmsg(0,"Return code from DosWrite was %#04x (%d)",
               (int) rc , (int) rc);
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

} /*nswrite*/

/*--------------------------------------------------------------------*/
/*    p s s e n d b r k                                               */
/*                                                                    */
/*    send a break signal out the serial port                         */
/*--------------------------------------------------------------------*/

void pssendbrk(unsigned int duration)
{
   printmsg(0,"pssendbrk: BREAK not supported with named pipes");
} /*nssendbrk*/

/*--------------------------------------------------------------------*/
/*    p c l o s e l i n e                                             */
/*                                                                    */
/*    Close the serial port down                                      */
/*--------------------------------------------------------------------*/

void pcloseline(void)
{
   APIRET rc;
   USHORT com_error;

#ifdef __OS2__
   ULONG ParmLengthInOut;
   ULONG DataLengthInOut;
#endif

   if ( ! portActive )
      panic();

   portActive = FALSE; /* flag port closed for error handler  */
   hangupNeeded = FALSE;  /* Don't fiddle with port any more  */

/*--------------------------------------------------------------------*/
/*                           Lower priority                           */
/*--------------------------------------------------------------------*/

   rc = DosSetPrty(PRTYS_PROCESS,
                   usPrevPriority >> 8 ,
                   usPrevPriority & 0xff, 0);
   if (rc)
   {
      printmsg(0,"closeline: Unable to set priority for task");
      printmsg(0,"Return code from DosSetPrty was %#04x (%d)",
               (int) rc , (int) rc);
   } /*if */

/*--------------------------------------------------------------------*/
/*                      Actually close the port                       */
/*--------------------------------------------------------------------*/

   rc = DosClose( com_handle );

   if ( rc != 0 )
      printmsg( 0,"Close of serial port failed, reason %d", (int) rc);

/*--------------------------------------------------------------------*/
/*                   Stop logging the data to disk                    */
/*--------------------------------------------------------------------*/

   traceStop();

} /* pcloseline */

/*--------------------------------------------------------------------*/
/*    p h a n g u p                                                   */
/*                                                                    */
/*    Hangup the link.  No operation on network.                      */
/*--------------------------------------------------------------------*/

void phangup( void )
{
   return;
} /* phangup */

/*--------------------------------------------------------------------*/
/*    p S I O S p e e d                                               */
/*                                                                    */
/*    Re-specify the speed of an opened serial port; no operation on  */
/*    network.                                                        */
/*--------------------------------------------------------------------*/

#ifdef __TURBOC__
#pragma argsused
#endif

void pSIOSpeed(BPS baud)
{
   currentSpeed = baud;
   return;

} /* pSIOSpeed */

/*--------------------------------------------------------------------*/
/*    p f l o w c o n t r o l                                         */
/*                                                                    */
/*    Enable/Disable in band (XON/XOFF) flow control                  */
/*--------------------------------------------------------------------*/

#ifdef __TURBOC__
#pragma argsused
#endif

void pflowcontrol( boolean flow )
{
   return;
} /* pflowcontrol */

/*--------------------------------------------------------------------*/
/*    n G e t S p e e d                                               */
/*                                                                    */
/*    Report current speed of communications connection               */
/*--------------------------------------------------------------------*/

BPS pGetSpeed( void )
{
   return currentSpeed;
} /* nGetSpeed */

/*--------------------------------------------------------------------*/
/*   p C D                                                            */
/*                                                                    */
/*   Return status of carrier detect                                  */
/*--------------------------------------------------------------------*/

boolean pCD( void )
{
   boolean previousCarrierDetect;

   return carrierDetect;

} /* pCD */
