/*--------------------------------------------------------------------*/
/*    s c r s i z e 2 .  c                                            */
/*                                                                    */
/*    Report screen size under OS/2                                   */
/*                                                                    */
/*    Copyright (c) 1992 by Kendra Electronic Wonderworks.            */
/*    All rights reserved except those explicitly granted by          */
/*    the UUPC/extended license.                                      */
/*--------------------------------------------------------------------*/

/*
 *    $Id: scrsize2.c 1.3 1994/02/19 03:57:54 ahd Exp $
 *
 *    $Log: scrsize2.c $
 *     Revision 1.3  1994/02/19  03:57:54  ahd
 *     Use standard first header
 *
 *     Revision 1.3  1994/02/19  03:57:54  ahd
 *     Use standard first header
 *
 *     Revision 1.2  1994/02/18  23:14:45  ahd
 *     Use standard first header
 *
 *     Revision 1.1  1992/11/27  14:36:10  ahd
 *     Initial revision
 *
 * Revision 1.1  1992/11/27  14:36:10  ahd
 * Initial revision
 *
 */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#define INCL_SUB
#define INCL_NOPM

#include "uupcmoah.h"

#include <os2.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "scrsize.h"

/*--------------------------------------------------------------------*/
/*    s c r s i z e                                                   */
/*                                                                    */
/*    Return screen size under OS/2                                   */
/*--------------------------------------------------------------------*/

short scrsize( void )
{
   VIOMODEINFO info;
   USHORT result;
   static boolean error = FALSE;

   if ( error )
      return PAGESIZE;

   info.cb = sizeof info;
   result = VioGetMode( &info, (HVIO) NULL );

   if ( result != 0 )
   {
      printmsg(0,"OS/2 error code %d retrieving video information",
               (int) result );
      error = TRUE;
      return PAGESIZE;
   }

   return info.row;
} /* scrsize */
