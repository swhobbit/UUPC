/*--------------------------------------------------------------------*/
/*    m k t e m p n m . c                                             */
/*                                                                    */
/*    Host Support routines for UUPC/extended                         */
/*                                                                    */
/*    Changes Copyright 1990, 1991 (c) Andrew H. Derbyshire           */
/*                                                                    */
/*    History:                                                        */
/*       21Nov1991 Break out of hlib.c                         ahd    */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: MKTEMPNM.C 1.4 1993/06/06 15:04:05 ahd Exp $
 *
 *    Revision history:
 *    $Log: MKTEMPNM.C $
 *     Revision 1.4  1993/06/06  15:04:05  ahd
 *     Use process id for first temp file number
 *
 *     Revision 1.3  1993/04/11  00:31:04  ahd
 *     Global edits for year, TEXT, etc.
 *
 * Revision 1.2  1992/11/19  02:57:07  ahd
 * drop rcsid
 *
 * Revision 1.1  1992/11/16  05:00:26  ahd
 * Initial revision
 *
 */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>

#include <io.h>
#include <process.h>


/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "hlib.h"

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

currentfile();

/*--------------------------------------------------------------------*/
/*    m k t e m p n a m e                                             */
/*                                                                    */
/*    Generate a temporary name with a pre-defined extension          */
/*--------------------------------------------------------------------*/

char *mktempname( char *buf, char *extension)
{
   static size_t file = 0;

   if ( file == 0 )
      file = getpid() & 0x7FFF;  /* Make unique number less than 32K  */

   if (buf == NULL)           /* Do we need to allocate buffer?       */
   {
      buf = malloc( FILENAME_MAX );
      checkref(buf);
   } /* if */

   for (file++ ; file < INT_MAX ; file++ )
   {
      sprintf(buf,"%s/uupc%04.4x.%s", E_tempdir, file, extension);
      if ( access( buf, 0 ))  /* Does the host file exist?            */
         break;               /* No  --> Use the name                 */
   } /* for */

   printmsg(5,"Generated temporary name: %s",buf);
   return buf;

} /* mktempname */
