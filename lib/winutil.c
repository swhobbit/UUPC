/*--------------------------------------------------------------------*/
/*       w i n u t i l . c                                            */
/*                                                                    */
/*       Windows 3.1 utility functions for UUPC/extended              */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Copyright (c) Robert Denny, 1993                             */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-2002 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: winutil.c 1.23 2001/03/12 13:52:56 ahd v1-13k $
 *
 *    Revision history:
 *    $Log: winutil.c $
 *    Revision 1.23  2001/03/12 13:52:56  ahd
 *    Annual copyright update
 *
 *    Revision 1.22  2000/05/12 12:29:45  ahd
 *    Annual copyright update
 *
 *    Revision 1.21  1999/01/08 02:20:43  ahd
 *    Convert currentfile() to RCSID()
 *
 *    Revision 1.20  1999/01/04 03:52:28  ahd
 *    Annual copyright change
 *
 *    Revision 1.19  1998/03/01 01:25:57  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.18  1997/03/31 07:07:58  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.17  1996/01/01 20:53:50  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.16  1995/01/07 16:15:11  ahd
 *    Change boolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.15  1994/12/22 00:12:11  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.14  1994/02/19 04:45:12  ahd
 *    Use standard first header
 *
 * Revision 1.13  1994/02/19  04:15:47  ahd
 * Use standard first header
 *
 * Revision 1.12  1994/02/19  04:00:44  ahd
 * Use standard first header
 *
 * Revision 1.11  1994/02/18  23:16:32  ahd
 * Use standard first header
 *
 * Revision 1.10  1994/01/01  19:07:03  ahd
 * Annual Copyright Update
 *
 * Revision 1.9  1993/12/26  16:20:17  ahd
 * Allow intercepting messages (none actually intercepted at this time)
 *
 * Revision 1.8  1993/10/16  15:13:17  ahd
 * Add _export keywords per Paul Steckler to fix system crashes
 *
 * Revision 1.7  1993/10/12  01:20:43  ahd
 * Normalize comments to PL/I style
 *
 * Revision 1.6  1993/10/12  00:48:44  ahd
 * Normalize comments
 *
 * Revision 1.5  1993/10/02  19:07:49  ahd
 * Print module name when winexec() fails
 *
 * Revision 1.4  1993/08/03  03:11:49  ahd
 * Further Windows 3.x fixes
 *
 * Revision 1.3  1993/08/02  03:24:59  ahd
 * Further changes in support of Robert Denny's Windows 3.x support
 *
 * Revision 1.2  1993/07/31  16:22:16  ahd
 * Changes in support of Robert Denny's Windows 3.x support
 *
 * Revision 1.1  1993/07/22  23:19:50  ahd
 * Initial revision
 *
 */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"

#include <windows.h>

#include <toolhelp.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "winutil.h"

RCSID("$Id: winutil.c 1.23 2001/03/12 13:52:56 ahd v1-13k $");

/*--------------------------------------------------------------------*/
/*                           Local defines                            */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          Private messages                          */
/*--------------------------------------------------------------------*/

#define PM_TASKEND WM_USER

/*--------------------------------------------------------------------*/
/*      Globals needed by callback functions & useful elsewhere       */
/*--------------------------------------------------------------------*/

HTASK hOurTask;               /* Our task handle                      */
HWND hOurWindow;              /* Our EasyWin main window handle       */

/*--------------------------------------------------------------------*/
/*                         Used only locally                          */
/*--------------------------------------------------------------------*/

static HINSTANCE hChildInst;            /* Instance of child proc     */
static HWND hTheWindow;                 /* Used by WindCatcher()
                                           during enumeration         */

BOOL CALLBACK _export WindCatcher(HWND hWnd, LPARAM lparam);
BOOL CALLBACK _export NotifyCatcher(WORD wID, DWORD dwData);

static LPFNNOTIFYCALLBACK lpfnNotifyCB;
static FARPROC lpfnEnumWinCB;

void _DoneEasyWin(void);      /* In TCWIN library                     */

