/*--------------------------------------------------------------------*/
/*    e x p a t h . c                                                 */
/*                                                                    */
/*    Path expansion functions for UUPC/extended                      */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-2001 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: expath.c 1.27 2000/05/12 12:29:45 ahd v1-13g $
 *
 *    Revision history:
 *    $Log: expath.c $
 *    Revision 1.27  2000/05/12 12:29:45  ahd
 *    Annual copyright update
 *
 *    Revision 1.26  1999/01/08 02:20:43  ahd
 *    Convert currentfile() to RCSID()
 *
 *    Revision 1.25  1999/01/04 03:52:28  ahd
 *    Annual copyright change
 *
 *    Revision 1.24  1998/03/01 01:23:30  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.23  1997/03/31 06:58:55  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.22  1996/01/01 20:51:04  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.21  1995/02/14 04:38:42  ahd
 *    Correct problems with directory processing under NT
 *
 *    Revision 1.20  1995/02/12 23:37:04  ahd
 *    compiler cleanup, NNS C/news support, optimize dir processing
 *
 *    Revision 1.19  1995/01/30 04:05:08  ahd
 *    Additional compiler warning fixes, optimize path normalizing
 *
 *    Revision 1.18  1995/01/29 16:43:03  ahd
 *    IBM C/Set compiler warnings
 *
 *    Revision 1.17  1995/01/28 23:13:11  ahd
 *    Use isAbsolutePath()
 *
 *    Revision 1.16  1995/01/07 16:12:21  ahd
 *    Change KWBoolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.15  1994/12/22 00:08:15  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.14  1994/02/19 04:40:47  ahd
 *    Use standard first header
 *
 *     Revision 1.13  1994/02/19  04:05:26  ahd
 *     Use standard first header
 *
 *     Revision 1.12  1994/02/19  03:49:31  ahd
 *     Use standard first header
 *
 *     Revision 1.11  1994/02/18  23:08:50  ahd
 *     Use standard first header
 *
 *     Revision 1.10  1994/01/01  19:01:38  ahd
 *     Annual Copyright Update
 *
 *     Revision 1.9  1993/11/06  17:54:55  rhg
 *     Drive Drew nuts by submitting cosmetic changes mixed in with bug fixes
 *
 *     Revision 1.8  1993/10/30  17:10:40  rhg
 *     Be more conservative about the need to push to new directory
 *
 *     Revision 1.7  1993/10/12  00:43:34  ahd
 *     Normalize comments
 *
 *     Revision 1.6  1993/08/08  17:39:09  ahd
 *     Denormalize path for opening on selected networks
 *
 */

/*--------------------------------------------------------------------*/
/*                    MS-DOS and OS/2 header files                    */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"

#include <ctype.h>

#include <direct.h>

/*--------------------------------------------------------------------*/
/*                     UUPC/extended header files                     */
/*--------------------------------------------------------------------*/

#include "expath.h"
#include "hostable.h"
#include "security.h"
#include "usertabl.h"
#include "pushpop.h"

/*--------------------------------------------------------------------*/
/*                          Local variables                           */
/*--------------------------------------------------------------------*/

RCSID("$Id: expath.c 1.27 2000/05/12 12:29:45 ahd v1-13g $");

/*--------------------------------------------------------------------*/
/*   e x p a n d _ p a t  h                                           */
/*                                                                    */
/*   Expands ~, ~/ and relative paths                                 */
/*--------------------------------------------------------------------*/

