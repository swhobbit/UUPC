/*--------------------------------------------------------------------*/
/*    m k t e m p n m . c                                             */
/*                                                                    */
/*    Host Support routines for UUPC/extended                         */
/*                                                                    */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1994 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: mktempnm.c 1.6 1993/10/30 02:29:46 ahd Exp $
 *
 *    Revision history:
 *    $Log: mktempnm.c $
 *     Revision 1.6  1993/10/30  02:29:46  ahd
 *     Trim trailing slash from root directories
 *
 *     Revision 1.5  1993/10/12  00:45:27  ahd
 *     Normalize comments
 *
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
   boolean slash;

/*--------------------------------------------------------------------*/
/*                   Initialize the file name counter                 */
/*--------------------------------------------------------------------*/

   if ( file == 0 )
      file = getpid() & 0x7FFF;  /* Make unique number less than 32K  */

/*--------------------------------------------------------------------*/
/*                Allocate a file name buffer if required             */
/*--------------------------------------------------------------------*/

   if (buf == NULL)           /* Do we need to allocate buffer?       */
   {
      buf = malloc( FILENAME_MAX );
      checkref(buf);
   } /* if */

/*--------------------------------------------------------------------*/
/*       Determine if we need have a slash at the end of the file     */
/*       name.  This should be reducible (only executed the first     */
/*       pass) but since RNEWS and UUCICO play with the temporary     */
/*       file name path to generate files in special directories,     */
/*       we check every time.                                         */
/*--------------------------------------------------------------------*/

   if ( E_tempdir[ strlen( E_tempdir ) - 1 ] == '/' )
      slash = TRUE;
   else
      slash = FALSE;

/*--------------------------------------------------------------------*/
/*        Loop looking for the non-existent file of our dreams        */
/*--------------------------------------------------------------------*/

   for (file++ ; file < INT_MAX ; file++ )
   {
      sprintf(buf,"%s%suupc%04.4x.%s",
                  E_tempdir,
                  slash ? "" : "/",
                  file,
                  extension);

      if ( access( buf, 0 ))  /* Does the host file exist?            */
         break;               /* No  --> Use the name                 */

   } /* for */

   printmsg(5,"Generated temporary name: %s",buf);
   return buf;

} /* mktempname */
