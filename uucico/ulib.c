/*--------------------------------------------------------------------*/
/*    u l i b . c                                                     */
/*                                                                    */
/*    Serial port interface to COMMFIFO.ASM for MS-DOS                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-2002 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: ulib.c 1.39 2001/03/12 13:54:49 ahd v1-13k $
 *
 *    $Log: ulib.c $
 *    Revision 1.39  2001/03/12 13:54:49  ahd
 *    Annual copyright update
 *
 *    Revision 1.38  2000/05/12 12:32:00  ahd
 *    Annual copyright update
 *
 *    Revision 1.37  1999/01/08 02:20:56  ahd
 *    Convert currentfile() to RCSID()
 *
 *    Revision 1.36  1999/01/04 03:53:30  ahd
 *    Annual copyright change
 *
 *    Revision 1.35  1998/03/01 01:40:31  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.34  1997/04/24 01:35:21  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.33  1996/01/01 21:19:28  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.32  1995/02/21 03:30:52  ahd
 *    More compiler warning cleanup, drop selected messages at compile
 *    time if not debugging.
 *
 *    Revision 1.31  1995/01/07 16:40:09  ahd
 *    Change boolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.30  1994/12/22 00:36:25  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.29  1994/05/23 22:46:32  ahd
 *    Print specific message when aborting because port is already closed
 *
 *        Revision 1.28  1994/05/07  21:45:33  ahd
 *        Correct CD() processing to be sticky -- once it fails, it
 *        keeps failing until reset by close or hangup.
 *
 *        Revision 1.27  1994/03/28  02:18:45  ahd
 *        Correct error message to report more possible reasons for
 *        serial port failing to install
 *
 * Revision 1.26  1994/02/19  05:10:55  ahd
 * Use standard first header
 *
 * Revision 1.25  1994/01/18  13:29:22  ahd
 * Delete comment
 *
 * Revision 1.24  1994/01/06  12:45:33  ahd
 * Oooops.  Use UUFAR pointer in nswrite()
 *
 * Revision 1.23  1994/01/01  19:21:17  ahd
 * Annual Copyright Update
 *
 * Revision 1.22  1993/12/30  03:11:05  ahd
 * Correct pointer to UUFAR
 *
 * Revision 1.21  1993/12/24  05:12:54  ahd
 * Use far buffer in 16 bit compilers
 *
 * Revision 1.20  1993/11/06  17:56:09  rhg
 * Drive Drew nuts by submitting cosmetic changes mixed in with bug fixes
 *
 * Revision 1.20  1993/11/06  17:56:09  rhg
 * Drive Drew nuts by submitting cosmetic changes mixed in with bug fixes
 *
 * Revision 1.19  1993/10/12  01:33:59  ahd
 * Normalize comments to PL/I style
 *
 * Revision 1.18  1993/10/03  22:09:09  ahd
 * Use unsigned long to display speed
 *
 * Revision 1.17  1993/09/29  13:18:06  ahd
 * Use new dummy setprty function
 *
 * Revision 1.16  1993/09/27  04:04:06  ahd
 * Normalize references to modem speed to avoid incorrect displays
 *
 * Revision 1.15  1993/09/27  00:45:20  ahd
 * Cosmetic clean up
 *
 * Revision 1.14  1993/09/25  03:07:56  ahd
 * Add dummy priority function
 *
 * Revision 1.13  1993/07/11  14:38:32  ahd
 * Correct routine names in displayed messages
 *
 * Revision 1.12  1993/05/30  15:25:50  ahd
 * Multiple driver support
 *
 * Revision 1.11  1993/05/30  00:08:03  ahd
 * Multiple communications drivers support
 * Don't lock port if not in multi-task mode
 * Break trace functions out of ulib.c into commlib.c
 *
 * Revision 1.10  1993/05/09  03:41:47  ahd
 * Make swrite accept constant input strings
 *
 * Revision 1.9  1993/04/11  00:33:54  ahd
 * Global edits for year, TEXT, etc.
 *
 * Revision 1.8  1993/04/05  04:35:40  ahd
 * Set/clear abort processing flags in modem.c
 *
 * Revision 1.7  1993/01/23  19:08:09  ahd
 * Don't attempt to detect lost carrier in sread()
 *
 * Revision 1.6  1992/12/30  05:27:11  plummer
 * MS C compile fixes
 * Add CD() to sread
 *
 * Revision 1.5  1992/12/12  16:12:13  ahd
 * Include header file for definition for memory avail routines
 *
 * Revision 1.4  1992/12/07  02:43:20  ahd
 * Improve error message when low memory prevents COMM port install
 *
 * Revision 1.3  1992/11/29  22:09:10  ahd
 * Use sopen() rather than FOPEN() to avoid retries on comm port
 *
 * Revision 1.2  1992/11/21  06:17:42  ahd
 * Delete old (pre-COMMFIFO) autobaud function
 *
 */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"

