/*--------------------------------------------------------------------*/
/*    p u s h p o p . c                                               */
/*                                                                    */
/*    Directory functions for UUPC/extended                           */
/*                                                                    */
/*    Changes Copyright (c) 1989 - 1993 by Kendra Electronic          */
/*    Wonderworks.   All rights reserved except as explicitly         */
/*    granted by the UUPC/extended license.                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: PUSHPOP.C 1.2 1992/11/22 21:06:14 ahd Exp $
 *
 *    $Log: PUSHPOP.C $
 * Revision 1.2  1992/11/22  21:06:14  ahd
 * Use strpool for memory allocation
 *
 */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#ifdef __GNUC__
#include <unistd.h>
#else
#include <direct.h>
#endif

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "pushpop.h"

#define MAXDEPTH 10

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

static char *dirstack[MAXDEPTH];
static depth = 0;

currentfile();

/*--------------------------------------------------------------------*/
/*            Change to a directory and push on our stack             */
/*--------------------------------------------------------------------*/

void PushDir( const char *directory )
{
   char cwd[FILENAME_MAX];
   if ( depth >= MAXDEPTH )
      panic();

#ifdef __TURBOC__
   dirstack[depth] = newstr( getcwd( cwd  , FILENAME_MAX ));
#else

#ifdef __GNUC__
   dirstack[depth] = newstr( getcwd( cwd , FILENAME_MAX ) );
#else
   dirstack[depth] = newstr( _getdcwd( 0, cwd , FILENAME_MAX ) );
#endif

#endif

   if (dirstack[depth] == NULL )
   {
      printerr("PushDir");
      panic();
   }
   CHDIR( directory );
   depth++;
   return;
} /* PushDir */

/*--------------------------------------------------------------------*/
/*               Return to a directory saved by PushDir               */
/*--------------------------------------------------------------------*/

void PopDir( void )
{
   if ( depth == 0 )
      panic();

   CHDIR( dirstack[--depth] );
   return;

} /* PopDir */
