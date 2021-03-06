/*--------------------------------------------------------------------*/
/*      d o s 2 u n i x . c                                           */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989 by Andrew H. Derbyshire.             */
/*                                                                    */
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
 *    $Id: dos2unix.c 1.21 2001/03/12 13:52:56 ahd v1-13k $
 *
 *    Revision history:
 *    $Log: dos2unix.c $
 *    Revision 1.21  2001/03/12 13:52:56  ahd
 *    Annual copyright update
 *
 *    Revision 1.20  2000/05/12 12:29:45  ahd
 *    Annual copyright update
 *
 *    Revision 1.19  1999/01/08 02:20:43  ahd
 *    Convert currentfile() to RCSID()
 *
 *    Revision 1.18  1999/01/04 03:52:28  ahd
 *    Annual copyright change
 *
 *    Revision 1.17  1998/03/01 01:23:24  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.16  1997/03/31 06:58:43  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.15  1996/01/01 20:50:43  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.14  1995/09/24 19:07:05  ahd
 *    Correct for 16 bit OS/2 compile
 *
 *    Revision 1.13  1995/09/04 19:37:49  dmwatt
 *    Correct time zone conversion when converting NT timestamp to UNIX format
 *
 *    Revision 1.12  1995/07/21 13:23:19  ahd
 *    Clean up OS/2 compiler warnings
 *
 *    Revision 1.11  1994/12/22 00:08:01  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.10  1994/02/20 19:07:38  ahd
 *    IBM C/Set 2 Conversion, memory leak cleanup
 *
 *     Revision 1.9  1994/02/19  04:40:35  ahd
 *     Use standard first header
 *
 *     Revision 1.8  1994/02/19  03:49:16  ahd
 *     Use standard first header
 *
 *     Revision 1.8  1994/02/19  03:49:16  ahd
 *     Use standard first header
 *
 *     Revision 1.7  1994/02/18  23:08:41  ahd
 *     Use standard first header
 *
 *     Revision 1.6  1994/01/24  03:07:23  ahd
 *     Annual Copyright Update
 *
 *     Revision 1.5  1994/01/01  19:01:18  ahd
 *     Annual Copyright Update
 *
 *     Revision 1.4  1993/04/10  21:22:29  dmwatt
 *     Windows/NT fixes
 *
 * Revision 1.4  1993/04/10  21:22:29  dmwatt
 * Windows/NT fixes
 *
 * Revision 1.3  1993/04/05  12:26:01  ahd
 * Correct headers to match gpkt
 *
 * Revision 1.2  1993/04/05  04:32:19  ahd
 * Add timestamp, size to information returned by directory searches
 *
 * Revision 1.1  1993/04/04  19:35:14  ahd
 * Initial revision
 *
 */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"

/*--------------------------------------------------------------------*/
/*                      Windows/NT include files                      */
/*--------------------------------------------------------------------*/

#ifdef WIN32
#include <windows.h>
#endif

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "dos2unix.h"

RCSID("$Id: dos2unix.c 1.21 2001/03/12 13:52:56 ahd v1-13k $");

/*--------------------------------------------------------------------*/
/*       d o s 2 u n i x                                              */
/*                                                                    */
/*       Convert a DOS file time stamp to unix t_time                 */
/*--------------------------------------------------------------------*/

time_t dos2unix( const FDATE ddmmyy,
                 const FTIME ssmmhh )
{

   struct tm  time_record;

   time_record.tm_sec = ssmmhh.twosecs * 2;
   time_record.tm_min = ssmmhh.minutes;
   time_record.tm_hour= ssmmhh.hours;

   time_record.tm_mday = ddmmyy.day;
   time_record.tm_mon  = ddmmyy.month - 1;
   time_record.tm_year = 80 + ddmmyy.year;

   time_record.tm_isdst = -1;

   return mktime(&time_record);

} /* dos2unix */

#ifdef WIN32

/*--------------------------------------------------------------------*/
/*       n t 2 u n i x                                              */
/*                                                                    */
/*       Convert an NT file time stamp to unix t_time                 */
/*--------------------------------------------------------------------*/

time_t nt2unix( FILETIME *nsec )
{
   SYSTEMTIME sysTime;
   FILETIME localtime;
   struct tm  time_record;

   FileTimeToLocalFileTime(nsec, &localtime);
   FileTimeToSystemTime(&localtime, &sysTime);

   time_record.tm_sec = sysTime.wSecond;
   time_record.tm_min = sysTime.wMinute;
   time_record.tm_hour= sysTime.wHour;

#ifdef UDEBUG
   printmsg(5,"nt2unix: Time stamp is %02d:%02d:02d",
               (int) sysTime.wSecond,
               (int) sysTime.wMinute,
               (int) sysTime.wHour );
#endif

   time_record.tm_mday = sysTime.wDay;
   time_record.tm_mon  = sysTime.wMonth - 1;
   time_record.tm_year = sysTime.wYear - 1900;

   time_record.tm_isdst = -1;

   return mktime(&time_record);

} /* nt2unix */
#endif
