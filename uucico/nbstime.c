/*--------------------------------------------------------------------*/
/*       n b s t i m e . c                                            */
/*                                                                    */
/*       Set local system clock from United States's National         */
/*       Institute of Standards and Technology (formerly National     */
/*       Bureau of Standards) Standard Time service                   */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-2000 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: nbstime.c 1.42 1999/01/08 02:20:52 ahd Exp $
 *
 *    Revision history:
 *    $Log: nbstime.c $
 *    Revision 1.42  1999/01/08 02:20:52  ahd
 *    Convert currentfile() to RCSID()
 *
 *    Revision 1.41  1999/01/04 03:53:30  ahd
 *    Annual copyright change
 *
 *    Revision 1.40  1998/11/05 03:29:03  ahd
 *    Move code to determine NT vs. 95 to configur.c
 *    ./
 *
 *    Revision 1.39  1998/03/01 01:39:58  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.38  1997/05/18 17:36:16  ahd
 *    Windows 95 support
 *
 *    Revision 1.37  1997/05/13 04:02:01  dmwatt
 *    Windows NT support
 *
 *    Revision 1.35  1996/01/01 21:21:51  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.34  1995/09/26 03:11:36  ahd
 *    Add missing BREAK after case 50:
 *
 *    Revision 1.33  1995/08/27 23:34:11  ahd
 *    Relax search for MJD string
 *
 *    Revision 1.32  1995/05/07 14:41:07  ahd
 *    The infamous bi-annual daylight savings time switch bug fest
 *
 *    Revision 1.31  1995/04/02 00:01:39  ahd
 *    Correct DOS clock setting to use correct TZ under MS compilers
 *
 *    Revision 1.30  1995/03/24 04:17:22  ahd
 *    Compiler warning message cleanup, optimize for low memory processing
 *
 *    Revision 1.29  1995/03/11 15:49:23  ahd
 *    Clean up compiler warnings, modify dcp/dcpsys/nbstime for better msgs
 *
 *    Revision 1.28  1995/01/30 04:08:36  ahd
 *    Additional compiler warning fixes
 *
 *    Revision 1.27  1995/01/07 16:39:13  ahd
 *    Change boolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.26  1994/12/22 00:35:44  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.25  1994/05/01 21:57:55  ahd
 *    Insert leading zeroes (0) in fixed format time output
 *    Correct IBM OS/2 daylight savings time error
 *
 *        Revision 1.24  1994/04/27  00:02:15  ahd
 *        Pick one: Hot handles support, OS/2 TCP/IP support,
 *                  title bar support
 *
 * Revision 1.23  1994/03/05  21:12:05  ahd
 * Correct display of (null) string in status messages
 *
 * Revision 1.22  1994/02/25  02:32:44  ahd
 * Correct time constant
 *
 * Revision 1.21  1994/02/23  04:17:23  ahd
 * More exact matching of input buffer (execute time set after
 * time mark, not two characters later)
 *
 * Revision 1.20  1994/02/22  02:59:10  ahd
 * Process input one character at a time to insure timely echo
 *
 * Revision 1.19  1994/02/21  16:38:58  ahd
 * Bulletproof buffer parse to insure garbled buffers are not
 * used to set system clock.
 *
 * Revision 1.18  1994/02/20  19:11:18  ahd
 * IBM C/Set 2 Conversion, memory leak cleanup
 *
 * Revision 1.17  1994/02/19  05:09:17  ahd
 * Use standard first header
 *
 * Revision 1.16  1994/01/24  03:04:44  ahd
 * Annual Copyright Update
 *
 * Revision 1.15  1994/01/01  19:20:23  ahd
 * Annual Copyright Update
 *
 * Revision 1.14  1993/12/30  02:56:56  dmwatt
 * Suppress NT compiler warnings
 *
 * Revision 1.13  1993/12/23  03:17:55  rommel
 * OS/2 32 bit support for additional compilers
 *
 * Revision 1.12  1993/10/28  12:19:01  ahd
 * Cosmetic time formatting twiddles and clean ups
 *
 * Revision 1.11  1993/10/09  22:21:55  rhg
 * ANSIfy source
 *
 * Revision 1.10  1993/09/26  03:32:27  dmwatt
 * Use Standard Windows NT error message module
 *
 * Revision 1.9  1993/09/20  04:46:34  ahd
 * OS/2 2.x support (BC++ 1.0 support)
 * TCP/IP support from Dave Watt
 * 't' protocol support
 *
 * Revision 1.8  1993/07/22  23:22:27  ahd
 * First pass at changes for Robert Denny's Windows 3.1 support
 *
 * Revision 1.7  1993/06/13  14:02:32  dmwatt
 * Additional Windows/NT fixes
 *
 * Revision 1.6  1993/05/30  00:04:53  ahd
 * Multiple communications drivers support
 *
 * Revision 1.5  1993/04/15  04:15:43  ahd
 * Twiddle OS/2 support and reduce number of #ifdef segments
 *
 * Revision 1.4  1993/04/13  02:27:31  dmwatt
 * Windows/NT updates
 *
 * Revision 1.3  1993/04/11  00:34:11  ahd
 * Global edits for year, TEXT, etc.
 *
 */

