/*--------------------------------------------------------------------*/
/*       f o s s i l . c                                              */
/*                                                                    */
/*       UUPC/extended defines for accessing FOSSIL functions via     */
/*       INT 14 under MS-DOS; also used for other INT14 interfaces.   */
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
 *    $Id: fossil.c 1.14 1999/01/08 02:20:56 ahd Exp $
 *
 *    Revision history:
 *    $Log: fossil.c $
 *    Revision 1.14  1999/01/08 02:20:56  ahd
 *    Convert currentfile() to RCSID()
 *
 *    Revision 1.13  1999/01/04 03:53:57  ahd
 *    Annual copyright change
 *
 *    Revision 1.12  1998/03/01 01:39:50  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.11  1997/04/24 01:34:16  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.10  1996/01/20 12:53:59  ahd
 *    Add debugging output for FOSSIL driver information retrieve
 *
 *    Revision 1.9  1996/01/01 21:21:42  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.8  1994/12/22 00:35:31  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.7  1994/02/19 05:08:28  ahd
 *    Use standard first header
 *
 * Revision 1.6  1994/01/24  03:04:32  ahd
 * Annual Copyright Update
 *
 * Revision 1.5  1994/01/01  19:20:01  ahd
 * Annual Copyright Update
 *
 * Revision 1.4  1993/11/06  17:56:09  rhg
 * Drive Drew nuts by submitting cosmetic changes mixed in with bug fixes
 *
 * Revision 1.4  1993/11/06  17:56:09  rhg
 * Drive Drew nuts by submitting cosmetic changes mixed in with bug fixes
 *
 * Revision 1.3  1993/10/12  01:33:59  ahd
 * Normalize comments to PL/I style
 *
 * Revision 1.2  1993/10/09  22:21:55  rhg
 * ANSIfy source
 *
 * Revision 1.1  1993/05/30  15:25:50  ahd
 * Initial revision
 *
 * Revision 1.1  1993/05/30  00:16:35  ahd
 * Initial revision
 *
 */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"

#include <dos.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "fossil.h"

RCSID("$Id: fossil.c 1.14 1999/01/08 02:20:56 ahd Exp $");

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

short portNum;                      /* Must be set by openline()     */

#ifdef UDEBUG

/*--------------------------------------------------------------------*/
/*       f o s s i l I n f o T r a c e                                */
/*                                                                    */
/*       Trace a fossil information buffer                            */
/*--------------------------------------------------------------------*/

void fossilInfoTrace( const char *prefix, const FS_INFO *debug )
{
   char buf[ sizeof (FS_INFO) * 2 + 1 ];
   char *data = (char *) debug;
   size_t subscript;

   for ( subscript = 0; subscript < sizeof (FS_INFO); subscript ++ )
   {
      sprintf( buf + (subscript * 2 ), "%2.2x", data[subscript] );
   } /* for */

   printmsg(5, "fosssilInfoTrace: %s: %d bytes at %p: %s",
               prefix,
               sizeof (FS_INFO),
               debug,
               buf );

} /* fossilInfoTrace */

#endif

/*--------------------------------------------------------------------*/
/*       F o s s i l C n t l                                          */
/*                                                                    */
/*       Perform control functions on a FOSSIL port                   */
/*--------------------------------------------------------------------*/

 short FossilCntl( const char function, const unsigned char info )
 {
   union REGS regsIn;
   union REGS regsOut;

   regsIn.x.dx = portNum;
   regsIn.h.ah = function;
   regsIn.h.al = info;
   regsIn.x.bx = 0x00;          /* Insure start function doesn't        */
                              /* cause ES:CX to be twiddled           */
   int86( FS_INTERRUPT, &regsIn, &regsOut);

   return (short) regsOut.x.ax;  /* Return AX to caller               */

 } /* FossilCntl */
