/*--------------------------------------------------------------------*/
/*       W I N S T D I O . C                                          */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Author:  Steve Drew, drew@cgo.dec.com                        */
/*                                                                    */
/*       This file provides a stdio type enviroment under win 32.     */
/*       Like easywin or quickwin use to provide.  It does so by      */
/*       #define'ing new functions for common stdio routines.  So     */
/*       the other modules must include winstdio.h                    */
/*                                                                    */
/*       It uses a floating window buffer that will hold MAX_LINES    */
/*       after MAX_LINES is exceeded the oldest lines are purged.     */
/*                                                                    */
/*       Note:  currently it does not supply functions for            */
/*       terminal input.                                              */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1999 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *       $Id: winstdio.c 1.2 1998/04/20 02:47:57 ahd v1-13f ahd $
 *
 *       Revision history:
 *       $Log: winstdio.c $
 *       Revision 1.2  1998/04/20 02:47:57  ahd
 *       TAPI/Windows 32 BIT GUI display support
 *
 *       Revision 1.1  1998/04/19 23:55:58  ahd
 *       Initial revision
 *
 */


#define NO_UUIO_REDEFINES        /* Don't redefine functions we use */
#include "uupcmoah.h"

#include <windows.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <io.h>
#include <process.h>

#define MAX_LINE_LENGTH 256
#define MAX_LINES 1000

RCSID("$Id: winstdio.c 1.2 1998/04/20 02:47:57 ahd v1-13f ahd $");

HWND hFrame;                        /* handle of main window */
HANDLE hInst;                       /* our instance */
static unsigned ThreadId;
static HANDLE hThread;

static char szAppName[128];
static char Buf[MAX_LINE_LENGTH + MAX_LINE_LENGTH + 1]; /* transfer buffer holds inbound text */

/*    Painting and Scrolling support */

static PAINTSTRUCT ps;
static TEXTMETRIC tm;
static SCROLLINFO si;

/* These variables are required to display text. */

static int xClientMax;  /* maximum width of client area          */

static int xChar;       /* horizontal scrolling unit             */
static int yChar;       /* vertical scrolling unit               */

static int xPos;        /* current horizontal scrolling position */
static int yPos;        /* current vertical scrolling position   */
static int xClient;     /* width of client area                  */
static int yClient;     /* height of client area                 */

static int xMax;        /* maximum horiz. scrolling position     */
static int yMax;        /* maximum vert. scrolling position      */
static int PageSize;

static int NumLines = 0;
static char *line_buf[MAX_LINES + 1];

static void (*atexit_funcs[20])();
static int atexit_fcount;

int (*WinAbort)(); /* When window is closed indicates optional function to execute. */

BOOL InitApp(HANDLE hInstance);
BOOL InitInstance(HANDLE hInstance, int nCmdShow);
BOOL FAR APIENTRY AboutDlgProc(HWND hWnd, UINT wMsg, UINT wParam, LONG lParam);
LONG FAR APIENTRY MyWndProc(HWND hWnd, UINT wMsg, UINT wParam, LONG lParam);
LONG DoCommand(HWND hWnd, UINT wMsg, UINT wParam, LONG lParam);
LONG DoMenuAbout(HWND hWnd, UINT wMsg, UINT wParam, LONG lParam);
LONG DoPaint(HWND hWnd, UINT wMsg, UINT wParam, LONG lParam);
LONG DoDestroy(HWND hWnd, UINT wMsg, UINT wParam, LONG lParam);
LONG DoClose(HWND hWnd, UINT wMsg, UINT wParam, LONG lParam);
LONG DoExit(HWND hWnd, UINT wMsg, UINT wParam, LONG lParam);
LONG DoCreate(HWND hWnd, UINT wMsg, UINT wParam, LONG lParam);
LONG DoSize(HWND hWnd, UINT wMsg, UINT wParam, LONG lParam);
LONG DoVScroll(HWND hWnd, UINT wMsg, UINT wParam, LONG lParam);
LONG DoHScroll(HWND hWnd, UINT wMsg, UINT wParam, LONG lParam);

LONG DoStartup(HWND hWnd, UINT wMsg, UINT wParam, LONG lParam);
void MsgWait(UINT interval);
void SizeWindow();