#include "uupcmoah.h"
#include "title.h"

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include <ctype.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "arpadate.h"
#include "dcp.h"
#include "dcpsys.h"
#include "hostable.h"
#include "nbstime.h"
#include "script.h"
#include "security.h"
#include "commlib.h"
#include "catcher.h"

#ifdef __IBMC__
#define timezone() _timezone
#else
#define timezone() timezone
#endif

RCSID("$Id: nbstime.c 1.42 1999/01/08 02:20:52 ahd Exp $");

#if defined(FAMILYAPI) || defined(__OS2__)

/*--------------------------------------------------------------------*/
/*                         OS/2 include files                         */
/*--------------------------------------------------------------------*/

#define INCL_BASE

#include <os2.h>

#ifdef FAMILYAPI
typedef USHORT APIRET ;  /* Define older API return type              */
#endif

/*--------------------------------------------------------------------*/
/*       s e t C l o c k                                              */
/*                                                                    */
/*       Set system clock under OS/2 1.x or 2.x                       */
/*--------------------------------------------------------------------*/

static time_t
setClock( time_t newClock, const KWBoolean daylightSavingsInEffect )
{
   DATETIME dateTime;
   struct tm *tp;

   APIRET rc = DosGetDateTime( &dateTime );

   if ( rc != 0 )
   {
      printmsg(0,"Return code from DosGetDateTime %d", rc);
      panic();
   }

   printmsg(3,"OS/2 time: %02d/%02d/%02d %02d:%02d:%02d tz %d, "
              "weekday %d DST %d",
              (int) dateTime.year,
              (int) dateTime.month,
              (int) dateTime.day ,
              (int) dateTime.hours,
              (int) dateTime.minutes,
              (int) dateTime.seconds ,
              (int) dateTime.timezone,
              (int) dateTime.weekday,
              daylight );

   newClock -= timezone();

#ifdef __OS2__
   if (daylight && daylightSavingsInEffect )
      newClock += 3600;             /* Valid for USA only            */
#endif

   tp = localtime(&newClock);       /* Get local time as record      */

   dateTime.year    = (USHORT) (tp->tm_year + 1900);
   dateTime.month   = (UCHAR) (tp->tm_mon + 1);
   dateTime.day     = (UCHAR) tp->tm_mday;
   dateTime.hours   = (UCHAR) tp->tm_hour;
   dateTime.minutes = (UCHAR) tp->tm_min;
   dateTime.seconds = (UCHAR) tp->tm_sec;

   printmsg(3,"NIST time: %02d/%02d/%02d %02d:%02d:%02d tz %d, "
              "weekday %d DST %d",
              (int) dateTime.year,
              (int) dateTime.month,
              (int) dateTime.day ,
              (int) dateTime.hours,
              (int) dateTime.minutes,
              (int) dateTime.seconds ,
              (int) dateTime.timezone,
              (int) dateTime.weekday,
              daylightSavingsInEffect ? 1 : 0 );

   rc = DosSetDateTime( &dateTime );

   if ( rc != 0 )
   {
      printmsg(0,"Return code from DosGetDateTime %d", rc);
      panic();
   }

   return newClock;

} /* setClock */

#elif defined( WIN32 )

