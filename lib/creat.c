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

#include "uupcmoah.h"

#include <fcntl.h>

#ifndef __GNUC__

#include <io.h>

RCSID("$Id: creat.c 1.7 1999/01/08 02:20:43 ahd v1-13g ahd $");

#endif

void FILEMODE();        /* Suppress error message for phantom routine */

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

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
