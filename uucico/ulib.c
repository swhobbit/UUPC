/*--------------------------------------------------------------------*/
/*    u l i b . c                                                     */
/*                                                                    */
/*    Serial port interface to COMMFIFO.ASM for MS-DOS                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989 by Andrew H. Derbyshire.  Changes    */
/*    Copyright (c) 1990-1993 by Kendra Electronic Wonderworks;       */
/*    all rights reserved except those explicitly granted by the      */
/*    UUPC/extended license.                                          */
/*--------------------------------------------------------------------*/

/*
 *    $Id: ULIB.C 1.9 1993/04/11 00:33:54 ahd Exp $
 *    $Log: ULIB.C $
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

#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <share.h>

#ifdef __TURBOC__
#include <alloc.h>
#endif

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "hlib.h"
#include "ulib.h"
#include "comm.h"
#include "ssleep.h"
#include "catcher.h"

/*--------------------------------------------------------------------*/
/*                        Internal prototypes                         */
/*--------------------------------------------------------------------*/

static void ShowModem( void );

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

boolean   port_active = FALSE;  /* TRUE = port handler handler active  */

static BPS current_bps;
static char current_direct;
static boolean carrierdetect;

currentfile();

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
#define LINELOG "LineData.Log"      /* log serial line data here */

static int log_handle;
static int logmode = 0;             /* Not yet logging            */
#define WRITING 1
#define READING 2
static FILE *log_stream;
static int com_handle;
static boolean hangup_needed = TRUE;

/*--------------------------------------------------------------------*/
/*    o p e n l i n e                                                 */
/*                                                                    */
/*    Open the serial port for I/O                                    */
/*--------------------------------------------------------------------*/

int openline(char *name, BPS bps, const boolean direct)
{
   int   value;

   if (port_active)              /* Was the port already active?     ahd   */
      closeline();               /* Yes --> Shutdown it before open  ahd   */

   printmsg(15, "openline: %s, %d", name, bps);

   logmode = 0;

   current_direct = (char) (direct ? 'D' : 'M') ;

   if (sscanf(name, "COM%d", &value) != 1)
   {
      printmsg(0,"Communications port must be format COMx, was %s",
                name);
      panic();
   }

   com_handle = sopen( name, O_BINARY | O_RDWR, SH_DENYRW );
                                 /* Used soly for lock abilities  */
   if ( com_handle == -1 )
   {
      printerr( name );
      return 1;
   }

   select_port(value);
   save_com();

   if (!install_com())
   {
      printmsg(0,"Commuications handler install failed; "
                  "probable cause ... memory shortage.");

#ifdef __TURBOC__
      printmsg(0,"FAR DOS Memory free = %ld bytes",
                  farcoreleft() );
#endif

      return 1;
   }

   open_com(bps, current_direct, 'N', STOPBIT, 'D');
   current_bps = bps;
   dtr_on();
   ssleep(2);                 /* Wait two seconds as required by V.24   */
   carrierdetect = FALSE;     /* No modem connected yet                 */

/*--------------------------------------------------------------------*/
/*        Log serial line data only if log file already exists        */
/*--------------------------------------------------------------------*/

   log_handle = open(LINELOG, O_WRONLY | O_TRUNC | O_BINARY);
   if (log_handle != -1) {
      printmsg(15, "openline: logging serial line data to %s", LINELOG);
      log_stream = fdopen(log_handle, "wb");
   }

   port_active = TRUE;     /* record status for error handler */

   return 0;

} /*openline*/


/*--------------------------------------------------------------------*/
/*    s r e a d                                                       */
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

unsigned int sread(char *buffer, unsigned int wanted, unsigned int timeout)
{
   time_t start;

   hangup_needed = TRUE;

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
         static boolean recurse = FALSE;
         if ( ! recurse )
         {
            printmsg(2,"sread: User aborted processing");
            recurse = TRUE;
         }
         return 0;
      }

      printmsg(20, "sread: pending=%d, wanted=%d", pending, wanted);

      if (pending >= wanted) {   /* got enough in the buffer? */
         unsigned int i;
         for (i = 0; i < wanted; i++)
         {
            int Received;

            Received = receive_com();       /* Get character from com port */
            if ( Received < 0 )
            {
                printmsg( 10, "sread: recv error" );
                return 0;                   /* Indicate carrier loss */
            }
            *buffer++ = (char) Received;
            printmsg( 19, "sread: char = %c", Received );
         }

         if (log_handle != -1)
         {
#ifdef VERBOSE
            char s[18];
#endif
            buffer -= wanted;
            if (logmode != READING)
            {
               fputs("\nRead:  ", log_stream);
               logmode = READING;
            } /* if */
#ifdef VERBOSE
            for (i = 0; i < wanted; i++) {
               itoa(0x100 | (unsigned) *buffer++, s, 16);
                                             /* Make it printable hex */
               fwrite(s, 1, 2, log_stream);  /* Write hex to the log    */
            } /* for */
#else
            fwrite(buffer, 1, wanted, log_stream);
                                             /* Write data to the log */
#endif
         } /* if (log_handle != -1) */

         return pending;
      } else {
         time_t   now     = time(nil(time_t));
         time_t   elapsed = now - start;

         if (elapsed >= ((time_t) timeout))
            return pending;

         ddelay(0);                    /* Surrender our time slice   */

      } /* else */
   } /* for ( ; ; ) */

} /*sread*/

