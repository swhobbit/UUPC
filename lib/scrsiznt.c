/*
 *    $Id: scrsiznt.c 1.10 1994/12/22 00:10:58 ahd Exp $
 *
 *    $Log: scrsiznt.c $
 *    Revision 1.10  1994/12/22 00:10:58  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.9  1994/02/19 04:46:33  ahd
 *    Use standard first header
 *
 *     Revision 1.8  1994/02/19  03:58:07  ahd
 *     Use standard first header
 *
 *     Revision 1.8  1994/02/19  03:58:07  ahd
 *     Use standard first header
 *
 *     Revision 1.7  1994/02/18  23:14:56  ahd
 *     Use standard first header
 *
 *     Revision 1.6  1994/01/01  19:05:13  ahd
 *     Annual Copyright Update
 *
 *     Revision 1.5  1993/09/26  03:32:27  dmwatt
 *     Use Standard Windows NT error message module
 *
 *     Revision 1.5  1993/09/26  03:32:27  dmwatt
 *     Use Standard Windows NT error message module
 *
 *     Revision 1.4  1993/04/10  21:22:29  dmwatt
 *     Windows/NT fixes
 *
 *     Revision 1.3  1992/12/30  13:09:25  dmwatt
 *     Correct KWBoolean compare
 *
 */

/*--------------------------------------------------------------------*/
/*    Copyright (c) David M. Watt 1993, All Rights Reserved           */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1995 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"

#include <windows.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

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

   if ( result != KWTrue )
   {
      DWORD dwError = GetLastError();
      printmsg(0,"scrsize:  could not retrieve screen information");
      printNTerror("GetConsoleScreenBufferInfo", dwError);
      return PAGESIZE;
   }

   return info.dwSize.Y;
} /* scrsize */

