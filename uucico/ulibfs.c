/*--------------------------------------------------------------------*/
/*       u l i b f s . c                                              */
/*                                                                    */
/*       UUPC/extended communications driver for MS-DOS FOSSIL        */
/*       based systems.                                               */
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
 *    $Id: ULIBFS.C 1.2 1993/05/30 15:25:50 ahd Exp $
 *
 *    History:
 *    $Log: ULIBFS.C $
 * Revision 1.2  1993/05/30  15:25:50  ahd
 * Multiple driver support
 *
 * Revision 1.1  1993/05/30  00:01:47  ahd
 * Initial revision
 *
 */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>        // For memset()
#include <stdlib.h>        // For max()
#include <dos.h>           // For FOSSIL interrupt calls
#include <time.h>          // For sleep.h support

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "ulibfs.h"
#include "commlib.h"
#include "fossil.h"
#include "catcher.h"       // For terminate processing flags
#include "ssleep.h"        // For ddelay, etc.

/*--------------------------------------------------------------------*/
/*                        Internal prototypes                         */
/*--------------------------------------------------------------------*/

static void showModem( const short );

static void getDriverInfo( FS_INFO *fossilData);

static short blockIO( char *buffer,
                      const short len,
                      const char function);

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

static BPS currentBPS;
static boolean currentDirect;
static boolean carrierDetect;
static boolean hangupNeeded;

currentfile();

/*--------------------------------------------------------------------*/
/*    f o p e n L i n e                                               */
/*                                                                    */
/*    Open a fossil controlled port                                   */
/*--------------------------------------------------------------------*/

int fopenline(char *name, BPS baud, const boolean direct)
{
   short result;
   FS_INFO fossilData;         // Information returned by FOSSIL

/*--------------------------------------------------------------------*/
/*                       Determine the port number                    */
/*--------------------------------------------------------------------*/

   if (sscanf(name, "COM%d", &portNum) != 1)
   {
      printmsg(0,"Communications port must be format COMx, was %s",
                name);
      panic();
   }

   portNum--;                 // FOSSIL uses offset, not ordinal number

/*--------------------------------------------------------------------*/
/*                   Attempt to initialize the driver                 */
/*--------------------------------------------------------------------*/

   result = FSOpen( );        // Try to open the port
   if ( result != FS_COOKIE )
   {
      printmsg(0,"fopenLine: Open failed, result %d",result );
      return TRUE;            // Report failure
   }

/*--------------------------------------------------------------------*/
/*          Now initialize the rest of the port information           */
/*--------------------------------------------------------------------*/

   ssleep(2);                 // Wait two seconds as required by V.24

   currentDirect = direct;    // Save for flow control processing
   carrierDetect = FALSE;     // No modem connected yet

   flowcontrol( FALSE );      // Set no (or hardware) flow control
   fSIOSpeed( baud );         // Set the port speed
   traceStart( name );        // Enable line tracing

   getDriverInfo( &fossilData );

   printmsg(4,"fopenline: Driver: %Fs (revision %d)",
                    fossilData.id,
              (int) fossilData.revision );
   printmsg(4,"fopenline: FOSSIL version %d, "
              "input buffer %d, output buffer %d",
              (int) fossilData.version,
              (int) fossilData.inputSize,
              (int) fossilData.outputSize );

   portActive = TRUE;         // Flag port is open
   return FALSE;              // Report success to caller

} /* fopenLine */

/*--------------------------------------------------------------------*/
/*       f s r e a d                                                  */
/*                                                                    */
/*       Read a specified number of bytes from the serial port        */
/*--------------------------------------------------------------------*/

