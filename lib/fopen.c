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
#include <string.h>
#include <time.h>

#define SHARE_OPEN

#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>
#include <io.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "hlib.h"

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

   printmsg(4, "Opening %s for %s, share flags %d",
            name, mode, share );

   results = _fsopen(name, mode, share );
#else
   results = fopen(name, mode );
#endif

   if ((results != nil(FILE)) || (*mode == 'r'))
      return results;

   /* verify all intermediate directories in the path exist */

   if ((last = strrchr(name, '/')) != nil(char))
   {
      *last = '\0';
      MKDIR(name);
      *last = '/';
   }

   /* now try open again */


#ifdef __SHAREOPEN__
   return _fsopen(name, mode, share);
#else
   return fopen(name, mode);
#endif

} /*FOPEN*/