void SetScrollData(HWND hWnd);
void StoreLineOverprint(char *newline);
void StoreLines(char *buf);
void StoreLine(char *newline);

#ifndef _WIN32
#define APIENTRY  PASCAL
#define WNDPROC   FARPROC
#endif

#define dim(x) (sizeof(x) / sizeof(x[0]))

typedef struct                      /* associates messages (or menu ids) */
{                                   /* with a handler function */
   UINT Code;
   LONG (*Fxn)(HWND, UINT, UINT, LONG);
}  DECODEWORD;

DECODEWORD frameMsgs[] =               /* windows messages & handlers */
{
   WM_COMMAND,    DoCommand,
   WM_PAINT,      DoPaint,
   WM_CLOSE,      DoClose,
   WM_DESTROY,    DoDestroy,
   WM_USER,      DoStartup,
   WM_CREATE,    DoCreate,
   WM_SIZE,      DoSize,
   WM_VSCROLL,   DoVScroll,
   WM_USER+1,     DoExit,
   WM_HSCROLL,   DoHScroll
};

DECODEWORD menuItems[] =            /* menu items & associated handlers */
{
   /*IDM_ABOUT, */DoMenuAbout,
   0,             0
};

/*--------------------------------------------------------------------*/
/*       w i n _ e x i t                                              */
/*                                                                    */
/*       This is called by the worker thread when it exits or         */
/*       calls exit().  Here we post to the window to close and       */
/*       end the thread                                               */
/*--------------------------------------------------------------------*/

void win_exit(int x)
{
   while(atexit_fcount > 0) {
      --atexit_fcount;
      if (atexit_funcs[atexit_fcount] != NULL)
         (atexit_funcs[atexit_fcount])();
   }
   MsgWait(2000);    /* sleep for 2 secs. */

   PostMessage(hFrame, WM_USER + 1, 0, 0);
   _endthreadex(x);
}

int win_atexit( void ( __cdecl *func )( void ) )
{
   if (atexit_fcount < 20) {
      atexit_funcs[atexit_fcount++] = func;
      return 0;
   }
   return -1;
}

int win_printf(char *fmt, ...)
{
   va_list arg_ptr;
   int a;

   va_start(arg_ptr,fmt);

   a = vsprintf(Buf, fmt, arg_ptr);

   StoreLines(Buf);
   return a;
}

int win_fprintf(FILE *stream, char *fmt, ...)
{
   va_list arg_ptr;
   int a;

   va_start(arg_ptr,fmt);
   if (stream == stdout || stream == stderr) {
     a = vsprintf(Buf, fmt, arg_ptr);
     StoreLines(Buf);
   }
   else {
     a = vfprintf(stream,fmt,arg_ptr);
   }
   return a;
}

int win_isatty(int filen)
{
   if (fileno(stdout) == filen)
      return(fileno(stdout));
   if (fileno(stderr) == filen)
      return(fileno(stderr));
   return(isatty(filen));
}

int win_ferror( FILE *stream )
{
   if (stream == stdout || stream == stderr) {
      return 0;
   }
   else
      return(ferror(stream));
}

int win_fputc( int c, FILE *stream )
{
   if (stream == stdout || stream == stderr) {
      sprintf(Buf,"%c",c);
      StoreLines(Buf);
      return(c);
   }
   else
      return(fputc(c,stream));

}

int win_vfprintf( FILE *stream, const char *format, va_list argptr )
{
   int a;
   if (stream == stdout || stream == stderr) {
      a = vsprintf(Buf,format,argptr);
      StoreLines(Buf);
   }
   else return(vfprintf(stream,format,argptr));
}

void win_setTitle( const char *text)
{
   SetWindowText(hFrame, text);
}

/*--------------------------------------------------------------------*/
/*       W i n M a i n                                                */
/*                                                                    */
/*       Windows calls this to start the application.                 */
/*--------------------------------------------------------------------*/

