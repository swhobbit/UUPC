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
 *    $Id: STRLWR.C 1.4 1993/04/11 00:32:05 ahd Exp $
 *
 *    Revision history:
 *    $Log: STRLWR.C $
 *     Revision 1.4  1993/04/11  00:32:05  ahd
 *     Global edits for year, TEXT, etc.
 *
 * Revision 1.3  1992/11/30  03:26:20  ahd
 * Much better if strlwr makes the string LOWER case
 *
 * Revision 1.2  1992/11/19  02:58:39  ahd
 * drop rcsid
 *
 * Revision 1.1  1992/11/16  05:00:26  ahd
 * Initial revision
 *
 */


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
      if ( isupper( *s ))
         *s = tolower( *s );
      s += 1;
   } /* while */

   return save;

} /* strlwr */
