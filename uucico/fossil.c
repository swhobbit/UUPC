/*--------------------------------------------------------------------*/
/*       f o s s i l . c                                              */
/*                                                                    */
/*       UUPC/extended defines for accessing FOSSIL functions via     */
/*       INT 14 under MS-DOS; also used for other INT14 interfaces.   */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1996 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: fossil.c 1.8 1994/12/22 00:35:31 ahd v1-12q $
 *
 *    Revision history:
 *    $Log: fossil.c $
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

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

short portNum;                      /* Must be set by openline()     */

/*--------------------------------------------------------------------*/
/*       F o s s i l C n t l                                          */
/*                                                                    */
/*       Perform control functions on a FOSSIL port                   */
/*--------------------------------------------------------------------*/

 short FossilCntl( const char function, const unsigned char info )
 {
   union REGS regs;

   regs.x.dx = portNum;
   regs.h.ah = function;
   regs.h.al = info;
   regs.x.bx = 0x00;          /* Insure start function doesn't        */
                              /* cause ES:CX to be twiddled           */
   int86( FS_INTERRUPT, &regs, &regs );

   return (short) regs.x.ax;  /* Return AX to caller                  */

 } /* FossilCntl */