int APIENTRY WinMain(HANDLE hInstance, HANDLE hPrevInstance,
                     LPSTR lpCmdLine, int nCmdShow)
{
   MSG msg;
   char *p;

   hInst = hInstance;

   p = strrchr(__argv[0],'\\');
   strcpy(szAppName,(p != NULL) ? p + 1 : __argv[0]);
   if (p = strrchr(szAppName,'.'))
       *p = '\0';

   if (!(InitApp(hInstance) && InitInstance(hInstance, nCmdShow)))
      return(FALSE);

   PostMessage(hFrame,WM_USER, 0,0);
   while (GetMessage(&msg, NULL, 0, 0) == TRUE)
   {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
   }
   return(msg.wParam);
}

/*--------------------------------------------------------------------*/
/*       InitApp                                                      */
/*                                                                    */
/*       Initialization for all instances of application.  This       */
/*       registers the main window class.                             */
/*--------------------------------------------------------------------*/

BOOL InitApp(HANDLE hInstance)
{
   WNDCLASS    wndclass;

   wndclass.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
   wndclass.lpfnWndProc   = MyWndProc;
   wndclass.cbClsExtra    = 0;
   wndclass.cbWndExtra    = 0;
   wndclass.hInstance     = hInstance;
   wndclass.hIcon         = LoadIcon(hInst, "Icon");
   wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
   wndclass.hbrBackground = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
   wndclass.lpszMenuName  = "Menu";

   wndclass.lpszClassName = szAppName;

   return(RegisterClass(&wndclass));
}

/*--------------------------------------------------------------------*/
/*       InitInstance                                                 */
/*                                                                    */
/*       Initializes this instance of app, creates windows.           */
/*--------------------------------------------------------------------*/

BOOL InitInstance(HANDLE hInstance, int nCmdShow)
{
   hFrame = CreateWindowEx( 0L, szAppName, szAppName,
                           WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL,
                           CW_USEDEFAULT, CW_USEDEFAULT,
                           CW_USEDEFAULT, CW_USEDEFAULT,
                           NULL, NULL, hInstance, NULL);
   if (!hFrame)
      return(FALSE);

   SizeWindow();                             /* set window & char sizes */
   ShowWindow(hFrame, nCmdShow);
   UpdateWindow(hFrame);

   return(TRUE);
}

/*--------------------------------------------------------------------*/
/*       SizeWindow                                                   */
/*                                                                    */
/*       Sets window's character and external dimensions.             */
/*--------------------------------------------------------------------*/

void SizeWindow()
{
   HDC hdc;
   TEXTMETRIC tm;
   RECT rect;
   int ychar, xchar;

   hdc = GetDC(hFrame);
   SelectObject(hdc, GetStockObject(SYSTEM_FIXED_FONT));
   GetTextMetrics(hdc, &tm);
   ychar = tm.tmHeight + tm.tmExternalLeading;
   xchar = tm.tmAveCharWidth;
   ReleaseDC(hFrame, hdc);

   /* set initial window width & height in chars */
   GetWindowRect(hFrame, &rect);
   MoveWindow( hFrame, rect.left, rect.top,
               80 * xchar,
               20 * ychar + GetSystemMetrics(SM_CYCAPTION) +
               GetSystemMetrics(SM_CYMENU), FALSE);
}

/*--------------------------------------------------------------------*/
/*       M y W n d P r o c                                            */
/*                                                                    */
/*       Callback function for application frame (main) window.       */
/*       Decodes message and routes to appropriate message            */
/*       handler.  If no handler found, calls DefWindowProc.          */
/*--------------------------------------------------------------------*/

LONG FAR APIENTRY MyWndProc(HWND hWnd, UINT wMsg, UINT wParam, LONG lParam)
{
   int i;

   for (i = 0; i < dim(frameMsgs); i++)
   {
      if (wMsg == frameMsgs[i].Code)
         return(*frameMsgs[i].Fxn)(hWnd, wMsg, wParam, lParam);
   }

   return(DefWindowProc(hWnd, wMsg, wParam, lParam));
}

/*--------------------------------------------------------------------*/
/*       DoCommand                                                    */
/*                                                                    */
/*       Demultiplexes WM_COMMAND messages resulting from menu        */
/*       selections, and routes to corresponding menu item            */
/*       handler.  Sends back any unrecognized messages to            */
/*       windows.                                                     */
/*--------------------------------------------------------------------*/

