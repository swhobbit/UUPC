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
 *    $Id: E:\src\uupc\lib\RCS\STRLWR.C 1.3 1992/11/30 03:26:20 ahd Exp $
 *
 *    Revision history:
 *    $Log: STRLWR.C $
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
      *s++;
   } /* while */

   return save;

} /* strlwr */
