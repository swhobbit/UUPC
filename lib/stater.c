/*--------------------------------------------------------------------*/
/*       s t a t e r . c                                              */
/*                                                                    */
/*       File time and size routines                                  */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1999 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: stater.c 1.15 1998/03/01 01:25:31 ahd v1-13f ahd $
 *
 *    Revision history:
 *    $Log: stater.c $
 *    Revision 1.15  1998/03/01 01:25:31  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.14  1997/12/14 21:13:45  ahd
 *    If called to determine size of directory, return an error
 *
 *    Revision 1.13  1997/03/31 07:06:55  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.12  1996/01/01 20:52:14  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.11  1995/01/07 15:43:07  ahd
 *    Don't try to dereference size pointer if NULL
 */

/*--------------------------------------------------------------------*/
/*                   Standard library include files                   */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

/*--------------------------------------------------------------------*/
/*                      Define current file name                      */
/*--------------------------------------------------------------------*/

RCSID("$Id: stater.c 1.15 1998/03/01 01:25:31 ahd v1-13f ahd $");
currentfile();

/*--------------------------------------------------------------------*/
/*    s t a t e r                                                     */
/*                                                                    */
/*    Report date and size of a file                                  */
/*--------------------------------------------------------------------*/

time_t stater(const char *file, long *size)
{
   struct stat statbuf;

/*--------------------------------------------------------------------*/
/*   If the file doesn't exist, give a nasty message to the caller    */
/*--------------------------------------------------------------------*/

   if(stat((char *) file, &statbuf) < 0 )
   {
      printmsg(0,"stater: cannot stat %s",file);
      printerr( file );
      if ( size != NULL )
         *size = 0;
      return (time_t) -1L;          /* Flag file as missing          */
   }

#ifdef S_IFDIR
   if ( statbuf.st_mode & S_IFDIR )
   {
      printmsg( 0, "stater: %s is a directory", file);
#ifdef EISDIR
      errno = EISDIR;
#endif
      return (time_t)   -1;         /* Report file as missing        */
   }
#else
#error Directory flag S_IFDIR is not defined!
#endif

/*--------------------------------------------------------------------*/
/*          We have the information; return it to the caller          */
/*--------------------------------------------------------------------*/

   printmsg(5,"stater: \"%s\" is %ld bytes; updated %.24s",
               file,
               statbuf.st_size,
               ctime( &statbuf.st_mtime));

   if ( size != NULL )
      *size = statbuf.st_size;

   return(statbuf.st_mtime);

} /* stater */
