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
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: normaliz.c 1.3 1993/06/15 12:18:06 ahd Exp $
 *
 *    Revision history:
 *    $Log: normaliz.c $
 *     Revision 1.3  1993/06/15  12:18:06  ahd
 *     Display last known directory name for debugging
 *
 *     Revision 1.2  1993/04/11  00:32:05  ahd
 *     Global edits for year, TEXT, etc.
 *
 * Revision 1.1  1992/11/22  21:06:14  ahd
 * Initial revision
 *
 *
 */

/*--------------------------------------------------------------------*/
/*                   Standard library include files                   */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"

currentfile();

/*--------------------------------------------------------------------*/
/*    n o r m a l i z e                                               */
/*                                                                    */
/*    Normalize a DOS Path                                            */
/*--------------------------------------------------------------------*/

char *normalize( const char *path )
{
   static char save[FILENAME_MAX];
   int column;

   char *p = _fullpath( save, path, sizeof save );

   if ( p == NULL )
   {
      printerr( path );
      return NULL;
   }

   while ((p = strchr(p,'\\')) != NULL)   // Back slashes to slashes
      *p++ = '/';

   p = strlwr( save );

   while ((p = strstr(p,"//")) != NULL)   // Drop all double slashes
      memmove(p, p+1, strlen(p+1));

   column = strlen( save ) - 1;
   if ((column > 2) && ( save[column] == '/' )) // Zap all but root trailing
       save[column] = '\0';


/*--------------------------------------------------------------------*/
/*               Print the results and return to caller               */
/*--------------------------------------------------------------------*/

   printmsg(5,"Normalize: cwd = %s, input = %s, output = %s",
               (E_cwd == NULL) ? "?" : E_cwd,
               path,
               save );

   return save;

} /* normalize */