unsigned int fsread(char *buffer,
                          unsigned int wanted,
                          unsigned int timeout)
{
   FS_INFO fossilData;         // Information returned by FOSSIL

   time_t quit  = time( NULL ) + timeout;

   showModem( FSStatus());    // Report modem status if changed

/*--------------------------------------------------------------------*/
/*                 Main loop to read data from FOSSIL                 */
/*--------------------------------------------------------------------*/

   for ( ;; )
   {
      unsigned int pending;

      getDriverInfo( &fossilData );
      pending = fossilData.inputSize - fossilData.inputFree;

      if ( pending >= wanted )
      {
         unsigned int moved = blockIO( buffer, wanted, FS_READBLOK );
                                             // Get the data
         traceData( buffer, moved, FALSE );  // Trace the data

         if ( moved < wanted)                // Promised data delivered?
         {                                   // NO --> Panic (literally)
            printmsg(0,"fsread: Read failed at %d of %d bytes"
                       "(%d bytes available)",
                        moved,
                        wanted,
                        pending );
            panic();
         }

         return pending;                            // Return success

      } /* if ( pending >= wanted ) */

/*--------------------------------------------------------------------*/
/*       We don't have the data we need (yet), see if we can wait     */
/*       for it.                                                      */
/*--------------------------------------------------------------------*/

      if (quit <= time(NULL))             // Any time left to wait?
      {
         printmsg(20, "fsread: pending=%d, wanted=%d",
                        pending, wanted);
         return pending;                  // No --> return quietly
      }

      if ( terminate_processing )         // User hit Ctrl-Break?
      {
         static boolean recurse = FALSE;

         if ( ! recurse )
         {
            printmsg(2,"fsread: User aborted processing");
            recurse = TRUE;
         }
         return 0;

      }  /* if ( terminate_processing ) */

      if ( fossilData.inputSize < (int) wanted )   // Sanity check this ...
      {                                      // last for performance reasons.
         printmsg(0,"fsread: FOSSIL queue too small (%d bytes) to"
                    "satisfy read of %d bytes",
                    fossilData.inputSize , wanted );
         panic();

      } /* if */

      ddelay(0);                          // Pause, then do it all again

   } /* for ( ;; ) */

} /* fsread */

/*--------------------------------------------------------------------*/
/*       f s w r i t e                                                */
/*                                                                    */
/*       Write data to a FOSSIL controled serial port                 */
/*--------------------------------------------------------------------*/

int fswrite(const char *data, unsigned int queued)
{

   int moved;
   int total;
   int spin;
   static int const max_spin = 20;
   FS_INFO fossilData;           // Information returned by FOSSIL

   hangupNeeded = TRUE;          // Serial port is now "dirty"

   showModem( FSStatus());       // Report modem status if changed

/*--------------------------------------------------------------------*/
/*                            Perform the I/O                         */
/*--------------------------------------------------------------------*/

  moved = blockIO( (char *) data, queued, FS_WRITBLOK );
                                 // Write the data to port

  traceData( data, moved, TRUE); // Trace our output

  if ( moved == (int) queued )   // Did it all get written out?
      return moved;              // Yes --> Return gracefully

   printmsg(4,"fswrite: Wrote %d bytes of %d", moved, queued);

/*--------------------------------------------------------------------*/
/*       The FOSSIL driver didn't have enough room in its buffer;     */
/*       determine how much we did send.                              */
/*--------------------------------------------------------------------*/

   getDriverInfo( &fossilData );

   total = moved;
   queued -= moved;                 // We only need to still move this

/*--------------------------------------------------------------------*/
/*       Block processing for a limited amount of time if we need     */
/*       more room in the FOSSIL buffer.                              */
/*--------------------------------------------------------------------*/

   spin = max_spin;

   while( spin && queued )
   {
      int wait;
      int needed;

      needed = max(fossilData.outputSize / 2, (int) queued );
                           /* Minimize thrashing by requiring
                              big chunks */

      wait = (int) ((long) needed * 10000L / (long) currentBPS);
                           /* Compute time in milliseconds
                              assuming 10 bits per byte           */

      printmsg(4,"fswrite: Waiting %d ms for %d bytes in queue"
                  ", pass %d",
                  wait, needed, spin);

      ddelay( wait );      /* Actually perform the wait           */

      moved = blockIO( (char *) data + total, queued, FS_WRITBLOK );
                                                // Write the data
      traceData( data + total, moved, TRUE);    // Trace our output

      if ( moved != 0)
         spin--;           /* No progress, consider timing out    */
      else {
         total += moved;
         queued -= moved;
      }                   /* Update our progress                 */

   } /* while(spin && wanted ) */

/*--------------------------------------------------------------------*/
/*             Handle buffer overflow which never went away           */
/*--------------------------------------------------------------------*/

   if ( queued )
   {
      printmsg(0,"fswrite: Buffer overflow, needed %d bytes",
                  queued);
      return 0;
   } /* if ( queued ) */

/*--------------------------------------------------------------------*/
/*                       Return success to caller                     */
/*--------------------------------------------------------------------*/

   return total;

} /* fswrite */