/*--------------------------------------------------------------------*/
/*       C l o s e E a s y W i n                                      */
/*                                                                    */
/*       Force EasyWin window to close on termination                 */
/*                                                                    */
/*       This function should be registered with a call to            */
/*       atexit() if you want to force the EasyWin window to          */
/*       close, permitting the application to exit, automatically.    */
/*       Normally, an EasyWin app hangs out in the "inactive"         */
/*       state until the user manually closes the window.  Using      */
/*       an atexit() function assures that the app will exit          */
/*       automatically, regardless of the means of generating the     */
/*       exit action (such as Ctrl-SysReq signal handlers, etc.).     */
/*--------------------------------------------------------------------*/

void CloseEasyWin(void)
{
   MSG msg;

/*--------------------------------------------------------------------*/
/*       This will assure that the EasyWin task yields to the         */
/*       system at least once before exiting.  See the comments       */
/*       next to the call to WinExec() below.                         */
/*--------------------------------------------------------------------*/

   PeekMessage(&msg, hOurWindow, NULL, NULL, PM_NOREMOVE);
   _DoneEasyWin();
}

/*--------------------------------------------------------------------*/
/*       S p a w n W a i t                                            */
/*                                                                    */
/*       Spawn a child process, wait for termination                  */
/*                                                                    */
/*       Use WinExec() to fire off a child process, then              */
/*       synchronize with its termination.  For use with EasyWin      */
/*       applications.                                                */
/*                                                                    */
/*       Inputs:                                                      */
/*          cmdLine     Command line including executable name        */
/*          fuCmdShow   How to display new main window (see           */
/*                      ShowWindow())                                 */
/*                                                                    */
/*       Returns:                                                     */
/*          -1          Big Trouble: NotifyRegister or WinExec        */
/*          failed                                                    */
/*          >=0         Exit status of child                          */
/*                                                                    */
/*       On Error:                                                    */
/*          Logs and displays an error message using printmsg()       */
/*--------------------------------------------------------------------*/

int SpawnWait( const char *command,
               const char *parameters,
               const KWBoolean synchronous,
               const UINT fuCmdShow)
{

   BOOL bChildIsExecuting = KWTrue;
   MSG msg;
   int iChildExitStatus = 0;

   if ( synchronous )
   {
      lpfnNotifyCB = (LPFNNOTIFYCALLBACK)
                           MakeProcInstance((FARPROC) NotifyCatcher,
                                             _hInstance);

      if (!NotifyRegister(hOurTask, lpfnNotifyCB, NF_NORMAL))
      {
           FreeProcInstance(lpfnNotifyCB);
           printmsg(0, "SpawnWait: NotifyRegister() failed.");
           return(-1);
      }
   }

/*--------------------------------------------------------------------*/
/*       Start up the child proc with the given command line.  To     */
/*       start a DOS box, use a .PIF file as the executable name      */
/*       (1st arg).                                                   */
/*                                                                    */
/*       WARNING!  The spawned task MUST give up control to the       */
/*       system at least once before exiting!  An EasyWin app that    */
/*       calls _DoneEasyWin() in an atexit() proc may never yiend     */
/*       to the system.  In this case WinExec() will not return       */
/*       here before the spawned task terminates.  The                */
/*       Notification callback will be called before WinExec()        */
/*       returns, and the hInstChild will not be set yet.  Result:    */
/*       This task will never see the termination notfication of      */
/*       the spawned task, and SpawnWait() will wait forever.         */
/*       CAVEAT!                                                      */
/*                                                                    */
/*       (To insure we do yield, banner() has a call to ddelay().)    */
/*--------------------------------------------------------------------*/

   if ( parameters == NULL )
      hChildInst = WinExec( command , fuCmdShow);
   else {
      char buf[BUFSIZ];
      strcpy( buf, command );
      strcat( buf, " " );
      strcat( buf, parameters );
      hChildInst = WinExec( buf , fuCmdShow);
   }

   if ( hChildInst < 32 )
   {

      if ( synchronous )
      {
         NotifyUnRegister(hOurTask);
         FreeProcInstance(lpfnNotifyCB);
      }

      printmsg(0, "SpawnWait: WinExec(%s %s) failed. Code = %d\n",
                  command,
                  parameters ? parameters : "",
                  (int)hChildInst);
      return(-1);

   }  /* if */

   if ( ! synchronous )
      return 0;

/*--------------------------------------------------------------------*/
/*       LOCAL MESSAGE LOOP - Service Windows while waiting for       */
/*       child proc to terminate.                                     */
/*--------------------------------------------------------------------*/

   while(bChildIsExecuting && GetMessage(&msg, NULL, NULL, NULL))
   {
      TranslateMessage(&msg);

      switch( msg.message )
      {
         case PM_TASKEND:
            bChildIsExecuting = KWFalse;
            iChildExitStatus = (int)(LOBYTE(msg.lParam));
            /* Fall through and dispatch message   */

         default:
            DispatchMessage(&msg);
            break;

      } /* switch( msg.message ) */

   } /* while */

   NotifyUnRegister(hOurTask);
   FreeProcInstance(lpfnNotifyCB);

   return(iChildExitStatus);

} /* SpawnWait */

