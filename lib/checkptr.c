/*--------------------------------------------------------------------*/
/*    c h e c k p t r . c                                             */
/*                                                                    */
/*    Support routines for UUPC/extended                              */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1994 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: checkptr.c 1.2 1993/12/24 05:12:54 ahd Exp $
 *
 *    Revision history:
 *    $Log: checkptr.c $
 *     Revision 1.2  1993/12/24  05:12:54  ahd
 *     With check in-lined, always abort if called
 *
 */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
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
/*    Report that a pointer is NULL                                   */
/*--------------------------------------------------------------------*/

void checkptr( const char *file, const int line)
{
   printmsg(0,"Storage allocation failure; possible cause: "
               " memory shortage.");
   bugout( line, file);
} /* checkptr */
