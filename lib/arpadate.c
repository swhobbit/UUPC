/*--------------------------------------------------------------------*/
/*    a r p a d a t e                                                 */
/*                                                                    */
/*    Return the current date/time in RFC 822 format                  */
/*                                                                    */
/*    ctime() format 'Mon Nov 21 11:31:54 1983\n\0'                   */
/*                                                                    */
/*    RFC822 format  'Mon, 21 Nov 1983 11:31:54 PST\0' or             */
/*    RFC822 format  'Mon, 16 May 1988 02:13:10 -0700\0'              */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1994 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: arpadate.c 1.8 1994/01/01 19:00:21 ahd Exp $
 *
 *    Revision history:
 *    $Log: arpadate.c $
 *     Revision 1.8  1994/01/01  19:00:21  ahd
 *     Annual Copyright Update
 *
 *     Revision 1.7  1993/11/30  04:18:14  ahd
 *     Correct for TZ computations crossing day boundaries
 *
 *     Revision 1.7  1993/11/30  04:18:14  ahd
 *     Correct for TZ computations crossing day boundaries
 *
 *     Revision 1.6  1993/11/15  05:43:29  ahd
 *     Twiddle, twiddle ...
 *
 *     Revision 1.5  1993/11/13  17:37:02  ahd
 *     More date twiddling
 *
 *     Revision 1.4  1993/11/06  16:56:13  ahd
 *     Use localtime to determine tz offset
 *
 *     Revision 1.3  1993/10/28  00:18:10  ahd
 *     Apply Kai Uwe Rommel's fix to use TZ offset in hours rather than
 *     time zone
 *
 */

#include <stdio.h>
#include <time.h>
#include <string.h>

#include "lib.h"
#include "arpadate.h"

currentfile();

/*--------------------------------------------------------------------*/
/*       a r p a d a t e                                              */
/*                                                                    */
/*       Format current time into RFC-822 date                        */
/*--------------------------------------------------------------------*/

char *arpadate( void )
{
   static char format[] = "%a, %d %b %Y %H:%M:%S";
   static char adate[64], zone[32];
   time_t t, ut;
   struct tm lt, gm;

   time(&t);
   lt = *localtime(&t);

/*--------------------------------------------------------------------*/
/*       Make time zone name, if necessary                            */
/*--------------------------------------------------------------------*/

   if (zone[0] == 0)              /* adjust for daylight savings time */
            {
      gm = *gmtime(&t);
      ut = mktime(&gm);
      sprintf(zone, " %+03d00", (t - ut) / 3600);
   }

/*--------------------------------------------------------------------*/
/*                 Format the new time and return it                  */
/*--------------------------------------------------------------------*/

   strftime(adate, sizeof(adate), format, &lt);
   strcat(adate, zone);

   return adate;

} /*arpadate*/
