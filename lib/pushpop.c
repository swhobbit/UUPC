/*--------------------------------------------------------------------*/
/*    p u s h p o p . c                                               */
/*                                                                    */
/*    Directory save/restore functions for UUPC/extended              */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1996 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: pushpop.c 1.17 1995/02/14 04:38:42 ahd v1-12q $
 *
 *    $Log: pushpop.c $
 *    Revision 1.17  1995/02/14 04:38:42  ahd
 *    Correct problems with directory processing under NT
 *
 *    Revision 1.16  1995/02/12 23:37:04  ahd
 *    compiler cleanup, NNS C/news support, optimize dir processing
 *
 *    Revision 1.15  1994/12/22 00:10:26  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.14  1994/03/15 03:02:26  ahd
 *    Correct spelling error
 *
 *     Revision 1.13  1994/02/20  19:07:38  ahd
 *     IBM C/Set 2 Conversion, memory leak cleanup
 *
 *     Revision 1.12  1994/02/19  04:45:32  ahd
 *     Use standard first header
 *
 *     Revision 1.11  1994/02/19  03:56:19  ahd
 *     Use standard first header
 *
 *     Revision 1.11  1994/02/19  03:56:19  ahd
 *     Use standard first header
 *
 *     Revision 1.10  1994/02/18  23:13:26  ahd
 *     Use standard first header
 *
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

#include <direct.h>
#include <ctype.h>

#ifdef __TURBOC__
#define _getcwd(path,length) getcwd(path,length)
#endif

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

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
/*       P u s h D i r                                                */
/*                                                                    */
/*       Change to a directory and push old one on our stack          */
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

   drivestack[depth] = getDrive( NULL ) - 'A' + 1;

   if (isalpha(*directory) && (directory[1] == ':'))
   {
      if (_chdrive( toupper(*directory) - 'A' + 1))
      {
         printerr("chdrive");
         panic();
      }
   }

/*--------------------------------------------------------------------*/
/*       We retrieve the current directory by drive letter because    */
/*       the IBM OS/2 C compiler 2.01 returns @ for the drive         */
/*       letter of the 0 (current) drive.  Windows NT is even more    */
/*       evil, because it returns information for the wrong drive.    */
/*--------------------------------------------------------------------*/

   dirstack[depth] = _getdcwd(drivestack[depth], cwd, FILENAME_MAX);

   if (dirstack[depth] == NULL )
   {
      printerr("PushDir");
      panic();
   }

   dirstack[depth] = newstr( cwd );

   if (equal(directory,"."))
      E_cwd = dirstack[depth];
   else
      CHDIR( directory );        /* CHDIR sets E_cwd                 */

#ifdef UDEBUG
   printmsg(5,"PushDir: pushed from %c %s (depth %d) to %s",
               drivestack[depth] + 'A' - 1,
               dirstack[depth],
               depth,
               E_cwd );
#endif

  depth++;

} /* PushDir */

/*--------------------------------------------------------------------*/
/*       P o p D i r                                                  */
/*                                                                    */
/*       Return to a directory saved by PushDir                       */
/*--------------------------------------------------------------------*/

void PopDir( void )
{
   char cwd[FILENAME_MAX];

#ifdef UDEBUG
   char *oldCWD = E_cwd;
#endif

   if ( depth-- == 0 )
      panic();

   if (chdir( dirstack[depth] ))
      panic();

   if ( _chdrive(drivestack[depth]) )
   {
      printerr("chdrive");
      panic();
   }

   E_cwd = _getdcwd(drivestack[depth], cwd, FILENAME_MAX);
   E_cwd = newstr( E_cwd );

#ifdef UDEBUG
   printmsg(5,"PopDir: popped from %s to %c %s (depth %d) %s",
               oldCWD,
               drivestack[depth] + 'A' - 1,
               dirstack[depth],
               depth,
               E_cwd );
#endif

} /* PopDir */
