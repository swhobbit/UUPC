/*--------------------------------------------------------------------*/
/*    i m p o r t n g . c                                             */
/*                                                                    */
/*    Create the fully-qualified path name for a news group           */
/*    under UUPC/extended.                                            */
/*                                                                    */
/*    Copyright (c) 1992, by Andrew H. Derbyshire; all rights         */
/*    reserved except as specified in UUPC/extended end-user          */
/*    documentation.                                                  */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <time.h>
#include <string.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "import.h"
#include "hlib.h"
#include "importng.h"

/*--------------------------------------------------------------------*/
/*    I m p o r t N e w s G r o u p                                   */
/*                                                                    */
/*    Takes a news group and returns a directory path for it          */
/*--------------------------------------------------------------------*/

char *ImportNewsGroup( char *local,
                       const char *group,
                       const long article )
{
   char fullpath[FILENAME_MAX];
   char partial[FILENAME_MAX];
   char *s;

/*--------------------------------------------------------------------*/
/*    Combine the group with the name of the file of in the           */
/*    newgroup, if needed.  Otherwise, copy the simple news group     */
/*    name to our work buffer                                         */
/*--------------------------------------------------------------------*/

   strcpy( partial, group);

   if ( article != 0 )
      sprintf(partial + strlen( partial ),"/%ld", article );

/*--------------------------------------------------------------------*/
/*    Transform all periods in the news group to slashes, which       */
/*    allows for longer group names than allowed by the simple DOS    */
/*    8 + 3 directory structure.                                      */
/*--------------------------------------------------------------------*/

   while ((s = strchr(partial, '.')) != NULL)
      *s = '/';

   while ((s = strchr(E_newsdir, '\\')) != NULL)
      *s = '/';                                 /* Normalize slashes */

/*--------------------------------------------------------------------*/
/*    Now combine the name with the root news directory; we do        */
/*    this before importing the file name to insure we have a         */
/*    path, which affects how ImportPath processes the file           */
/*--------------------------------------------------------------------*/

   mkfilename( fullpath , E_newsdir, partial ); /* Build the name    */

   importpath( local, fullpath, NULL );        /* Truncate name and
                                                   zap bad chars as
                                                   needed            */
   printmsg(2,"ImportNewsGroup: %s(%ld) --> %s", group, article, local );
   return local;

} /* ImportNewsGroup */
