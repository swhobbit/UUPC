/*--------------------------------------------------------------------*/
/*       a b s p a t h . c                                            */
/*                                                                    */
/*       Determine if a path is absolute                              */
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
 *    $Id: lib.h 1.29 1995/01/29 14:09:13 ahd Exp $
 *
 *    Revision history:
 *    $Log: lib.h $
 */

#include <uupcmoah.h>
#include <ctype.h>

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
