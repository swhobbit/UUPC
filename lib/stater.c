/*--------------------------------------------------------------------*/
/*    s t a t e r . c                                                 */
/*                                                                    */
/*    File time and size routines                                     */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1994 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: stater.c 1.5 1994/01/01 19:06:07 ahd Exp $
 *
 *    Revision history:
 *    $Log: stater.c $
 *     Revision 1.5  1994/01/01  19:06:07  ahd
 *     Annual Copyright Update
 *
 *     Revision 1.4  1993/10/28  12:19:01  ahd
 *     Cosmetic time formatting twiddles and clean ups
 *
 *     Revision 1.3  1993/10/09  15:47:51  rhg
 *     ANSIify the source
 *
 *     Revision 1.3  1993/10/09  15:47:51  rhg
 *     ANSIify the source
 *
 *     Revision 1.2  1993/07/20  21:45:37  ahd
 *     Report last modified time, not created time, per Kae Uwe Rommel
 *
 */

/*--------------------------------------------------------------------*/
/*                   Standard library include files                   */
/*--------------------------------------------------------------------*/


#include "uupcmoah.h"
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"

/*--------------------------------------------------------------------*/
/*                      Define current file name                      */
/*--------------------------------------------------------------------*/

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
      printmsg(0,"cannot stat %s",file);
      printerr( file );
      if ( size != NULL )
         *size = 0;
      return (time_t) -1L;    /* Flag file as missing          */
   }

/*--------------------------------------------------------------------*/
/*          We have the information; return it to the caller          */
/*--------------------------------------------------------------------*/

   if ( size != NULL )
      *size = statbuf.st_size;

   printmsg(5,"stater: \"%s\" is %ld bytes; updated %.24s",
         file, *size, ctime( &statbuf.st_mtime));
   return(statbuf.st_mtime);

} /* stater */
