/*
 *    $Id: SCRSIZNT.C 1.4 1993/04/10 21:22:29 dmwatt Exp $
 *
 *    $Log: SCRSIZNT.C $
 *     Revision 1.4  1993/04/10  21:22:29  dmwatt
 *     Windows/NT fixes
 *
 *     Revision 1.3  1992/12/30  13:09:25  dmwatt
 *     Correct boolean compare
 *
 */

/*--------------------------------------------------------------------*/
/*    Copyright (c) David M. Watt 1993, All Rights Reserved           */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1993 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include <stdio.h>

#include <windows.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "scrsize.h"
#include "pnterr.h"

/*--------------------------------------------------------------------*/
/*                    Internal function prototypes                    */
/*--------------------------------------------------------------------*/

currentfile();

/*--------------------------------------------------------------------*/
/*    s c r s i z e                                                   */
/*                                                                    */
/*    Return screen size under Windows/NT                             */
/*--------------------------------------------------------------------*/

short scrsize( void )
{
   CONSOLE_SCREEN_BUFFER_INFO info;
   BOOL result;
   HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);

   result = GetConsoleScreenBufferInfo(hStdout, &info);

   if ( result != TRUE )
   {
      DWORD dwError = GetLastError();
      printmsg(0,"scrsize:  could not retrieve screen information");
      printNTerror("GetConsoleScreenBufferInfo", dwError);
      return PAGESIZE;
   }

   return info.dwSize.Y;
} /* scrsize */

