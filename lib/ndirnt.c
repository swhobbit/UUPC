/*--------------------------------------------------------------------*/
/*    ndir.c for MS-DOS by Samuel Lam <skl@van-bc.UUCP>, June/87      */
/*    ndir.c for MS-OS2 by Drew Derbyshire (help@kendra.kew.com>,     */
/*           April/91                                                 */
/*    ndir.c for Windows/NT by Tom Loebach (loebach@mips.com),        */
/*           April/92                                                 */
/*    ndir.c for NT extended to include timestamp information by      */
/*           Dave Watt, April/93                                      */
/*                                                                    */
/*         Berkeley-style directory reading routine on Windows NT     */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1996 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*
 *       $Id: ndirnt.c 1.15 1995/09/04 18:43:37 ahd v1-12q $
 *
 *       $Log: ndirnt.c $
 *       Revision 1.15  1995/09/04 18:43:37  ahd
 *       Shorten debugging output
 *
 *       Revision 1.14  1995/01/09 12:35:15  ahd
 *       Correct VC++ compiler warnings
 *
 *       Revision 1.13  1995/01/07 15:46:08  ahd
 *       Clean up debugging messages, dropping new lines
 *
 *       Revision 1.12  1994/12/22 00:09:47  ahd
 *       Annual Copyright Update
 *
 *       Revision 1.11  1994/02/19 04:44:20  ahd
 *       Use standard first header
 *
 *     Revision 1.10  1994/02/19  04:09:26  ahd
 *     Use standard first header
 *
 *     Revision 1.9  1994/02/19  03:54:48  ahd
 *     Use standard first header
 *
 *     Revision 1.8  1994/02/18  23:12:19  ahd
 *     Use standard first header
 *
 *     Revision 1.7  1994/01/24  03:09:04  ahd
 *     Annual Copyright Update
 *
 *     Revision 1.6  1994/01/01  19:03:30  ahd
 *     Annual Copyright Update
 *
 *     Revision 1.5  1993/10/31  22:07:05  ahd
 *     GRACEFULLY trap argument to closedir being NULL
 *
 *     Revision 1.5  1993/10/31  22:07:05  ahd
 *     GRACEFULLY trap argument to closedir being NULL
 *
 *     Revision 1.4  1993/04/11  00:33:38  dmwatt
 *     Global edits for year, TEXT, etc.
 *
 *     Revision 1.4  1993/04/11  00:33:38  dmwatt
 *     Global edits for year, TEXT, etc.
 *
 *     Revision 1.3  1993/04/10  21:22:29  dmwatt
 *     Windows/NT fixes
 *
 *     Revision 1.2  1993/01/01  01:21:29  dmwatt
 *     Add currentfile() to support strpool memory handling
 *
 *     Revision 1.2  1993/01/01  01:21:29  dmwatt
 *     Add currentfile() to support strpool memory handling
 *
 */

#include "uupcmoah.h"

#include <ctype.h>

#include <assert.h>

/*--------------------------------------------------------------------*/
/*                         Windows/NT include files                   */
/*--------------------------------------------------------------------*/

#define INCL_BASE

#include <WINDOWS.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "uundir.h"
#include "dos2unix.h"

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

   printmsg(5, "dirhandle = %d, file = %s", dirHandle, dirData.cFileName );

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
   } else {
      printmsg(5, "dirhandle = %d",dirHandle);
      strcpy( dirData.cFileName, "." );
   }

   while( equal(dirData.cFileName,".") ||
          equal(dirData.cFileName,".."))
   {
      rc = FindNextFile(dirHandle, &dirData);
      if ( rc )
         printmsg(9, "file = %s", dirData.cFileName);
      else
         break;
   }

   if ( rc )
   {

      dirp->dirent.d_ino = -1;   /* no inode information */
      strlwr(strcpy(dirp->dirent.d_name, dirData.cFileName));
      dirp->dirent.d_namlen = strlen(dirData.cFileName);

      printmsg(9, "%d",dirp->dirent.d_namlen);
      dirp->dirent.d_modified = nt2unix(&dirData.ftLastWriteTime);

      if (dirData.nFileSizeHigh > 0) {
         printmsg(0, "readdir:  File %s larger than 2^32 bits?!",
            dirData.cFileName);
         panic();
      }

      dirp->dirent.d_size = dirData.nFileSizeLow;
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

   if ( (dirp == NULL) || ! equal(dirp->dirid, "DIR"))
   {
      printmsg(0,"closedir: Invalid pointer argument (%p)",
                  dirp );
      panic();
   }

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
