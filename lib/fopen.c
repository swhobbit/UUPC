/*--------------------------------------------------------------------*/
/*    f o p e n . c                                                   */
/*                                                                    */
/*    File open function retry and locking                            */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1995 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: fopen.c 1.12 1994/06/13 00:09:40 ahd v1-12k $
 *
 *    Revision history:
 *    $Log: fopen.c $
 *    Revision 1.12  1994/06/13 00:09:40  ahd
 *    Correct inverted test for multitask mode when determining
 *    retry count.
 *
 *     Revision 1.11  1994/06/05  01:57:26  ahd
 *     Correct trap sharing violations under OS/2 32 bit compilers
 *
 */

#include "uupcmoah.h"

#include <fcntl.h>

#include <errno.h>
#include <sys/stat.h>
#include <share.h>
#include <io.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "ssleep.h"

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
   int maxRetries = bflag[ F_MULTITASK ] ? 10 : 0;
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

   if (results != nil(FILE))
      return results;

   if (*mode == 'r')
   {
      if ((!bflag[ F_MULTITASK ]) || access( fname, 0 ))
         return results;
   }
   else if ((last = strrchr(name, '/')) != nil(char))
   {                                /* Make any needed directories    */
      *last = '\0';
      MKDIR(name);
      *last = '/';
   } /* else */

/*--------------------------------------------------------------------*/
/*                         Now try open again                         */
/*--------------------------------------------------------------------*/

   while( (results = _fsopen(fname, mode, share)) == NULL )
   {
      perror( fname );

      if ( retries++ > maxRetries )   /* Fall through and return     */
         break;

/*--------------------------------------------------------------------*/
/*                Determine if we care about the error                */
/*--------------------------------------------------------------------*/

      switch( errno )
      {
#ifdef EISOPEN
         case EISOPEN:              /* File is open (OS/2 only)      */
#endif
         case EACCES:               /* DOS share error               */
            ssleep( retries * 2);
            break;

        default:                    /* Error we don't attempt to trap*/
            return results;         /* So just return immediately    */

      } /* switch */

   }  /* while */

/*--------------------------------------------------------------------*/
/*              Return final success or failure to user               */
/*--------------------------------------------------------------------*/

   return results;

} /* FOPEN */
