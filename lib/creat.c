/*--------------------------------------------------------------------*/
/*    C R E A T . C                                                   */
/*                                                                    */
/*    Support routines for UUPC/extended                              */
/*                                                                    */
/*    Changes Copyright 1990, 1991 (c) Andrew H. Derbyshire           */
/*                                                                    */
/*    History:                                                        */
/*       21Nov1991 Break out of lib.c                          ahd    */
/*--------------------------------------------------------------------*/

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef __GNUC__
#include <io.h>
#endif

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "hlib.h"

/*--------------------------------------------------------------------*/
/*    C R E A T                                                       */
/*                                                                    */
/*    Create a file with the specified mode                           */
/*--------------------------------------------------------------------*/

int CREAT(const char *name, const int mode, const char ftyp)
{

   char *last;
   char *path;
   int results;

   /* are we opening for write or append */
   FILEMODE(ftyp);
   results = creat(name, mode);

   if (results != -1)
      return results;

   /* see if we need to make any intermediate directories */
   path = normalize( name );

   if ((last = strrchr(path, '/')) != nil(char))
   {
      *last = '\0';
      MKDIR(path);
   }

   /* now try open again */
   return creat(name, mode);

} /*CREAT*/