/*--------------------------------------------------------------------*/
/*    f s s e n d b r k                                               */
/*                                                                    */
/*    Send a break to the remote system                               */
/*--------------------------------------------------------------------*/

void fssendbrk(unsigned int duration)
{

   FSBreak( TRUE );
   printmsg(4, "fssendbrk: %d", duration);
   ddelay( duration * 100 );
   FSBreak( FALSE );

} /* fssendbrk */

/*--------------------------------------------------------------------*/
/*    f c l o s e l i n e                                             */
/*                                                                    */
/*    Close a FOSSIL controlled port                                  */
/*--------------------------------------------------------------------*/

void fcloseline(void)
{
   if (!portActive)
      panic();

   portActive = FALSE;        // Flag port closed for error handler

   FSFlushXmit();             // Drop XMIT queue if not empty
   FSFlushRecv();             // Drop Recv queue as well

   FSDTR( FALSE );
   ddelay(500);               // Required for V.24

   FSClose();
   traceStop();

} /* fcloseline */

/*--------------------------------------------------------------------*/
/*    F S S e t S p e e d                                             */
/*                                                                    */
/*    Set current speed port; also sets parity and associated         */
/*    parameters.                                                     */
/*--------------------------------------------------------------------*/

void fSIOSpeed(BPS bps)
{

   static const long rates[] =
         { 19200, 38400, 300, 600, 1200, 2400, 4800, 9600, -19200 };

   short speed = 0;        // Actual value used by FOSSIL for speed

   short best = 3;

   while( (rates[speed] > 0) && (rates[speed] != (long) bps ))
   {
      if ( (rates[speed] / 100) == ((long) bps / 100) )
         best = speed;

      speed++;
   }

   if ( rates[speed] < 0 )
   {
      printmsg(0,"fSIOSpeed: Invalid modem speed %lu, using %lu",
                  (unsigned long) bps,
                  (unsigned long) rates[speed]);
      speed = best;
   }

   printmsg(4,"fSIOSspeed: Changing port speed from %lu BPS to %lu BPS",
               (unsigned long) currentBPS,
               (unsigned long) rates[speed]);

   FSSetSpeed( speed, FS_NO_PARITY, FS_STOPBIT_1, FS_CHARLEN_8);

   showModem( FSStatus());    // Report modem status if changed

   currentBPS = rates[speed];

} /* fSIOSpeed */

/*--------------------------------------------------------------------*/
/*       f f l o w c o n t r o l                                      */
/*                                                                    */
/*       Enable flow control for FOSSIL driven port                   */
/*--------------------------------------------------------------------*/

void fflowcontrol( boolean flow )
{
   printmsg(4,"fflowcontrol: %sabling in-band flow control",
               flow ? "en" : "dis");
   if ( flow )
      FSFlowControl( FS_XONXMIT | FS_XONRECV );
   else if ( currentDirect )    // Direct means no flow control
      FSFlowControl( FS_NOFLOW );
   else
      FSFlowControl( FS_CTSRTS );

} /* fflowcontrol */

/*--------------------------------------------------------------------*/
/*       f h a n g u p                                                */
/*                                                                    */
/*       Perform hangup via DTR drop on FOSSIL controlled port        */
/*--------------------------------------------------------------------*/

void fhangup( void )
{
   if (!hangupNeeded)
      return;

   hangupNeeded = FALSE;

   FSFlushXmit();             // Drop XMIT queue if not empty
   FSFlushRecv();             // Drop Recv queue as well

   FSDTR( FALSE );         /* Hang the phone up                         */
   ddelay(500);            /* Really only need 250 milliseconds         */
   FSDTR( TRUE );          /* Bring the modem back on-line              */
   ddelay(2000);           /* Now wait for the poor thing to recover    */
                           /* two seconds is required by V.24           */

   printmsg(3,"fhangup: complete.");
   carrierDetect = FALSE;  /* No modem connected yet                    */

} /* fhangup */

