/*--------------------------------------------------------------------*/
/*       s e t s t d i n . c                                          */
/*                                                                    */
/*       Set Standard input for Windows/NT programs                   */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1994 by Kendra Electronic         */
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
 *    $Id: setstdin.c 1.3 1993/10/26 12:45:46 ahd Exp $
 *
 *    $Log: setstdin.c $
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

#include <stdio.h>
#include <windows.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "setstdin.h"

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
