/*--------------------------------------------------------------------*/
/*    s t a t e r . c                                                 */
/*                                                                    */
/*    File time and size routines                                     */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1993 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: lib.h 1.9 1993/07/19 02:53:32 ahd Exp $
 *
 *    Revision history:
 *    $Log: lib.h $
 */

/*--------------------------------------------------------------------*/
/*                   Standard library include files                   */
/*--------------------------------------------------------------------*/

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
      return -1;              /* Flag file as missing          */
   }

/*--------------------------------------------------------------------*/
/*          We have the information; return it to the caller          */
/*--------------------------------------------------------------------*/

   if ( size != NULL )
      *size = statbuf.st_size;

   printmsg(5,"stater: \"%s\" is %ld bytes; updated %s",
         file, *size, ctime( &statbuf.st_mtime));
   return(statbuf.st_mtime);

} /* stater */