/*--------------------------------------------------------------------*/
/*                      Windows NT include files                      */
/*--------------------------------------------------------------------*/

#include <windows.h>
#include "pnterr.h"

/*--------------------------------------------------------------------*/
/*       s e t C l o c k                                              */
/*                                                                    */
/*       Set system clock under Windows NT                            */
/*--------------------------------------------------------------------*/

static time_t
setClock( struct tm *txp )
{
   SYSTEMTIME dateTime;
   TOKEN_PRIVILEGES tkp;
   HANDLE hToken;
   DWORD dwError;
   int rc;

   GetSystemTime( &dateTime );

   printmsg(3,"Date time: %02d/%02d/%02d %02d:%02d:%02d, weekday %d",
              (int) dateTime.wYear,
              (int) dateTime.wMonth,
              (int) dateTime.wDay ,
              (int) dateTime.wHour,
              (int) dateTime.wMinute,
              (int) dateTime.wSecond ,
              (int) dateTime.wDayOfWeek );

   dateTime.wYear    = (WORD) txp->tm_year + 1900;
   dateTime.wMonth   = (WORD) txp->tm_mon + 1;
   dateTime.wDay     = (WORD) txp->tm_mday;
   dateTime.wHour    = (WORD) txp->tm_hour;
   dateTime.wMinute  = (WORD) txp->tm_min;
   dateTime.wSecond  = (WORD) txp->tm_sec;

   printmsg(3,"Date time: %02d/%02d/%02d %02d:%02d:%02d, weekday %d",
              (int) dateTime.wYear,
              (int) dateTime.wMonth,
              (int) dateTime.wDay ,
              (int) dateTime.wHour,
              (int) dateTime.wMinute,
              (int) dateTime.wSecond ,
              (int) dateTime.wDayOfWeek );

   /* Open the process token to get permission to adjust the time
      on Windows NT;  on Windows 95, just party on...              */

   if (isWinNT())
   {
      if (!OpenProcessToken(GetCurrentProcess(),
         TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
         &hToken))
      {
         dwError = GetLastError();
         printmsg(0, "nbstime: OpenProcessToken failed");
         printNTerror("OpenProcessToken", dwError);
         return 0;
      }

      LookupPrivilegeValue(NULL,
                           "SeSystemtimePrivilege",
                           &tkp.Privileges[0].Luid);
      tkp.PrivilegeCount = 1;
      tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

      if (!AdjustTokenPrivileges(hToken, KWFalse, &tkp, 0,
         (PTOKEN_PRIVILEGES)NULL, 0))
      {
         dwError = GetLastError();
         printmsg(0, "nbstime: first AdjustTokenPrivilege failed");
         printNTerror("AdjustTokenPrivileges", dwError);
         return 0;
      }
   }

   rc = SetSystemTime( &dateTime );

   if ( !rc )
   {
      dwError = GetLastError();
      printmsg(0, "nbstime: SetSystemTime failed");
      printNTerror("SetSystemTime", dwError);
   }

   if (isWinNT())
   {
      tkp.Privileges[0].Attributes = 0;

      if (!AdjustTokenPrivileges(hToken, KWFalse, &tkp, 0,
         (PTOKEN_PRIVILEGES)NULL, 0))
      {
         dwError = GetLastError();
         printmsg(0, "nbstime: AdjustTokenPrivileges disable failed");
         printNTerror("AdjustTokenPrivileges", dwError);
         return 0;
      }
   }

   return time( NULL );             /* Since we didn't set the time
                                       via newClock, get it now to
                                       compare                       */

} /* setClock */

#else

#ifndef __TURBOC__

/*--------------------------------------------------------------------*/
/*                     DOS specific include files                     */
/*--------------------------------------------------------------------*/

#include <dos.h>


/*--------------------------------------------------------------------*/
/*       s e t C l o c k                                              */
/*                                                                    */
/*       Given the local time, set the system clock under DOS         */
/*--------------------------------------------------------------------*/

