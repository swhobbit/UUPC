/*--------------------------------------------------------------------*/
/*    d a t e r . c                                                   */
/*                                                                    */
/*    Date formatting routines for UUPC/extended                      */
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
 *    $Id: dater.c 1.2 1993/10/09 15:46:15 rhg Exp $
 *
 *    Revision history:
 *    $Log: dater.c $
 *     Revision 1.2  1993/10/09  15:46:15  rhg
 *     ANSIify the source
 *
 *     Revision 1.2  1993/10/09  15:46:15  rhg
 *     ANSIify the source
 *
 */

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
   static time_t last = (time_t) -1L;

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