#include <fcntl.h>
#include <io.h>
#include <share.h>

#ifdef __TURBOC__
#include <alloc.h>
#endif

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "ulib.h"
#include "comm.h"
#include "ssleep.h"
#include "catcher.h"
#include "commlib.h"

/*--------------------------------------------------------------------*/
/*                        Internal prototypes                         */
/*--------------------------------------------------------------------*/

static void ShowModem( void );

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

static BPS current_bps;
static char current_direct;

RCSID("$Id: ulib.c 1.39 2001/03/12 13:54:49 ahd v1-13k $");

/* IBM-PC I/O routines */

/* "DCP" a uucp clone. Copyright Richard H. Lamb 1985,1986,1987 */

/*************** BASIC I/O ***************************/
/* Saltzers serial package (aka Info-IBMPC COM_PKG2):
 * Some notes:  When packets are flying in both directions, there seems to
 * be some interrupt handling problems as far as receiving.  Checksum errors
 * may therefore occur often even though we recover from them.  This is
 * especially true with sliding windows.  Errors are very few in the VMS
 * version.  RH Lamb
 */

#define  STOPBIT  1

static int com_handle;
static KWBoolean hangup_needed = KWTrue;

/*--------------------------------------------------------------------*/
/*    n o p e n l i n e                                               */
/*                                                                    */
/*    Open the serial port for I/O                                    */
/*--------------------------------------------------------------------*/

int nopenline(char *name, BPS bps, const KWBoolean direct)
{
   int   value;

   if (portActive)              /* Was the port already active?     */
      closeline();              /* Yes --> Shutdown it before open  */

   printmsg(15, "openline: %s, %lu", name, bps);

   current_direct = (char) (direct ? 'D' : 'M') ;

   if (sscanf(name, "COM%d", &value) != 1)
   {
      printmsg(0,"Communications port must be format COMx, was %s",
                name);
      panic();
   }

   if ( bflag[F_MULTITASK] )
   {
      com_handle = sopen( name, O_BINARY | O_RDWR, SH_DENYRW );
                                 /* Used soly for lock abilities  */
      if ( com_handle == -1 )
      {
         printerr( name );
         return 1;
      }
   }

   select_port(value);
   save_com();

   if (!install_com())
   {
      printmsg(0,"Communications handler install failed; "
                  "probable cause ... memory shortage or invalid serial port configuration.");

#ifdef __TURBOC__
      printmsg(0,"FAR DOS Memory free = %ld bytes",
                  farcoreleft() );
#endif

      return 1;
   }

   current_bps = bps;
   open_com((unsigned) current_bps, current_direct, 'N', STOPBIT, 'D');
   dtr_on();
   ssleep(2);                 /* Wait two seconds as required by V.24  */

   traceStart( name );

   portActive = KWTrue;    /* record status for error handler */

   return 0;

} /* nopenline */

