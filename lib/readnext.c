/*--------------------------------------------------------------------*/
/*    r e a d n e x t . c                                             */
/*                                                                    */
/*    Reads a spooling directory with optional pattern matching       */
/*                                                                    */
/*    Copyright 1991 (C), Andrew H. Derbyshire                        */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "readnext.h"
#include "ndir.h"

/*--------------------------------------------------------------------*/
/*    r e a d n e x t                                                 */
/*                                                                    */
/*    Read a directory into a linked list                             */
/*--------------------------------------------------------------------*/

char     *readnext(char *xname,
          const char *remote,
          const char *subdir,
          char *pattern )
{
   static DIR *dirp;
   static char *saveremote = NULL;
   static char remotedir[FILENAME_MAX];

   struct direct *dp;

/*--------------------------------------------------------------------*/
/*          Determine if we must restart the directory scan           */
/*--------------------------------------------------------------------*/

   if ( (remote == NULL) || (saveremote == NULL ) ||
        !equal(remote, saveremote) )
   {
      if ( saveremote != NULL )  /* Clean up old directory? */
      {                          /* Yes --> Do so           */
         closedir(dirp);
         saveremote = NULL;
      } /* if */

      if ( remote == NULL )      /* Clean up only, no new search? */
         return NULL;            /* Yes --> Return to caller      */

      if ( pattern == NULL )
         pattern = "*.*";

      sprintf(remotedir,"%s/%.8s/%s",E_spooldir,remote, subdir);
      if ((dirp = opendirx(remotedir,pattern)) == nil(DIR))
      {
         printmsg(2, "readnext: couldn't opendir() %s", remotedir);
         return NULL;
      } /* if */

      saveremote = (char *) remote;
                              /* Flag we have an active search    */
   } /* if */

/*--------------------------------------------------------------------*/
/*              Look for the next file in the directory               */
/*--------------------------------------------------------------------*/

   if ((dp = readdir(dirp)) != nil(struct direct))
   {
      sprintf(xname, "%s/%s", remotedir, dp->d_name);
      printmsg(5, "readnext: matched \"%s\"",xname);
      return xname;
   }

/*--------------------------------------------------------------------*/
/*     No hit; clean up after ourselves and return to the caller      */
/*--------------------------------------------------------------------*/

   printmsg(5, "readnext: \"%s\" not matched", remotedir);
   closedir(dirp);
   saveremote = NULL;
   return NULL;

} /*readnext*/