/*--------------------------------------------------------------------*/
/*    f G e t S p e e d                                               */
/*                                                                    */
/*    Return current port speed                                       */
/*--------------------------------------------------------------------*/

BPS fGetSpeed( void )
{
   return currentBPS;
}

/*--------------------------------------------------------------------*/
/*       f C D                                                        */
/*                                                                    */
/*       Determine if Carrier Detect has dropped                      */
/*--------------------------------------------------------------------*/

boolean fCD( void )
{
   boolean saveCD = carrierDetect;

   short status = FSStatus();

   showModem( status );
   if ( status & FS_STAT_DCD )
      carrierDetect = TRUE;

   if ((! saveCD) || carrierDetect)
      return TRUE;

   printmsg(0,"fCD: Lost carrier detect");
   return FALSE;

} /* fCD */

/*--------------------------------------------------------------------*/
/*       s h o w M o d e m                                            */
/*                                                                    */
/*       Display current modem status                                 */
/*--------------------------------------------------------------------*/

#define mannounce(flag, bits, text ) ((flag & bits) ? text : "" )

static void showModem( const short status )
{
   static int old_status = 0xDEAD;

   if ( debuglevel < 4 )
      return;

   if (status == old_status)
      return;

   printmsg(0, "showModem: %#04x%s%s%s%s%s",
      status,
      mannounce(FS_STAT_DCD,        status, "\tCarrier Detect"),
      mannounce(FS_STAT_OVERRUN,    status, "\tR-Overrun"),
      mannounce(FS_STAT_INQUEUED,   status, "\tR-Pending"),
      mannounce(FS_STAT_OUTPROOM,   status, "\tX-Free"),
      mannounce(FS_STAT_OUTPEMPT,   status, "\tX-Empty")
      );

   old_status = status;

} /* showModem */

/*--------------------------------------------------------------------*/
/*       g e t D r i v e r I n f o                                    */
/*                                                                    */
/*       Return current FOSSIL driver information                     */
/*--------------------------------------------------------------------*/

static void getDriverInfo( FS_INFO *fossilData)
{
   union REGS regs;
   struct SREGS sregs;

   regs.h.ah = FS_DRIVINFO;            // Get driver information
   regs.x.cx = sizeof *fossilData;     // Into buffer this long
   sregs.es  = FP_SEG( fossilData );   // Use segment of buffer
   regs.x.di = FP_OFF( fossilData );   // Use offset of buffer
   regs.x.dx = portNum;                // For this port

   int86x( FS_INTERRUPT, &regs, &regs, &sregs);

   if ( regs.x.ax != sizeof *fossilData )
   {
      printmsg(0,"getDriverInfo: Read of FOSSIL information failed, "
                 " %d bytes returned", regs.x.ax  );
      panic();
   }

} /* getDriverInfo */

/*--------------------------------------------------------------------*/
/*       b l o c k I O                                                */
/*                                                                    */
/*       Perform block I/O between memory and the FOSSIL data         */
/*       queues                                                       */
/*--------------------------------------------------------------------*/

static short blockIO( char *buffer, const short len, const char function)
{
   union REGS regs;
   struct SREGS sregs;

   regs.x.dx = portNum;                // Perform function against port

   regs.h.ah = FS_STATPORT;            // First, set up to get status
   int86( FS_INTERRUPT, &regs, &regs); // ... get the info ...
   showModem ( regs.x.ax );            // ... and report it

   regs.h.ah = function;               // Input or output function
   regs.x.cx = len;                    // Into buffer this long
   sregs.es = FP_SEG( buffer);         // Use segment of the buffer
   regs.x.di = FP_OFF( buffer );       // Use offset of buffer

   int86x( FS_INTERRUPT, &regs, &regs, &sregs);

   if ( len > (short) regs.x.ax )
      printmsg(4,"blockIO: Buffer %d bytes, only moved %d bytes",
                 len,
                 regs.x.ax );
   else if ( len < (short) regs.x.ax )
   {
      printmsg(4,"blockIO: BUFFER (%d bytes) OVERRUN, moved %d bytes",
                 len,
                 regs.x.ax );
      panic();
   }

   return (short) regs.x.ax;           // Return bytes moved

} /* blockIO */
