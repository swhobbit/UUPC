/*
 *    $Id: scrsiznt.c 1.19 2001/03/12 13:54:04 ahd v1-13k $
 *
 *    $Log: scrsiznt.c $
 *    Revision 1.19  2001/03/12 13:54:04  ahd
 *    Annual Copyright update
 *
 *    Revision 1.18  2000/05/12 12:30:48  ahd
 *    Annual copyright update
 *
 *    Revision 1.17  1999/01/08 02:20:48  ahd
 *    Convert currentfile() to RCSID()
 *
 *    Revision 1.16  1999/01/04 03:52:55  ahd
 *    Annual copyright change
 *
 *    Revision 1.15  1998/03/01 01:25:18  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.14  1997/03/31 07:06:29  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.13  1996/01/01 20:55:00  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.12  1995/09/04 18:43:37  ahd
 *    Correct compile warning
 *
 *    Revision 1.11  1995/01/07 16:14:28  ahd
 *    Change boolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.10  1994/12/22 00:10:58  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.9  1994/02/19 04:46:33  ahd
 *    Use standard first header
 *
 *     Revision 1.6  1994/01/01  19:05:13  ahd
 *     Annual Copyright Update
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
/*    Changes Copyright (c) 1989-2002 by Kendra Electronic            */
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

RCSID("$Id: scrsiznt.c 1.19 2001/03/12 13:54:04 ahd v1-13k $");

/*--------------------------------------------------------------------*/
/*    s c r s i z e                                                   */
/*                                                                    */
/*    Return screen size under Windows/NT                             */
/*--------------------------------------------------------------------*/

unsigned short scrsize( void )
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

   return (unsigned short) info.dwSize.Y;

} /* scrsize */
