/*--------------------------------------------------------------------*/
/*       n d i r o s 2 . c                                            */
/*                                                                    */
/*       UUPC/extended OS/2 directory search functions                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1999 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: ndiros2.c 1.20 1999/01/04 03:52:55 ahd Exp $
 *
 *    Revision history:
 *    $Log: ndiros2.c $
 *    Revision 1.20  1999/01/04 03:52:55  ahd
 *    Annual copyright change
 *
 *    Revision 1.19  1998/03/01 01:24:33  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.18  1997/03/31 07:05:12  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.17  1996/01/01 20:54:31  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.16  1995/09/24 19:07:05  ahd
 *    Correct for 16 bit OS/2 compile
 *
 *    Revision 1.15  1995/07/21 13:23:19  ahd
 *    Clean up OS/2 compiler warnings
 *
 *    Revision 1.14  1994/12/22 00:09:52  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.13  1994/02/20 19:05:02  ahd
 *    IBM C/Set 2 Conversion, memory leak cleanup
 *
 *     Revision 1.12  1994/02/19  04:44:30  ahd
 *     Use standard first header
 *
 *     Revision 1.11  1994/02/19  04:09:37  ahd
 *     Use standard first header
 *
 *     Revision 1.10  1994/02/19  03:55:03  ahd
 *     Use standard first header
 *
 *     Revision 1.9  1994/02/18  23:12:30  ahd
 *     Use standard first header
 *
 *     Revision 1.8  1994/01/01  19:03:38  ahd
 *     Annual Copyright Update
 *
 *     Revision 1.7  1993/10/03  20:37:34  ahd
 *     Delete debugging message of returned value to make consistent with
 *     other environment messages from ndir()
 *
 *     Revision 1.7  1993/10/03  20:37:34  ahd
 *     Delete debugging message of returned value to make consistent with
 *     other environment messages from ndir()
 *
 *     Revision 1.6  1993/09/20  04:39:51  ahd
 *     OS/2 2.x support
 *
 *     Revision 1.5  1993/07/20  21:45:37  ahd
 *     Clean up header
 *
 *     Revision 1.4  1993/04/05  12:26:01  ahd
 *     Drop RCSID
 *
 *     Revision 1.3  1993/04/05  04:32:19  ahd
 *     Add timestamp, size to information returned by directory searches
 *
 *     Revision 1.2  1993/04/04  19:35:14  ahd
 *     Include time_t timestamp
 *
 */

/*--------------------------------------------------------------------*/
/*    ndir.c for MS-DOS by Samuel Lam <skl@van-bc.UUCP>, June/87      */
/*    ndir.c for MS-OS2 by Drew Derbyshire (help@kendra.kew.com>,     */
/*           April/91                                                 */
/*                                                                    */
/*         Berkeley-style directory reading routine on MS-OS2         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"

#include <ctype.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "uundir.h"
#include "dos2unix.h"

static HDIR dir_handle;
static char *pathname = NULL;

#if defined(__OS2__)

#define FINDFIRST_LEVEL (FIL_STANDARD)    /* OS/2 2.x requires different
                                             parameters to find first
                                             file */

static FILEFINDBUF3 findbuf;

#else

typedef USHORT APIRET ;  /* Define older API return type              */

#define FINDFIRST_LEVEL (0L)
static FILEFINDBUF findbuf;

#endif

RCSID("$Id$");

/*--------------------------------------------------------------------*/
/*    o p e n d i r                                                   */
/*                                                                    */
/*    Open a directory                                                */
/*--------------------------------------------------------------------*/

extern DIR *opendirx( const char *dirname, char *pattern)
{

   DIR *dirp;

#ifdef __OS2__
   ULONG count = 1L;
#else
   USHORT count = 1;
#endif

   APIRET rc;

   pathname = malloc( strlen( dirname ) + strlen( pattern ) + 2 );
   strcpy(pathname, dirname);

   if ((*pattern != '/') && (dirname[ strlen(dirname) - 1] != '/'))
      strcat(pathname,"/");
   strcat(pathname,pattern);
   printmsg(5,"opendir: Opening directory %s", pathname );

/*--------------------------------------------------------------------*/
/*                Read the first file in the directory                */
/*--------------------------------------------------------------------*/

   dir_handle = (HDIR) HDIR_CREATE;

   rc = DosFindFirst( pathname,
            &dir_handle,
            FILE_NORMAL,
            &findbuf,
            sizeof( findbuf ),
            &count,
            FINDFIRST_LEVEL );

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
      free( pathname );
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

#if defined(__OS2__)
   ULONG count = 1L;
#else
   APIRET count = 1;
#endif

   APIRET rc = 0;

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
      dirp->dirent.d_reclen = (short) (sizeof(struct direct) - (MAXNAMLEN + 1) +
         ((((dirp->dirent.d_namlen + 1) + 3) / 4) * 4));
      dirp->dirent.d_modified = dos2unix( findbuf.fdateLastWrite,
                                         findbuf.ftimeLastWrite );
      dirp->dirent.d_size     = (long) findbuf.cbFile;

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
   APIRET rc;

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
