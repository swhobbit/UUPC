/*
   arpadate - return the current date/time in RFC 822 format

   ctime() format 'Mon Nov 21 11:31:54 1983\n\0'

   RFC822 format  'Mon, 21 Nov 1983 11:31:54 PST\0' or
   RFC822 format  'Mon, 16 May 1988 02:13:10 -0700\0'
*/

#include <stdio.h>
#include <time.h>

#include "lib.h"
#include "arpadate.h"

char *arpadate()
{
   static char adate[40];
   time_t t;
   static time_t save_t = 0;

/*--------------------------------------------------------------------*/
/*       Microsoft and Turbo don't agree on timezone (%z or %Z)       */
/*--------------------------------------------------------------------*/

#ifdef __TURBOC__
   static char format[] = "%a, %d %b %Y %H:%M:%S %Z";
#else
   static char format[] = "%a, %d %b %Y %H:%M:%S %z";
#endif

/*--------------------------------------------------------------------*/
/*     Return previously formatted buffer if time has not changed     */
/*--------------------------------------------------------------------*/

   time( &t );
   if ( save_t == t)
      return adate;

/*--------------------------------------------------------------------*/
/*                 Format the new time and return it                  */
/*--------------------------------------------------------------------*/

   strftime( adate , sizeof( adate ) , format ,  localtime( &t ));
   printmsg(5, "date=%s", adate);
   save_t = t;
   return adate;

} /*arpadate*/
