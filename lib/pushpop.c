/*--------------------------------------------------------------------*/
/*    p u s h p o p . c                                               */
/*                                                                    */
/*    Directory save/restore functions for UUPC/extended              */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1994 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: pushpop.c 1.9 1994/01/01 19:04:18 ahd Exp $
 *
 *    $Log: pushpop.c $
 *     Revision 1.9  1994/01/01  19:04:18  ahd
 *     Annual Copyright Update
 *
 *     Revision 1.8  1993/12/23  03:11:17  rommel
 *     OS/2 32 bit support for additional compilers
 *
 *     Revision 1.7  1993/11/06  17:54:55  rhg
 *     Drive Drew nuts by submitting cosmetic changes mixed in with bug fixes
 *
 *     Revision 1.6  1993/10/24  21:45:49  rhg
 *     Save the changed directory of the correct drive!
 *
 *     Revision 1.5  1993/06/15  12:37:16  ahd
 *     Correct compile warning message about const assignment
 *
 *     Revision 1.4  1993/06/15  12:18:06  ahd
 *     Save pushed directory name for debugging
 *
 *     Revision 1.3  1993/06/13  14:06:00  ahd
 *     Insure directories PUSHED are POPPED
 *
 * Revision 1.2  1992/11/22  21:06:14  ahd
 * Use strpool for memory allocation
 *
 */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/


#include "uupcmoah.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include <direct.h>
#include <string.h>
#include <ctype.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "pushpop.h"

#define MAXDEPTH 10

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

static char *dirstack[MAXDEPTH];
static int drivestack[MAXDEPTH];
static depth = 0;

currentfile();

/*--------------------------------------------------------------------*/
/*        Change to a directory and push old one on our stack         */
/*--------------------------------------------------------------------*/

void PushDir( const char *directory )
{
   char cwd[FILENAME_MAX];

   if ( depth >= MAXDEPTH )
      panic();

/*--------------------------------------------------------------------*/
/*       Unlike Unix, DOS has a CWD per drive.  We must,              */
/*       therefore, change to the new drive before we save the        */
/*       CWD.  Otherwise we will lose the old CWD for the new         */
/*       drive.  For straight PushDir/PopDir pairing, we need not     */
/*       save the CWD on the old drive because we won't be changing   */
/*       it.  Note, however, that the sequence PushDir/CHDIR/         */
/*       PopDir (as opposed to just PushDir/PopDir pairs) could       */
/*       lose information if the CHDIR changes back to the drive      */
/*       from which PushDir changed away.  If any UUPC/extended       */
/*       code really did this, then drivestack would have to be       */
/*       changed to a full pathname like dirstack.  Lucky for us,     */
/*       it doesn't do so in any known sequences.  --RHG/AHD          */
/*--------------------------------------------------------------------*/

#ifdef __TURBOC__
   drivestack[depth] = getdisk();
   if (isalpha(*directory) && (directory[1] == ':'))
      setdisk(toupper(*directory) - 'A');
   dirstack[depth] = newstr( getcwd( cwd, FILENAME_MAX ) );
#else
   drivestack[depth] = _getdrive();
   if (isalpha(*directory) && (directory[1] == ':'))
      _chdrive( toupper(*directory) - 'A' + 1);

   dirstack[depth] = newstr(_getdcwd(drivestack[depth], cwd, FILENAME_MAX));
#endif

   if (dirstack[depth] == NULL )
   {
      printerr("PushDir");
      panic();
   }

   depth++;

   if (equal(directory,"."))
      E_cwd = dirstack[depth - 1];
   else
      CHDIR( directory );        /* CHDIR sets E_cwd                 */

} /* PushDir */

/*--------------------------------------------------------------------*/
/*               Return to a directory saved by PushDir               */
/*--------------------------------------------------------------------*/

void PopDir( void )
{
   char cwd[FILENAME_MAX];

   if ( depth == 0 )
      panic();

   CHDIR( dirstack[--depth] );

#ifdef __TURBOC__
   setdisk(drivestack[depth]);

   E_cwd = newstr( getcwd( cwd, FILENAME_MAX ) );
#else
   _chdrive(drivestack[depth]);

   E_cwd = newstr( _getdcwd( 0, cwd, FILENAME_MAX ) );
#endif

} /* PopDir */
