#include "uupcmoah.h"

/*--------------------------------------------------------------------*/
/*    c h e c k p t r . c                                             */
/*                                                                    */
/*    Support routines for UUPC/extended                              */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1996 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: checkptr.c 1.9 1995/01/29 14:07:59 ahd v1-12q $
 *
 *    Revision history:
 *    $Log: checkptr.c $
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

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    c h e c k p t r                                                 */
/*                                                                    */
/*    Report that a pointer is NULL                                   */
/*--------------------------------------------------------------------*/

void checkptr( const char *file, const size_t line)
{
   printmsg(0,"Storage allocation failure; possible cause: "
               " memory shortage.");
   bugout( line, file);
} /* checkptr */
