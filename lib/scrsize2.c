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
 *    $Id$
 *
 *    $Log$
 */


/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include <stdio.h>

#define INCL_SUB
#define INCL_NOPM
#include <os2.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
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