LONG DoCommand(HWND hWnd, UINT wMsg, UINT wParam, LONG lParam)
{
   int i;

   for (i = 0; i < dim(menuItems); i++)
   {
      if (wParam == menuItems[i].Code)
         return(*menuItems[i].Fxn)(hWnd, wMsg, wParam, lParam);
   }

   return(DefWindowProc(hWnd, wMsg, wParam, lParam));
}

LONG DoClose(HWND hWnd, UINT wMsg, UINT wParam, LONG lParam)
{

/*--------------------------------------------------------------------*/
/*       User hit close button                                        */
/*                                                                    */
/*       if WinAbort ptr set to non null then use it as the abort     */
/*       routine. other wise just exit and this will kill and         */
/*       cleanup the worker thread.                                   */
/*--------------------------------------------------------------------*/

   StoreLine("aborting.....Please wait.");

   if (ThreadId)
    PostThreadMessage(ThreadId,WM_CLOSE,0,0);

   if (WinAbort != NULL)
      (*WinAbort)();
   else
      DoExit(hWnd,wMsg,0,0);
   return(FALSE);
}

LONG DoExit(HWND hWnd, UINT wMsg, UINT wParam, LONG lParam)
{
   CloseHandle(hThread);
   DestroyWindow(hWnd);
   return(FALSE);
}

LONG DoStartup(HWND hWnd, UINT wMsg, UINT wParam, LONG lParam)
{
   unsigned __stdcall dowin_main(void *ignore);
   hThread = (HANDLE) _beginthreadex(NULL, 0, &dowin_main, NULL, 0, &ThreadId);
   return FALSE;
}

/*--------------------------------------------------------------------*/
/*    d o w i n _ m a i n                                             */
/*                                                                    */
/*    Calls the main routine intended for non windows app             */
/*--------------------------------------------------------------------*/

unsigned __stdcall dowin_main(void *ignore)
{
   win_exit(real_main(__argc,__argv));
   return 0;
}

/*--------------------------------------------------------------------*/
/*       DoDestroy                                                    */
/*                                                                    */
/*       Posts a WM_QUIT message to the task's win queue, which       */
/*       causes the main translate & dispatch loop to exit, and       */
/*       the app to terminate.                                        */
/*--------------------------------------------------------------------*/

LONG DoDestroy(HWND hWnd, UINT wMsg, UINT wParam, LONG lParam)
{
   PostQuitMessage(0);
   return(FALSE);
}

/*--------------------------------------------------------------------*/
/*       DoMenuAbout                                                  */
/*                                                                    */
/*       Respond to "About..." menu selection by invoking the         */
/*       "About" dialog box.                                          */
/*--------------------------------------------------------------------*/

LONG DoMenuAbout(HWND hWnd, UINT wMsg, UINT wParam, LONG lParam)
{
   WNDPROC lpProcAbout;

   lpProcAbout = MakeProcInstance((WNDPROC)AboutDlgProc, hInst);
   DialogBox(hInst, "AboutBox", hWnd, lpProcAbout);
   FreeProcInstance(lpProcAbout);

   return(FALSE);
}

/*--------------------------------------------------------------------*/
/*       AboutDlgProc                                                 */
/*                                                                    */
/*       Callback for the "About" dialog box                          */
/*--------------------------------------------------------------------*/

BOOL FAR APIENTRY AboutDlgProc(HWND hWnd, UINT wMsg, UINT wParam, LONG lParam)
{
   if ((wMsg == WM_COMMAND) && (wParam == IDOK))   /* dismiss dialog if OK */
      EndDialog(hWnd, 0);

   return(FALSE);                                  /* otherwise just sit there */
}

/*--------------------------------------------------------------------*/
/*       S t o r e L i n e s                                          */
/*                                                                    */
/*       buf contains zero or more lines buf usually points to        */
/*       Buf[1024] which is safe to modify at this point              */
/*--------------------------------------------------------------------*/

