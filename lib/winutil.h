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
/*       Changes Copyright (c) 1989-1993 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: winutil.h 1.2 1993/07/31 16:28:59 ahd Exp $
 *
 *    Revision history:
 *    $Log: winutil.h $
 * Revision 1.2  1993/07/31  16:28:59  ahd
 * Changes in support of Robert Denny's Windows Support
 *
 * Revision 1.1  1993/07/22  23:26:19  ahd
 * Initial revision
 */

extern HTASK hOurTask;           // Our task handle
extern HWND hOurWindow;          // Our EasyWin main window handle
extern HINSTANCE _hInstance;     // Defined/set by TCWIN runtime


HWND FindTaskWindow(HTASK hTask, LPSTR lpszClassName);

void SetWndFixedFont(HWND hWnd, int nPtSize, char *lfFaceName);

int SpawnWait( const char *cmdLine,
               const char *parameters,
               const boolean synchronous,
               const UINT fuCmdShow);

void CloseEasyWin(void);

#endif
