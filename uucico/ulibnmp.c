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
 *       $Id: ulibnmp.c 1.7 1993/10/03 22:09:09 ahd Exp $
 *       $Log: ulibnmp.c $
 * Revision 1.7  1993/10/03  22:09:09  ahd
 * Use unsigned long to display speed
 *
 * Revision 1.6  1993/10/03  20:37:34  ahd
 * Delete redundant error messages
 *
 * Revision 1.5  1993/09/29  04:49:20  ahd
 * Delete obsolete variable
 *
 * Revision 1.4  1993/09/27  00:45:20  ahd
 * OS/2 16 bit support
 *
 * Revision 1.3  1993/09/25  03:07:56  ahd
 * Various small bug fixes
 *
 * Revision 1.2  1993/09/24  03:43:27  ahd
 * General bug fixes to make work
 *
 * Revision 1.1  1993/09/23  03:26:51  ahd
 * Initial revision
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
#include "ulibnmp.h"
#include "ssleep.h"
#include "catcher.h"

#include "commlib.h"
#include "pos2err.h"

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

currentfile();

static boolean   carrierDetect = FALSE;  /* Modem is not connected     */

static boolean hangupNeeded = FALSE;

static BPS currentSpeed = 0;

static boolean passive;

#define FAR_NULL ((PVOID) 0L)

/*--------------------------------------------------------------------*/
/*           Definitions of control structures for DOS API            */
/*--------------------------------------------------------------------*/

static HPIPE pipeHandle;

static USHORT writeWait = 200;
static USHORT readWait = 50;

static int writes, reads, writeSpins, readSpins;

/*--------------------------------------------------------------------*/
/*    p p a s s i v e o p e n l i n e                                 */
/*                                                                    */
/*    Open a server pipe connection                                   */
/*--------------------------------------------------------------------*/

#ifdef __TURBOC__
#pragma argsused
#endif

int ppassiveopenline(char *name, BPS baud, const boolean direct )
{

   APIRET rc;

   if (portActive)                  /* Was the port already active?    */
      closeline();                  /* Yes --> Shutdown it before open */

#ifdef UDEBUG
   printmsg(15, "ppassiveopenline: %s, %d", name, baud);
#endif

/*--------------------------------------------------------------------*/
/*                          Perform the open                          */
/*--------------------------------------------------------------------*/

#ifdef __OS2__
   rc =  DosCreateNPipe( (PSZ) "\\pipe\\uucp",
                         &pipeHandle,
                         NP_ACCESS_DUPLEX | NP_INHERIT | NP_NOWRITEBEHIND,
                         NP_NOWAIT | 1,
                         32 * 1024 ,
                         32 * 1024 ,
                         30000 );
#else
   rc =  DosMakeNmPipe(  (PSZ) "\\pipe\\uucp",
                         &pipeHandle,
                         NP_ACCESS_DUPLEX | NP_INHERIT | NP_NOWRITEBEHIND,
                         NP_NOWAIT | 1,
                         32 * 1024 ,
                         32 * 1024 ,
                         30000 );
#endif

/*--------------------------------------------------------------------*/
/*    Check the open worked.  We translation the common obvious       */
/*    error of file in use to english, for all other errors are we    */
/*    report the raw error code.                                      */
/*--------------------------------------------------------------------*/

   if ( rc )
   {
      printOS2error("DosCreateNPipe", rc );
      return TRUE;
   }

/*--------------------------------------------------------------------*/
/*                           Set baud rate                            */
/*--------------------------------------------------------------------*/

   SIOSpeed(baud);         // Just any old large number.

   traceStart( name );     // Enable logging

   portActive = TRUE;      /* Record status for error handler        */
   passive    = TRUE;
   carrierDetect = FALSE;  /* Modem is not connected                 */

   return 0;

} /* ppassiveopenline */

/*--------------------------------------------------------------------*/
/*       p W a i t F o r N e t C o n n e c t                          */
/*                                                                    */
/*       Wait for network connection                                  */
/*--------------------------------------------------------------------*/

