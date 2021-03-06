/*--------------------------------------------------------------------*/
/*    d a t e r . c                                                   */
/*                                                                    */
/*    Date formatting routines for UUPC/extended                      */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-2002 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: dater.c 1.14 2001/03/12 13:52:56 ahd v1-13k $
 *
 *    Revision history:
 *    $Log: dater.c $
 *    Revision 1.14  2001/03/12 13:52:56  ahd
 *    Annual copyright update
 *
 *    Revision 1.13  2000/05/12 12:29:45  ahd
 *    Annual copyright update
 *
 *    Revision 1.12  1999/01/08 02:20:43  ahd
 *    Convert currentfile() to RCSID()
 *
 *    Revision 1.11  1999/01/04 03:52:28  ahd
 *    Annual copyright change
 *
 *    Revision 1.10  1998/03/01 01:23:21  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.9  1997/03/31 06:58:39  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.8  1996/01/01 20:50:36  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.7  1994/12/22 00:07:53  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.6  1994/02/19 04:40:26  ahd
 *    Use standard first header
 *
 *     Revision 1.5  1994/02/19  03:49:03  ahd
 *     Use standard first header
 *
 *     Revision 1.5  1994/02/19  03:49:03  ahd
 *     Use standard first header
 *
 *     Revision 1.4  1994/02/18  23:08:33  ahd
 *     Use standard first header
 *
 *     Revision 1.3  1994/01/01  19:01:09  ahd
 *     Annual Copyright Update
 *
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

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"

#include "dater.h"

RCSID("$Id: dater.c 1.14 2001/03/12 13:52:56 ahd v1-13k $");

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