/*--------------------------------------------------------------------*/
/*    s w r i t e                                                     */
/*                                                                    */
/*    Write to the serial port                                        */
/*--------------------------------------------------------------------*/

int swrite(const char *input, unsigned int len)
{
   unsigned int i;
   char *data = (char *) input;

   hangup_needed = TRUE;

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
         printmsg(0,"swrite: Transmit buffer overflow; buffer size %d, "
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

         printmsg(4,"swrite: Waiting %d milliseconds for %d bytes in queue"
                     ", pass %d",
                     wait, needed, spin);

         ddelay( wait );      /* Actually perform the wait           */

         new_free = s_count_free();

         if ( new_free == queue_free )
            spin++;           /* No progress, consider timing out    */
         else
            queue_free = new_free;
                              /* Update our progress                 */

      } /* while( (len > queue_free) && spin ) */

      if ( queue_free < (int) len )
      {
         printmsg(0,"swrite: Transmit buffer overflow, needed %d bytes",
                     len);
      } /* if ( queue_free < len ) */
      return 0;

   } /* if ( s_count_free() < len ) */

/*--------------------------------------------------------------------*/
/*            Send the data to the communications package             */
/*--------------------------------------------------------------------*/

   for (i = 0; i < len; i++)
      send_com(*data++);

/*--------------------------------------------------------------------*/
/*                Log the transmitted data, if desired                */
/*--------------------------------------------------------------------*/

   if (log_handle != -1) {
#ifdef VERBOSE
      char s[18];
#endif
      if (logmode != WRITING)
      {
         fputs("\nWrite: ", log_stream);
         logmode = WRITING;
      } /* if */
      data -= len;
#ifdef VERBOSE
      for (i = 0; i < len; i++) {
         itoa(0x100 | (unsigned) *data++, s, 16);
                                        /* Make it printable hex  ahd */
         fwrite(s, 1, 2, log_stream);
      } /* for */
#else
      fwrite(data, 1, len, log_stream);   /* Write data to the log */
#endif
   } /* if */

/*--------------------------------------------------------------------*/
/*              Return byte count transmitted to caller               */
/*--------------------------------------------------------------------*/

   return len;

} /*swrite*/


/*--------------------------------------------------------------------*/
/*    s s e n d b r k                                                 */
/*                                                                    */
/*    Send a break signal out the serial port                         */
/*--------------------------------------------------------------------*/

void ssendbrk(unsigned int duration)
{

   printmsg(12, "ssendbrk: %d", duration);

   break_com();

} /*ssendbrk*/


/*--------------------------------------------------------------------*/
/*    c l o s e l i n e                                               */
/*                                                                    */
/*    Close the serial port down                                      */
/*--------------------------------------------------------------------*/

void closeline(void)
{
   int far *stats;

   if (!port_active)
      panic();

   port_active = FALSE; /* flag port closed for error handler  */

   dtr_off();
   ddelay(500);               /* Required for V.24             */
   close_com();
   restore_com();
   close( com_handle );

   if (log_handle != -1) {    /* close serial line log file */
      fclose(log_stream);
      close(log_handle);
   };

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
      dtr_off();              /* Hang the phone up                         */
      ddelay(500);            /* Really only need 250 milliseconds         */
      dtr_on();               /* Bring the modem back on-line              */
      ddelay(2000);           /* Now wait for the poor thing to recover    */
                              /* two seconds is required by V.24           */
      printmsg(3,"hangup: complete.");
      carrierdetect = FALSE;  /* No modem connected yet                    */

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

void SIOSpeed(BPS bps)
{

   printmsg(4,"SIOSspeed: Changing port speed from %ld BPS to %ld BPS",
               (long) current_bps, (long) bps);
   ioctl_com(0, bps);

   ShowModem();
   current_bps = bps;

} /*SIOSpeed*/

/*--------------------------------------------------------------------*/
/*    f l o w c o n t r o l                                           */
/*                                                                    */
/*    Enable/Disable in band (XON/XOFF) flow control                  */
/*--------------------------------------------------------------------*/

void flowcontrol( boolean flow )
{
   printmsg(4,"flowcontrol: Closing port");
   close_com();
   ShowModem();
   printmsg(4,"flowcontrol: Opening port to %sable flow control",
               flow ? "en" : "dis");
   open_com(current_bps, current_direct, 'N', STOPBIT, flow ? 'E' : 'D');
   ShowModem();

} /*flowcontrol*/

/*--------------------------------------------------------------------*/
/*    G e t S p e e d                                                 */
/*                                                                    */
/*    Report current speed of communications connection               */
/*--------------------------------------------------------------------*/

BPS GetSpeed( void )
{
   return current_bps;
} /* GetSpeed */

/*--------------------------------------------------------------------*/
/*    C D                                                             */
/*                                                                    */
/*    Report if we have carrier detect and lost it                    */
/*--------------------------------------------------------------------*/

boolean CD( void )
{
   boolean online = carrierdetect;

   ShowModem();
   carrierdetect = is_cd_high();

/*--------------------------------------------------------------------*/
/*    If we previously had carrier detect but have lost it, we        */
/*    report it was lost.  If we do not yet have carrier detect,      */
/*    we return success because we may not have connected yet.        */
/*--------------------------------------------------------------------*/

   if (online)
      return carrierdetect && is_dsr_high();
   else
      return is_dsr_high();

} /* CD */

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