boolean pWaitForNetConnect(int timeout)
{

   time_t stop;

   stop  = time( NULL ) + timeout;

   do {

#ifdef __OS2__
      APIRET rc = DosConnectNPipe( pipeHandle );
#else
      APIRET rc = DosConnectNmPipe( pipeHandle );
#endif

      if ( rc == 0 )
      {
         hangupNeeded = TRUE;      /* Flag that the pipe is now dirty  */
         return TRUE;
      }
      else if ( rc == ERROR_PIPE_NOT_CONNECTED )
         ssleep(5);
      else {
         printOS2error("DosConnectNPipe", rc );
         return FALSE;
      } /* else */

   } while( (stop > time( NULL )) && ! terminate_processing );

   return FALSE;

}  /* pWaitForNetConnect */

/*--------------------------------------------------------------------*/
/*    p a c t i v e o p e n l i n e                                   */
/*                                                                    */
/*    Open a client pipe connection                                   */
/*--------------------------------------------------------------------*/

#ifdef __TURBOC__
#pragma argsused
#endif

int pactiveopenline(char *name, BPS baud, const boolean direct )
{

   APIRET rc;

#ifdef __OS2__
   ULONG action;
#else
   USHORT action;
#endif

   if (portActive)                  /* Was the port already active?     */
      closeline();                  /* Yes --> Shutdown it before open */

   printmsg(15, "pactiveopenline: %s", name);

/*--------------------------------------------------------------------*/
/*                          Perform the open                          */
/*--------------------------------------------------------------------*/

   rc = DosOpen( name,
                 &pipeHandle,
                 &action,
                 0L,
                 0 ,
                 FILE_OPEN ,
                 OPEN_FLAGS_FAIL_ON_ERROR |
                 OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYREADWRITE, 0L );

/*--------------------------------------------------------------------*/
/*                       Check the open worked.                       */
/*--------------------------------------------------------------------*/

   if ( rc )
   {
      printOS2error("DosOpen", rc );
      return TRUE;
   }

/*--------------------------------------------------------------------*/
/*                           Set baud rate                            */
/*--------------------------------------------------------------------*/

   SIOSpeed(57600);

   traceStart( name );     // Enable logging

   portActive = TRUE;      /* Record status for error handler        */
   passive    = FALSE;
   carrierDetect = FALSE;  /* Modem is not connected                 */

   return 0;

} /* pactiveopenline */

/*--------------------------------------------------------------------*/
/*    p s r e a d                                                     */
/*                                                                    */
/*    Read from the pipe                                              */
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
#ifdef __OS2__
      ULONG received;
#else
      USHORT received;
#endif

   APIRET rc;
   time_t stop_time ;
   time_t now ;

   reads++;

   if ( wanted > commBufferLength )
   {
      printmsg(0,"psread: Overlength read!");
      panic();
   }

/*--------------------------------------------------------------------*/
/*      Perform a buffered read if our lookaside buffer is empty      */
/*--------------------------------------------------------------------*/

   if ( ! commBufferUsed )
   {
      rc = DosRead( pipeHandle,
                    commBuffer,
                    commBufferLength,
                    &received );

      if ( rc == ERROR_NO_DATA)
         commBufferUsed = 0;
      else if ( rc )
      {
         printmsg(0,"psread: Read from pipe for %d bytes failed.",
                     commBufferLength );
         printOS2error("DosRead", rc );
      }
      else {
         commBufferUsed = received;
         traceData( commBuffer, commBufferUsed , FALSE );
      }

   } /* if ( ! commBufferUsed ) */

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
      return wanted;
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
      USHORT needed =  (USHORT) wanted - commBufferUsed;


/*--------------------------------------------------------------------*/
/*                     Handle an aborted program                      */
/*--------------------------------------------------------------------*/

      if ( terminate_processing )
      {
         static boolean recurse = FALSE;
         if ( ! recurse )
         {
            printmsg(2,"psread: User aborted processing");
            recurse = TRUE;
         }
         return 0;
      }

      readSpins++;

      ddelay(readWait);

/*--------------------------------------------------------------------*/
/*                 Read the data from the named pipe                  */
/*--------------------------------------------------------------------*/

      rc = DosRead( pipeHandle,
                    commBuffer + commBufferUsed,
                    needed,
                    &received );

      if ( rc == ERROR_NO_DATA)
      {
         received = 0;
      }
      else if ( rc )
      {
         printmsg(0,"psread: Read from pipe for %d bytes failed.",
                  needed);
         printOS2error("DosRead", rc );
         commBufferUsed = 0;
         return 0;
      }

