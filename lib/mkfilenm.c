/*--------------------------------------------------------------------*/
/*    m k f i l e n a m e . c                                         */
/*                                                                    */
/*    Support routines for UUPC/extended                              */
/*                                                                    */
/*    Changes Copyright 1990, 1991 (c) Andrew H. Derbyshire           */
/*                                                                    */
/*    History:                                                        */
/*       21Nov1991 Break out of hlib.c                         ahd    */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "hlib.h"
#include "lib.h"

/*--------------------------------------------------------------------*/
/*    m k f i l e n a m e                                             */
/*                                                                    */
/*    Build a path name out of a directory name and a file name       */
/*--------------------------------------------------------------------*/

void mkfilename(char *pathname,
                const char *path,
                const char *name)
{
   sprintf(pathname, "%s/%s", path, name);
   strlwr(pathname);             /* Normalize the name for messages     */

} /*mkfilename*/
