/*--------------------------------------------------------------------*/
/*    ndir.c for MS-DOS by Samuel Lam <skl@van-bc.UUCP>, June/87      */
/*    ndir.c for MS-OS2 by Drew Derbyshire (help@kendra.kew.com>,     */
/*           April/91                                                 */
/*                                                                    */
/*         Berkeley-style directory reading routine on MS-OS2         */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

/*--------------------------------------------------------------------*/
/*                         OS/2 include files                         */
/*--------------------------------------------------------------------*/

#define INCL_BASE
#include <os2.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "ndir.h"

static HDIR dir_handle;
static char *pathname = NULL;
static struct _FILEFINDBUF findbuf;

currentfile();

/*--------------------------------------------------------------------*/
/*    o p e n d i r                                                   */
/*                                                                    */
/*    Open a directory                                                */
/*--------------------------------------------------------------------*/

extern DIR *opendirx( const char *dirname, char *pattern)
{

   DIR *dirp;
   USHORT rc;
   USHORT count = 1;

   pathname = malloc( strlen( dirname ) + strlen( pattern ) + 2 );
   strcpy(pathname, dirname);

   if ((*pattern != '/') && (dirname[ strlen(dirname) - 1] != '/'))
      strcat(pathname,"/");
   strcat(pathname,pattern);
   printmsg(5,"opendir: Opening directory %s", pathname );

/*--------------------------------------------------------------------*/
/*                Read the first file in the directory                */
/*--------------------------------------------------------------------*/

   dir_handle = HDIR_CREATE;
   rc = DosFindFirst( pathname,
            &dir_handle,
            FILE_NORMAL,
            &findbuf,
            sizeof( findbuf ),
            &count,
            0L );

/*--------------------------------------------------------------------*/
/*            Process the return code from the first file             */
/*--------------------------------------------------------------------*/

   if ( rc == 0 )
   {
      dirp = malloc( sizeof( DIR ));
      dirp->dirfirst = 1;
      strcpy(dirp->dirid, "DIR");
      return dirp;
   }
   else {
      if (( rc != ERROR_NO_MORE_FILES ) &&
          ( rc != ERROR_PATH_NOT_FOUND))
         printmsg(4,"opendir: Error %d on directory %s",
                  (int) rc, pathname );
      return NULL;
   } /* else */

} /*opendir*/

/*--------------------------------------------------------------------*/
/*    r e a d d i r                                                   */
/*                                                                    */
/*    Get next entry in a directory                                   */
/*--------------------------------------------------------------------*/

struct direct *readdir(DIR *dirp)
{
   USHORT rc = 0;
   USHORT count = 1;

   if ( ! equal(dirp->dirid, "DIR" ))
   {
      printmsg(0,"readdir: No directory open to read");
      panic();
   }

   if (dirp->dirfirst)
   {
      printmsg(5,"readdir: Opening directory %s", pathname );
      dirp->dirfirst = 0;
   }
   else
      rc = DosFindNext( dir_handle,
               &findbuf,
               sizeof( findbuf ) ,
               &count );

   if ( rc == 0 )
   {
      dirp->dirent.d_ino = -1;   /* no inode information */
      strlwr(strcpy(dirp->dirent.d_name, findbuf.achName ));
      dirp->dirent.d_namlen = findbuf.cchName;
      dirp->dirent.d_reclen = sizeof(struct direct) - (MAXNAMLEN + 1) +
         ((((dirp->dirent.d_namlen + 1) + 3) / 4) * 4);
      printmsg(4,"readdir: Returning \"%s\"", dirp->dirent.d_name);
      return &(dirp->dirent);
   }
   else {
      if ( rc != ERROR_NO_MORE_FILES )
         printmsg(0,"readdir: Error %d on directory %s",
                  (int) rc, pathname );
      return NULL;
   } /* else */

} /*readdir*/

/*--------------------------------------------------------------------*/
/*    c l o s e d i r                                                 */
/*                                                                    */
/*    Close a directory                                               */
/*--------------------------------------------------------------------*/

void closedir(DIR *dirp)
{
   USHORT rc;

   if ( ! equal(dirp->dirid, "DIR" ))
   {
      printmsg(0,"closedir: No directory open");
      panic();
   }

   printmsg(5,"closedir: Closing directory %s", pathname );
   rc = DosFindClose( dir_handle );
   if ( rc != 0 )
     printmsg(0,"closedir: Error %d on directory %s",
              (int) rc, pathname );
   free( dirp );
   dirp = NULL;
   free( pathname );
   pathname = NULL;

} /*closedir*/
