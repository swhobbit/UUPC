/*--------------------------------------------------------------------*/
/*       m k f i l e n a m e . c                                      */
/*                                                                    */
/*       Make a qualified file name from path name and simple file    */
/*       name.                                                        */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1990-1993 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: lib.h 1.8 1993/06/13 14:12:29 ahd Exp $
 *
 *    Revision history:
 *    $Log: lib.h $
 */

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
   strcpy( pathname, normalize( pathname ));

} /*mkfilename*/
