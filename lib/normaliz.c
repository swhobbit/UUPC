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
 *    $Id: E:\SRC\UUPC\LIB\RCS\NORMALIZ.C 1.1 1992/11/22 21:06:14 ahd Exp $
 *
 *    Revision history:
 *    $Log: NORMALIZ.C $
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
      return NULL;

   while ((p = strchr(p,'\\')) != NULL)
      *p++ = '/';

   column = strlen( save ) - 1;
   if ( save[column] == '/' )
       save[column] = '\0';

   return strlwr( save );

} /* normalize */
