/*--------------------------------------------------------------------*/
/*       u s r c a t c h e r . c                                      */
/*                                                                    */
/*       Ctrl-Break handler for UUPC/extended                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-2001 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: usrcatch.c 1.16 2000/05/12 12:29:45 ahd v1-13g $
 *
 *    Revision history:
 *    $Log: usrcatch.c $
 *    Revision 1.16  2000/05/12 12:29:45  ahd
 *    Annual copyright update
 *
 *    Revision 1.15  1999/01/08 02:20:43  ahd
 *    Convert currentfile() to RCSID()
 *
 *    Revision 1.14  1999/01/04 03:52:28  ahd
 *    Annual copyright change
 *
 *    Revision 1.13  1998/03/01 01:25:52  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.12  1997/03/31 07:07:38  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.11  1996/01/01 20:53:22  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.10  1995/01/29 16:43:03  ahd
 *    IBM C/Set compiler warnings
 *
 *    Revision 1.9  1994/12/22 00:12:05  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.8  1994/02/19 04:47:36  ahd
 *    Use standard first header
 *
 * Revision 1.7  1994/02/19  04:12:49  ahd
 * Use standard first header
 *
 * Revision 1.6  1994/02/19  04:00:20  ahd
 * Use standard first header
 *
 * Revision 1.5  1994/02/18  23:16:09  ahd
 * Use standard first header
 *
 * Revision 1.4  1994/01/01  19:06:51  ahd
 * Annual Copyright Update
 *
 * Revision 1.3  1993/09/30  03:06:28  ahd
 * Use straight signal value as flag
 *
 * Revision 1.3  1993/09/30  03:06:28  ahd
 * Use straight signal value as flag
 *
 * Revision 1.2  1993/09/29  13:18:06  ahd
 * Drop unneeded OS/2 ifdef
 *
 * Revision 1.1  1993/09/29  04:49:20  ahd
 * Initial revision
 *
 */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"

#include <signal.h>

#include <process.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "timestmp.h"
#include "usrcatch.h"

RCSID("$Id: usrcatch.c 1.16 2000/05/12 12:29:45 ahd v1-13g $");

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

extern unsigned long raised = 0;

/*--------------------------------------------------------------------*/
/*    u s r 1 h a n d l e r                                           */
/*                                                                    */
/*    Handles SIGUSR interrupt; from MicroSoft Programmer's           */
/*    Workbench QuickHelp samples                                     */
/*--------------------------------------------------------------------*/

void
#ifdef __TURBOC__
__cdecl
#endif
usrhandler( int sig )
{
   raised = (unsigned long) sig;

   if ( raised == 0 )
      abort();

/*--------------------------------------------------------------------*/
/*    The handler pointer must be reset to our handler since by       */
/*    default it is reset to the system handler.                      */
/*--------------------------------------------------------------------*/

   signal( sig , usrhandler );

} /* usrhandler */
