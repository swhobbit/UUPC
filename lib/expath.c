/*--------------------------------------------------------------------*/
/*    e x p a t h . c                                                 */
/*                                                                    */
/*    Path expansion functions for UUPC/extended                      */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                    MS-DOS and OS/2 header files                    */
/*--------------------------------------------------------------------*/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#ifndef __GNUC__
#include <direct.h>
#endif

/*--------------------------------------------------------------------*/
/*                     UUPC/extended header files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "expath.h"
#include "hlib.h"
#include "hostable.h"
#include "security.h"
#include "usertabl.h"

/*--------------------------------------------------------------------*/
/*   e x p a n d _ p a t  h                                           */
/*                                                                    */
/*   Expands ~, ~/ and relative paths                                 */
/*--------------------------------------------------------------------*/

char *expand_path(char *path,          /* Input/output path name     */
                  const char *cur_dir, /* Default directory path     */
                  const char *home,    /* Default home directory     */
                  const char *ftype )  /* Default extension          */
{
   char        *p, *fname;
   char        save[FILENAME_MAX];
   struct UserTable *userp;

/*--------------------------------------------------------------------*/
/*                   Convert backslashes to slashes                   */
/*--------------------------------------------------------------------*/

   p  = path;
   while ((p = strchr(p,'\\')) != NULL)
      *p++ = '/';

/*--------------------------------------------------------------------*/
/*                 Add optional extension, if needed                  */
/*--------------------------------------------------------------------*/

   if ( ftype != NULL )
   {
      p = strrchr(path,'/');  /* Get the last slash in name          */

      if ( p == NULL )        /* No slash?                           */
         p = path;            /* Okay, look at entire name           */

      if ( strchr( p , '.') == NULL )  /* Does name have a period?   */
         strcat( strcat(p, ".") ,ftype );
                              /* No --> Add extension                */
   } /* if ( ftype != NULL ) */

/*--------------------------------------------------------------------*/
/*               If a fully qualified path name, return               */
/*--------------------------------------------------------------------*/

#ifdef __GNUC__
   if (*path == '/')
      return path;            /* nothing to do */
#endif

   if ((*path == '/') || (isalpha( *path ) && (path[1] == ':')))
   {
#ifdef __GNUC__
      if (path[2] == '/')     /* Absolute path on drive?             */
         return path;         /* Yes --> Leave it alone              */

      printf(0,"Relative path \"%s\" not supported in GNU C",
               path);
      return NULL;
#else
      strcpy( save, path );
      p = _fullpath( path, save, sizeof save );

      while ((p = strchr(p,'\\')) != NULL)
         *p++ = '/';

      return path;
#endif

   } /* if */

/*--------------------------------------------------------------------*/
/*            Try to translate the file as a home directory path      */
/*--------------------------------------------------------------------*/

   p = path;                  /* Copy entire path                    */
   strcpy(save, p);
   if (save[0] == '~')  {
      if (save[1] == '/')  {
         strcpy(path, home);  /* Use home dir for this user          */
         fname = save + 2;    /* Step past directory for simple name */
      }
      else  {
         if ((fname = strchr(save + 1, '/')) == NULL)
         {
            printmsg(0,"expand_path: path \"%s\" illegal",p);
            return NULL;
         }

/*--------------------------------------------------------------------*/
/*                Look in /etc/passwd for the user id                 */
/*--------------------------------------------------------------------*/

         *fname++ = '\0';           /* End string, step past it */
         userp = checkuser(save + 1);  /* Locate user id in table  */
         if ( userp == BADUSER )    /* Invalid user id?         */
         {                          /* Yes --> Dump in trash    */
            printmsg(0,"expand_path: User \"%s\" is invalid", save + 1);
            return NULL;
         } /* if */
         strcpy(path, userp->homedir);
      } /* else */
   } /* if (save[0] == '~')  */

/*--------------------------------------------------------------------*/
/*    No user id appears in the path; just append the input data      */
/*    to the current directory to convert the relative path to an     */
/*    absolute path                                                   */
/*--------------------------------------------------------------------*/

   else {
         fname = save;              /* Give it the file name - 6/23/91  */
         if ( cur_dir == NULL )
            getcwd( path, FILENAME_MAX);
         else if ( equal(cur_dir,"."))
         {
            strcpy( path, save );
            return path;
         }
         else
            strcpy( path, cur_dir );
   } /* else */

/*--------------------------------------------------------------------*/
/*             Normalize the path, and then add the name              */
/*--------------------------------------------------------------------*/

   while ((p = strchr(p,'\\')) != NULL)
      *p++ = '/';
   if ( path[ strlen( path ) - 1 ] != '/' )
      strcat( path, "/");
   strlwr( path );            /* Can lower case path, but not the
                                 name because name may be UNIX!      */
   strcat( path, fname );

/*--------------------------------------------------------------------*/
/*                       Return data to caller                        */
/*--------------------------------------------------------------------*/

   return path;
} /* expand_path */