void StoreLines(char *buf)
{
   static char partial_line[MAX_LINE_LENGTH + MAX_LINE_LENGTH];
   char *p, *line_start;
   BOOL bPartial = FALSE;
   char sav;
   int i;

   if (*partial_line) {
      strcat(partial_line,buf);
      strcpy(buf,partial_line);
      *partial_line = '\0';
      bPartial = TRUE;
   }

   for(sav = '\0', i = 0, p = line_start = buf; *p; ++p,++i) {
      if (*p == '\n' || i == MAX_LINE_LENGTH) {
         sav = *p;
         *p = '\0';

         if (bPartial == TRUE) {
            StoreLineOverprint(line_start);
            bPartial = FALSE;
         }
         else
            StoreLine(line_start);
         if (sav == '\n')
            ++p;
         else {
            *p = sav;
            i = 0;
            sav = '\0';
         }
         line_start = p;
      }
   }

   /* last line was did not have newline in it */
   /* store partial line and print it too. */
   if (*line_start) {
      strcpy(partial_line,line_start);
      if (bPartial) /* are we continuing partial line */
         StoreLineOverprint(line_start);
      else
         StoreLine(line_start);
   }
   else
      *partial_line = '\0';
}

/*--------------------------------------------------------------------*/
/*       S t o r e L i n e O v e r p r i n t                          */
/*                                                                    */
/*       Replace the last line with new contents and invalidate       */
/*       it, if its currently being displayed.                        */
/*--------------------------------------------------------------------*/

void StoreLineOverprint(char *newline)
{
   /* Potential conflict here if DoPaint access line_buf[NumLines] after */
   /* we free but before we malloc. So decr NumLines during this process. */
   --NumLines;
   free(line_buf[NumLines]);
   line_buf[NumLines] = malloc(strlen(newline)+1);
   strcpy(line_buf[NumLines],newline);
   ++NumLines;
   if ( (yClient && NumLines <= PageSize) || /* less than 1 page */
       ((NumLines - yPos) == (PageSize + 1)) ) {  /* Scrolled to bottom */
      RECT rect;
      rect.top = ((NumLines - 1) - yPos) * yChar;
      rect.bottom = yClient;
      rect.left = 0;
      rect.right = xClient;
      InvalidateRect(hFrame,&rect,FALSE);
      PostMessage(hFrame,WM_PAINT,0,0);
      /*UpdateWindow(hFrame); */
   }
}

void StoreLine(char *newline)
{
   int i;

/*--------------------------------------------------------------------*/
/*       New line to store.                                           */
/*                                                                    */
/*       1.) if we are not at max scroll pos then just add to         */
/*       array and return.                                            */
/*                                                                    */
/*       2.) if we are max scroll pos and more or equal to a page     */
/*       full of lines exist then just scroll up 1 line and           */
/*       invalidate.                                                  */
/*                                                                    */
/*       3.) if less than a page full of lines exist, just            */
/*       invalidate the next line.  If we have maxed out backup       */
/*       ypos until 0 then we need to fix the display to scroll       */
/*       off lines that are no longer in line_buf.                    */
/*                                                                    */
/*       In all cases we need to update the scroll params.            */
/*--------------------------------------------------------------------*/

   int PrevNumLines = NumLines;
   int bMaxedOut = 0;

   if (NumLines == MAX_LINES) {
      free(line_buf[0]);
      for(i = 0; i < (NumLines - 1); ++i) {
         line_buf[i] = line_buf[i+1];
      }
      bMaxedOut = 1;
      --NumLines;
   }

   line_buf[NumLines] = malloc(strlen(newline)+1);
   strcpy(line_buf[NumLines],newline);
   ++NumLines;

   if (IsIconic(hFrame))
      return;

   if (PrevNumLines != NumLines)
       SetScrollData(hFrame);

   if (yClient && NumLines <= PageSize) {  /* just invalidate no scrolling */
      RECT rect;
      rect.top = (PrevNumLines * yChar);
      rect.bottom = yClient;
      rect.left = 0;
      rect.right = xClient;
      InvalidateRect(hFrame,&rect,FALSE);
      PostMessage(hFrame,WM_PAINT,0,0);
      /*UpdateWindow(hFrame); */
   }
   else if ((NumLines - yPos) == (PageSize + 1))   {
      if (bMaxedOut) {
         --yPos;
      }
      DoVScroll(hFrame, 0, SB_LINEDOWN, 0);
   }
   else {     /* screen possibly unchanged */
      if (bMaxedOut) {
         --yPos;
      }
      if (yPos < 0)
         DoVScroll(hFrame, 0, SB_LINEDOWN, 0);
      return;
   }
}

