/*--------------------------------------------------------------------*/
/*    r e a d n e x t . c                                             */
/*                                                                    */
/*    Reads a spooling directory with optional pattern matching       */
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
 *    $Id: readnext.c 1.16 1999/01/04 03:52:55 ahd Exp $
 *
 *    $Log: readnext.c $
 *    Revision 1.16  1999/01/04 03:52:55  ahd
 *    Annual copyright change
 *
 *    Revision 1.15  1998/03/01 01:24:58  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.14  1997/03/31 07:05:58  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.13  1996/01/01 20:53:11  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.12  1995/03/24 04:17:22  ahd
 *    Compiler warning message cleanup, optimize for low memory processing
 *
 *    Revision 1.11  1994/12/22 00:10:38  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.10  1994/02/19 04:45:50  ahd
 *    Use standard first header
 *
 *     Revision 1.9  1994/02/19  03:56:48  ahd
 *     Use standard first header
 *
 *     Revision 1.9  1994/02/19  03:56:48  ahd
 *     Use standard first header
 *
 *     Revision 1.8  1994/02/18  23:13:48  ahd
 *     Use standard first header
 *
 *     Revision 1.7  1994/01/24  03:09:22  ahd
 *     Annual Copyright Update
 *
 *     Revision 1.6  1994/01/01  19:04:31  ahd
 *     Annual Copyright Update
 *
 *     Revision 1.5  1993/08/26  05:00:25  ahd
 *     Debugging code for odd failures on J. McBride's network
 *
 *     Revision 1.5  1993/08/26  05:00:25  ahd
 *     Debugging code for odd failures on J. McBride's network
 *
 *     Revision 1.4  1993/04/05  04:32:19  ahd
 *     Add timestamp, size to information returned by directory searches
 *
 *     Revision 1.3  1992/11/22  20:58:55  ahd
 *     Use strpool to allocate const strings
 *
 */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"

#include "readnext.h"
#include "uundir.h"
#include "hostable.h"
#include "security.h"

RCSID("$Id$");

/*--------------------------------------------------------------------*/
/*    r e a d n e x t                                                 */
/*                                                                    */
/*    Read a directory into a linked list                             */
/*--------------------------------------------------------------------*/

char     *readnext(char *xname,
                   const char *remote,
                   const char *subdir,
                   char *pattern,
                   time_t *modified,
                   long   *size )
{
   static DIR *dirp = NULL;
   static char *SaveRemote = NULL;
   static char remotedir[FILENAME_MAX];

   struct direct *dp;

/*--------------------------------------------------------------------*/
/*          Determine if we must restart the directory scan           */
/*--------------------------------------------------------------------*/

   if ( (remote == NULL) || ( SaveRemote == NULL ) ||
        !equal(remote, SaveRemote ) )
   {
      char *p;

      if ( SaveRemote != NULL )   /* Clean up old directory? */
      {                           /* Yes --> Do so           */
         closedir(dirp);
         dirp = NULL;
         SaveRemote = NULL;
      } /* if */

      if ( remote == NULL )      /* Clean up only, no new search? */
         return NULL;            /* Yes --> Return to caller      */

      if ( pattern == NULL )
         pattern = "*.*";

/*--------------------------------------------------------------------*/
/*            Build spool directory/host name combination             */
/*--------------------------------------------------------------------*/

      p = strcpy( remotedir, E_spooldir );
      p += strlen( remotedir );
      *p++ = '/';

      strncpy( p, remote, 8);
      p[8] = '\0';

      strcat( p, "/" );
      strcat( p, subdir );

/*--------------------------------------------------------------------*/
/*             Look for files in our specified directory              */
/*--------------------------------------------------------------------*/

      if ((dirp = opendirx(remotedir,pattern)) == nil(DIR))
      {
         printmsg(5, "readnext: couldn't opendir() %s", remotedir);
         dirp = NULL;
         return NULL;
      } /* if */

      SaveRemote = newstr( remote );
                              /* Flag we have an active search    */
   } /* if */

/*--------------------------------------------------------------------*/
/*              Look for the next file in the directory               */
/*--------------------------------------------------------------------*/

   if ((dp = readdir(dirp)) != nil(struct direct))
   {
      mkfilename( xname, remotedir, dp->d_name );

      printmsg(5, "readnext: matched \"%s\"",xname);

      if ( modified != NULL )
         *modified = dp->d_modified;

      if ( size != NULL )
         *size = dp->d_size;

      return xname;
   }

/*--------------------------------------------------------------------*/
/*     No hit; clean up after ourselves and return to the caller      */
/*--------------------------------------------------------------------*/

   printmsg(5, "readnext: \"%s\" not matched", remotedir);
   closedir(dirp);
   SaveRemote = NULL;
   dirp = NULL;
   return NULL;

} /*readnext*/
