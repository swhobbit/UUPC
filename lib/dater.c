/*--------------------------------------------------------------------*/
/*    d a t e r . c                                                   */
/*                                                                    */
/*    Date formatting routines for UUPC/extended                      */
/*                                                                    */
/*    Copyright (c) 1991, Andrew H. Derbyshire                        */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <time.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "dater.h"

/*--------------------------------------------------------------------*/
/*    d a t e r                                                       */
/*                                                                    */
/*    Format the date and time as mm/dd-hh:mm                         */
/*--------------------------------------------------------------------*/

char *dater( const time_t t , char *buf)
{                              /* ....+....1. + 1 to terminate */
   static char format[DATEBUF] = "%m/%d-%H:%M";
   static char mybuf[DATEBUF]  = "           ";
   static char sabuf[DATEBUF]  = "           ";
   static char never[DATEBUF]  = "  (never)  ";
   static char missing[DATEBUF]= " (missing) ";
   static time_t last = -1;

   if ( buf == NULL )
      buf = mybuf;

   if ( t == 0 )
      strcpy( buf, never);
   else if ( t == -1 )
      strcpy( buf, missing );
   else {
      time_t now = t / 60;
      if ( last != now )
      {
         strftime( sabuf, sizeof( format ) , format ,  localtime( &t ));
         last = now;
      }
      strcpy( buf, sabuf );
   } /* else */

   return buf;
} /* dater */
