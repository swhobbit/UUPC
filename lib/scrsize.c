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
 *    $Id$
 *
 *    $Log$
 */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include <stdio.h>
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

   static boolean error = FALSE;

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

   if ((_osmajor < 4) || error )
      return PAGESIZE;

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
   if ( regs.x.cflag )
   {
      printmsg(0,"DOS error %d retrieving screen size", (int) regs.x.ax );
      error = TRUE;
      return PAGESIZE;
   }

   return info.dmRows;

} /* scrsize */
