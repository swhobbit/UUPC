/*--------------------------------------------------------------------*/
/*    m k d i r . c                                                   */
/*                                                                    */
/*    Support routines for UUPC/extended                              */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989 by Andrew H. Derbyshire.             */
/*                                                                    */
/*    Changes Copyright (c) 1990-1992 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: LIB.H 1.4 1993/03/06 23:09:50 ahd Exp $
 *
 *    Revision history:
 *    $Log: LIB.H $
 */

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
