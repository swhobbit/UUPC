/*--------------------------------------------------------------------*/
/*       m k f i l e n a m e . c                                      */
/*                                                                    */
/*       Make a qualified file name from path name and simple file    */
/*       name.                                                        */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1995 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: mkfilenm.c 1.10 1994/12/22 00:09:29 ahd v1-12n $
 *
 *    Revision history:
 *    $Log: mkfilenm.c $
 *    Revision 1.10  1994/12/22 00:09:29  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.9  1994/02/19 04:43:47  ahd
 *    Use standard first header
 *
 *     Revision 1.8  1994/02/19  04:08:44  ahd
 *     Use standard first header
 *
 *     Revision 1.7  1994/02/19  03:53:42  ahd
 *     Use standard first header
 *
 *     Revision 1.6  1994/02/18  23:11:29  ahd
 *     Use standard first header
 *
 *     Revision 1.5  1994/01/24  03:08:46  ahd
 *     Annual Copyright Update
 *
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

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"


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
   strcpy( s, path );
   strcat( s, "/" );
   strcat( s, name );

   while ((s = strchr(s, '\\')) != NULL)
      *s++ = '/';                             /* Normalize slashes */

} /*mkfilename*/