LONG DoCreate(HWND hWnd, UINT wMsg, UINT wParam, LONG lParam)  /* WM_CREATE */
{
   HDC hdc;
   int xUpper;      /* average width of uppercase letters    */

   /* Get the handle of the client area's device context. */

   hdc = GetDC (hWnd);

   /* Extract font dimensions from the text metrics. */

   GetTextMetrics (hdc, &tm);
   xChar = tm.tmAveCharWidth;
   xUpper = (tm.tmPitchAndFamily & 1 ? 3 : 2) * xChar/2;
   yChar = tm.tmHeight + tm.tmExternalLeading;

   /* Free the device context. */

   ReleaseDC (hWnd, hdc);

   /*
    * Set an arbitrary maximum width for client area.
    * (xClientMax is the sum of the widths of 60 average
    * lowercase letters and 12 uppercase letters.)
    */
   xClientMax = (160 * xUpper);

   return 0;
}

LONG DoSize(HWND hWnd, UINT wMsg, UINT wParam, LONG lParam)  /* WM_SIZE */
{

   /* Retrieve the dimensions of the client area. */

    yClient = HIWORD (lParam);
    xClient = LOWORD (lParam);

   SetScrollData(hWnd);
   return 0;

}

void SetScrollData(HWND hWnd)
{
    /*
     * Determine the maximum vertical scrolling position.
     * The two is added for extra space below the lines
     * of text.
     */
   PageSize = (yClient / yChar);
   yMax = max(NumLines - 1, 0);

    /*
     * Make sure the current vertical scrolling position
     * does not exceed the maximum.
     */
    yPos = min (yPos, yMax);

    /*
     * Adjust the vertical scrolling range and scroll box
     * position to reflect the new yMax and yPos values.
     */
     si.cbSize = sizeof(si);
     si.fMask  = SIF_RANGE | SIF_PAGE | SIF_POS;
     si.nMin   = 0;
     si.nMax   = yMax;
     si.nPage  = PageSize;
     si.nPos   = yPos;
     SetScrollInfo(hWnd, SB_VERT, &si, TRUE); /* this mode changes the scroll bar size */
                                    /* and does'nt page up past the bottom. */

    /*SetScrollRange(hWnd,SB_VERT,0,yMax,TRUE); */
    /*
     * Determine the maximum horizontal scrolling position.
     * The two is added for extra space to the right of the
     * lines of text.
     */

     xMax = max (0, 2 + (xClientMax - xClient)/xChar);

    /*
     * Make sure the current horizontal scrolling position
     * does not exceed the maximum.
     */

     xPos = min (xPos, xMax);

    /*
     * Adjust the horizontal scrolling range and scroll box
     * position to reflect the new xMax and xPos values.
     */
     si.cbSize = sizeof(si);
     si.fMask  = SIF_RANGE | SIF_PAGE | SIF_POS;
     si.nMin   = 0;
     si.nMax   = xMax;
     si.nPage  = xClient / xChar;
     si.nPos   = xPos;
     SetScrollInfo(hWnd, SB_HORZ, &si, TRUE);

}

/*--------------------------------------------------------------------*/
/*       D o P a i n t                                                */
/*                                                                    */
/*       Paint the client window with the contents of the transfer    */
/*       buffer.                                                      */
/*--------------------------------------------------------------------*/

LONG DoPaint(HWND hWnd, UINT wMsg, UINT wParam, LONG lParam)
{
   HDC hdc;
   PAINTSTRUCT ps;
   int i,x,y;
   int FirstLine, LastLine;

   hdc = BeginPaint(hWnd, &ps);

   /* Only draw the invalidated portion of the window */
   FirstLine = max (0, ((yPos + ps.rcPaint.top)/yChar) - 2);
   LastLine = min (NumLines, (1 + yPos + ps.rcPaint.bottom/yChar));

   for (i = FirstLine;i < LastLine;i++) {
        x = xChar * (1 - xPos);
        y = (yChar * (-yPos + i));

      if ((y + yChar) > (yChar * PageSize)) /* dont print partial lines */
         break;

      TextOut(hdc, x, y, line_buf[i], lstrlen(line_buf[i]));
    }

    EndPaint(hWnd, &ps);

   return FALSE;
}

