/*
   ibmpc/ulib.c

   DCP system-dependent library, with support for INT14 drivers.

   Services provided by ulib.c:

   - login
   - UNIX commands simulation
   - serial I/O
   - rnews

   Updated:

      14May89  - Added hangup() procedure                               ahd
      21Jan90  - Replaced code for rnews() from Wolfgang Tremmel
                 <tremmel@garf.ira.uka.de> to correct failure to
                 properly read compressed news.                         ahd
      6 Sep 90 - Change logging of line data to printable               ahd
      8 Sep 90 - Split ulib.c into dcplib.c and ulib.c                  ahd
     30 Nov 92 - Adapt for use with INT14 drivers.
                 Currently this is guaranteed to work only with
                 Artisoft's ARTICOMM driver, but porting to others
                 should be minor.  Mods by Mark W. Schumann
                 <mark@whizbang.wariat.org>

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

#ifndef __BORLANDC__
  #error You must have Borland C/C++ to use the INT14 feature!
#endif

#include <dos.h>    /* Declares union REGS and int86(). */

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
static int bps_table (int);

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
static FILE *com_stream;
static boolean hangup_needed = TRUE;

union REGS regs;          /* Scratch area for interrupt calls. */
int portnum;              /* Zero-based serial port number. */

/*--------------------------------------------------------------------*/
/*    b p s _ t a b l e                                               */
/*                                                                    */
/*    Look up the INT14 baud rate value corresponding to (bps).       */
/*    This is used with the INT14 modifications only.                 */
/*--------------------------------------------------------------------*/

static int bps_table (int bps)

{

static int tab[] = {110, 150, 300, 600, 1200, 2400, 4800, 9600, -1};
int i;

      for (i = 0; tab[i] > 0; i++) {
              if (tab[i] == bps)
                      return i;
              }

      return -1;
      }


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

   norecovery = FALSE;

   /*
      With the INT14 setup, we don't worry about the lock file
      since our modem sharing software is supposed to deal with
      resource contention.  There is also no "communications
      handler" to install since that was supposed to be done by
      the modem sharing software.
   */
   portnum = value - 1;   /* Standard INT14 interface port setup: */
   regs.x.ax = 0x0400;    /* Initialize port */
   regs.h.bh = 0x00;      /* No parity */
   regs.h.bl = 0x00;      /* One stop bit */
   regs.h.ch = 0x03;      /* Eight-bit words */
   regs.h.cl = bps_table (bps);  /* Calculate baud rate pointer */
   regs.x.dx = portnum;   /* Zero-based port number */
   int86 (0x14, &regs, &regs);
   current_bps = bps;

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

   /* With INT14, don't bother with a counter; just set a timeout    */
   /*  value with INT14/AX=8009.  This is Artisoft-specific.         */
   unsigned count = 0;

   hangup_needed = TRUE;

/*--------------------------------------------------------------------*/
/*                  Report the current modem status                   */
/*--------------------------------------------------------------------*/

   ShowModem();

/*--------------------------------------------------------------------*/
/*             Now actually try to read a buffer of data              */
/*--------------------------------------------------------------------*/

   regs.x.ax = 0x8009;            /* Set timeouts */
   regs.x.bx = timeout * 91 / 5;  /* Send timeout in ticks */
   regs.x.cx = timeout * 91 / 5;  /* Receive timeout in ticks */
   regs.x.dx = portnum;           /* Port number */
   int86 (0x14, &regs, &regs);

   while (wanted > 0) {

      regs.x.ax = 0x0200;         /* Read a byte */
      regs.x.dx = portnum;
      int86 (0x14, &regs, &regs);
      if (regs.x.ax & 0x8000) {   /* Timed out? */
         printmsg (20, "Timeout in sread().");
         break; /* Timeout occurred. */
         }
      *buffer++ = regs.h.al;
      wanted--;
      count++;
      }

   return count;

} /*sread*/

/*--------------------------------------------------------------------*/
/*    s w r i t e                                                     */
/*                                                                    */
/*    Write to the serial port                                        */
/*--------------------------------------------------------------------*/

