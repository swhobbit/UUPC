/*--------------------------------------------------------------------*/
/*       f i l e b k u p                                              */
/*                                                                    */
/*       UUPC/extended file backup routines                           */
/*                                                                    */
/*       Copyright (c) 1991-1992 by Andrew H. Derbyshire;             */
/*       all rights reserved except as granted by UUPC/extended       */
/*       license included with documention.                           */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Header$
 *
 *    Revision history:
 *    $Log$
 */

static char rcsid[] = "$Id$";


/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef __GNUC__
#include <direct.h>
#endif

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "hlib.h"

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

   if ( !bflag[F_BACKUP] )    /* Are we to not back it up?           */
      return 1;               /* Yes --> Report we did not           */

#ifdef __TURBOC__
   fnsplit( input, fdrive, fpath, fname, ftype );

   if ( E_backup == NULL )
      strcpy(ftype, ".BAK" );
   else if ( *E_backup == '.' )
      strcpy( ftype, E_backup );
   else {
      *ftype = '.';
      strcpy(  ftype + 1 , E_backup );
   } /* else */

   fnmerge( backup, fdrive, fpath, fname, ftype );
#else

   if ( E_backup == NULL )
      E_backup = ".BAK";

   _splitpath( input , fdrive, fpath, fname, ftype );
   _makepath( backup , fdrive, fpath, fname, E_backup );
#endif /* __TURBOC__ */

   remove( backup );

   if (rename( input, backup ))
   {
      printerr( backup );
      printmsg(1,"Unable to rename %s to %s\n", input, backup );
      return 1;

   } /* if (rename( input, backup )) */
   else
      return 0;

} /* filebkup */
