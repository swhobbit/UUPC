/*--------------------------------------------------------------------*/
/*       a b s p a t h . c                                            */
/*                                                                    */
/*       Determine if a path is absolute                              */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-2000 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: abspath.c 1.6 1999/01/08 02:20:43 ahd Exp $
 *
 *    Revision history:
 *    $Log: abspath.c $
 *    Revision 1.6  1999/01/08 02:20:43  ahd
 *    Convert currentfile() to RCSID()
 *
 *    Revision 1.5  1999/01/04 03:52:28  ahd
 *    Annual copyright change
 *
 *    Revision 1.4  1998/03/01 01:22:56  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.3  1997/03/31 06:57:56  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.2  1996/01/01 20:50:58  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.1  1995/01/30 04:05:08  ahd
 *    Additional compiler warning fixes, optimize path normalizing
 *
 */

#include <uupcmoah.h>
#include <ctype.h>

RCSID("$Id: abspath.c 1.6 1999/01/08 02:20:43 ahd Exp $");

/*--------------------------------------------------------------------*/
/*       i s A b s o l u t e P a t h                                  */
/*                                                                    */
/*       Report if the argument is an absolute path                   */
/*--------------------------------------------------------------------*/

KWBoolean isAbsolutePath( const char *path)
{

/*--------------------------------------------------------------------*/
/*     Path on current drive or network drive is always absolute      */
/*--------------------------------------------------------------------*/

   if (( path[0] == '/' ) || ( path[0] == '\\' ))
      return KWTrue;

/*--------------------------------------------------------------------*/
/*       Otherwise, must be begin with a letter colon (x:),           */
/*       followed by a directory separator.                           */
/*--------------------------------------------------------------------*/

   if ( ! isalpha( path[0] ) || (path[1] != ':'))
      return KWFalse;

   if (( path[2] == '/' ) || ( path[2] == '\\' ))
      return KWTrue;

   return KWFalse;

} /* isAbsolutePath */
