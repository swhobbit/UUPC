/*--------------------------------------------------------------------*/
/*    u l i b n m p . c                                               */
/*                                                                    */
/*    OS/2 named pipe support for UUCICO                              */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-2000 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *       $Id: ulibnmp.c 1.28 1999/01/08 02:20:56 ahd Exp $
 *       $Log: ulibnmp.c $
 *       Revision 1.28  1999/01/08 02:20:56  ahd
 *       Convert currentfile() to RCSID()
 *
 *       Revision 1.27  1999/01/04 03:53:57  ahd
 *       Annual copyright change
 *
 *       Revision 1.26  1998/03/01 01:40:49  ahd
 *       Annual Copyright Update
 *
 *       Revision 1.25  1997/04/24 01:35:57  ahd
 *       Annual Copyright Update
 *
 *       Revision 1.24  1996/01/01 21:22:12  ahd
 *       Annual Copyright Update
 *
 *       Revision 1.23  1995/01/07 16:40:45  ahd
 *       Change boolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *       Revision 1.22  1994/12/22 00:37:24  ahd
 *       Annual Copyright Update
 *
 *       Revision 1.21  1994/05/07 21:45:33  ahd
 *       Correct CD() processing to be sticky -- once it fails, it
 *       keeps failing until reset by close or hangup.
 *
 *        Revision 1.20  1994/05/06  03:55:50  ahd
 *        Hot login support
 *
 *        Revision 1.19  1994/04/27  00:02:15  ahd
 *        Pick one: Hot handles support, OS/2 TCP/IP support,
 *                  title bar support
 *
 * Revision 1.18  1994/02/19  05:12:04  ahd
 * Use standard first header
 *
 * Revision 1.17  1994/01/01  19:21:51  ahd
 * Annual Copyright Update
 *
 * Revision 1.16  1993/12/26  16:20:17  ahd
 * Add missing cast to far pointer
 *
 * Revision 1.15  1993/12/24  05:12:54  ahd
 * Use far buffer for master communications buffer
 *
 * Revision 1.14  1993/11/20  14:48:53  ahd
 * Add support for passing port name/port handle/port speed/user id to child
 *
 * Revision 1.14  1993/11/20  14:48:53  ahd
 * Add support for passing port name/port handle/port speed/user id to child
 *
 * Revision 1.13  1993/11/14  20:51:37  ahd
 * Normalize internal speed for network links to 115200 (a large number)
 *
 * Revision 1.12  1993/11/08  04:46:49  ahd
 * Up buffer size for named pipes
 *
 * Revision 1.11  1993/11/06  17:57:09  rhg
 * Drive Drew nuts by submitting cosmetic changes mixed in with bug fixes
 *
 * Revision 1.10  1993/10/24  12:48:56  ahd
 * Additional bug fixes
 *
 * Revision 1.9  1993/10/12  01:33:23  ahd
 * Normalize comments to PL/I style
 *
 * Revision 1.8  1993/10/07  22:51:00  ahd
 * Use dynamically allocated buffer
 *
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

#include "uupcmoah.h"

#include <fcntl.h>
#include <io.h>

/*--------------------------------------------------------------------*/
/*                         OS/2 include files                         */
/*--------------------------------------------------------------------*/

#define INCL_DOSDEVIOCTL
#define INCL_BASE
#include <os2.h>
#include <limits.h>

#ifndef __OS2__
typedef USHORT APIRET ;  /* Define older API return type              */
#endif

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "ulibnmp.h"
#include "ssleep.h"
#include "catcher.h"

#include "commlib.h"
#include "pos2err.h"

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

RCSID("$Id: ulibnmp.c 1.28 1999/01/08 02:20:56 ahd Exp $");

static KWBoolean hangupNeeded = KWFalse;

static BPS currentSpeed = 0;

static KWBoolean passive;

#define FAR_NULL ((PVOID) 0L)

#define PIPE_BUFFER (63*1024)

/*--------------------------------------------------------------------*/
/*           Definitions of control structures for DOS API            */
/*--------------------------------------------------------------------*/

static HPIPE pipeHandle = -1;

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

int ppassiveopenline(char *name, BPS baud, const KWBoolean direct )
{

   APIRET rc;
   static char *pipeName = "\\pipe\\uucp";

   if (portActive)                  /* Was the port already active?   */
      closeline();                  /* Yes --> Shutdown it before open */

#ifdef UDEBUG
   printmsg(15, "ppassiveopenline: %s, %d", name, baud);
#endif

/*--------------------------------------------------------------------*/
/*                          Perform the open                          */
/*--------------------------------------------------------------------*/

   if ( pipeHandle == -1 )
   {
#ifdef __OS2__
      rc =  DosCreateNPipe( (PSZ) pipeName,
                            &pipeHandle,
                            NP_ACCESS_DUPLEX | NP_INHERIT | NP_NOWRITEBEHIND,
                            NP_NOWAIT | 1,
                            PIPE_BUFFER,
                            PIPE_BUFFER,
                            30000 );
#else
      rc =  DosMakeNmPipe(  (PSZ) pipeName,
                            &pipeHandle,
                            NP_ACCESS_DUPLEX | NP_INHERIT | NP_NOWRITEBEHIND,
                            NP_NOWAIT | 1,
                            PIPE_BUFFER,
                            PIPE_BUFFER,
                            30000 );
#endif

/*--------------------------------------------------------------------*/
/*    Check the open worked.  We translation the common obvious       */
/*    error of file in use to english, for all other errors are we    */
/*    report the raw error code.                                      */
/*--------------------------------------------------------------------*/

      if ( rc )
      {
         printOS2error(pipeName, rc );
         return KWTrue;
      }

   } /* if ( pipeHandle == -1 ) */

/*--------------------------------------------------------------------*/
/*                           Set baud rate                            */
/*--------------------------------------------------------------------*/

   SIOSpeed(baud);         /* Just any old large number.              */

   traceStart( name );     /* Enable logging                          */

   portActive = KWTrue;     /* Record status for error handler        */
   passive    = KWTrue;

   return 0;

} /* ppassiveopenline */