LONG DoHScroll(HWND hWnd, UINT wMsg, UINT wParam, LONG lParam) /* WM_HSCROLL */
{
    int xInc;               /* horizontal scrolling increment        */

   switch(LOWORD (wParam)) {

            /* User clicked shaft left of the scroll box. */

            case SB_PAGEUP:
                 xInc = -8;
                 break;

            /* User clicked shaft right of the scroll box. */

            case SB_PAGEDOWN:
                 xInc = 8;
                 break;

            /* User clicked the left arrow. */

            case SB_LINEUP:
                 xInc = -1;
                 break;

            /* User clicked the right arrow. */

            case SB_LINEDOWN:
                 xInc = 1;
                 break;

            /* User dragged the scroll box. */

            case SB_THUMBTRACK:
                 xInc = HIWORD(wParam) - xPos;
                 break;

            default:
                 xInc = 0;

        }

        /*
         * If applying the horizontal scrolling increment does not
         * take the scrolling position out of the scrolling range,
         * increment the scrolling position, adjust the position
         * of the scroll box, and update the window.
         */

        if (xInc = max (-xPos, min (xInc, xMax - xPos))) {
            xPos += xInc;
            ScrollWindowEx (hWnd, -xChar * xInc, 0,
                (CONST RECT *) NULL, (CONST RECT *) NULL,
                (HRGN) NULL, (LPRECT) NULL, SW_ERASE | SW_INVALIDATE);
            si.cbSize = sizeof(si);
            si.fMask  = SIF_POS;
            si.nPos   = xPos;
            SetScrollInfo(hWnd, SB_HORZ, &si, TRUE);
            UpdateWindow (hWnd);
        }

        return 0;
}

LONG DoVScroll(HWND hWnd, UINT wMsg, UINT wParam, LONG lParam)
{
   int yInc;               /* vertical scrolling increment          */

   switch(LOWORD (wParam)) {

            /* User clicked the shaft above the scroll box. */

            case SB_PAGEUP:
                 yInc = min(-1, -yClient / yChar);
                 break;

            /* User clicked the shaft below the scroll box. */

            case SB_PAGEDOWN:
                 yInc = max(1, yClient / yChar);
                 break;

            /* User clicked the top arrow. */

            case SB_LINEUP:
                 yInc = -1;
                 break;

            /* User clicked the bottom arrow. */

            case SB_LINEDOWN:
                 yInc = 1;
                 break;

            /* User dragged the scroll box. */

            case SB_THUMBTRACK:
                 yInc = HIWORD(wParam) - yPos;
                 break;

            default:
                 yInc = 0;

        }

        /*
         * If applying the vertical scrolling increment does not
         * take the scrolling position out of the scrolling range,
         * increment the scrolling position, adjust the position
         * of the scroll box, and update the window. UpdateWindow
         * sends the WM_PAINT message.
         */
   if ((yPos + yInc + PageSize) > NumLines) {
      yInc = (NumLines - (yPos + PageSize));
   }

   if (yInc = max(-yPos, min(yInc, yMax - yPos) )) {
         RECT rect;

            yPos += yInc;
         rect.top = 0;
         rect.left = 0;
         rect.right = xClient;
         rect.bottom = yChar * PageSize;

            ScrollWindowEx(hWnd, 0, -yChar * yInc,
                (CONST RECT *) &rect, (CONST RECT *) &rect,
            (HRGN) NULL, (LPRECT) NULL, SW_ERASE | SW_INVALIDATE);

            si.cbSize = sizeof(si);
            si.fMask  = SIF_POS;
            si.nPos   = yPos;
            SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
           /* PostMessage(hWnd,WM_PAINT,0,0); */
         UpdateWindow(hWnd);
   }

    return 0;
}

/*--------------------------------------------------------------------*/
/*       M s g W a i t                                                */
/*                                                                    */
/*       Check messages for interval ms then return                   */
/*--------------------------------------------------------------------*/

void MsgWait(UINT interval)
{
   MSG msg;
   UINT timer = 0;

   timer = SetTimer(NULL,0,interval,NULL);

   while (GetMessage(&msg, NULL, 0, 0) == TRUE) {
      if (msg.message == WM_TIMER) {
         break;
      }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

   if (timer)
      KillTimer(NULL,timer);
}
