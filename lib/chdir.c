/*--------------------------------------------------------------------*/
/*    c h d i r . c                                                   */
/*                                                                    */
/*    Support routines for UUPC/extended                              */
/*                                                                    */
/*    Changes Copyright 1990, 1991 (c) Andrew H. Derbyshire           */
/*                                                                    */
/*    History:                                                        */
/*       21Nov1991 Break out of lib.c                          ahd    */
/*--------------------------------------------------------------------*/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef __GNUC__
#include <dos.h>
#include <direct.h>
#endif

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "hlib.h"

static int changedir( const char *path);

/*--------------------------------------------------------------------*/
/*    C H D I R                                                       */
/*                                                                    */
/*    Like chdir() but create the directory if necessary              */
/*--------------------------------------------------------------------*/

int CHDIR(const char *path)
{

   if (*path == '\0')
      return 0;

/*--------------------------------------------------------------------*/
/*        Try to change directories, returning if successfull         */
/*--------------------------------------------------------------------*/

   if (!changedir( path ))
      return 0;

/*--------------------------------------------------------------------*/
/*                      Try making the directory                      */
/*--------------------------------------------------------------------*/

   MKDIR(path);

   /* change to last directory */
   return changedir(path);

} /*CHDIR*/

/*--------------------------------------------------------------------*/
/*    c h a n g e d i r                                               */
/*                                                                    */
/*    Like chdir() but also changes the current drive                 */
/*--------------------------------------------------------------------*/

static int changedir(const char *path)
{

   if ((*path != '\0') && (path[1] == ':')) {
      unsigned char drive = (unsigned char) toupper(*path);
      if ((drive >= 'A') && (drive <= 'Z'))
      {
#ifdef __TURBOC__
         setdisk(drive - (unsigned char)'A');
#else
         if (_chdrive( drive - (unsigned char)'A' + 1))  /* MS C     */
            return -1;                 /* Return if failure          */
#endif
      } /* if */
      else
         return -1;
   }

   E_cwd = (char *) path;

   return chdir(path);

} /*changedir*/
