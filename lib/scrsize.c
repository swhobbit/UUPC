/*--------------------------------------------------------------------*/
/*    s c r s i z e .  c                                              */
/*                                                                    */
/*    Report screen size under MS-DOS                                 */
/*                                                                    */
/*    Copyright (c) 1992 by Kendra Electronic Wonderworks.            */
/*    All rights reserved except those explicitly granted by          */
/*    the UUPC/extended license.                                      */
/*--------------------------------------------------------------------*/

/*
 *    $Id: SCRSIZE.C 1.5 1992/12/30 05:27:11 plummer Exp $
 *
 *    $Log: SCRSIZE.C $
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

#include <stdio.h>
#include <stdlib.h>
#include <dos.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "scrsize.h"

/*--------------------------------------------------------------------*/
/*    s c r s i z e                                                   */
/*                                                                    */
/*    Return screen size under MS-DOS 4.0 and 5.0                     */
/*--------------------------------------------------------------------*/

short scrsize( void )
{

#ifdef __TURBOC__
   static unsigned char far *bios_rows = MK_FP( 0x0040, 0x0084 );
/* static unsigned char far *bios_cols = MK_FP( 0x40, 0x4a ); */
#else
   static unsigned char far *bios_rows = 0x0484;
#endif

   static boolean error = FALSE;
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
      default_rows = 24;

   if ((_osmajor < 4) || error )
      return (short) (default_rows ? default_rows : *bios_rows);
                                 /* Faster, but not well documented  */

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
      error = TRUE;
      return (short) (default_rows ? default_rows : *bios_rows);
                                 /* Faster, but not well documented  */
   }
   else
      return info.dmRows;

} /* scrsize */
