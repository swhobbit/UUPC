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
 *    $Id: pushpop.c 1.4 1993/06/15 12:18:06 ahd Exp $
 *
 *    $Log: pushpop.c $
 *     Revision 1.4  1993/06/15  12:18:06  ahd
 *     Save pushed directory name for debugging
 *
 *     Revision 1.3  1993/06/13  14:06:00  ahd
 *     Insure directories PUSHED are POPPED
 *
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

#include <direct.h>
#include <string.h>

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
      printerr("getcwd");
      panic();
   }

   CHDIR( directory );

   E_cwd = equal(directory,".") ? dirstack[depth] : (char *) directory;

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
   E_cwd = dirstack[depth];
   return;

} /* PopDir */
