/*--------------------------------------------------------------------*/
/*       r e m o v e 2 . c                                            */
/*                                                                    */
/*       OS/2 specific delete processing                              */
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
 *    $Id: remove2.c 1.1 1995/03/12 16:42:24 ahd Exp $
 *
 *    Revision history:
 *    $Log: remove2.c $
 *    Revision 1.1  1995/03/12 16:42:24  ahd
 *    Initial revision
 *
 */

#include "uupcmoah.h"
#include "pos2err.h"

#include "errno.h"

#define INCL_DOS
#define INCL_ERRORS
#include <os2.h>

/*--------------------------------------------------------------------*/
/*       r e m o v e O S 2 F i l e                                    */
/*                                                                    */
/*       Perform special delete processing under OS/2, if so          */
/*       configured.                                                  */
/*--------------------------------------------------------------------*/

int removeOS2File( const char *fname, const char *file, const size_t line )
{

   APIRET rc;

/*--------------------------------------------------------------------*/
/*       If the user wants normal UNDELETE processing to proceed,     */
/*       we just use the regular UNIX system call                     */
/*--------------------------------------------------------------------*/

   if ( bflag[F_UNDELETE] )
      return unlink( fname );

   if ( fname == NULL )
   {
      errno = EINVAL;
      return -1;
   }

   rc = DosForceDelete( (PSZ) fname );

   if ( rc == 0 )
      return 0;

/*--------------------------------------------------------------------*/
/*       We had a problem.  Report the OS/2 error if unique,          */
/*       otherwise just translate the return code and pray.           */
/*--------------------------------------------------------------------*/

   switch( rc )
   {
      case ERROR_FILE_NOT_FOUND:
      case ERROR_PATH_NOT_FOUND:
         errno = ENOTEXIST;
         break;

      case ERROR_ACCESS_DENIED:
         errno = EACCESS;
         break;

      default:
         pOS2Err( line, file, fname, rc);
         errno = EOS2ERR;
         break;

   } /* switch */

   return (int) rc;

} /* removeOS2File */
