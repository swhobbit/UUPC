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
 *    $Id: lib.h 1.9 1993/07/19 02:53:32 ahd Exp $
 *
 *    Revision history:
 *    $Log: lib.h $
 */

extern HTASK hOurTask;           // Our task handle
extern HWND hOurWindow;          // Our EasyWin main window handle
extern HINSTANCE _hInstance;     // Defined/set by TCWIN runtime


HWND FindTaskWindow(HTASK hTask, LPSTR lpszClassName);
void SetWndFixedFont(HWND hWnd, int nPtSize, char *lfFaceName);
int SpawnWait(char *cmdLine, UINT fuCmdShow);
void CloseEasyWin(void);
