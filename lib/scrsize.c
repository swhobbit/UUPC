/*--------------------------------------------------------------------*/
/*    s c r s i z e .  c                                              */
/*                                                                    */
/*    Report screen size under MS-DOS                                 */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1995 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: scrsize.c 1.13 1995/01/07 16:14:16 ahd Exp $
 *
 *    $Log: scrsize.c $
 *    Revision 1.13  1995/01/07 16:14:16  ahd
 *    Change KWBoolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.12  1994/02/19 04:46:18  ahd
 *    Use standard first header
 *
 *     Revision 1.11  1994/02/19  04:11:25  ahd
 *     Use standard first header
 *
 *     Revision 1.10  1994/02/19  03:57:42  ahd
 *     Use standard first header
 *
 *     Revision 1.9  1994/02/18  23:14:34  ahd
 *     Use standard first header
 *
 *     Revision 1.8  1993/10/12  00:45:27  ahd
 *     Normalize comments
 *
 *     Revision 1.7  1993/10/09  15:46:15  rhg
 *     ANSIify the source
 *
 *     Revision 1.6  1993/05/03  02:41:57  ahd
 *     Trap funky screen size on pre-DOS 4.0 boxes (poor babies...)
 *
 *     Revision 1.5  1992/12/30  05:27:11  plummer
 *     MS C compile fixes
 *
 * Revision 1.4  1992/12/18  12:05:57  ahd
 * Fix query for ANSI sys
 *
 * Revision 1.3  1992/12/11  12:45:11  ahd
 * Use BIOS values if no ANSI driver
 *
 * Revision 1.2  1992/11/29  22:09:10  ahd
 * Add stdlib.h for _osmajor under MSC
 *
 * Revision 1.1  1992/11/27  14:36:10  ahd
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

#include "scrsize.h"

/*--------------------------------------------------------------------*/
/*    s c r s i z e                                                   */
/*                                                                    */
/*    Return screen size under MS-DOS 4.0 and 5.0                     */
/*--------------------------------------------------------------------*/

unsigned short scrsize( void )
{

#ifdef __TURBOC__
   static unsigned char far *bios_rows = MK_FP( 0x0040, 0x0084 );
/* static unsigned char far *bios_cols = MK_FP( 0x40, 0x4a ); */
#else
   static unsigned char far *bios_rows = (unsigned char far *) 0x0484L;
#endif

   static KWBoolean error = KWFalse;
   static short default_rows = 0;

   typedef struct _DISPLAYMODE   /* Page 310 MS-DOS 5.0 PGMR Reference */
   {
      char  dmInfoLevel;
      char  dmReserved1;
      short dmDataLength;
      short dmFlags;
      char  dmMode;
      char  dmReserved2;
      short dmColors;
      short dmWidth;
      short dmLength;
      short dmColumns;
      short dmRows;

   } DISPLAYMODE;

   DISPLAYMODE info;

   union REGS regs;

/*--------------------------------------------------------------------*/
/*            If an old version of DOS, return stock size             */
/*--------------------------------------------------------------------*/

   if ((*bios_rows < 20 ) || (*bios_rows > 99)) /* Sanity check   */
      default_rows = PAGESIZE;

   if ((_osmajor < 4) || error )
      return (unsigned short) (default_rows ? default_rows : *bios_rows);
                                 /* Faster, but not well documented   */

/*--------------------------------------------------------------------*/
/*             Fill in information to perform processing              */
/*--------------------------------------------------------------------*/

   info.dmInfoLevel   = 0;       /* Magic number in book          */
   info.dmReserved1   = 0;       /* Magic number in book          */
   info.dmReserved2   = 0;       /* Magic number in book          */
   info.dmDataLength  = 14;      /* Magic number in book          */

   regs.x.bx = 0x0001;           /* STDOUT file handle            */
   regs.h.ch = 0x03;             /* Screen device category        */
   regs.h.cl = 0x7f;             /* Get display mode              */
   regs.x.ax = 0x440c;           /* Video Status                  */
   regs.x.dx = (short) &info;    /* Address of structure          */

   intdos(&regs, &regs );

/*--------------------------------------------------------------------*/
/*    If we have an error, set up to use the BIOS information (or     */
/*    a fixed default) on future calls.  Otherwise, return the        */
/*    ANSI supplied value.                                            */
/*--------------------------------------------------------------------*/

   if ( regs.x.cflag )
   {
      printmsg(2,"DOS error %d retrieving screen size, using BIOS value %d",
                  (int) regs.x.ax,
                  (short) (default_rows ? default_rows : *bios_rows ));
      error = KWTrue;

      return (unsigned short) (default_rows ? default_rows : *bios_rows);
                                 /* Faster, but not well documented   */

   } /* if ( regs.x.cflag ) */
   else
      return (unsigned short) info.dmRows;

} /* scrsize */
