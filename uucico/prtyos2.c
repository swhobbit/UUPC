/*--------------------------------------------------------------------*/
/*       p r t y o s 2 . c                                            */
/*                                                                    */
/*       Set task priority for OS/2 tasks under UUPC/extended         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1993 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: prtyos2.c 1.1 1993/09/25 03:07:56 ahd Exp $
 *
 *    Revision history:
 *    $Log: prtyos2.c $
 * Revision 1.1  1993/09/25  03:07:56  ahd
 * Initial revision
 *
 */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include <stdio.h>

#define INCL_NOPMAPI
#define INCL_BASE
#include <os2.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "lib.h"
#include "pos2err.h"

/*--------------------------------------------------------------------*/
/*                          Local variables                           */
/*--------------------------------------------------------------------*/

currentfile();

#ifndef __OS2__
typedef USHORT APIRET ;  // Define older API return type
#endif

#ifdef __OS2__
static ULONG usPrevPriority;
#else
static USHORT usPrevPriority;
#endif

static boolean restore = FALSE;

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
      restore = TRUE;

   rc = DosSetPrty(PRTYS_PROCESS, priority, prioritydelta, 0);

   if (rc)
   {
      printmsg(0,"setPrty: Unable to set priority %u,%u for task",
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

   restore = FALSE;

} /* resetPrty */
