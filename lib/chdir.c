/*--------------------------------------------------------------------*/
/*       c h d i r . c                                                */
/*                                                                    */
/*       Change directory for UUPC/extended; builds directory tree    */
/*       if needed and saves the nane of the new directory in         */
/*       E_cwd at no extra charge.                                    */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1995 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: chdir.c 1.7 1994/02/20 19:05:02 ahd v1-12k $
 *
 *    Revision history:
 *    $Log: chdir.c $
 *    Revision 1.7  1994/02/20 19:05:02  ahd
 *    IBM C/Set 2 Conversion, memory leak cleanup
 *
 */

#include "uupcmoah.h"

#include <ctype.h>
#include <direct.h>

/*--------------------------------------------------------------------*/
/*                          Local prototypes                          */
/*--------------------------------------------------------------------*/

static int changedir( const char *path);

currentfile();

/*--------------------------------------------------------------------*/
/*       C H D I R                                                    */
/*                                                                    */
/*       Like chdir() but also change drives and create the           */
/*       directory if necessary.                                      */
/*--------------------------------------------------------------------*/

int CHDIR(const char *path)
{

   int result;
   int originalDrive = _getdrive(); /* Remember in case we fail       */

   if (*path == '\0')
      return 0;

/*--------------------------------------------------------------------*/
/*                      Change to the new drive                       */
/*--------------------------------------------------------------------*/

   if ((*path != '\0') && (path[1] == ':'))
   {
      if (isalpha(*path))
      {
         if (_chdrive( toupper(*path) - 'A' + 1))
            return -1;                 /* Return if failure          */
      } /* if */
      else {
         printmsg(0,"changedir: Drive letter is not numeric: %s",
                     path );
         return -1;
      } /* else */

   } /* if */

/*--------------------------------------------------------------------*/
/*        Try to change directories, returning if successful          */
/*--------------------------------------------------------------------*/

   if (!changedir( path ))
      return 0;

/*--------------------------------------------------------------------*/
/*                      Try making the directory                      */
/*--------------------------------------------------------------------*/

   MKDIR(path);

/*--------------------------------------------------------------------*/
/*                   Change to the directory again                    */
/*--------------------------------------------------------------------*/

   result = changedir(path);

   if ( result )
   {
      printerr("chdir");         /* Report the error, real problem   */
      _chdrive( originalDrive ); /* Return to original drive         */
   }

   return result;

} /*CHDIR*/

/*--------------------------------------------------------------------*/
/*       c h a n g e d i r                                            */
/*                                                                    */
/*       Like chdir() but also saves the path we changed into         */
/*--------------------------------------------------------------------*/

static int changedir(const char *path)
{
   static char savePath[FILENAME_MAX];

   int result = chdir((char *) path);     /* Perform the change      */

   if ( ! result )               /* Did it work?                     */
      E_cwd = strcpy( savePath, path );   /* yes --> Save directory  */

   return result;

} /*changedir*/
