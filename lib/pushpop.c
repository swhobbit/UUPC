/*--------------------------------------------------------------------*/
/*    p u s h p o p . c                                               */
/*                                                                    */
/*    Directory functions for UUPC/extended                           */
/*                                                                    */
/*    Copyright (c) 1989, 1991 Andrew H. Derbyhshire                  */
/*--------------------------------------------------------------------*/

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
   if ( depth >= MAXDEPTH )
      panic();

#ifdef __TURBOC__
   dirstack[depth] = getcwd( NULL , FILENAME_MAX );
#else

#ifdef __GNUC__
   dirstack[depth] = getcwd( NULL , FILENAME_MAX );
#else
   dirstack[depth] = _getdcwd( 0, NULL , FILENAME_MAX );
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
   free( dirstack[depth] );
   return;

} /* PopDir */
