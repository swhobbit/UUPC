#ifndef _WINUTIL_H
#define _WINUTIL_H

/*--------------------------------------------------------------------*/
/*       w i n u t i l . h                                            */
/*                                                                    */
/*       Windows 3.1 utility functions for UUPC/extended              */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Copyright (c) Robert Denny, 1993                             */
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
 *    $Id: winutil.h 1.11 1999/01/04 03:55:03 ahd Exp $
 *
 *    Revision history:
 *    $Log: winutil.h $
 *    Revision 1.11  1999/01/04 03:55:03  ahd
 *    Annual copyright change
 *
 *    Revision 1.10  1998/03/01 01:28:24  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.9  1997/03/31 07:08:01  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.8  1996/01/01 21:00:42  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.7  1995/01/07 16:16:45  ahd
 *    Change boolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.6  1994/12/22 00:14:46  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.5  1994/01/01 19:10:27  ahd
 *    Annual Copyright Update
 *
 * Revision 1.4  1993/10/12  01:22:27  ahd
 * Normalize comments to PL/I style
 *
 * Revision 1.3  1993/08/02  03:26:05  ahd
 * Further changes in support of Robert Denny's Windows 3.x support
 *
 * Revision 1.2  1993/07/31  16:28:59  ahd
 * Changes in support of Robert Denny's Windows Support
 *
 * Revision 1.1  1993/07/22  23:26:19  ahd
 * Initial revision
 */

extern HTASK hOurTask;           /* Our task handle                    */
extern HWND hOurWindow;          /* Our EasyWin main window handle     */
extern HINSTANCE _hInstance;     /* Defined/set by TCWIN runtime      */


HWND FindTaskWindow(HTASK hTask, LPSTR lpszClassName);

void SetWndFixedFont(HWND hWnd, int nPtSize, char *lfFaceName);

int SpawnWait( const char *cmdLine,
               const char *parameters,
               const KWBoolean synchronous,
               const UINT fuCmdShow);

void CloseEasyWin(void);

#endif