/*--------------------------------------------------------------------*/
/*    n s r e a d                                                     */
/*                                                                    */
/*    Read from the serial port                                       */
/*                                                                    */
/*    Non-blocking read essential to "g" protocol.  See               */
/*    "dcpgpkt.c" for description.  This all changes in a             */
/*    multi-tasking system.  Requests for I/O should get queued       */
/*    and an event flag given.  Then the requesting process (e.g.     */
/*    gmachine()) waits for the event flag to fire processing         */
/*    either a read or a write.  Could be implemented on VAX/VMS      */
/*    or DG but not MS-DOS.                                           */
/*--------------------------------------------------------------------*/

unsigned int nsread(char UUFAR *input,
                    unsigned int wanted,
                    unsigned int timeout)
{
   time_t start;

   hangup_needed = KWTrue;

   start = time(nil(time_t)); /* Remember when we started processing */

/*--------------------------------------------------------------------*/
/*                  Report the current modem status                   */
/*--------------------------------------------------------------------*/

   ShowModem();

/*--------------------------------------------------------------------*/
/*             Now actually try to read a buffer of data              */
/*--------------------------------------------------------------------*/

   for ( ; ; )
   {
      unsigned int pending;
      pending = r_count_pending();

      if ( terminate_processing )
      {
         static KWBoolean recurse = KWFalse;
         if ( ! recurse )
         {
            printmsg(2,"nsread: User aborted processing");
            recurse = KWTrue;
         }
         return 0;
      }

#ifdef UDEBUG
      printmsg(20, "nsread: pending=%d, wanted=%d", pending, wanted);
#endif

      if (pending >= wanted) {   /* got enough in the buffer? */
         unsigned int i;

         char UUFAR *buffer = input;

         for (i = 0; i < wanted; i++)
         {
            int Received;

            Received = receive_com();       /* Get character from com port */
            if ( Received < 0 )
            {
                printmsg( 10, "nsread: recv error" );
                return 0;                   /* Indicate carrier loss */
            }
            *buffer++ = (char) Received;

#ifdef UDEBUG
            if ( debuglevel >= 19 )
               printmsg( 19, "nsread: char = %c", Received );
#endif

         }

         traceData( input, wanted, KWFalse );

         return pending;

      } else {
         time_t   now     = time(nil(time_t));
         time_t   elapsed = now - start;

         if (elapsed >= ((time_t) timeout))
            return pending;

         ddelay(0);                    /* Surrender our time slice   */

      } /* else */
   } /* for ( ; ; ) */

} /* nsread */

/*--------------------------------------------------------------------*/
/*    n s w r i t e                                                   */
/*                                                                    */
/*    Write to the serial port                                        */
/*--------------------------------------------------------------------*/

int nswrite(const char UUFAR *input, unsigned int len)
{
   unsigned int i;
   char UUFAR *data = (char UUFAR *) input;

   hangup_needed = KWTrue;

/*--------------------------------------------------------------------*/
/*                      Report our modem status                       */
/*--------------------------------------------------------------------*/

  ShowModem();

/*--------------------------------------------------------------------*/
/*    Introduce a little flow control - Actual line pacing is         */
/*    handled at a lower level                                        */
/*--------------------------------------------------------------------*/

   if ( s_count_free() < (int) len )
   {
      int spin = 0;
      static int const max_spin = 20;

      int queue_size = s_count_size();
      int queue_free = s_count_free();

      if ( (int) len > queue_size )
      {
         printmsg(0,"nswrite: Transmit buffer overflow; buffer size %d, "
                    "needed %d",
               queue_size,len);
         panic();
      }

      while( ((int)len > queue_free) && (spin < max_spin) )
      {
         int wait;
         int needed;
         int new_free;

         needed = max(queue_size/2, ((int)len)-queue_free);
                              /* Minimize thrashing by requiring
                                 big chunks */

         wait = (int) ((long) needed * 10000L / (long) current_bps);
                              /* Compute time in milliseconds
                                 assuming 10 bits per byte           */

         printmsg(4,"nswrite: Waiting %d ms for %d bytes in queue"
                     ", pass %d",
                     wait, needed, spin);

         ddelay( (KEWSHORT) wait ); /* Actually perform the wait     */

         new_free = s_count_free();

         if ( new_free == queue_free )
            spin++;           /* No progress, consider timing out    */
         else
            queue_free = new_free;
                              /* Update our progress                 */

      } /* while( (len > queue_free) && spin ) */

      if ( queue_free < (int) len )
      {
         printmsg(0,"nswrite: Buffer overflow, needed %d bytes"
                     " from queue of %d",
                     len, queue_size);
         return 0;
      } /* if ( queue_free < len ) */

   } /* if ( s_count_free() < len ) */

/*--------------------------------------------------------------------*/
/*            Send the data to the communications package             */
/*--------------------------------------------------------------------*/

   for (i = 0; i < len; i++)
      send_com(*data++);

   traceData( input, len, KWTrue );

/*--------------------------------------------------------------------*/
/*              Return byte count transmitted to caller               */
/*--------------------------------------------------------------------*/

   return len;

} /* nswrite */