/*--------------------------------------------------------------------*/
/*       N o t i f y C a t c h e r                                    */
/*                                                                    */
/*       Notification Callback                                        */
/*--------------------------------------------------------------------*/

#ifdef __TURBOC__
#pragma argsused
#endif

BOOL CALLBACK _export NotifyCatcher (WORD wID, DWORD dwData)
{
   HTASK hCurTask;                     /* handle of the task that     */
                                       /* called notification call    */
                                       /* back                        */
   TASKENTRY te;

/*--------------------------------------------------------------------*/
/*                        Check for task exiting                      */
/*--------------------------------------------------------------------*/

   switch (wID)
   {
      case NFY_EXITTASK:
/*--------------------------------------------------------------------*/
/*            Obtain info about the task that is terminating          */
/*--------------------------------------------------------------------*/
         hCurTask = GetCurrentTask();
         te.dwSize = sizeof(TASKENTRY);
         TaskFindHandle(&te, hCurTask);

/*--------------------------------------------------------------------*/
/*       Check if the task that is terminating is our child task.     */
/*       Also check if the hInstance of the task that is              */
/*       terminating in the same as the hInstance of the task that    */
/*       was WinExec'd by us earlier in this file.  This              */
/*       additional check is added because the Task List that is      */
/*       brought up by selecting 'Switch To ...' from the system      */
/*       menu is also run as a child task of the application and      */
/*       consequently the hInstance needs to be checked to            */
/*       determine which child task is terminating.  Obviously,       */
/*       the parent may have intentionally WinExec'ed more than       */
/*       one child on purpose as well.                                */
/*                                                                    */
/*       Note that the PM_TASKEND message is sent with its LPARAM     */
/*       set to the dwData passed to this callback.  The low byte     */
/*       of this is the child's exit status.                          */
/*--------------------------------------------------------------------*/

         if (te.hTaskParent == hOurTask && te.hInst == hChildInst)
            PostMessage(hOurWindow, PM_TASKEND, (WORD)hOurTask, dwData );

         break;

         default:
            break;
   }

/*--------------------------------------------------------------------*/
/*             Pass notification to other callback functions          */
/*--------------------------------------------------------------------*/

   return KWFalse;

}

/*--------------------------------------------------------------------*/
/*       F i n d T a s k W i n d o w                                  */
/*                                                                    */
/*       Find handle to one of our windows, by name                   */
/*--------------------------------------------------------------------*/

HWND FindTaskWindow (HTASK hTask, LPSTR lpszClassName)
{

   hTheWindow = NULL;

   lpfnEnumWinCB = MakeProcInstance((FARPROC)WindCatcher, _hInstance);
   EnumTaskWindows(hTask, lpfnEnumWinCB, (LPARAM)lpszClassName);
   FreeProcInstance(lpfnEnumWinCB);

   return(hTheWindow);
}

/*--------------------------------------------------------------------*/
/*       W i n d C a t c h e r                                        */
/*                                                                    */
/*       Callback function for EnumTaskWindows().  Passes back a      */
/*       copy of the next window handle.                              */
/*--------------------------------------------------------------------*/

#define BUF_LEN 32
BOOL CALLBACK _export WindCatcher (HWND hWnd, LPARAM lparam)
{
    char buf[BUF_LEN+1];
    int i;

    if ((i = GetClassName(hWnd, (LPSTR)buf, BUF_LEN)) == 0)
       return(KWFalse);                     /* OOPS!                   */

    buf[i] = '\0';                         /* Make cstr               */
    if (lstrcmpi((LPCSTR)buf, (LPCSTR)lparam) == 0) /* If we found it */
    {
        hTheWindow = hWnd;                 /* Save it for called func */
        return(KWFalse);
    }
    return(KWTrue);
}
