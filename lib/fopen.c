/*--------------------------------------------------------------------*/
/*       f o p e n . c                                                */
/*                                                                    */
/*       File open function retry and locking                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1998 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: fopen.c 1.18 1997/12/15 03:54:19 ahd Exp $
 *
 *    Revision history:
 *    $Log: fopen.c $
 *    Revision 1.18  1997/12/15 03:54:19  ahd
 *    Add missing header for stater
 *
 *    Revision 1.17  1997/12/14 21:12:18  ahd
 *    Don't loop if error is trying to open directory under NT
 *
 *    Revision 1.16  1997/03/31 06:59:11  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.15  1996/01/01 20:51:43  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.14  1995/03/11 22:33:46  ahd
 *    Make IBM C/Set++ specific macro local
 *
 *    Revision 1.13  1994/12/22 00:08:27  ahd
 *    Annual Copyright Update
 *
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

#ifdef __IBMC__
#define _fsopen(n, m, s) fopen(n, m)
#endif

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "ssleep.h"
#include "stater.h"

RCSID("$Id: fopen.c 1.18 1997/12/15 03:54:19 ahd Exp $");

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
   long size;

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

/*--------------------------------------------------------------------*/
/*                Determine if we care about the error                */
/*--------------------------------------------------------------------*/

      switch( errno )
      {
         case EACCES:               /* DOS share error               */
#ifdef EISDIR
            if (!retries &&
                (stater(fname, &size) == -1) &&
                (errno == EISDIR))
               return results;      /* No, really directory!         */
#endif /* EISDIR */
            /* Fall through */

#ifdef EISOPEN
         case EISOPEN:              /* File is open (OS/2 only)      */
#endif
            ssleep( retries * 2);
            break;

        default:                    /* Error we don't attempt to trap*/
            return results;         /* So just return immediately    */

      } /* switch */

      if ( retries++ > maxRetries )   /* Fall through and return     */
         break;

   }  /* while */

/*--------------------------------------------------------------------*/
/*              Return final success or failure to user               */
/*--------------------------------------------------------------------*/

   return results;

} /* FOPEN */