/*--------------------------------------------------------------------*/
/*       p W a i t F o r N e t C o n n e c t                          */
/*                                                                    */
/*       Wait for network connection                                  */
/*--------------------------------------------------------------------*/

KWBoolean pWaitForNetConnect(const unsigned int timeout)
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
         hangupNeeded = KWTrue;     /* Flag that the pipe is now dirty */
         return KWTrue;
      }
      else if ( rc == ERROR_PIPE_NOT_CONNECTED )
         ssleep(5);
      else {
         printOS2error("DosConnectNPipe", rc );
         return KWFalse;
      } /* else */

   } while( (stop > time( NULL )) && ! terminate_processing );

   return KWFalse;

}  /* pWaitForNetConnect */

/*--------------------------------------------------------------------*/
/*    p a c t i v e o p e n l i n e                                   */
/*                                                                    */
/*    Open a client pipe connection                                   */
/*--------------------------------------------------------------------*/

#ifdef __TURBOC__
#pragma argsused
#endif

int pactiveopenline(char *name, BPS baud, const KWBoolean direct )
{

   APIRET rc;

#ifdef __OS2__
   ULONG action;
#else
   USHORT action;
#endif

   if (portActive)                  /* Was the port already active?    */
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
      printOS2error(name , rc );
      pipeHandle = -1;
      return KWTrue;
   }

/*--------------------------------------------------------------------*/
/*                           Set baud rate                            */
/*--------------------------------------------------------------------*/

   SIOSpeed(baud);

   traceStart( name );     /* Enable logging                          */

   portActive = KWTrue;     /* Record status for error handler        */
   passive    = KWFalse;

   return 0;

} /* pactiveopenline */

/*--------------------------------------------------------------------*/
/*    p s r e a d                                                     */
/*                                                                    */
/*    Read from the pipe                                              */
/*                                                                    */
/*   Non-blocking read essential to "g" protocol.  See "dcpgpkt.c"    */
/*   for description.                                                 */
/*--------------------------------------------------------------------*/

unsigned int psread(char UUFAR *output,
                    unsigned int wanted,
                    unsigned int timeout)
{
#ifdef __OS2__
   ULONG received;
#else
   USHORT received;
#endif

   APIRET rc;
   time_t stop_time ;
   time_t now ;

   KWBoolean firstPass = KWTrue;

   reads++;

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

/*--------------------------------------------------------------------*/
/*                     Handle an aborted program                      */
/*--------------------------------------------------------------------*/

      if ( terminate_processing )
      {
         static KWBoolean recurse = KWFalse;
         if ( ! recurse )
         {
            printmsg(2,"psread: User aborted processing");
            recurse = KWTrue;
         }
         return 0;
      }

      if ( firstPass )
         firstPass = KWFalse;
      else {
         readSpins++;

         ddelay(readWait);
      } /* else */

/*--------------------------------------------------------------------*/
/*                 Read the data from the named pipe                  */
/*--------------------------------------------------------------------*/

      rc = DosRead( pipeHandle,
                    commBuffer + commBufferUsed,
                    commBufferLength - commBufferUsed,
                    &received );

      if ( rc == ERROR_NO_DATA)
      {
         received = 0;
      }
      else if ( rc )
      {
         printmsg(0,"psread: Read from pipe for %d bytes failed.",
                     (int) (commBufferLength - commBufferUsed));
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

      traceData( commBuffer + commBufferUsed, (unsigned) received, KWFalse );

/*--------------------------------------------------------------------*/
/*            If we got the data, return it to the caller             */
/*--------------------------------------------------------------------*/

      commBufferUsed += received;

      if (commBufferUsed >= wanted)
      {
         MEMCPY( output, commBuffer, wanted );
         commBufferUsed -= wanted;
         if ( commBufferUsed )   /* Any data left over?              */
            MEMMOVE( commBuffer, commBuffer + wanted, commBufferUsed );
                                 /* Yes --> Save it                  */
         return wanted + commBufferUsed;
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

   printmsg(0,"psread: Wanted %d bytes in %d seconds, only have %d bytes",
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

int pswrite(const char UUFAR *input, unsigned int len)
{

   char UUFAR *data = (char UUFAR *) input;
   unsigned int left = len;

#ifdef __OS2__
    ULONG bytes;
#else
   size_t bytes;
#endif

   APIRET rc;

   hangupNeeded = KWTrue;     /* Flag that the pipe is now dirty  */
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

   traceData( data, len, KWTrue);

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

   portActive = KWFalse; /* flag pipe closed for error handler  */
   hangupNeeded = KWFalse;  /* Don't fiddle with pipe any more  */

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

   pipeHandle = -1;

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
      hangupNeeded = KWFalse;
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

void pflowcontrol( KWBoolean flow )
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

KWBoolean pCD( void )
{
   return KWTrue;
} /* pCD */

/*--------------------------------------------------------------------*/
/*       p G e t C o m H a n d l e                                    */
/*                                                                    */
/*       Return current handle number to caller                       */
/*--------------------------------------------------------------------*/

int pGetComHandle( void )
{
   return (int) pipeHandle;
}

/*--------------------------------------------------------------------*/
/*       n S e t C o m H a n d l e                                    */
/*                                                                    */
/*       Set handle number for hot login (start by INETD)             */
/*--------------------------------------------------------------------*/

void pSetComHandle( const int handle )
{
   pipeHandle =  handle;
}