/*--------------------------------------------------------------------*/
/*    n s s e n d b r k                                               */
/*                                                                    */
/*    Send a break signal out the serial port                         */
/*--------------------------------------------------------------------*/

void nssendbrk(unsigned int duration)
{

#ifdef UDEBUG
   printmsg(12, "nssendbrk: %d", duration);
#endif

   break_com();

} /* nssendbrk */

/*--------------------------------------------------------------------*/
/*    n c l o s e l i n e                                             */
/*                                                                    */
/*    Close the serial port down                                      */
/*--------------------------------------------------------------------*/

void ncloseline(void)
{
   int far *stats;

   if (!portActive)
   {
      printmsg(0,"ncloseline: Port already closed");
      panic();
   }

   portActive = KWFalse;       /* Flag port closed for error handler  */

   dtr_off();
   ddelay(500);               /* Required for V.24             */
   close_com();
   restore_com();

   if ( bflag[F_MULTITASK] )
      close( com_handle );

   traceStop();

   stats = com_errors();
   printmsg(3, "Buffer overflows: %-4d", stats[COM_EOVFLOW]);
   printmsg(3, "Receive overruns: %-4d", stats[COM_EOVRUN]);
   printmsg(3, "Break characters: %-4d", stats[COM_EBREAK]);
   printmsg(3, "Framing errors:   %-4d", stats[COM_EFRAME]);
   printmsg(3, "Parity errors:    %-4d", stats[COM_EPARITY]);
   printmsg(3, "Transmit errors:  %-4d", stats[COM_EXMIT]);
   printmsg(3, "DSR errors:       %-4d", stats[COM_EDSR]);
   printmsg(3, "CTS errors:       %-4d", stats[COM_ECTS]);

} /*closeline*/

/*--------------------------------------------------------------------*/
/*    n h a n g u p                                                   */
/*                                                                    */
/*    Hangup the telephone by dropping DTR.  Works with HAYES and     */
/*    many compatibles.                                               */
/*--------------------------------------------------------------------*/

