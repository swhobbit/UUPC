/*--------------------------------------------------------------------*/
/*       c h d i r . c                                                */
/*                                                                    */
/*       Change directory for UUPC/extended; builds directory tree    */
/*       if needed and saves the nane of the new directory in         */
/*       E_cwd at no extra charge.                                    */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1999 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: chdir.c 1.14 1998/03/01 01:23:11 ahd v1-13f ahd $
 *
 *    Revision history:
 *    $Log: chdir.c $
 *    Revision 1.14  1998/03/01 01:23:11  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.13  1997/03/31 06:58:20  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.12  1996/01/01 20:50:06  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.11  1995/02/14 04:38:42  ahd
 *    Correct problems with directory processing under NT
 *
 *    Revision 1.10  1995/02/12 23:35:59  ahd
 *    'compiler
 *
 *    Revision 1.9  1995/01/30 04:03:57  dmwatt
 *    Optimize Windows NT processing
 *
 *    Revision 1.8  1994/12/22 00:07:33  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.7  1994/02/20 19:05:02  ahd
 *    IBM C/Set 2 Conversion, memory leak cleanup
 *
 */

#include "uupcmoah.h"

#include <ctype.h>
#include <direct.h>

#ifdef WIN32
#include <windows.h>
#endif

#ifdef __TURBOC__
#define _getcwd(path,length) getcwd(path,length)
#endif

/*--------------------------------------------------------------------*/
/*                          Local prototypes                          */
/*--------------------------------------------------------------------*/

static int changedir( const char *path, const int drive );

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
   int originalDrive = getDrive( NULL );  /* Remember should CD fail */
   int newDrive = originalDrive;

   if (*path == '\0')
      return 0;

/*--------------------------------------------------------------------*/
/*       Change to the new drive, except under NT (which can swap     */
/*       drives when it changes directories in changedir()            */
/*--------------------------------------------------------------------*/

   if (path[1] == ':')
   {
      if (isalpha(*path))
      {
         newDrive = toupper(*path) - 'A' + 1;

         if (_chdrive( newDrive ))
            return -1;                 /* Return if failure          */

      } /* if */
      else {

         printmsg(0,"changedir: Drive letter is not alphabetic: %s",
                     path );

         return -1;

      } /* else */

   } /* if */

/*--------------------------------------------------------------------*/
/*        Try to change directories, returning if successful          */
/*--------------------------------------------------------------------*/

   if (!changedir( path, newDrive ))
      return 0;

/*--------------------------------------------------------------------*/
/*                      Try making the directory                      */
/*--------------------------------------------------------------------*/

   MKDIR(path);

/*--------------------------------------------------------------------*/
/*                   Change to the directory again                    */
/*--------------------------------------------------------------------*/

   result = changedir(path, newDrive );

   if ( result )
   {
      printerr("chdir");         /* Report the error, real problem   */

      _chdrive( originalDrive - 'A' + 1); /* Return to original drive   */

   }

   return result;

} /* CHDIR */

/*--------------------------------------------------------------------*/
/*       c h a n g e d i r                                            */
/*                                                                    */
/*       Like chdir() but also saves the path we changed into         */
/*--------------------------------------------------------------------*/

static int changedir(const char *path, const int drive)
{

   int result = chdir((char *) path);     /* Perform the change      */

   if ( ! result )                  /* Did it work?                  */
   {                                /* Yes --> Save directory name   */

      static char savePath[FILENAME_MAX];

      _getdcwd(drive, savePath, sizeof savePath);

      E_cwd = savePath;             /* Yes --> Save directory        */

      if ( isalpha( *E_cwd ) && islower( *E_cwd ))
         *E_cwd = (char) toupper( *E_cwd );
                                    /* Insure driver letter is upper
                                       case                          */

   }

   return result;

} /* changedir */
