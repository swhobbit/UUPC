/*--------------------------------------------------------------------*/
/*       n d i r w i n . c                                            */
/*                                                                    */
/*       UUPC/extended directory search routine for MS-Windows 3.x    */
/*       environment.                                                 */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1993 by Robert Denny                   */
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
 *    $Id: ndirwin.c 1.11 1998/03/01 01:24:35 ahd v1-13f ahd $
 *
 *    Revision history:
 *    $Log: ndirwin.c $
 *    Revision 1.11  1998/03/01 01:24:35  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.10  1997/03/31 07:05:16  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.9  1996/01/01 20:49:42  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.8  1994/12/22 00:09:57  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.7  1994/02/19 04:44:40  ahd
 *    Use standard first header
 *
 * Revision 1.6  1994/02/19  04:09:47  ahd
 * Use standard first header
 *
 * Revision 1.5  1994/02/19  03:55:17  ahd
 * Use standard first header
 *
 * Revision 1.4  1994/02/18  23:12:41  ahd
 * Use standard first header
 *
 * Revision 1.3  1994/01/01  19:03:46  ahd
 * Annual Copyright Update
 *
 * Revision 1.2  1993/07/31  16:22:16  ahd
 * Changes in support of Robert Denny's Windows 3.x support
 *
 * Revision 1.2  1993/07/31  16:22:16  ahd
 * Changes in support of Robert Denny's Windows 3.x support
 *
 * Revision 1.1  1993/07/22  23:19:50  ahd
 * Initial revision
 *
 */

#include "uupcmoah.h"

#include <ctype.h>

#include <dir.h>
#include <dos.h>
#include <errno.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "uundir.h"
#include "dos2unix.h"

#define USHORT unsigned short

static char *pathname = NULL;
static struct ffblk findbuf;

currentfile();

/*--------------------------------------------------------------------*/
/*    o p e n d i r x                                                 */
/*                                                                    */
/*    Open a directory                                                */
/*--------------------------------------------------------------------*/

extern DIR *opendirx( const char *dirname, char *pattern)
{

   DIR *dirp;
   USHORT rc;

   pathname = malloc( strlen( dirname ) + strlen( pattern ) + 2 );
   strcpy(pathname, dirname);

   if ((*pattern != '/') && (dirname[ strlen(dirname) - 1] != '/'))
      strcat(pathname,"/");
   strcat(pathname,pattern);
   printmsg(5,"opendir: Opening directory %s", pathname );

/*--------------------------------------------------------------------*/
/*                Read the first file in the directory                */
/*--------------------------------------------------------------------*/

   rc = findfirst( pathname, &findbuf, FA_NORMAL);

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
   else
   {
          if (( rc != ENMFILE ) && ( rc != ENOENT ))
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
          rc = findnext(&findbuf);

   if ( rc == 0 )
   {
      dirp->dirent.d_ino = -1;   /* no inode information */
      strlwr(strcpy(dirp->dirent.d_name, findbuf.ff_name ));
          dirp->dirent.d_namlen = strlen(findbuf.ff_name);
      dirp->dirent.d_reclen = sizeof(struct direct) - (MAXNAMLEN + 1) +
         ((((dirp->dirent.d_namlen + 1) + 3) / 4) * 4);

      dirp->dirent.d_modified = dos2unix( *((FDATE *) &findbuf.ff_fdate),
                                          *((FTIME *) &findbuf.ff_ftime));

      printmsg(4,"readdir: Returning \"%s\"", dirp->dirent.d_name);
      return &(dirp->dirent);
   }
   else {
          if (( errno != ENMFILE ) && ( errno != ENOENT ))
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

   if ( ! equal(dirp->dirid, "DIR" ))
   {
      printmsg(0,"closedir: No directory open");
      panic();
   }

   printmsg(5,"closedir: Closing directory %s", pathname );
   free( dirp );
   dirp = NULL;
   free( pathname );
   pathname = NULL;

} /*closedir*/
