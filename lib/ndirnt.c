/*--------------------------------------------------------------------*/
/*    ndir.c for MS-DOS by Samuel Lam <skl@van-bc.UUCP>, June/87      */
/*    ndir.c for MS-OS2 by Drew Derbyshire (help@kendra.kew.com>,     */
/*           April/91                                                 */
/*    ndir.c for Windows/NT by Tom Loebach (loebach@mips.com),        */
/*           April/92                                                 */
/*                                                                    */
/*         Berkeley-style directory reading routine on Windows NT     */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1990-1992 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*
 *       $Id$
 *
 *       $Log$
 */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

/*--------------------------------------------------------------------*/
/*                         Windows/NT include files                   */
/*--------------------------------------------------------------------*/

#define INCL_BASE

#include <WINDOWS.h>


/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "ndir.h"


static char *pathname = NULL;
static HANDLE dirHandle;
static WIN32_FIND_DATA dirData;
currentfile();

/*--------------------------------------------------------------------*/
/*    o p e n d i r                                                   */
/*                                                                    */
/*    Open a directory                                                */
/*--------------------------------------------------------------------*/

extern DIR *opendirx( const char *dirname, char *pattern)
{

   DIR *dirp;

   pathname = malloc( strlen( dirname ) + strlen( pattern ) + 2 );
   strcpy(pathname, dirname);


   if ((*pattern != '\\') || (dirname[ strlen(dirname) - 1] != '\\'))
      strcat(pathname,"\\");


   strcat(pathname,pattern);
   printmsg(5,"opendir: Opening directory %s", pathname );

/*--------------------------------------------------------------------*/
/*                Read the first file in the directory                */
/*--------------------------------------------------------------------*/


   dirHandle = FindFirstFile(pathname, &dirData);

   printmsg(5, "dirhandle = %d\n",dirHandle);
   printmsg(5, "file, = %s\n", dirData.cFileName);

   if ((int)dirHandle == -1) {
      printmsg(2,"opendir: Error on directory %s",pathname );
      return NULL;
   }
   else {
      dirp = malloc( sizeof( DIR ));
      dirp->dirfirst = 1;
      strcpy(dirp->dirid, "DIR");
      return dirp;
   }


} /*opendir*/

/*--------------------------------------------------------------------*/
/*    r e a d d i r                                                   */
/*                                                                    */
/*    Get next entry in a directory                                   */
/*--------------------------------------------------------------------*/

struct direct *readdir(DIR *dirp)
{

   BOOL rc;

   assert(strcmp(dirp->dirid, "DIR") == 0);
   if (dirp->dirfirst)
   {
      printmsg(5,"readdir: Opening directory %s", pathname );
      dirp->dirfirst = 0;
   }else {
      printmsg(5, "dirhandle = %d\n",dirHandle);
      rc = FindNextFile(dirHandle, &dirData);
   }

   if (!strcmp(dirData.cFileName,"."))
      rc = FindNextFile(dirHandle, &dirData);

   printmsg(9, "readdir: file = %s\n", dirData.cFileName);

   if (!strcmp(dirData.cFileName,".."))
      rc = FindNextFile(dirHandle, &dirData);

        printmsg(9, "file = %s\n", dirData.cFileName);

   if ( rc )
   {
      printmsg(9, "file = %s\n", dirData.cFileName);

      dirp->dirent.d_ino = -1;   /* no inode information */
      strlwr(strcpy(dirp->dirent.d_name, dirData.cFileName));
      dirp->dirent.d_namlen = strlen(dirData.cFileName);

      printmsg(9, "%d \n",dirp->dirent.d_namlen);

      dirp->dirent.d_reclen = sizeof(struct direct) - (MAXNAMLEN + 1) +
         ((((dirp->dirent.d_namlen + 1) + 3) / 4) * 4);
      return &(dirp->dirent);
   } else {

      printmsg(5,"readdir: Error on directory %s",pathname );
      return NULL;
   }
} /*readdir*/

/*--------------------------------------------------------------------*/
/*    c l o s e d i r                                                 */
/*                                                                    */
/*    Close a directory                                               */
/*--------------------------------------------------------------------*/

void closedir(DIR *dirp)
{

   BOOL rc;

   assert(strcmp(dirp->dirid, "DIR") == 0);

   printmsg(5,"closedir: Closing directory %s", pathname );

   rc = FindClose(dirHandle);
   if (rc == 0)
     printmsg(0,"closedir: Error %d on directory %s",
              (int) rc, pathname );

   free( dirp );
   dirp = NULL;
   free( pathname );
   pathname = NULL;
} /*closedir*/
