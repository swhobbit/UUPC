/*--------------------------------------------------------------------*/
/*    f o p e n . c                                                   */
/*                                                                    */
/*    Support routines for UUPC/extended                              */
/*                                                                    */
/*    Changes Copyright 1990, 1991 (c) Andrew H. Derbyshire           */
/*                                                                    */
/*    History:                                                        */
/*       21Nov1991 Break out of lib.c                          ahd    */
/*--------------------------------------------------------------------*/

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>
#include <io.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "hlib.h"
#include "ssleep.h"

#define SHARE_OPEN

/*--------------------------------------------------------------------*/
/*    F O P E N                                                       */
/*                                                                    */
/*    Like fopen() but create imtermediate directories                */
/*                                                                    */
/*    This routine has dependency on the path separator characters    */
/*    being '/', we should relove that somehow someday.               */
/*--------------------------------------------------------------------*/

FILE *FSOPEN(const char *name, const char *mode)
{

   char *last;
   FILE *results;
   char fname[FILENAME_MAX];

   int share = SH_DENYWR;
   int retries = 0;

   strcpy( fname, name );
   denormalize( fname );

/*--------------------------------------------------------------------*/
/*                       Open file (first try)                        */
/*--------------------------------------------------------------------*/

   results = _fsopen(fname, mode, share );

/*--------------------------------------------------------------------*/
/*       Return if the file opened, or if in read mode (no            */
/*       directories need to be built) and not in multi-tasking       */
/*       mode (no retries).                                           */
/*--------------------------------------------------------------------*/

   if (results != nil(FILE) ||
      ( (*mode == 'r') && !bflag[ F_MULTITASK ]) && (errno != EACCES) )
      return results;

/*--------------------------------------------------------------------*/
/*       Verify all intermediate directories in the path exist        */
/*--------------------------------------------------------------------*/


   if ((*mode != 'r') && ((last = strrchr(name, '/')) != nil(char)))
   {
      *last = '\0';
      MKDIR(name);
      *last = '/';
   }

/*--------------------------------------------------------------------*/
/*                         Now try open again                         */
/*--------------------------------------------------------------------*/

   for ( ;; )
   {
      results = _fsopen(fname, mode, share);
      if (( results != NULL ) || (!bflag[ F_MULTITASK ]) ||
          (errno != EACCES)   || (retries++ > 10))
         return results;
      perror( fname );
      ssleep( retries * 2);
   }

} /*FOPEN*/
