/*--------------------------------------------------------------------*/
/*       s e t s t d i n . c                                          */
/*                                                                    */
/*       Set Standard input for Windows/NT programs                   */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-2002 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Copyright (c) David M. Watt 1993                             */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: setstdin.c 1.16 2001/03/12 13:54:04 ahd v1-13k $
 *
 *    $Log: setstdin.c $
 *    Revision 1.16  2001/03/12 13:54:04  ahd
 *    Annual Copyright update
 *
 *    Revision 1.15  2000/05/12 12:30:48  ahd
 *    Annual copyright update
 *
 *    Revision 1.14  1999/01/08 02:20:48  ahd
 *    Convert currentfile() to RCSID()
 *
 *    Revision 1.13  1999/01/04 03:52:55  ahd
 *    Annual copyright change
 *
 *    Revision 1.12  1998/03/01 01:25:24  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.11  1997/03/31 07:06:41  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.10  1996/01/01 20:55:27  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.9  1994/12/22 00:11:13  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.8  1994/02/19 04:46:41  ahd
 *    Use standard first header
 *
 *     Revision 1.7  1994/02/19  04:11:48  ahd
 *     Use standard first header
 *
 *     Revision 1.6  1994/02/19  03:58:21  ahd
 *     Use standard first header
 *
 *     Revision 1.5  1994/02/18  23:15:06  ahd
 *     Use standard first header
 *
 *     Revision 1.4  1994/01/01  19:05:42  ahd
 *     Annual Copyright Update
 *
 *     Revision 1.3  1993/10/26  12:45:46  ahd
 *     Add include of own header file
 *
 *     Revision 1.2  1993/10/12  00:47:57  ahd
 *     Normalize comments
 *
 * Revision 1.1  1993/04/10  21:22:29  dmwatt
 * Initial revision
 *
 *     Revision 1.1  1993/04/09  06:22:00  dmwatt
 *     Written
 */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"

#include <windows.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "setstdin.h"

RCSID("$Id: setstdin.c 1.16 2001/03/12 13:54:04 ahd v1-13k $");

/*--------------------------------------------------------------------*/
/*    s e t s t d i n m o d e                                         */
/*                                                                    */
/*    Set standard input on NT console for single char I/O            */
/*--------------------------------------------------------------------*/

void setstdinmode(void)
{
   HANDLE hStdIn = GetStdHandle(STD_INPUT_HANDLE);
   DWORD mode;
   BOOL bSuccess;

   bSuccess = GetConsoleMode(hStdIn, &mode);

/* Disable mouse events so that later Peeks() only get characters */
   mode &= ~ENABLE_WINDOW_INPUT;
   mode &= ~ENABLE_MOUSE_INPUT;
   mode &= ~ENABLE_LINE_INPUT;
   mode |= ENABLE_PROCESSED_INPUT;

   SetConsoleMode(hStdIn, mode);

}  /* setstdinmode */
