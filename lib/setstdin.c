/*
 *    $Id: SETSTDIN.C 1.1 1993/04/10 21:22:29 dmwatt Exp $
 *
 *    $Log: SETSTDIN.C $
 * Revision 1.1  1993/04/10  21:22:29  dmwatt
 * Initial revision
 *
 *     Revision 1.1  1993/04/09  06:22:00  dmwatt
 *     Written

*
 */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include <stdio.h>

#ifdef WIN32

#include <windows.h>

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
}

#endif