static void
stime( const time_t *localClock )
{

   struct dosdate_t ddate;
   struct dostime_t dtime;
   struct tm *tp;
   unsigned rc;

   tp = localtime(localClock);      /* Get time as a record, do NOT
                                       readjust for TZ offset        */

/*--------------------------------------------------------------------*/
/*              Determine current local time in DOS format            */
/*--------------------------------------------------------------------*/

   ddate.day     = (unsigned char) tp->tm_mday;
   ddate.month   = (unsigned char) (tp->tm_mon + 1);
   ddate.year    = (unsigned int)  (tp->tm_year + 1900);
   ddate.dayofweek = (unsigned char) tp->tm_mday;  /* 0-6, 0=Sunday  */

   dtime.hour    = (unsigned char) tp->tm_hour;
   dtime.minute  = (unsigned char) tp->tm_min;
   dtime.second  = (unsigned char) tp->tm_sec;
   dtime.hsecond = (unsigned char) 0;

   printmsg(3,"stime: Date time: %02d/%02d/%02d %02d:%02d:%02d "
              " weekday %d",
              (int) ddate.year,
              (int) ddate.month,
              (int) ddate.day,
              (int) dtime.hour,
              (int) dtime.minute,
              (int) dtime.second,
              (int) ddate.dayofweek );

/*--------------------------------------------------------------------*/
/*                 Set time under DOS with MS C compiler              */
/*--------------------------------------------------------------------*/

   if ( (rc = _dos_settime( &dtime )) != 0 )
   {
      printmsg(0,"Return code from _dos_settime %d", rc);
      panic();
   }

   if ( (rc = _dos_setdate( &ddate )) != 0 )
   {
      printmsg(0,"Return code from _dos_setdate %d", rc);
      panic();
   }

} /* stime */

#endif /* __TURBOC__ */

/*--------------------------------------------------------------------*/
/*       s e t C l o c k                                              */
/*                                                                    */
/*       Set system clock under MS-DOS                                */
/*--------------------------------------------------------------------*/

static time_t
setClock( time_t newClock, const KWBoolean daylightSavingsInEffect )
{

/*--------------------------------------------------------------------*/
/*       Neither Borland C++ nor MS C adjust for the local time       */
/*       zone properly; perform the adjustment by hand.               */
/*--------------------------------------------------------------------*/

   newClock -= timezone;

#ifdef __TURBOC__

/*--------------------------------------------------------------------*/
/*    If this timezone uses daylight savings and we are in the        */
/*    period to spring forward, do so.                                */
/*--------------------------------------------------------------------*/

   if (daylight && daylightSavingsInEffect )
      newClock += 3600;             /* Valid for the USA only        */

#endif

/*--------------------------------------------------------------------*/
/*                        Actually set the clock                      */
/*--------------------------------------------------------------------*/

   stime( &newClock );

/*--------------------------------------------------------------------*/
/*                  Return our local time to the caller               */
/*--------------------------------------------------------------------*/

   return newClock;

} /* setClock */

#endif

/*--------------------------------------------------------------------*/
/*    n b s t i m e                                                   */
/*                                                                    */
/*    Set system clock from using time from NBS of the format:        */
/*                                                                    */
/*                  MJD  YR MO DA  H  M  S ST S UT1 msADV         OTM */
/*  nbs format-->  47511 88-12-16 06:03:44 00 0 -.1 045.0 UTC(NIST) * */
/*  @ 1200 baud    47511 88-12-16 06:03:45 00 0 -.1 045.0 UTC(NIST) * */
/*                 49809 95-04-02 14:15:58 51 0 +.2 045.0 UTC(NIST) * */
/*--------------------------------------------------------------------*/

