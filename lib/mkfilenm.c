/*--------------------------------------------------------------------*/
/*       m k f i l e n a m e . c                                      */
/*                                                                    */
/*       Make a qualified file name from path name and simple file    */
/*       name.                                                        */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1994 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: mkfilenm.c 1.4 1994/01/01 19:03:08 ahd Exp $
 *
 *    Revision history:
 *    $Log: mkfilenm.c $
 *     Revision 1.4  1994/01/01  19:03:08  ahd
 *     Annual Copyright Update
 *
 *     Revision 1.3  1993/10/03  20:37:34  ahd
 *     Don't formally normalize paths, invalid file names cause normalize()
 *     to panic() in BCC for OS/2
 *
 *     Revision 1.3  1993/10/03  20:37:34  ahd
 *     Don't formally normalize paths, invalid file names cause normalize()
 *     to panic() in BCC for OS/2
 *
 *     Revision 1.2  1993/06/16  04:03:25  ahd
 *     drop duplicated slashes (caused by root directory support *sigh*)
 *
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
   char *s = pathname;
   sprintf(pathname, "%s/%s", path, name);

   while ((s = strchr(s, '\\')) != NULL)
      *s++ = '/';                             /* Normalize slashes */

} /*mkfilename*/
