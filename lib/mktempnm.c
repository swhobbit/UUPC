/*--------------------------------------------------------------------*/
/*    m k t e m p n m . c                                             */
/*                                                                    */
/*    Host Support routines for UUPC/extended                         */
/*                                                                    */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-2001 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: mktempnm.c 1.21 2000/05/12 12:30:48 ahd v1-13g $
 *
 *    Revision history:
 *    $Log: mktempnm.c $
 *    Revision 1.21  2000/05/12 12:30:48  ahd
 *    Annual copyright update
 *
 *    Revision 1.20  1999/01/08 02:20:48  ahd
 *    Convert currentfile() to RCSID()
 *
 *    Revision 1.19  1999/01/04 03:52:55  ahd
 *    Annual copyright change
 *
 *    Revision 1.18  1998/03/01 01:24:24  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.17  1997/03/31 07:05:03  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.16  1996/01/01 20:52:59  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.15  1995/02/12 23:37:04  ahd
 *    compiler cleanup, NNS C/news support, optimize dir processing
 *
 *    Revision 1.14  1995/01/07 16:13:22  ahd
 *    Change KWBoolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.13  1994/12/31 03:41:08  ahd
 *    First pass of integrating Mike McLagan's news SYS file suuport
 *
 *    Revision 1.12  1994/12/22 00:09:40  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.11  1994/02/19 04:44:03  ahd
 *    Use standard first header
 *
 *     Revision 1.7  1994/01/01  19:03:21  ahd
 *     Annual Copyright Update
 *
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

#include "uupcmoah.h"

#include <limits.h>

#include <io.h>
#include <process.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

RCSID("$Id: mktempnm.c 1.21 2000/05/12 12:30:48 ahd v1-13g $");

/*--------------------------------------------------------------------*/
/*    m k t e m p n a m e                                             */
/*                                                                    */
/*    Generate a temporary name with a pre-defined extension          */
/*--------------------------------------------------------------------*/

char *mktempname( char *buf, const char *extension)
{
   static size_t tempSequence = 0;
   KWBoolean slash;

/*--------------------------------------------------------------------*/
/*                   Initialize the file name counter                 */
/*--------------------------------------------------------------------*/

   if ( tempSequence == 0 )
      tempSequence = (unsigned) getpid() % 32767;
                              /* Make unique number less than 32K  */

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
      slash = KWTrue;
   else
      slash = KWFalse;

/*--------------------------------------------------------------------*/
/*        Loop looking for the non-existent file of our dreams        */
/*--------------------------------------------------------------------*/

   for (tempSequence++ ; tempSequence < INT_MAX ; tempSequence++ )
   {
      sprintf(buf,"%s%suupc%04.4x.%s",
                  E_tempdir,
                  slash ? "" : "/",
                  tempSequence,
                  extension);

      if ( access( buf, 0 ))  /* Does the host file exist?            */
         break;               /* No  --> Use the name                 */

   } /* for */

   printmsg(5,"Generated temporary name: %s",buf);
   return buf;

} /* mktempname */

/*--------------------------------------------------------------------*/
/*    m k d i r f i l e n a m e                                       */
/*                                                                    */
/*    Generate a temporary name with a pre-defined extension in given */
/*    directory                                                       */
/*--------------------------------------------------------------------*/

char *mkdirfilename( char *buf, const char *dir, const char *extension)
{
   char *result;
   char *savePath = E_tempdir;      /* Save normal path              */

   E_tempdir = (char *) dir;        /* Use new path for file         */

   result = mktempname( buf, extension );

   E_tempdir = savePath;            /* Restore original temp path    */

   return result;

} /* mkdirfilename */
