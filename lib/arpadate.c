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
/*       Changes Copyright (c) 1989-1998 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: arpadate.c 1.17 1997/12/20 22:26:06 ahd Exp $
 *
 *    Revision history:
 *    $Log: arpadate.c $
 *    Revision 1.17  1997/12/20 22:26:06  ahd
 *    Support whacko compilers that think time_t is unsigned
 *
 *    Revision 1.16  1997/03/31 06:58:04  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.15  1996/01/01 20:49:17  ahd
 *    Annual Copyright Update
 *
 */

#include "uupcmoah.h"
#include "arpadate.h"

RCSID("$Id: arpadate.c 1.17 1997/12/20 22:26:06 ahd Exp $");

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
      sprintf(zone, " %+03ld00", (((long) t) - (long) ut) / 3600);
   }

/*--------------------------------------------------------------------*/
/*                 Format the new time and return it                  */
/*--------------------------------------------------------------------*/

   strftime(adate, sizeof(adate), format, &lt);
   strcat(adate, zone);

   return adate;

} /*arpadate*/
