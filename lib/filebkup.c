/*--------------------------------------------------------------------*/
/*       f i l e b k u p                                              */
/*                                                                    */
/*       UUPC/extended file backup routines                           */
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
 *    $Id: filebkup.c 1.9 1995/03/11 22:33:46 ahd v1-12q $
 *
 *    Revision history:
 *    $Log: filebkup.c $
 *    Revision 1.9  1995/03/11 22:33:46  ahd
 *    Blow off undelete processing if so configured under OS/2
 *
 *    Revision 1.8  1995/01/29 14:07:59  ahd
 *    Clean up most IBM C/Set Compiler Warnings
 *
 *    Revision 1.7  1994/02/20 19:05:02  ahd
 *    IBM C/Set 2 Conversion, memory leak cleanup
 *
 *     Revision 1.6  1994/01/01  19:01:54  ahd
 *     Annual Copyright Update
 *
 *     Revision 1.5  1993/12/23  03:11:17  rommel
 *     OS/2 32 bit support for additional compilers
 *
 *     Revision 1.4  1993/10/12  00:43:34  ahd
 *     Normalize comments
 *
 *     Revision 1.3  1993/04/11  00:33:38  dmwatt
 *     Global edits for year, TEXT, etc.
 *
 * Revision 1.2  1992/11/19  02:57:51  ahd
 * drop rcsid
 *
 * Revision 1.1  1992/11/16  05:00:26  ahd
 * Initial revision
 *
 */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#ifndef __GNUC__

#include "uupcmoah.h"

#include <direct.h>
#endif

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

currentfile();

/*--------------------------------------------------------------------*/
/*    f i l e b k u p                                                 */
/*                                                                    */
/*    Backup a file to the standard UUPC/extended defined             */
/*    extension.                                                      */
/*--------------------------------------------------------------------*/

int filebkup( const char *input )
{
   char fdrive[FILENAME_MAX];
   char fpath[FILENAME_MAX];
   char fname[FILENAME_MAX];
   char ftype[FILENAME_MAX];
   char backup[FILENAME_MAX];

   if ( !bflag[F_BACKUP] )    /* Are we to not back it up?            */
      return 1;               /* Yes --> Report we did not            */

#ifdef __TURBOC__
   fnsplit( input, fdrive, fpath, fname, ftype );

   if ( E_backup == NULL )
      strcpy(ftype, ".bak" );
   else if ( *E_backup == '.' )
      strcpy( ftype, E_backup );
   else {
      *ftype = '.';
      strcpy(  ftype + 1 , E_backup );
   } /* else */

   fnmerge( backup, fdrive, fpath, fname, ftype );
#else

   if ( E_backup == NULL )
      E_backup = ".bak";

   _splitpath( (char *) input , fdrive, fpath, fname, ftype );
   _makepath( backup , fdrive, fpath, fname, E_backup );
#endif /* __TURBOC__ */

   REMOVE( backup );

   if (rename( input, backup ))
   {
      printerr( backup );
      printmsg(1,"Unable to rename %s to %s", input, backup );
      return 1;

   } /* if (rename( input, backup )) */
   else
      return 0;

} /* filebkup */