int swrite(char *data, unsigned int len)
{
   unsigned int i;

   ShowModem();

/*
   All this couting and shuffling doesn't need to be done with INT14.
   Simply go into a loop and spit bytes out until there aren't any
   more.  INT14/AH=1 is the standard interface to write characters
   to the serial port.
*/

   for (i = 0; i < len; i++) {
      regs.h.ah = 0x01;
      regs.h.al = *data++;
      regs.x.dx = portnum;
      int86 (0x14, &regs, &regs);
      }

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

/*
   With INT14, we drop DSR for a quarter of a second to indicate
   a BREAK sequence.
*/
   regs.x.ax = 0x0500;    /* Extd port control: get modem control register */
   regs.x.dx = portnum;
   int86 (0x14, &regs, &regs);
   regs.x.ax = 0x0501;    /* Set modem control register */
   regs.x.dx = portnum;
   regs.h.bl |= 0x20;     /* Drop DSR */
   int86 (0x14, &regs, &regs);
   ddelay (250);          /* Wait a quarter second */
   regs.x.ax = 0x0501;
   regs.x.dx = portnum;
   regs.h.bl &= ~0x20;    /* Raise DSR */
   int86 (0x14, &regs, &regs);

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

   regs.x.ax = 0x0500;   /* Get modem control register */
   regs.x.dx = portnum;  /* Zero-based port number */
   int86 (0x14, &regs, &regs);
   regs.h.bl &= ~0x20;   /* Drop DSR */
   regs.x.ax = 0x0501;   /* Set modem control register */
   regs.x.dx = portnum;
   int86 (0x14, &regs, &regs);
   ddelay (500);

   fclose( com_stream );
   norecovery = TRUE;

   if (log_handle != -1) {    /* close serial line log file */
      fclose(log_stream);
      close(log_handle);
   };

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

      regs.x.ax = 0x0500;      /* Get modem control register */
      regs.x.dx = portnum;
      int86 (0x14, &regs, &regs);
      regs.x.ax = 0x0501;      /* Set modem control register */
      regs.h.bl &= ~0x20;      /* Mask to reset DSR bit */
      regs.x.dx = portnum;
      int86 (0x14, &regs, &regs);
      ddelay (500);            /* Pause half a second */
      regs.x.ax = 0x0501;      /* Set modem control register */
      regs.h.bl |= 0x20;       /* Set DSR bit */
      regs.x.dx = portnum;
      int86 (0x14, &regs, &regs);
      ddelay (2000);           /* Wait two seconds to recover */

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

   regs.h.ah = 0x00;            /* Initialize modem */
   regs.x.ax = 0x0400;          /* Initialize port */
   regs.h.bh = 0x00;            /* No parity */
   regs.h.bl = 0x00;            /* One stop bit */
   regs.h.ch = 0x03;            /* Eight-bit words */
   regs.h.cl = bps_table (bps); /* New baud rate */
   regs.x.dx = portnum;
   int86 (0x14, &regs, &regs);

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

/*
   With INT14, don't open and close the port; toggle it in place
   by using INT14/AX=800A, which is again Artisoft-specific.
*/
   printmsg (4, "flowcontrol: %abling flow control", (flow ? "en" : "dis"));
   regs.x.ax = 0x800A;
   regs.h.bl = flow ? 2 : 1;   /* 2 is hardware, 1 is XON/XOFF */
   regs.x.dx = portnum;
   int86 (0x14, &regs, &regs);

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

   regs.x.ax = 0x0300;     /* Get port status */
   regs.x.dx = portnum;
   int86 (0x14, &regs, &regs);
   status = regs.h.al;     /* Result is in AL */

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

/*--------------------------------------------------------------------*/
/*    m o d e m _ s t a t u s                                         */
/*                                                                    */
/*    Get current modem status                                        */
/*    This is the INT14 version.  It uses the standard INT14          */
/*    call to get modem status register.                              */
/*--------------------------------------------------------------------*/

int far modem_status (void)

{

      regs.x.ax = 0x0300;
      regs.x.dx = portnum;
      int86 (0x14, &regs, &regs);
      return regs.h.al;

      }
