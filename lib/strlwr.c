/*--------------------------------------------------------------------*/
/*       UUPC/extended string lower function                          */
/*                                                                    */
/*       Copyright 1992, Andrew H. Derbyshire                         */
/*                                                                    */
/*       Why this function doesn't exist in GCC is beyond me          */
/*--------------------------------------------------------------------*/



/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Header$
 *
 *    Revision history:
 *    $Log$
 */

static char rcsid[] = "$Id$";

/*--------------------------------------------------------------------*/
/*                       Standard include files                       */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"

/*--------------------------------------------------------------------*/
/*       s t r l w r                                                  */
/*                                                                    */
/*       Convert a string to lower case                               */
/*--------------------------------------------------------------------*/

char *strlwr( char *s )
{
   char *save = s;

   if ( s == NULL )
      return s;

   while ( *s != '\0' )
   {
      if ( islower( *s ))
         *s = toupper( *s );
      *s++;
   } /* while */

   return save;

} /* strlwr */
