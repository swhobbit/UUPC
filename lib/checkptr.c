/*--------------------------------------------------------------------*/
/*    c h e c k p t r . c                                             */
/*                                                                    */
/*    Support routines for UUPC/extended                              */
/*                                                                    */
/*    Changes Copyright 1990, 1991 (c) Andrew H. Derbyshire           */
/*                                                                    */
/*    History:                                                        */
/*       21Nov1991 Break out of lib.c                          ahd    */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"

/*--------------------------------------------------------------------*/
/*    c h e c k p t r                                                 */
/*                                                                    */
/*    Verfiy that a pointer is not null                               */
/*--------------------------------------------------------------------*/

void checkptr(const void *block, const char *file, const int line)
{
   if (block == NULL)
   {
      printmsg(0,"Storage allocation failure; possible cause:\
 memory shortage.");
      bugout( line, file);
   }
} /* checkptr */