KWBoolean nbstime( void )
{
   static const char model[] =
         "\r\n##### ##-##-## ##:##:## ## # +.# ###.# UTC(NIST) *";
   char buf[sizeof model + 1];
   struct tm  tx;
   int cycles = 11;
   KWBoolean firstPass = KWTrue;
   int dst = 0;
   KWBoolean daylightSavingsInEffect;
   char sync = '?';
   int errors = 0;

   time_t oldClock;
   time_t newClock;
   time_t nistClock;

   setTitle("Determining current time");

   memset( &tx , '\0', sizeof tx);  /* Clear all time info           */
   if (!expectstr("MJD", 10, NULL )) /* Margaret Jane Derbyshire? :-) */
   {
      printmsg(0,"nbstime: Did not find NIST literal in data from remote");
      return KWFalse;
   }

/*--------------------------------------------------------------------*/
/*                  Begin main loop to get the time                   */
/*--------------------------------------------------------------------*/

   while ((sync != '#') && cycles-- && ! terminate_processing )
   {
      int column;
      KWBoolean error = KWFalse;

/*--------------------------------------------------------------------*/
/*       Read and verify entire contents of the buffer.               */
/*--------------------------------------------------------------------*/

      for ( column = 0;
            (column < (sizeof model - 1)) && ! error;
            column++ )
      {
         if (sread(buf + column, 1, 2) < 1)
         {
            printmsg(0,"nbstime: Timeout reading character");
            return KWFalse;
         }

         swrite( buf + column, 1 ); /* Echo character to time server */

         switch( model[column] )
         {
            case '?':               /* Any character, ignore it      */
               break;

            case '+':               /* Sign (+ or -) character?      */
               if (( buf[column] != '-') && ( buf[column] != '+'))
                  error = KWTrue;   /* Bad character, throw buf away */
               break;

            case '#':               /* Numeric digit expected?       */
               if (!isdigit(buf[column] ))
                  error = KWTrue;   /* Bad character, throw buf away */
               break;

            case '*':               /* Synchronize flag?             */
               if ( buf[column] == '#' ) /* Sync char?               */
                  sync = buf[column];    /* Yes --> We're all done   */
               else if ( buf[column] != '*' )
                                    /* Correct but not sync?         */
                  error = KWTrue;   /* No --> Bad character          */
               break;

            default:                /* Perfect match required        */
               if (model[column] != buf[column] )
                  error = KWTrue;   /* Bad character, throw buf away */
               break;

         } /* switch( model[column] ) */

      } /* for */

      if ( ! *buf )                 /* Empty line?                   */
         continue;                  /* Yes --> Ignore it             */

/*--------------------------------------------------------------------*/
/*               Common processing for error conditions               */
/*--------------------------------------------------------------------*/

      if ( error )                  /* Found a bad character?        */
      {
         if ( firstPass )           /* Getting in sync with remote?  */
            firstPass = KWFalse;    /* Yes --> Don't flag as error   */
         else {
            if ( debuglevel > 2 )
               printmsg(3,
                        "nbstime: Buffer error at column %d: \"%s\"",
                        column,
                        buf );
            errors++;
         }

/*--------------------------------------------------------------------*/
/*                     Flush the rest of the line                     */
/*--------------------------------------------------------------------*/

         column = 0;             /* Read up to one full line of data */

         while( (sread(buf, 1, 1) >= 1) && (column++ <= sizeof model ))
         {
            swrite(buf, 1);

            if ( *buf == model[ sizeof model - 2 ] )
               break;

         } /* while */

         *buf = '\0';            /* Discard our invalid buffer       */

      } /* if ( error ) */
      else
         buf[ sizeof model - 1 ] = '\0';  /* Terminate string        */

   } /* while */

/*--------------------------------------------------------------------*/
/*                    Verify we got a valid buffer                    */
/*--------------------------------------------------------------------*/

   if ( ! *buf )
   {
      printmsg(0,"nbstime: Buffer reads failed with "
                 "%d errors%s",
                 errors,
                 debuglevel < 3 ?
                     ", rerun with debug -x 3 for details" :
                     "");
      return KWFalse;
   }

   time(&oldClock);                 /* Remember time before setting  */

/*--------------------------------------------------------------------*/
/*                   Determine the time we received                   */
/*--------------------------------------------------------------------*/

   sscanf(buf,"%*s %d-%d-%d %d:%d:%d %d ",
         &tx.tm_year, &tx.tm_mon, &tx.tm_mday ,
         &tx.tm_hour, &tx.tm_min, &tx.tm_sec, &dst);

   tx.tm_mon--;                     /* tm record counts from zero    */

/*--------------------------------------------------------------------*/
/*       Perform a sanity check; the time must be 25 years past       */
/*       1970 and not exactly midnight GMT.                           */
/*--------------------------------------------------------------------*/

   if (( nistClock < (25L * 365L * 86400L)) &&
       !( tx.tm_hour || tx.tm_min || tx.tm_sec ))
   {
      printmsg(0,"nbstime: Time warp error (%.24s), clock not set",
                  ctime( &nistClock ));
      return KWFalse;
   }

#if defined(WIN32)

/*--------------------------------------------------------------------*/
/*       mktime()'s "renormalizing" the tm struct confuses NT, so     */
/*       set the clock first under NT.                                */
/*--------------------------------------------------------------------*/

   newClock = setClock( &tx );

   if ( ! newClock )                /* Did clock set fail?           */
      return KWFalse;               /* Yes --> Return failure        */

#endif

   nistClock = mktime(&tx);         /* Current UTC (GMT) time in sec */

/*--------------------------------------------------------------------*/
/*       Determine if US Daylight Savings Time is in effect           */
/*                                                                    */
/*       This will be wrong before 0000 GMT on the day clocks are     */
/*       changed forward/back for time zones more than two/one        */
/*       hours east of Greenwich Mean Time (i.e.  Asia and much of    */
/*       Europe).  However, the NIST DST flag is only meaningful      */
/*       for US time zones anyway.                                    */
/*--------------------------------------------------------------------*/

   switch( dst )
   {
      case 0:
         daylightSavingsInEffect = KWFalse;
         break;

      case 50:
         daylightSavingsInEffect = KWTrue;
         break;

      case 51:
         if ( (tx.tm_hour - timezone() / 3600 ) >= 2 )
            daylightSavingsInEffect = KWTrue;
         else
            daylightSavingsInEffect = KWFalse;

#ifdef UDEBUG
         printmsg(3,"nbstime: Hour = %d, DST = %sabled" ,
                     tx.tm_hour - timezone() / 3600,
                     daylightSavingsInEffect ? "en" : "dis" );
#endif
         break;

      case 1:
         if ( (tx.tm_hour - timezone() / 3600 ) >= 1 )
            daylightSavingsInEffect = KWFalse;
         else
            daylightSavingsInEffect = KWTrue;

#ifdef UDEBUG
         printmsg(3,"nbstime: Hour = %d, DST = %sabled" ,
                     tx.tm_hour - timezone() / 3600,
                     daylightSavingsInEffect ? "en" : "dis" );
#endif
         break;

      default:
         if ( dst > 50 )
            daylightSavingsInEffect = KWFalse;
         else
            daylightSavingsInEffect = KWTrue;
         break;

   } /* switch( dst ) */

/*--------------------------------------------------------------------*/
/*                       Report our computed time                     */
/*--------------------------------------------------------------------*/

   if ( debuglevel > 2 )
   {
      printmsg(3,"%02d/%02d/%02d %02d:%02d:%02d %02d %c "
                 "translates to %ld or %.24s",
                  tx.tm_year,
                  tx.tm_mon + 1,
                  tx.tm_mday,
                  tx.tm_hour,
                  tx.tm_min,
                  tx.tm_sec,
                  dst,
                  sync ,
                  nistClock,
                  ctime( &nistClock ));
   }

/*--------------------------------------------------------------------*/
/*                        Set the system clock                        */
/*--------------------------------------------------------------------*/

#ifndef WIN32
   newClock = setClock( nistClock, daylightSavingsInEffect );
#endif

/*--------------------------------------------------------------------*/
/*       Print the raw input buffer, if desired, and always print     */
/*       the new system time.                                         */
/*--------------------------------------------------------------------*/

   printmsg(3, "nbstime: \"%s\"", buf + 2);

/*--------------------------------------------------------------------*/
/*                Report previous offset of clock to NIST             */
/*--------------------------------------------------------------------*/

   if ( oldClock > newClock )
      printmsg(1,"nbstime: Local system clock was %ld seconds fast.",
              (long) (oldClock - newClock) );
   else if ( oldClock < newClock )
      printmsg(1,"nbstime: Local system clock was %ld seconds slow.",
              (long) (newClock - oldClock) );
   else
      printmsg(1, "nbstime: Local system clock was already correct.");

   if (sync == '#')
      printmsg(1, "Note: Modem was able to synchronize with NIST." );

   printmsg(0, "nbstime: New system time is %s", arpadate() );

/*--------------------------------------------------------------------*/
/*                       Return success to caller                     */
/*--------------------------------------------------------------------*/

   return KWTrue;

} /* nbstime */
