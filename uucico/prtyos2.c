/*--------------------------------------------------------------------*/
/*       p r t y o s 2 . c                                            */
/*                                                                    */
/*       Set task priority for OS/2 tasks under UUPC/extended         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-2000 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: prtyos2.c 1.13 1999/01/08 02:20:52 ahd Exp $
 *
 *    Revision history:
 *    $Log: prtyos2.c $
 *    Revision 1.13  1999/01/08 02:20:52  ahd
 *    Convert currentfile() to RCSID()
 *
 *    Revision 1.12  1999/01/04 03:53:30  ahd
 *    Annual copyright change
 *
 *    Revision 1.11  1998/03/01 01:40:04  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.10  1997/04/24 01:34:38  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.9  1996/01/01 21:21:59  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.8  1995/01/07 16:39:25  ahd
 *    Change boolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.7  1994/12/22 00:35:49  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.6  1994/02/19 05:09:35  ahd
 *    Use standard first header
 *
 * Revision 1.5  1994/01/01  19:20:33  ahd
 * Annual Copyright Update
 *
 * Revision 1.4  1993/10/12  01:33:23  ahd
 * Normalize comments to PL/I style
 *
 * Revision 1.4  1993/10/12  01:33:23  ahd
 * Normalize comments to PL/I style
 *
 * Revision 1.3  1993/10/03  22:34:33  ahd
 * Alter format of numbers printed
 *
 * Revision 1.2  1993/09/29  04:52:03  ahd
 * Pass priority values as parameters
 *
 * Revision 1.1  1993/09/25  03:07:56  ahd
 * Initial revision
 *
 */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#define INCL_NOPMAPI
#define INCL_BASE
#include "uupcmoah.h"

#include <os2.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "pos2err.h"

/*--------------------------------------------------------------------*/
/*                          Local variables                           */
/*--------------------------------------------------------------------*/

RCSID("$Id: prtyos2.c 1.13 1999/01/08 02:20:52 ahd Exp $");

#ifndef __OS2__
typedef USHORT APIRET ;  /* Define older API return type              */
#endif

#ifdef __OS2__
static ULONG usPrevPriority;
#else
static USHORT usPrevPriority;
#endif

static KWBoolean restore = KWFalse;

/*--------------------------------------------------------------------*/
/*       s e t P r t y                                                */
/*                                                                    */
/*       Set priority to configuration defined value                  */
/*--------------------------------------------------------------------*/

void setPrty( const KEWSHORT priorityIn, const KEWSHORT prioritydeltaIn )
{
   USHORT priority = (priorityIn == 999) ?
                           PRTYC_FOREGROUNDSERVER : (USHORT) priorityIn;
   USHORT prioritydelta = (prioritydeltaIn == 999) ?
                           0 : (USHORT) (prioritydeltaIn + PRTYD_MINIMUM);

   APIRET rc;

#ifdef __OS2__

   PTIB ptib;
   PPIB ppib;

   rc = DosGetInfoBlocks( &ptib, &ppib);
   if ( !rc )
      usPrevPriority = (ptib->tib_ptib2)->tib2_ulpri;
#else
   rc = DosGetPrty(PRTYS_PROCESS, &usPrevPriority, 0);
#endif

   if (rc)
   {
      printOS2error( "DosGetPrty", rc );
      panic();
   } /*if */
   else
      restore = KWTrue;

   rc = DosSetPrty(PRTYS_PROCESS, priority, prioritydelta, 0);

   if (rc)
   {
      printmsg(0,"setPrty: Unable to set priority %hu,%hu for task",
                   priority, prioritydelta);
      printOS2error( "DosSetPrty", rc );

   } /*if */

} /* SetPrty */

/*--------------------------------------------------------------------*/
/*       r e s e t P r t y                                            */
/*                                                                    */
/*       Restore priority saved by SetPrty                            */
/*--------------------------------------------------------------------*/

void resetPrty( void )
{

   APIRET rc;

   if ( !restore )
      return;

   rc = DosSetPrty(PRTYS_PROCESS,
                   usPrevPriority >> 8 ,
                   usPrevPriority & 0xff, 0);

   if (rc)
      printOS2error( "DosSetPrty", rc );

   restore = KWFalse;

} /* resetPrty */
