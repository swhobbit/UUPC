/*--------------------------------------------------------------------*/
/*    s c r s i z e 2 .  c                                            */
/*                                                                    */
/*    Report screen size under OS/2                                   */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-2000 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: scrsize2.c 1.11 1999/01/08 02:20:48 ahd Exp $
 *
 *    $Log: scrsize2.c $
 *    Revision 1.11  1999/01/08 02:20:48  ahd
 *    Convert currentfile() to RCSID()
 *
 *    Revision 1.10  1999/01/04 03:52:55  ahd
 *    Annual copyright change
 *
 *    Revision 1.9  1998/03/01 01:25:15  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.8  1997/03/31 07:06:25  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.7  1996/01/01 20:51:37  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.6  1995/01/29 16:43:03  ahd
 *    IBM C/Set compiler warnings
 *
 *    Revision 1.5  1995/01/07 16:14:22  ahd
 *    Change KWBoolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.4  1994/02/19 04:46:26  ahd
 *    Use standard first header
 *
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

RCSID("$Id: scrsize2.c 1.11 1999/01/08 02:20:48 ahd Exp $");

/*--------------------------------------------------------------------*/
/*    s c r s i z e                                                   */
/*                                                                    */
/*    Return screen size under OS/2                                   */
/*--------------------------------------------------------------------*/

unsigned short scrsize( void )
{
   VIOMODEINFO info;
   USHORT result;
   static KWBoolean error = KWFalse;

   if ( error )
      return PAGESIZE;

   info.cb = sizeof info;
   result = VioGetMode( &info, (HVIO) NULL );

   if ( result != 0 )
   {
      printmsg(0,"OS/2 error code %d retrieving video information",
               (int) result );
      error = KWTrue;
      return PAGESIZE;
   }

   return info.row;

} /* scrsize */
