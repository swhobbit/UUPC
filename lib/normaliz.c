/*--------------------------------------------------------------------*/
/*    n o r m a l i z . c                                             */
/*                                                                    */
/*    Normalize a path for UUPC/extended                              */
/*                                                                    */
/*    Copyright (c) 1992 by Kendra Electronic Wonderworks; all        */
/*    rights reserved except those explicitly granted by the          */
/*    UUPC/extended license.                                          */
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
 *    $Id: normaliz.c 1.12 1994/02/19 03:55:32 ahd Exp $
 *
 *    Revision history:
 *    $Log: normaliz.c $
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
   int column;
   char *p;

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
/*                    Now actually expand the path                    */
/*--------------------------------------------------------------------*/

   p = _fullpath( save, path, sizeof save );

   if ( p == NULL )
   {
      printerr( path );
      panic();
   }

   while ((p = strchr(p,'\\')) != NULL)   /* Back slashes to slashes  */
      *p++ = '/';

   if ( equaln( save + 1, "://", 3))
      p = save + 2;                       /* Drop drive if really network */
   else
      p = save;                           /* Else use as-is           */

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
