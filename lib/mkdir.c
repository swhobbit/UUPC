/*--------------------------------------------------------------------*/
/*    m k d i r . c                                                   */
/*                                                                    */
/*    Support routines for UUPC/extended                              */
/*                                                                    */
/*    Changes Copyright 1990, 1991 (c) Andrew H. Derbyshire           */
/*                                                                    */
/*    History:                                                        */
/*       21Nov1991 Break out of lib.c                          ahd    */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef __GNUC__
#include <os2.h>
#else
#include <direct.h>
#endif


/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"

currentfile();

/*--------------------------------------------------------------------*/
/*    M K D I R                                                       */
/*                                                                    */
/*    Like mkdir() but create intermediate directories as well        */
/*--------------------------------------------------------------------*/

int MKDIR(const char *inpath)
{
   char *cp;
   char *path;

   if (*inpath == '\0')
      return 0;

   cp = path = normalize(inpath );


/*--------------------------------------------------------------------*/
/*        See if we need to make any intermediate directories         */
/*--------------------------------------------------------------------*/

   cp = path ;
   while ((cp = strchr(cp, '/')) != nil(char)) {
      *cp = '\0';

#ifndef __GNUC__
      mkdir(path);
#else
      DosCreateDir( path, 0);
#endif
      *cp = '/';
      cp++;
   }

/*--------------------------------------------------------------------*/
/*                           Make last dir                            */
/*--------------------------------------------------------------------*/

   return mkdir(inpath);

} /*MKDIR*/