void nhangup( void )
{
      if (!hangup_needed)
         return;

      hangup_needed = KWFalse;
      carrierDetect = KWFalse;  /* No modem connected yet               */

      dtr_off();              /* Hang the phone up                     */
      ddelay(500);            /* Really only need 250 milliseconds     */
      dtr_on();               /* Bring the modem back on-line          */
      ddelay(2000);           /* Now wait for the poor thing to recover*/
                              /* two seconds is required by V.24       */

      printmsg(3,"nhangup: complete.");

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

void nSIOSpeed(BPS bps)
{

   printmsg(4,"SIOSspeed: Changing port speed from %lu BPS to %lu BPS",
               (unsigned long) current_bps, (unsigned long) bps);
   current_bps = bps;
   ioctl_com(0, (unsigned) current_bps);

   ShowModem();

} /* nSIOSpeed */

/*--------------------------------------------------------------------*/
/*    n f l o w c o n t r o l                                         */
/*                                                                    */
/*    Enable/Disable in band (XON/XOFF) flow control                  */
/*--------------------------------------------------------------------*/

void nflowcontrol( KWBoolean flow )
{
   printmsg(4,"flowcontrol: Closing port");
   close_com();
   ShowModem();
   printmsg(4,"flowcontrol: Opening port to %sable flow control",
               flow ? "en" : "dis");
   open_com((unsigned) current_bps, current_direct, 'N', STOPBIT, flow ? 'E' : 'D');
   ShowModem();

} /* nflowcontrol */

/*--------------------------------------------------------------------*/
/*    n G e t S p e e d                                               */
/*                                                                    */
/*    Report current speed of communications connection               */
/*--------------------------------------------------------------------*/

BPS nGetSpeed( void )
{

   return current_bps;

} /* nGetSpeed */

/*--------------------------------------------------------------------*/
/*    n C D                                                           */
/*                                                                    */
/*    Report if we have carrier detect and lost it                    */
/*--------------------------------------------------------------------*/

KWBoolean nCD( void )
{
   KWBoolean newCarrierDetect;

   ShowModem();
   newCarrierDetect = is_cd_high();

   if ( newCarrierDetect )
      carrierDetect = newCarrierDetect;

/*--------------------------------------------------------------------*/
/*    If we previously had carrier detect but have lost it, we        */
/*    report it was lost.  If we do not yet have carrier detect,      */
/*    we return success because we may not have connected yet.        */
/*--------------------------------------------------------------------*/

   if (carrierDetect)
      return newCarrierDetect && is_dsr_high();
   else
      return is_dsr_high();

} /* nCD */

/*--------------------------------------------------------------------*/
/*    S h o w M o d e m                                               */
/*                                                                    */
/*    Report current modem status                                     */
/*--------------------------------------------------------------------*/

#define mannounce(flag, bits, text ) ((flag & bits) ? text : "" )

static void ShowModem( void )
{
   static int old_status = 0xDEAD;
   int status;

   if ( debuglevel < 4 )
      return;

   status = modem_status();
   if (status == old_status)
      return;

   printmsg(0, "ShowModem: %#02x%s%s%s%s%s%s%s%s",
      status,
      mannounce(MDM_CD,   status, "\tCarrier Detect"),
      mannounce(MDM_RI,   status, "\tRing Indicator"),
      mannounce(MDM_DSR,  status, "\tData Set Ready"),
      mannounce(MDM_CTS,  status, "\tClear to Send"),
      mannounce(MDM_CDC,  status, "\tCD changed"),
      mannounce(MDM_TRI,  status, "\tRI went OFF"),
      mannounce(MDM_DSRC, status, "\tDSR changed"),
      mannounce(MDM_CTSC, status, "\tCTS changed"));
   old_status = status;

} /* ShowModem */

#ifdef __TURBOC__
#pragma argsused
#elif _MSC_VER >= 700
#pragma warning(disable:4100)   /* suppress unref'ed formal param. warnings */
#endif

/*--------------------------------------------------------------------*/
/*       s e t P r t y                                                */
/*                                                                    */
/*       No operation under DOS                                       */
/*--------------------------------------------------------------------*/

void setPrty( const KEWSHORT priorityIn, const KEWSHORT prioritydeltaIn )
{
}

#if _MSC_VER >= 700
#pragma warning(default:4100)   /* restore unref'ed formal param. warnings */
#endif

/*--------------------------------------------------------------------*/
/*       r e s e t P r t y                                            */
/*                                                                    */
/*       No operation under DOS                                       */
/*--------------------------------------------------------------------*/

void resetPrty( void ) { }