#ifdef UDEBUG
      printmsg(15,"psread: Want %d characters, received %d, total %d in buffer",
                  (int) wanted,
                  (int) received,
                  (int) commBufferUsed + received);
#endif

/*--------------------------------------------------------------------*/
/*                    Log the newly received data                     */
/*--------------------------------------------------------------------*/

      traceData( commBuffer + commBufferUsed, received, FALSE );

/*--------------------------------------------------------------------*/
/*            If we got the data, return it to the caller             */
/*--------------------------------------------------------------------*/

      commBufferUsed += received;
      if ( commBufferUsed == wanted )
      {
         memmove( output, commBuffer, commBufferUsed);
         commBufferUsed = 0;

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

   printmsg(0,"psread: User wanted %d bytes in %d seconds, we have %d",
              (int) wanted,
              (int) timeout,
              (int) commBufferUsed );
   return commBufferUsed;

} /* psread */

/*--------------------------------------------------------------------*/
/*    p s w r i t e                                                   */
/*                                                                    */
/*    Write to the named pipe                                         */
/*--------------------------------------------------------------------*/

int pswrite(const char *input, unsigned int len)
{

   char *data = (char *) input;
   unsigned int left = len;

#ifdef __OS2__
    ULONG bytes;
#else
   size_t bytes;
#endif

   APIRET rc;

   hangupNeeded = TRUE;      /* Flag that the pipe is now dirty  */
   writes ++;

/*--------------------------------------------------------------------*/
/*         Write the data out as the queue becomes available          */
/*--------------------------------------------------------------------*/

   do {

      rc = DosWrite( pipeHandle, data + len - left, left, &bytes);

      if (rc)
      {
         printOS2error("DosWrite", rc );
         return bytes;
      } /*if */

      left -= bytes;

      ddelay( writeWait );
      writeSpins ++;

   } while( left > 0 );

/*--------------------------------------------------------------------*/
/*                        Log the data written                        */
/*--------------------------------------------------------------------*/

   traceData( data, len, TRUE);

/*--------------------------------------------------------------------*/
/*            Return bytes written to the pipe to the caller          */
/*--------------------------------------------------------------------*/

   return len;

} /* pswrite */

#ifdef __TURBOC__
#pragma argsused
#endif

/*--------------------------------------------------------------------*/
/*    p s s e n d b r k                                               */
/*                                                                    */
/*    send a break signal out the pipe                                */
/*--------------------------------------------------------------------*/

void pssendbrk(unsigned int duration)
{
   printmsg(0,"pssendbrk: BREAK not supported with named pipes");
} /* nssendbrk */

/*--------------------------------------------------------------------*/
/*    p c l o s e l i n e                                             */
/*                                                                    */
/*    Close the named pipe down                                       */
/*--------------------------------------------------------------------*/

void pcloseline(void)
{
   APIRET rc;

   if ( ! portActive )
      panic();

   portActive = FALSE; /* flag pipe closed for error handler  */
   hangupNeeded = FALSE;  /* Don't fiddle with pipe any more  */

   printmsg(4,
         "pcloseline: Read delay %d ms, Write delay %d ms",
         (int) readWait,
         (int) writeWait );

   printmsg(4,
         "pcloseline: %d reads (%d waits), %d writes (%d waits)",
         (int) reads,
         (int) readSpins,
         (int) writes,
         (int) writeSpins - writes );

/*--------------------------------------------------------------------*/
/*                      Actually close the pipe                       */
/*--------------------------------------------------------------------*/

   rc = DosClose( pipeHandle );

   if ( rc )
      printOS2error("DosClose", rc );

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
   if ( passive && hangupNeeded)
   {

#ifdef __OS2__
      DosDisConnectNPipe( pipeHandle );
#else
      DosDisConnectNmPipe( pipeHandle );
#endif
      hangupNeeded = FALSE;
   }

   return;

} /* phangup */

/*--------------------------------------------------------------------*/
/*    p S I O S p e e d                                               */
/*                                                                    */
/*    Re-specify the speed of an opened pipe; no operation, actually. */
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
   return carrierDetect;

} /* pCD */