char *expand_path(char *input,         /* Input/output path name      */
                  const char *cur_dir, /* Default directory path      */
                  const char *home,    /* Default home directory      */
                  const char *ftype )  /* Default extension           */
{
   char        *p, *fname;
   char        save[FILENAME_MAX];
   char        path[FILENAME_MAX];
   struct UserTable *userp;

/*--------------------------------------------------------------------*/
/*                   Convert backslashes to slashes                   */
/*--------------------------------------------------------------------*/

   strcpy(path, input);
   renormalize( path );

/*--------------------------------------------------------------------*/
/*                 Add optional extension, if needed                  */
/*--------------------------------------------------------------------*/

   if ( ftype != NULL )
   {
      p = strrchr(path,'/');  /* Get the last slash in name           */

      if ( p == NULL )        /* No slash?                            */
         p = path;            /* Okay, look at entire name            */

      if (( strchr( p , '.') == NULL ) && (*p != '~'))
                              /* Does name have a period?             */
         strcat( strcat(p, ".") ,ftype );
                              /* No --> Add extension                 */
   } /* if ( ftype != NULL ) */

/*--------------------------------------------------------------------*/
/*               If a fully qualified path name, return               */
/*--------------------------------------------------------------------*/

   if (isAbsolutePath( path ))
   {

      if ( strstr( path, "..") == NULL )
      {
         if (( path[0] == '/' ) && ( path[1] != '/' ))
         {
            memmove( path + 2, path, strlen( path ) + 1);

            path[0] = getDrive( cur_dir );
            path[1] = ':';
         }
      }
      else {
         KWBoolean push;

         strcpy( save, path );

         if (cur_dir == NULL )
            push = KWFalse;
         else {

            PushDir( cur_dir );
            push = KWTrue;

         }

         p = _fullpath( path, save, sizeof save );

         if ( p == NULL )
         {
            printerr( path );
            return NULL;
         }

         if (push)
            PopDir();
      }

      renormalize( path );

#ifdef UDEBUG
      printmsg(5,"expand_path: cwd = %s, input = %s, output = %s",
                  cur_dir ? cur_dir : E_cwd,
                  input,
                  path );
#endif

      return strcpy( input, path );

   } /* if */

/*--------------------------------------------------------------------*/
/*            Try to translate the file as a home directory path      */
/*--------------------------------------------------------------------*/

   p = path;                  /* Copy entire path                     */
   strcpy(save, p);

   if (save[0] == '~')
   {
      if (save[1] == '/')
      {
         strcpy(path, home);  /* Use home dir for this user           */
         fname = save + 2;    /* Step past directory for simple name  */
      }
      else if ( save[1] == '\0')
      {
         strcpy(path, home);  /* Use home dir for this user           */
         fname = save + 1;    /* Step past directory for simple name  */
      }
      else {

         if ((fname = strchr(save + 1, '/')) == NULL)
            fname = save + strlen(save);  /* That's all, folks!       */
         else
            *fname++ = '\0';           /* End string, step past it */

/*--------------------------------------------------------------------*/
/*                Look in /etc/passwd for the user id                 */
/*--------------------------------------------------------------------*/

         userp = checkuser(save + 1);  /* Locate user id in table  */

         if ( userp == BADUSER )    /* Invalid user id?         */
         {                          /* Yes --> Dump in trash    */
            printmsg(0,"expand_path: User \"%s\" is invalid", save + 1);
            return NULL;
         } /* if */

         strcpy(path, userp->homedir);

      } /* else */

   } /* if (save[0] == '~') */

/*--------------------------------------------------------------------*/
/*    No user id appears in the path; just append the input data      */
/*    to the current directory to convert the relative path to an     */
/*    absolute path                                                   */
/*--------------------------------------------------------------------*/

   else {
         fname = save;              /* Save entire file name          */

         if ( cur_dir != NULL )
            strcpy( path, cur_dir );
         else if ( E_cwd != NULL )
            strcpy( path, E_cwd );
         else
            getcwd( path, FILENAME_MAX);

   } /* else */

/*--------------------------------------------------------------------*/
/*             Normalize the path, and then add the name              */
/*--------------------------------------------------------------------*/

   renormalize( p );

   strlwr( path );            /* Can lower case path, but not the
                                 name because name may be UNIX!       */

   p = path + strlen(path);
   if (*path == '\0' || p[-1] != '/')
      *p++ = '/';

   strcpy( p, fname );

/*--------------------------------------------------------------------*/
/*                       Return data to caller                        */
/*--------------------------------------------------------------------*/

#ifdef UDEBUG
   printmsg(5,"expand_path: cwd = %s, input = %s, output = %s",
               cur_dir ? cur_dir : E_cwd,
               input,
               path );
#endif

   return strcpy( input, path );

} /* expand_path */
