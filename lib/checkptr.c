/*--------------------------------------------------------------------*/
/*       c h e c k p t r . c                                          */
/*                                                                    */
/*       Print standard error message and panic; called when a        */
/*       null pointer is detected in-line by checkref macro.          */
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
 *    $Id: checkptr.c 1.15 1999/01/08 02:20:43 ahd Exp $
 *
 *    Revision history:
 *    $Log: checkptr.c $
 *    Revision 1.15  1999/01/08 02:20:43  ahd
 *    Convert currentfile() to RCSID()
 *
 *    Revision 1.14  1999/01/04 03:52:55  ahd
 *    Annual copyright change
 *
 *    Revision 1.13  1998/03/01 01:23:13  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.12  1997/03/31 06:58:23  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.11  1996/11/18 04:46:49  ahd
 *    Normalize arguments to bugout
 *    Reset title after exec of sub-modules
 *    Normalize host status names to use HS_ prefix
 *
 *    Revision 1.10  1996/01/01 20:50:21  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.9  1995/01/29 14:07:59  ahd
 *    Clean up most IBM C/Set Compiler Warnings
 *
 *    Revision 1.8  1994/12/22 00:07:39  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.7  1994/02/19 04:39:43  ahd
 *    Use standard first header
 *
 *     Revision 1.6  1994/02/19  04:04:36  ahd
 *     Use standard first header
 *
 *     Revision 1.5  1994/02/19  03:48:10  ahd
 *     Use standard first header
 *
 *     Revision 1.4  1994/02/18  23:07:57  ahd
 *     Use standard first header
 *
 *     Revision 1.3  1994/01/01  19:00:45  ahd
 *     Annual Copyright Update
 *
 *     Revision 1.2  1993/12/24  05:12:54  ahd
 *     With check in-lined, always abort if called
 *
 */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"

RCSID( "$Id: checkptr.c 1.15 1999/01/08 02:20:43 ahd Exp $" );

/*--------------------------------------------------------------------*/
/*    c h e c k p t r                                                 */
/*                                                                    */
/*    Report that a pointer is NULL                                   */
/*--------------------------------------------------------------------*/

void checkptr( const char UUFAR *file, const size_t line)
{
   printmsg(0,"Storage allocation failure; possible cause: "
               " memory shortage.");

   bugout( file, line);

} /* checkptr */
