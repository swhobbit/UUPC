/*--------------------------------------------------------------------*/
/*    n o r m a l i z . c                                             */
/*                                                                    */
/*    Normalize a path for UUPC/extended                              */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1998 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: normaliz.c 1.18 1997/03/31 07:05:20 ahd v1-12u $
 *
 *    Revision history:
 *    $Log: normaliz.c $
 *    Revision 1.18  1997/03/31 07:05:20  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.17  1996/01/01 20:54:25  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.16  1995/02/12 23:37:04  ahd
 *    compiler cleanup, NNS C/news support, optimize dir processing
 *
 *    Revision 1.15  1995/01/30 04:05:08  ahd
 *    Additional compiler warning fixes, optimize path normalizing
 *
 *    Revision 1.14  1994/12/22 00:10:03  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.13  1994/02/19 04:44:48  ahd
 *    Use standard first header
 *
 *     Revision 1.12  1994/02/19  03:55:32  ahd
 *     Use standard first header
 *
 *     Revision 1.12  1994/02/19  03:55:32  ahd
 *     Use standard first header
 *
 *     Revision 1.11  1994/02/18  23:12:51  ahd
 *     Use standard first header
 *
 *     Revision 1.10  1994/01/01  19:03:52  ahd
 *     Annual Copyright Update
 *
 *     Revision 1.9  1993/10/12  00:46:16  ahd
 *     Normalize comments
 *
 *     Revision 1.8  1993/09/20  04:38:11  ahd
 *     TCP/IP support from Dave Watt
 *     't' protocol support
 *     OS/2 2.x support
 *
 *     Revision 1.7  1993/08/08  17:39:09  ahd
 *     Denormalize path for opening on selected networks
 *
 *     Revision 1.6  1993/07/06  10:55:20  ahd
 *     Drop doubled path delimiters before calling _fullpath
 *     Abort, not return NULL, if _fullpath fails
 *
 *     Revision 1.5  1993/06/16  04:19:31  ahd
 *     Copy trailing null when copying slashes
 *
 *     Revision 1.4  1993/06/16  04:03:25  ahd
 *     drop duplicated slashes (caused by root directory support *sigh*)
 *
 *     Revision 1.3  1993/06/15  12:18:06  ahd
 *     Display last known directory name for debugging
 *
 *     Revision 1.2  1993/04/11  00:32:05  ahd
 *     Global edits for year, TEXT, etc.
 *
 * Revision 1.1  1992/11/22  21:06:14  ahd
 * Initial revision
 *
 */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"

#include <ctype.h>
#include <direct.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

currentfile();

/*--------------------------------------------------------------------*/
/*    n o r m a l i z e                                               */
/*                                                                    */
/*    Normalize a DOS Path                                            */
/*--------------------------------------------------------------------*/

char *normalize( const char *pathx )
{
   static char save[FILENAME_MAX];
   char path[FILENAME_MAX];
   size_t column;
   char *p;

   if ( pathx == NULL )
      panic();

   if ( *pathx == '\0' )
   {
      printmsg(0,"Empty path passed to normalize");
      panic();
   }

/*--------------------------------------------------------------------*/
/*                      Normalize the seperators                      */
/*--------------------------------------------------------------------*/

   strcpy( path, pathx );
   denormalize( path );

/*--------------------------------------------------------------------*/
/*                    Add leading path, if needed                     */
/*--------------------------------------------------------------------*/

   if (  ( E_cwd != NULL ) &&
         equaln( E_cwd, "//", 2 ) &&               /* Network CWD drive */
       ! (isalpha( *path ) && (path[1] == ':')) && /* Not explicit drive */
         (*path != '\\'))                          /* Not explicit path */
   {
      column = strlen( E_cwd );
      memmove( path + column + 1, path, strlen(path) + 1 );
                                          /* Make room for path       */
      memcpy( path, E_cwd, column );      /* Insert path              */
      path[column] = '\\';                /* Add directory sep        */
   }

   p = path + 1;                 /* Allow leading double slash for    */
                                 /* Network drives                    */

   while ((p = strstr(p,"\\\\")) != NULL)  /* Drop all double slashes */
      memmove(p, p+1, strlen(p));          /* Includes trailing NULL  */

/*--------------------------------------------------------------------*/
/*           Drop trailing slashes, OS/2 doesn't like them            */
/*--------------------------------------------------------------------*/

   column = strlen( path ) - 1;

   if ( (column > 2) && (path[column] == '\\') )
      path[column] = '\0';

/*--------------------------------------------------------------------*/
/*       Now actually expand the path if needed because it's not      */
/*       absolute or refers to parent directories.  We take           */
/*       special care to insert the drive letter if needed for an     */
/*       absolute path which does not have a drive letter and is      */
/*       not a UNC (network) drive reference.                         */
/*--------------------------------------------------------------------*/

   if (isAbsolutePath(path) && (strstr( path, ".." ) == NULL ))
   {
      p = save;

      if (( path[0] == '\\' ) && ( path[1] != '\\' ))
      {
         *p++ = getDrive( NULL );
         *p++ = ':';
      }

      strcpy( p, path );

   } /* if (isAbsolutePath(path) && (strstr( path, ".." ) == NULL )) */
   else {
      p = _fullpath( save, path, sizeof save );

      if ( p == NULL )
      {
         printerr( path );
         panic();
      }
   }

   renormalize( save );             /* Revert backslashes to slashes */

   p = save;

   if ( equaln( p + 1, "://", 3))
      p = p + 2;                    /* Drop drive if really network  */

   column = strlen( p ) - 1;

   if ((column > 2) && ( p[column] == '/' )) /* Zap all but root trailing */
       p[column] = '\0';

/*--------------------------------------------------------------------*/
/*               Print the results and return to caller               */
/*--------------------------------------------------------------------*/

   printmsg(5,"Normalize: cwd = %s, input = %s, output = %s",
               (E_cwd == NULL) ? "?" : E_cwd,
               pathx,
               p );

   return p;

} /* normalize */
