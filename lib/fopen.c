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

   /* are we opening for write or append */

#ifdef SHARE_OPEN
   int share = SH_DENYWR;
   int retries = 0;

   results = _fsopen(name, mode, share );
#else
   results = fopen(name, mode );
#endif

   if ((results != nil(FILE)) || (*mode == 'r'))
      return results;

/*--------------------------------------------------------------------*/
/*       Verify all intermediate directories in the path exist        */
/*--------------------------------------------------------------------*/


   if ((last = strrchr(name, '/')) != nil(char))
   {
      *last = '\0';
      MKDIR(name);
      *last = '/';
   }

/*--------------------------------------------------------------------*/
/*                         Now try open again                         */
/*--------------------------------------------------------------------*/

#ifdef SHARE_OPEN
   for ( ;; )
   {
      results = _fsopen(name, mode, share);
      if (( results != NULL ) || (!bflag[ F_MULTITASK ]) ||
          (errno != EACCES)   || (retries++ > 10))
         return results;
      perror( name );
      ssleep( retries * 2);
   }
#else
   return fopen(name, mode);
#endif

} /*FOPEN*/
