/*--------------------------------------------------------------------*/
/*       u u t a p i . c                                              */
/*                                                                    */
/*       All the TAPI support code.                                   */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1998 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: LIB.H 1.42 1998/03/01 01:26:54 ahd v1-12v $
 *
 *    Revision history:
 *    $Log: LIB.H $
 */

/*--------------------------------------------------------------------*/
/*       Author:  Steve Drew <drew@cgo.dec.com>, with help from       */
/*       Tapi SDK & TapiComm example program.                         */
/*                                                                    */
/*       Main entry points, Tapi_AnswerCall, Tapi_Init,               */
/*       TapiDialCall                                                 */
/*                                                                    */
/*       Passing comm port name of "tapi" opens first available       */
/*       tapi modem device. otherwise name must match a substring     */
/*       of the tapi name. ie setting device to "Blaster" would       */
/*       match a tapi string of "Creative Labs Modem Blaster 28.8"    */
/*       a substring is used since uupc only uses first token of      */
/*       the Device= in the MDM file.                                 */
/*                                                                    */
/*       To be passed a call from other apps like winfax if it is     */
/*       a datacall and you modem supports call descrimination        */
/*       (most do) then add "uucico.exe" to the end of the            */
/*       datamodem line in the handoff section of telepon.ini in      */
/*       the windows directory. ie [HandoffPriorities]                */
/*       datamodem=wfxmod32.exe,rasapi32.dll,uucico.exe               */
/*                                                                    */
/*       uucico Debug level of 4, gets all debug results from this    */
/*       tapi module                                                  */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"

#include <windows.h>
#include <string.h>
#include <tapi.h>
#include <stdio.h>
#include <stdlib.h>
#include "pnterr.h"
#include "uutapi.h"

RCSID("$Id$");

extern HWND     hFrame;             /* handle of main window */
extern HANDLE   hInst;              /* our instance */

#define SUCCESS 0
#define EXPECTED_TAPI_VERSION 0x00010004
#define EARLY_TAPI_VERSION 0x00010003
#define WAITERR_WAITABORTED  1
#define WAITERR_WAITTIMEDOUT  2

/*  90 sec timeout if no reply, should never happen. */
#define WAITTIMEOUT 90000

/*  Constant used in WaitForCallState when any new */
/*  callstate message is acceptable. */
#define I_LINECALLSTATE_ANY 0

static DWORD    dwNumDevs;          /* the number of line devices available */
static DWORD    dwDeviceID;         /* the device ID */
static DWORD    dwRequestedID;
static LONG     lAsyncReply;

char            TapiLineName[150];  /* Set to TAPI device name After
                                     * sucessful Tapi_Init() */
char           *TapiMsg;            /* NULL or LPCSTR to error message if
                                     * occured. */

/*  BOOLEANS to handle reentrancy */
static BOOL     bShuttingDown;
static BOOL     bStoppingCall;
static BOOL     bInitializing;
static BOOL     bConnected;
static BOOL     bReplyReceived;     /* LINE_REPLY msg received */
static BOOL     bTapiInUse;         /* whether TAPI is in use or not */
static BOOL     bInitialized;       /* whether TAPI has been initialized */
static HLINEAPP hLineApp;           /* the usage handle of this application
                                     * for TAPI */
static HCALL    hCall;              /* handle to the call */
static HLINE    hLine;              /* handle to the open line */
static DWORD    dwAPIVersion;       /* the API version */
static char     szPhoneNumber[64];  /* the phone number to call */
static HANDLE   hCommFile = INVALID_HANDLE_VALUE;       /* Handle to the comm
                                                         * port once connected. */
static BOOL     bCallStateReceived;
static DWORD    dwCallState;

static BOOL     Tapi_HangupCall();
static BOOL     Tapi_HandleLineErr(long lLineErr);
static BOOL     Tapi_GetDeviceLine(DWORD * dwAPIVersion, char *szDeviceName, DWORD dwDeviceStart);
static LPLINEDEVCAPS
Tapi_MylineGetDevCaps(LPLINEDEVCAPS lpLineDevCaps,
                      DWORD dwDeviceID, DWORD dwAPIVersion);
static LPVOID   CheckAndReAllocBuffer(LPVOID lpBuffer, size_t sizeBufferMinimum);
static LPLINEADDRESSCAPS Tapi_MylineGetAddressCaps (LPLINEADDRESSCAPS lpLineAddressCaps,
        DWORD dwDeviceID, DWORD dwAddressID,DWORD dwAPIVersion, DWORD dwExtVersion);
static BOOL     Tapi_MakeTheCall(LPCSTR lpszAddress);
static LPLINECALLPARAMS Tapi_CreateCallParams(LPLINECALLPARAMS lpCallParams, LPCSTR lpszDisplayableAddress);
static long     Tapi_WaitForReply(long lRequestID);
static void
Tapi_HandleLineCallState(DWORD dwDevice, DWORD dwMessage, DWORD dwCallbackInstance,
                         DWORD dwParam1, DWORD dwParam2, DWORD dwParam3);
static void
Tapi_HandleLineDevState(
                  DWORD dwDevice, DWORD dwMessage, DWORD dwCallbackInstance,
                        DWORD dwParam1, DWORD dwParam2, DWORD dwParam3);
static void CALLBACK
Tapi_lineCallbackFunc(
                      DWORD dwDevice, DWORD dwMsg, DWORD dwCallbackInstance,
                      DWORD dwParam1, DWORD dwParam2, DWORD dwParam3);
static long     WaitForCallState(DWORD dwDesiredCallState, int Timeout);
static int      Tapi_GetCallRate(HCALL hCall);
static BOOL     Tapi_LineStatus(HLINE hLine);

/*--------------------------------------------------------------------*/
/*       Currently szDeviceName is used to match against the TAPI     */
/*       modem name to find the port to use.  The COM port number     */
/*       does not even enter the picture.  In fact you have to        */
/*       actually open the device before you can find out what com    */
/*       port you got.  So Set the Device= in you MDM file to be      */
/*       any substring of the TAPI modem name or set it to ANY to     */
/*       pick the first availble modem                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Set our abort routine if user closes window in winstdio.c    */
/*       After aborting this thread returns and post close message    */
/*       to parent thread who then exits gracefully                   */
/*--------------------------------------------------------------------*/

extern int      (*WinAbort) ();

uucico_tapi_abort()
{
   extern int      terminate_processing;
   BOOL            AbortComm(void);

   terminate_processing = TRUE;
   AbortComm();
   return FALSE;
}

/*--------------------------------------------------------------------*/
/*      T a p i _ I n i t                                             */
/*                                                                    */
/*      PURPOSE: Initializes TAPI                                     */
/*                                                                    */
/*      1.) Init TAPI API layer                                       */
/*      2.) Get a device line handle                                  */
/*      3.) Open the line with Answer and originate privs             */
/*--------------------------------------------------------------------*/

BOOL
Tapi_Init(char *szDeviceName)
{
   long            lReturn;
   int             win_atexit(void (__cdecl * func) (void));

   /* If we're already initialized, then initialization succeeds. */
   if (hLineApp)
      return TRUE;

   /* If we're in the middle of initializing, then fail, we're not done. */
   if (bInitializing)
      return FALSE;

   bInitializing = TRUE;

   win_atexit(Tapi_Shutdown);
   WinAbort = uucico_tapi_abort;

   /* Initialize TAPI */
   lReturn = lineInitialize(&hLineApp, hInst, Tapi_lineCallbackFunc,
                            NULL,
                            &dwNumDevs);

   if (!Tapi_HandleLineErr(lReturn))
   {
      bInitializing = FALSE;
      return FALSE;
   }
   if (dwNumDevs == 0)
   {
      TapiMsg = "There are no telephony devices installed";
      bInitializing = FALSE;
      Tapi_Shutdown();
      return FALSE;
   }
/*--------------------------------------------------------------------*/
/*       Find a line based on name.                                   */
/*       IF name = "tapi" and line in use, try next line              */
/*--------------------------------------------------------------------*/

   dwDeviceID = 0;
   lReturn = -1;
   while (Tapi_GetDeviceLine(&dwAPIVersion, szDeviceName, dwDeviceID) == TRUE)
   {

      lReturn = lineOpen(hLineApp, dwDeviceID, &hLine, dwAPIVersion, 0, 0,
                         LINECALLPRIVILEGE_OWNER,
                         LINEMEDIAMODE_DATAMODEM, 0);

      if (lReturn != SUCCESS)
      {
         if ((lReturn == LINEERR_ALLOCATED) || (lReturn == LINEERR_RESOURCEUNAVAIL))
         {
            TapiMsg = "Line is already in use";
         }
         Tapi_HandleLineErr(lReturn);   /* just set TapiMsg; */

         if (stricmp(szDeviceName, "tapi") == 0)
         {
            ++dwDeviceID;
            continue;
         }
      }
      break;
   }

   if (lReturn != SUCCESS)
   {
      Tapi_Shutdown();
      bInitializing = FALSE;
      return FALSE;
   }

/*--------------------------------------------------------------------*/
/*       Using the SetAppPriority call caused TAPIEXE crashes for     */
/*       some reason?                                                 */
/*--------------------------------------------------------------------*/

   lineSetAppPriority("uucico.exe",
                      LINEMEDIAMODE_DATAMODEM,
                      NULL,
                      0,
                      NULL,
                      1);

   Tapi_LineStatus(hLine);

   printmsg(2, "Tapi: Line '%s' has been opened", TapiLineName);

   bInitializing = FALSE;
   return (TRUE);
}

/*--------------------------------------------------------------------*/
/*       T a p i _ D i a l C a l l                                    */
/*                                                                    */
/*       Get a number from the user and dial it.                      */
/*--------------------------------------------------------------------*/

HANDLE
Tapi_DialCall(char *szDeviceName, char *szPhoneNumber, int Timeout)
{
   if (szPhoneNumber == (char *) NULL)
   {
      return FALSE;
   }
   if (Tapi_Init(szDeviceName) == FALSE)
   {
      return INVALID_HANDLE_VALUE;
   }
   printmsg(2, "Tapi: Dial %s", szPhoneNumber);
   /* Start dialing the number */
   if (Tapi_MakeTheCall(szPhoneNumber))
   {

/*--------------------------------------------------------------------*/
/*       Number has been dailed wait for connect                      */
/*                                                                    */
/*       We only wait for the connected state, but if the call        */
/*       fails the reply function calls shutdown which will cause     */
/*       waitforstate to be aborted                                   */
/*--------------------------------------------------------------------*/

      printmsg(3, "Tapi: Waiting for CONNECTED state");
      if (WaitForCallState(LINECALLSTATE_CONNECTED, Timeout) == SUCCESS)
      {
         /* Make sure we have a good comm handle */
         if (GetFileType(hCommFile) == FILE_TYPE_CHAR)
         {
            return hCommFile;
         }
      }
   }
   Tapi_Shutdown();
   return hCommFile;                /* will be INVALID_HANDLE_VALUE */

} /* Tapi_DialCall */

/*--------------------------------------------------------------------*/
/*       T a p i _ A n s w e r C a l l                                */
/*                                                                    */
/*       Answer an inbound call                                       */
/*--------------------------------------------------------------------*/

HANDLE
Tapi_AnswerCall(char *szDeviceName,
                UINT interval,
                long *TapiShutdown)
{                                   /* interval = msec */
   void            MsgWait(UINT interval);      /* in winstdio.c */

   if (Tapi_Init(szDeviceName) == FALSE)
   {
      *TapiShutdown = 1;
      return 0;
   }
   MsgWait(interval);

   *TapiShutdown = (hLineApp == NULL) ? 1 : 0;

   return hCommFile;
}

BOOL
Tapi_MsgCheck()
{
   MSG             msg;

   while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
   {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
   }

/*--------------------------------------------------------------------*/
/*      If NULL then we've gone thru shutdown via a message reply     */
/*      so let our caller know that tapi is shutdown.                 */
/*--------------------------------------------------------------------*/

   return (bTapiInUse);

}  /* Tapi_MsgCheck */

static BOOL
Tapi_LineStatus(HLINE hLine)
{
   LPLINEDEVSTATUS lpLineDevStatus = malloc(sizeof(LINEDEVSTATUS) + 1024);
   long            lReturn;

   lpLineDevStatus->dwTotalSize = (sizeof(LINEDEVSTATUS) + 1024);

   lReturn = lineGetLineDevStatus(hLine, lpLineDevStatus);
   if (!Tapi_HandleLineErr(lReturn))
   {
      free(lpLineDevStatus);
      return FALSE;
   }
   printmsg(4, "Tapi: Device Opens = %d, Calls Active = %d",
            lpLineDevStatus->dwNumOpens, lpLineDevStatus->dwNumActiveCalls);

   free(lpLineDevStatus);
   return TRUE;
}

/*--------------------------------------------------------------------*/
/*       T a p i _ G e t D e v i c e L i n e                          */
/*                                                                    */
/*       PURPOSE:  Gets the first available line device.              */
/*                                                                    */
/*       Sets TapiMsg to error string Sets dwDeviceID to a good       */
/*       TAPI device                                                  */
/*--------------------------------------------------------------------*/

static          BOOL
Tapi_GetDeviceLine(DWORD * pdwAPIVersion,
                   char *szDeviceNameRequested,
                   DWORD dwDeviceStart)
{
   long            lReturn;
   LINEEXTENSIONID lineExtID;
   LPLINEDEVCAPS   lpLineDevCaps = NULL;

   /* Go thru all devices and find the one we need */
   for (dwDeviceID = dwDeviceStart; dwDeviceID < dwNumDevs; dwDeviceID++)
   {

      lReturn = lineNegotiateAPIVersion(hLineApp, dwDeviceID,
                                  EARLY_TAPI_VERSION, EXPECTED_TAPI_VERSION,
                                        pdwAPIVersion, &lineExtID);

      if ((Tapi_HandleLineErr(lReturn)) && (*pdwAPIVersion))
      {

         lpLineDevCaps = Tapi_MylineGetDevCaps(lpLineDevCaps, dwDeviceID, *pdwAPIVersion);

         if (lpLineDevCaps && (lpLineDevCaps->dwLineNameSize) &&
             (lpLineDevCaps->dwLineNameOffset) &&
             (lpLineDevCaps->dwStringFormat == STRINGFORMAT_ASCII))
             {

            /* This is the name of the device. */
            strcpy(TapiLineName, ((char *) lpLineDevCaps) + lpLineDevCaps->dwLineNameOffset);

            if (TapiMsg == NULL)
               TapiMsg = "No Tapi line Available";

            if (!(lpLineDevCaps->dwBearerModes & LINEBEARERMODE_VOICE))
               continue;            /* Try next line */

            if (!(lpLineDevCaps->dwLineFeatures & LINEFEATURE_MAKECALL))
               continue;

            if (!(lpLineDevCaps->dwMediaModes & LINEMEDIAMODE_DATAMODEM))
               continue;

            if (stricmp(szDeviceNameRequested, "tapi") != 0 &&
                strstr(TapiLineName, szDeviceNameRequested) == NULL)
                {
               static char     NiceMsg[140];

               sprintf(NiceMsg, "'%s' Tapi device not found, but did find: '%s'",
                       szDeviceNameRequested, TapiLineName);
               TapiMsg = NiceMsg;
               continue;
            }

            /* Got a good line */
            TapiMsg = NULL;
            LocalFree(lpLineDevCaps);
            return (TRUE);
         }
      } else
       /* Couldn't NegotiateAPIVersion.  Line is unavail. */ if (TapiMsg == NULL)
         TapiMsg = "Line is Unavailable";

   }
   dwDeviceID = MAXDWORD;

   if (lpLineDevCaps)
      LocalFree(lpLineDevCaps);
   return (FALSE);

} /* Tapi_GetDeviceLine */

/*--------------------------------------------------------------------*/
/*  FUNCTION: MylineGetDevCaps(LPLINEDEVCAPS, DWORD , DWORD)          */
/*                                                                    */
/*  PURPOSE: Gets a LINEDEVCAPS structure for the specified line.     */
/*                                                                    */
/*  COMMENTS:                                                         */
/*                                                                    */
/*       This function is a wrapper around lineGetDevCaps to make     */
/*       it easy to handle the variable sized structure and any       */
/*       errors received.                                             */
/*                                                                    */
/*       The returned structure has been allocated with               */
/*       LocalAlloc, so LocalFree has to be called on it when         */
/*       you're finished with it, or there will be a memory leak.     */
/*                                                                    */
/*       Similarly, if a lpLineDevCaps structure is passed in, it     */
/*       *must* have been allocated with LocalAlloc and it could      */
/*       potentially be LocalFree()d.                                 */
/*                                                                    */
/*       If lpLineDevCaps == NULL, then a new structure is            */
/*       allocated.  It is normal to pass in NULL for this            */
/*       parameter unless you want to use a lpLineDevCaps that has    */
/*       been returned by a previous I_lineGetDevCaps call.           */
/*--------------------------------------------------------------------*/

static LPLINEDEVCAPS
Tapi_MylineGetDevCaps(LPLINEDEVCAPS lpLineDevCaps,
                      DWORD dwDeviceID, DWORD dwAPIVersion)
{
   /* Allocate enough space for the structure plus 1024. */
   size_t          sizeofLineDevCaps = sizeof(LINEDEVCAPS) + 1024;
   long            lReturn;

   /* Continue this loop until the structure is big enough. */
   while (TRUE)
   {
      /* Make sure the buffer exists, is valid and big enough. */
      lpLineDevCaps =
         (LPLINEDEVCAPS) CheckAndReAllocBuffer(
                                               (LPVOID) lpLineDevCaps,  /* Pointer to existing
                                                                         * buffer, if any */
                                               sizeofLineDevCaps);      /* Minimum size the
                                                                         * buffer should be */

      if (lpLineDevCaps == NULL)
         return NULL;

      /* Make the call to fill the structure. */
      do {
         lReturn = lineGetDevCaps(hLineApp, dwDeviceID, dwAPIVersion, 0, lpLineDevCaps);
         if (Tapi_HandleLineErr(lReturn))
            continue;
         else {
            LocalFree(lpLineDevCaps);
            return NULL;
         }
      }
      while (lReturn != SUCCESS);

      /* If the buffer was big enough, then succeed. */
      if ((lpLineDevCaps->dwNeededSize) <= (lpLineDevCaps->dwTotalSize))
         return lpLineDevCaps;

      /* Buffer wasn't big enough.  Make it bigger and try again. */
      sizeofLineDevCaps = lpLineDevCaps->dwNeededSize;
   }

} /* Tapi_MylineGetDevCaps */

/*--------------------------------------------------------------------*/
/*  FUNCTION: LPVOID CheckAndReAllocBuffer(LPVOID, size_t, LPCSTR)    */
/*                                                                    */
/*  PURPOSE: Checks and ReAllocates a buffer if necessary.            */
/*--------------------------------------------------------------------*/
static LPVOID
CheckAndReAllocBuffer(LPVOID lpBuffer, size_t sizeBufferMinimum)
{
   size_t          sizeBuffer;

   if (lpBuffer == NULL)
   {          /* Allocate the buffer if necessary. */
      sizeBuffer = sizeBufferMinimum;
      lpBuffer = (LPVOID) LocalAlloc(LPTR, sizeBuffer);

      if (lpBuffer == NULL)
      {
         return NULL;
      }
   } else {                         /* If the structure already exists, make
                                     * sure its good. */
      sizeBuffer = LocalSize((HLOCAL) lpBuffer);

      if (sizeBuffer == 0)          /* Bad pointer? */
      {
         return NULL;
      }
      /* Was the buffer big enough for the structure? */
      if (sizeBuffer < sizeBufferMinimum)
      {
         LocalFree(lpBuffer);
         return CheckAndReAllocBuffer(NULL, sizeBufferMinimum);
      }
   }
   ((LPVARSTRING) lpBuffer)->dwTotalSize = (DWORD) sizeBuffer;
   return lpBuffer;
}

/*--------------------------------------------------------------------*/
/*       T a p i _ M y l i n e G e t A d d r e s s C a p s            */
/*                                                                    */
/*       Return a LINEADDRESSCAPS structure for the                   */
/*       specified line.  Use to get current state of line.           */
/*--------------------------------------------------------------------*/

static LPLINEADDRESSCAPS
Tapi_MylineGetAddressCaps(
    LPLINEADDRESSCAPS lpLineAddressCaps,
    DWORD dwDeviceID, DWORD dwAddressID,
    DWORD dwAPIVersion, DWORD dwExtVersion)
{

    size_t sizeofLineAddressCaps = sizeof(LINEADDRESSCAPS) + 1024;
    long lReturn;

    /* Continue this loop until the structure is big enough. */
    for (;;)
    {
        /* Make sure the buffer exists, is valid and big enough. */
        lpLineAddressCaps =  (LPLINEADDRESSCAPS) CheckAndReAllocBuffer(
                                           (LPVOID) lpLineAddressCaps,
                                                 sizeofLineAddressCaps);
        if (lpLineAddressCaps == NULL)
            return NULL;

        /* Make the call to fill the structure. */
        do {
            lReturn = lineGetAddressCaps(hLineApp, dwDeviceID, dwAddressID,
                              dwAPIVersion, dwExtVersion, lpLineAddressCaps);

            if (Tapi_HandleLineErr(lReturn))
                continue;
            else {
                LocalFree(lpLineAddressCaps);
                return NULL;
            }
        } while (lReturn != SUCCESS);

        /* If the buffer was big enough, then succeed. */
        if ((lpLineAddressCaps -> dwNeededSize) <=
            (lpLineAddressCaps -> dwTotalSize))
        {
            return lpLineAddressCaps;
        }

        /* Buffer wasn't big enough.  Make it bigger and try again. */
        sizeofLineAddressCaps = lpLineAddressCaps -> dwNeededSize;
    }

} /* Tapi_MylineGetAddressCaps */

/*--------------------------------------------------------------------*/
/*  FUNCTION: Tapi_MakeTheCall(LPCSTR)                                */
/*                                                                    */
/*  PURPOSE: Dials the call                                           */
/*--------------------------------------------------------------------*/

static BOOL
Tapi_MakeTheCall(LPCTSTR lpszAddress)
{
   LPLINECALLPARAMS lpCallParams = NULL;
   LPLINEADDRESSCAPS lpAddressCaps = NULL;

   long            lReturn;
   BOOL            bFirstDial = TRUE;

   /* Get the capabilities for the line device we're going to use. */
   lpAddressCaps = Tapi_MylineGetAddressCaps(lpAddressCaps,
                                             dwDeviceID,
                                             0,
                                             dwAPIVersion,
                                             0);

   if (lpAddressCaps == NULL)
      return FALSE;

   /* Setup our CallParams. */
   lpCallParams = Tapi_CreateCallParams(lpCallParams, lpszAddress);
   if (lpCallParams == NULL)
      return FALSE;

   bTapiInUse = TRUE;
   do {
      if (bFirstDial)
         lReturn = Tapi_WaitForReply(lineMakeCall(hLine, &hCall, lpszAddress,
                                                  0, lpCallParams));
      else
         lReturn = Tapi_WaitForReply(lineDial(hCall, lpszAddress, 0));

      if (lReturn == WAITERR_WAITABORTED)
         goto errExit;

      if (Tapi_HandleLineErr(lReturn))
         continue;
      else
         goto errExit;
   }
   while (lReturn != SUCCESS);

   bFirstDial = FALSE;

   if (lpCallParams)
      LocalFree(lpCallParams);
   if (lpAddressCaps)
      LocalFree(lpAddressCaps);

   TapiMsg = NULL;

   return TRUE;

errExit:
   if (lpCallParams)
      LocalFree(lpCallParams);
   if (lpAddressCaps)
      LocalFree(lpAddressCaps);

   return FALSE;
}

/*--------------------------------------------------------------------*/
/*  FUNCTION: Tapi_CreateCallParams(LPLINECALLPARAMS, LPCSTR)         */
/*                                                                    */
/*  PURPOSE: Allocates and fills a LINECALLPARAMS structure           */
/*--------------------------------------------------------------------*/

static LPLINECALLPARAMS
Tapi_CreateCallParams(
               LPLINECALLPARAMS lpCallParams, LPCSTR lpszDisplayableAddress)
{
   size_t          sizeDisplayableAddress;

   if (lpszDisplayableAddress == NULL)
      lpszDisplayableAddress = "";

   sizeDisplayableAddress = strlen(lpszDisplayableAddress) + 1;

   lpCallParams = (LPLINECALLPARAMS) CheckAndReAllocBuffer(
                                                      (LPVOID) lpCallParams,
                           sizeof(LINECALLPARAMS) + sizeDisplayableAddress);

   if (lpCallParams == NULL)
      return NULL;

   memset(lpCallParams, 0, sizeof(lpCallParams));

   /* This is where we configure the line. */
   lpCallParams->dwBearerMode = LINEBEARERMODE_VOICE;
   lpCallParams->dwMediaMode = LINEMEDIAMODE_DATAMODEM;

   /* This specifies that we want to use only IDLE calls and */
   /* don't want to cut into a call that might not be IDLE (ie, in use). */
   lpCallParams->dwCallParamFlags = LINECALLPARAMFLAGS_IDLE;

   /* Address we are dialing. */
   lpCallParams->dwAddressMode = LINEADDRESSMODE_DIALABLEADDR;
   lpCallParams->dwDisplayableAddressOffset = sizeof(LINECALLPARAMS);
   lpCallParams->dwDisplayableAddressSize = sizeDisplayableAddress;
   strcpy((LPSTR) lpCallParams + sizeof(LINECALLPARAMS),
          lpszDisplayableAddress);
   lpCallParams->dwTotalSize = sizeof(LINECALLPARAMS);
   return lpCallParams;
}

/*--------------------------------------------------------------------*/
/*       T a p i _ G e t C a l l R a t e                              */
/*                                                                    */
/*       Returns BPS of current call or 0 if not connected            */
/*--------------------------------------------------------------------*/

static int
Tapi_GetCallRate(HCALL hCall)
{
   LPLINECALLINFO  lpCallInfo;
   LONG            lReturn;
   int             bps;

   lpCallInfo = (LPLINECALLINFO) LocalAlloc(LPTR, sizeof(LINECALLINFO));

   if (lpCallInfo == 0)
      return 0;
   lpCallInfo->dwTotalSize = sizeof(LINECALLINFO);

   lReturn = lineGetCallInfo(hCall, lpCallInfo);

   if (lReturn == SUCCESS)
   {
      bps = lpCallInfo->dwRate;
   } else
      bps = 0;
   LocalFree(lpCallInfo);

   return bps;
}

/*--------------------------------------------------------------------*/
/*   FUNCTION: long Tapi_WaitForReply(long)                           */
/*                                                                    */
/*   PURPOSE: Resynchronize by waiting for a LINE_REPLY               */
/*                                                                    */
/*   PARAMETERS:                                                      */
/*     lRequestID - The asynchronous request ID that we're            */
/*                  on a LINE_REPLY for.                              */
/*                                                                    */
/*   RETURN VALUE:                                                    */
/*     - 0 if LINE_REPLY responded with a success.                    */
/*     - LINEERR constant if LINE_REPLY responded with a LINEERR      */
/*     - 1 if the line was shut down before LINE_REPLY is received.   */
/*                                                                    */
/*   COMMENTS:                                                        */
/*                                                                    */
/*       This function allows us to resynchronize an asynchronous     */
/*       TAPI line call by waiting for the LINE_REPLY message.  It    */
/*       waits until a LINE_REPLY is received or the line is shut     */
/*       down.                                                        */
/*                                                                    */
/*       Note that this could cause re-entrancy problems as well      */
/*       as mess with any message preprocessing that might occur      */
/*       on this thread (such as TranslateAccelerator).               */
/*                                                                    */
/*       This function should to be called from the thread that       */
/*       did lineInitialize, or the PeekMessage is on the wrong       */
/*       thread and the synchronization is not guaranteed to work.    */
/*       Also note that if another PeekMessage loop is entered        */
/*       while waiting, this could also cause synchronization         */
/*       problems.                                                    */
/*                                                                    */
/*       One more note.  This function can potentially be             */
/*       re-entered if the call is dropped for any reason while       */
/*       waiting.  If this happens, just drop out and assume the      */
/*       wait has been canceled.  This is signaled by setting         */
/*       bReentered to FALSE when the function is entered and TRUE    */
/*       when it is left.  If bReentered is ever TRUE during the      */
/*       function, then the function was re-entered.                  */
/*                                                                    */
/*       This function times out and returns WAITERR_WAITTIMEDOUT     */
/*       after WAITTIMEOUT milliseconds have elapsed.                 */
/*--------------------------------------------------------------------*/

static long
Tapi_WaitForReply(long lRequestID)
{
   static BOOL     bReentered;

   bReentered = FALSE;

   if (lRequestID > SUCCESS)
   {

      MSG             msg;
      UINT            timer;

      bReplyReceived = FALSE;
      dwRequestedID = (DWORD) lRequestID;

      /* Initializing this just in case there is a bug */
      /* that sets bReplyRecieved without setting the reply value. */
      lAsyncReply = LINEERR_OPERATIONFAILED;

      timer = SetTimer(NULL, 0, WAITTIMEOUT, NULL);

      /* Not recommended to really time out here but its more */
      /* of a fail safe. */
      printmsg(4, "Tapi: waiting for reply");

      while (!bReplyReceived && GetMessage(&msg, NULL, 0, 0) == TRUE)
      {
         if (msg.message == WM_TIMER)
         {
            if (TapiMsg == NULL)
               TapiMsg = "Timed out waiting for Tapi Reply";
            printmsg(2, "Tapi: REPLY timed out");
            KillTimer(NULL, timer);
            return WAITERR_WAITTIMEDOUT;
         }
         TranslateMessage(&msg);
         DispatchMessage(&msg);

         /* This should only occur if the line is */
         /* shut down while waiting. */
         if ((hCall != NULL) && (!bTapiInUse || bReentered))
         {
            bReentered = TRUE;
            return WAITERR_WAITABORTED;
         }
      }
      KillTimer(NULL, timer);

      if (bReplyReceived)
      {
         bReentered = TRUE;         /* we are done, */
         /* hopefully lAyncReply = success */
         printmsg(4, "Tapi: Async Reply = %s", (lAsyncReply == SUCCESS) ? "Success" : "Failed");
         return lAsyncReply;
      }
   }
   bReentered = TRUE;
   return lRequestID;
}

/*--------------------------------------------------------------------*/
/*  FUNCTION: long WaitForCallState(DWORD,TIMEOUT)                    */
/*                                                                    */
/*  PURPOSE: Wait for the line to reach a specific CallState.         */
/*                                                                    */
/*  PARAMETERS:                                                       */
/*    dwDesiredCallState - specific CallState to wait for.            */
/*                                                                    */
/*  RETURN VALUE:                                                     */
/*    Returns 0 (SUCCESS) when we reach the Desired CallState.        */
/*    Returns WAITERR_WAITTIMEDOUT if timed out.                      */
/*    Returns WAITERR_WAITABORTED if call was closed while waiting.   */
/*                                                                    */
/*  COMMENTS:                                                         */
/*                                                                    */
/*       This function allows us to synchronously wait for a line     */
/*       to reach a specific LINESTATE or until the line is shut      */
/*       down.                                                        */
/*                                                                    */
/*       Note that this could cause re-entrancy problems as well      */
/*       as mess with any message preprocessing that might occur      */
/*       on this thread (such as TranslateAccelerator).               */
/*                                                                    */
/*       One more note.  This function can potentially be             */
/*       re-entered if the call is dropped for any reason while       */
/*       waiting.  If this happens, just drop out and assume the      */
/*       wait has been canceled.  This is signaled by setting         */
/*       bReentered to FALSE when the function is entered and TRUE    */
/*       when it is left.  If bReentered is ever TRUE during the      */
/*       function, then the function was re-entered.                  */
/*                                                                    */
/*       This function should to be called from the thread that       */
/*       did lineInitialize, or the PeekMessage is on the wrong       */
/*       thread and the synchronization is not guaranteed to work.    */
/*       Also note that if another PeekMessage loop is entered        */
/*       while waiting, this could also cause synchronization         */
/*       problems.                                                    */
/*                                                                    */
/*       If the constant value I_LINECALLSTATE_ANY is used for the    */
/*       dwDesiredCallState, then WaitForCallState will return        */
/*       SUCCESS upon receiving any CALLSTATE messages.               */
/*--------------------------------------------------------------------*/

static long
WaitForCallState(DWORD dwDesiredCallState, int Timeout)
{
   MSG             msg;
   static BOOL     bReentered = FALSE;
   UINT            timer;

   bReentered = FALSE;

   bCallStateReceived = FALSE;

   timer = SetTimer(NULL, 0, (UINT) Timeout * 1000, NULL);

   while (((dwDesiredCallState == I_LINECALLSTATE_ANY) ||
           (dwCallState != dwDesiredCallState)) &&
            GetMessage(&msg, NULL, 0, 0) == TRUE)
           {

      if (msg.message == WM_CLOSE)
      {
         return WAITERR_WAITABORTED;
      }
      if (msg.message == WM_TIMER)
      {
         bReentered = TRUE;
         KillTimer(NULL, timer);
         printmsg(2, "Tapi: Wait for Connection timed out (%d seconds)", Timeout);
         if (TapiMsg == NULL)
            TapiMsg = "Timeout waiting for Tapi to connect";

         return WAITERR_WAITTIMEDOUT;
      }
      TranslateMessage(&msg);
      DispatchMessage(&msg);
      /* If we are waiting for any call state and get one, succeed. */
      if ((dwDesiredCallState == I_LINECALLSTATE_ANY) &&
          bCallStateReceived)
      {
         break;
      }
      /* This should only occur if the line is shut down */
      /* while waiting. */
      if (!bTapiInUse || bReentered)
      {
         bReentered = TRUE;
         KillTimer(NULL, timer);
         return WAITERR_WAITABORTED;
      }
   }
   KillTimer(NULL, timer);

   bReentered = TRUE;
   return SUCCESS;
}

/*--------------------------------------------------------------------*/
/*                                                                    */
/*  FUNCTION: Tapi_lineCallbackFunc(..)                               */
/*                                                                    */
/*  PURPOSE: Receive asynchronous TAPI events                         */
/*                                                                    */
/*--------------------------------------------------------------------*/

static void CALLBACK
Tapi_lineCallbackFunc(
                      DWORD dwDevice, DWORD dwMsg, DWORD dwCallbackInstance,
                      DWORD dwParam1, DWORD dwParam2, DWORD dwParam3)
{

   printmsg(4, "Tapi: CallBack Msg: %X", dwMsg);
   /* Handle the line messages */
   switch (dwMsg)
   {

      case LINE_LINEDEVSTATE:
         Tapi_HandleLineDevState(dwDevice, dwParam2, dwCallbackInstance,
                                 dwParam3, 0, 0);
         break;

      case LINE_CALLSTATE:
         Tapi_HandleLineCallState(dwDevice, dwMsg, dwCallbackInstance, dwParam1, dwParam2,
                                  dwParam3);
         break;

      case LINE_CLOSE:              /* Someone shut our line down. */
         hLine = NULL;
         hCall = NULL;
         Tapi_Shutdown();           /* all handles invalidated by this time */
         break;

      case LINE_REPLY:
         lAsyncReply = dwParam2;
         bReplyReceived = TRUE;

         if ((long) dwParam2 != SUCCESS)
         {
            TapiMsg = "LINE_REPLY error";
            Tapi_Shutdown();
         }
         break;

      case LINE_CREATE:     /* New LINE device has been added to
                                     * system ie plug n play */
         break;

      default:
         break;
   }
   return;
}

/*--------------------------------------------------------------------*/
/*       Based on unknown bit still being set, could hand off to a    */
/*       higher priority call.  In reality not needed at the          */
/*       moment so not used.                                          */
/*--------------------------------------------------------------------*/

static BOOL
Tapi_Handoff()
{
   LPLINECALLINFO  lpCallInfo;
   LONG            lReturn;
   DWORD           mode;
   BOOL            bHandedOff = FALSE;

   lpCallInfo = (LPLINECALLINFO) LocalAlloc(LPTR, sizeof(LINECALLINFO));

   if (lpCallInfo == 0)
      return FALSE;

   lpCallInfo->dwTotalSize = sizeof(LINECALLINFO);

   lReturn = lineGetCallInfo(hCall, lpCallInfo);

   if (lReturn != SUCCESS)
      return FALSE;

   mode = lpCallInfo->dwMediaMode;

   if (mode & LINEMEDIAMODE_UNKNOWN)
   {
      DWORD           bit;

      mode &= ~LINEMEDIAMODE_UNKNOWN;

      for (bit = 1; bit < LAST_LINEMEDIAMODE; bit = (bit << 1))
      {
         if (bit & mode)
         {
            lReturn = lineHandoff(hCall, NULL, bit);
            switch (lReturn)
            {
               case 0:
                  bHandedOff = TRUE;
                  mode = bit;
                  break;
               case LINEERR_TARGETSELF:
                  mode = bit;
                  break;
               default:
                  break;
            }
         }
      }
      lineSetMediaMode(hCall, mode);
   }
   LocalFree(lpCallInfo);
   return bHandedOff;

} /* Tapi_Handoff */

/*--------------------------------------------------------------------*/
/*  FUNCTION: HandleLineCallState(..)                                 */
/*                                                                    */
/*  PURPOSE: Handle LINE_CALLSTATE asynchronous messages.             */
/*--------------------------------------------------------------------*/

static void
Tapi_HandleLineCallState(
                  DWORD dwDevice, DWORD dwMessage, DWORD dwCallbackInstance,
                         DWORD dwParam1, DWORD dwParam2, DWORD dwParam3)
{

   printmsg(4, "Tapi: CallState: %X", dwParam1);

   if (dwParam3 && dwParam3 == LINECALLPRIVILEGE_MONITOR)       /* we are not the owner */
   {
      return;
   } else if (dwParam3 && dwParam3 == LINECALLPRIVILEGE_OWNER) /* we are the new owner */
   {
      hCall = (HCALL) dwDevice;
      bTapiInUse = TRUE;
   }
   /* Error if this CALLSTATE doesn't apply to our call in progress. */
   else if ((HCALL) dwDevice != hCall)
   {
      return;
   }
   dwCallState = dwParam1;
   bCallStateReceived = TRUE;

   /* dwParam1 is the specific CALLSTATE change that is occurring. */
   switch (dwParam1)
   {

      case LINECALLSTATE_DIALTONE:
         break;

      case LINECALLSTATE_DIALING:
         printmsg(4, "Tapi: Dialing...");
         break;

      case LINECALLSTATE_PROCEEDING:
         break;

      case LINECALLSTATE_RINGBACK:
         break;

      case LINECALLSTATE_BUSY:
         printmsg(2, "Tapi: Line busy");
         Tapi_Shutdown();
         TapiMsg = "Line Busy";
         break;

      case LINECALLSTATE_IDLE:
         printmsg(2, "Tapi: Callstate IDLE");
         Tapi_Shutdown();
         break;

      case LINECALLSTATE_SPECIALINFO:
         Tapi_Shutdown();
         break;

      case LINECALLSTATE_OFFERING:
         printmsg(0, "TAPI: Answering a call...");
         if (hCall)
            Tapi_WaitForReply(lineAnswer(hCall, NULL, 0));
         break;

      case LINECALLSTATE_CONNECTED:/* CONNECTED!!! */
         {
            LPVARSTRING     lpVarString = NULL;
            DWORD           dwSizeofVarString = sizeof(VARSTRING) + 1024;
            long            lReturn;

            /* Very first, make sure this isn't a duplicated message. */
            /* A CALLSTATE message can be sent whenever there is a */
            /* change to the capabilities of a line, meaning that it is */
            /* possible to receive multiple CONNECTED messages per call. */
            /* The CONNECTED CALLSTATE message is the only one in TapiComm */
            /* where it would cause problems if it where sent more */
            /* than once. */

            if (hCommFile != INVALID_HANDLE_VALUE)
               break;

            /* Possbily handoff if not our media type */

            /*
             * if (Tapi_Handoff())     { lineDeallocateCall(hCall); hCall =
             * 0; printmsg(2,"Tapi: Call was handed off"); bTapiInUse = 0;
             * break; }
             */

            /*
             * Get the handle to the comm port from the driver so we can
             * start
             */
            /* communicating.  This is returned in a LPVARSTRING structure. */

            do {
               /* Allocate the VARSTRING structure */
               lpVarString = CheckAndReAllocBuffer((LPVOID) lpVarString,
                                                   dwSizeofVarString);

               if (lpVarString == NULL)
                  goto ErrorConnecting;

               /* Fill the VARSTRING structure */
               lReturn = lineGetID(0, 0, hCall, LINECALLSELECT_CALL,
                                   lpVarString, "comm/datamodem");

               if (!Tapi_HandleLineErr(lReturn))
               {
                  OutputDebugString("lineGetID unhandled error");
                  goto ErrorConnecting;
               }
               /* If the VARSTRING wasn't big enough, loop again. */
               if ((lpVarString->dwNeededSize) > (lpVarString->dwTotalSize))
               {
                  dwSizeofVarString = lpVarString->dwNeededSize;
                  lReturn = -1;     /* Lets loop again. */
               }
            }
            while (lReturn != SUCCESS);

            /* The handle to the comm port is contained in a */
            /* LPVARSTRING structure.  Thus, the handle is the very first */
            /* thing after the end of the structure. */
            hCommFile = *((LPHANDLE) ((LPBYTE) lpVarString +
                                      lpVarString->dwStringOffset));

            printmsg(0, "Tapi: Connection established at %d baud", Tapi_GetCallRate(hCall));
            LocalFree(lpVarString);
            bConnected = TRUE;

            break;

      ErrorConnecting:

            if (hCommFile != INVALID_HANDLE_VALUE)
               CloseHandle(hCommFile);
            hCommFile = INVALID_HANDLE_VALUE;
            Tapi_Shutdown();
            if (lpVarString)
               LocalFree(lpVarString);

            break;
         }

      case LINECALLSTATE_ACCEPTED:
         break;

      case LINECALLSTATE_DISCONNECTED:
         {
            LPSTR           pszReasonDisconnected;

            switch (dwParam2)
            {

               case LINEDISCONNECTMODE_NORMAL:
                  pszReasonDisconnected = "Remote Party Disconnected";
                  break;

               case LINEDISCONNECTMODE_UNKNOWN:
                  pszReasonDisconnected = "Disconnected: Unknown reason";
                  break;

               case LINEDISCONNECTMODE_REJECT:
                  pszReasonDisconnected = "Remote Party rejected call";
                  break;

               case LINEDISCONNECTMODE_PICKUP:
                  pszReasonDisconnected = "Disconnected: Local phone picked up";
                  break;

               case LINEDISCONNECTMODE_FORWARDED:
                  pszReasonDisconnected = "Disconnected: Forwarded";
                  break;

               case LINEDISCONNECTMODE_BUSY:
                  pszReasonDisconnected = "Busy";
                  break;

               case LINEDISCONNECTMODE_NOANSWER:
                  pszReasonDisconnected = "No Answer";
                  break;

               case LINEDISCONNECTMODE_BADADDRESS:
                  pszReasonDisconnected = "Bad Address";
                  break;

               case LINEDISCONNECTMODE_UNREACHABLE:
                  pszReasonDisconnected = "Unreachable";
                  break;

               case LINEDISCONNECTMODE_CONGESTION:
                  pszReasonDisconnected = "Congestion";
                  break;

               case LINEDISCONNECTMODE_INCOMPATIBLE:
                  pszReasonDisconnected = "Incompatible";
                  break;

               case LINEDISCONNECTMODE_UNAVAIL:
                  pszReasonDisconnected = "No Carrier";
                  break;

               case LINEDISCONNECTMODE_NODIALTONE:
                  pszReasonDisconnected = "No Dial Tone";
                  break;

               default:
                  pszReasonDisconnected =
                     "Disconnected: Unknown Reason";
                  break;

            }
            if (TapiMsg == NULL)
               TapiMsg = pszReasonDisconnected;

            printmsg(2, "Tapi: %s", pszReasonDisconnected);

            /*
             * calling Shutdown from 2nd instance of uucico when dialing and
             * failed to connect
             */

            /*
             * caused gpf faults in tapi, so only shudown if connected.
             * During dialing tapi goes back in idle state by its self.
             */
            if (bConnected)
               Tapi_Shutdown();     /* only if connected */
            break;
         }
      default:
         break;
   }

} /* Tapi_HandleLineCallState */

/*--------------------------------------------------------------------*/
/*       FUNCTION: Tapi_HandleLineDevState(..)                        */
/*                                                                    */
/*       PURPOSE: Handle LINE_LINEDEVSTATE asynchronous messages.     */
/*                                                                    */
/*       PARAMETERS:                                                  */
/*         dwDevice  - Line Handle that was closed.                   */
/*         dwMsg     - Should always be LINE_LINEDEVSTATE.            */
/*         dwCallbackInstance - Unused by this sample.                */
/*         dwParam1  - LINEDEVSTATE constant.                         */
/*         dwParam2  - Depends on dwParam1.                           */
/*         dwParam3  - Depends on dwParam1.                           */
/*                                                                    */
/*       RETURN VALUE:                                                */
/*         none                                                       */
/*                                                                    */
/*       COMMENTS:                                                    */
/*                                                                    */
/*       The LINE_LINEDEVSTATE message is received if the state of    */
/*       the line changes.  Examples are RINGING, MAINTENANCE,        */
/*       MSGWAITON.  Very few of these are relevant to this           */
/*       sample.                                                      */
/*                                                                    */
/*       Assuming that any LINEDEVSTATE that removes the line from    */
/*       use by TAPI will also send a LINE_CLOSE message.             */
/*--------------------------------------------------------------------*/

static void
Tapi_HandleLineDevState(
                  DWORD dwDevice, DWORD dwMessage, DWORD dwCallbackInstance,
                        DWORD dwParam1, DWORD dwParam2, DWORD dwParam3)
{

   printmsg(3, "Tapi: LineDevState %X", dwParam1);

   switch (dwParam1)
   {
      case LINEDEVSTATE_RINGING:
         printmsg(5, "Tapi: Line Ringing");
         break;

      case LINEDEVSTATE_REINIT:

/*--------------------------------------------------------------------*/
/*       This is an important case!  Usually means that a service     */
/*       provider has changed in such a way that requires TAPI to     */
/*       REINIT.  Note that there are both 'soft' REINITs and         */
/*       'hard' REINITs.  Soft REINITs don't actually require a       */
/*       full shutdown but is instead just an informational change    */
/*       that historically required a REINIT to force the             */
/*       application to deal with.  TAPI API Version 1.3 apps will    */
/*       still need to do a full REINIT for both hard and soft        */
/*       REINITs.                                                     */
/*--------------------------------------------------------------------*/

         switch (dwParam2)
         {

/*--------------------------------------------------------------------*/
/*       This is the hard REINIT.  No reason given, just REINIT.      */
/*       TAPI is waiting for everyone to shutdown.  Our response      */
/*       is to immediately shutdown any calls, shutdown our use of    */
/*       TAPI and notify the user.                                    */
/*--------------------------------------------------------------------*/

            case 0:
               Tapi_Shutdown();
               break;

            case LINE_CREATE:       /* New line has been added to system -
                                     * ignore */
               break;

            case LINE_LINEDEVSTATE:
               printmsg(4, "Tapi: Soft REINIT");
               Tapi_HandleLineDevState(dwDevice, dwParam2, dwCallbackInstance,
                                       dwParam3, 0, 0);
               break;

               /* There might be other reasons to send a soft reinit. */
               /* No need to to shutdown for these. */
            default:
               /* Ignoring soft REINIT; */
               break;
         }
         break;

      case LINEDEVSTATE_OUTOFSERVICE:
         Tapi_Shutdown();
         break;

      case LINEDEVSTATE_DISCONNECTED:
         if (TapiMsg == NULL)
            TapiMsg = "The line is now Disconnected";
         Tapi_Shutdown();
         break;

      default:
         break;
   }
}

/*--------------------------------------------------------------------*/
/*  FUNCTION: Tapi_HandleLineErr(long)                                */
/*                                                                    */
/*  PURPOSE: Handle several of the standard LINEERR errors            */
/*--------------------------------------------------------------------*/

static BOOL
Tapi_HandleLineErr(long lLineErr)
{
   BOOL            bRet = FALSE;
   char           *ErrMsg = NULL;
   static char     UnknownErr[80];

   /* lLineErr is really an async request ID, not an error. */
   if (lLineErr > SUCCESS)
      return bRet;

   /* All we do is dispatch the correct error handler. */
   switch (lLineErr)
   {

      case SUCCESS:
         bRet = TRUE;
         break;
      case LINEERR_CALLUNAVAIL:
         ErrMsg = "Line is currently unavailable";
         break;

      case LINEERR_INVALCARD:
      case LINEERR_INVALLOCATION:
      case LINEERR_INIFILECORRUPT:
         ErrMsg = "The values in the TELEPHON.INI file are invalid";
         break;

      case LINEERR_NODRIVER:
         ErrMsg = "There is a problem with your Telephony device driver";
         break;

      case LINEERR_REINIT:
         ErrMsg = "Reinit Tapi Error";
         Tapi_Shutdown();
         break;

      case LINEERR_OPERATIONFAILED:
         ErrMsg = "TAPI operation failed";
         break;

      case LINEERR_RESOURCEUNAVAIL:
         ErrMsg = "A TAPI resource is unavailable at this time";
         break;

      case LINEERR_INVALLINEHANDLE:
         ErrMsg = "Invalid handle";
         break;

      case LINEERR_STRUCTURETOOSMALL:
         ErrMsg = "Structure too small";
         break;

      case LINEERR_INVALBEARERMODE:
         ErrMsg = "Invalid BEARER MODE";
         break;

      case LINEERR_INVALADDRESSMODE:
         ErrMsg = "Invalid Addressmode";
         break;
      case LINEERR_INVALMEDIAMODE:
         ErrMsg = "Invalid Media Mode";
         break;

      case LINEERR_INUSE:
         ErrMsg = "Line in use";
         break;

         /* Unhandled errors fail. */
      default:
         sprintf(UnknownErr, "Unexpected TAPI error: %X", lLineErr);
         ErrMsg = UnknownErr;
         break;
   }
   if (TapiMsg == NULL)
   {
      TapiMsg = ErrMsg;             /* in case we are cleaning up and get
                                     * another error dont overwrite. */
   }
   if (ErrMsg != NULL)
      printmsg(4, "Tapi DEBUG: handleError: %s", TapiMsg);
   return bRet;
}

/*--------------------------------------------------------------------*/
/*  FUNCTION: BOOL Tapi_HangupCall()                                  */
/*                                                                    */
/*  PURPOSE: Hangup the call in progress if it exists.                */
/*--------------------------------------------------------------------*/

static BOOL
Tapi_HangupCall()
{
   LPLINECALLSTATUS pLineCallStatus = NULL;

   /* Prevent HangupCall re-entrancy problems. */
   if (bStoppingCall)
      return TRUE;

   bStoppingCall = TRUE;
   bConnected = FALSE;

   /* If there is a call in progress, drop and deallocate it. */
   if (hCall)
   {

      printmsg(3, "Tapi: dropping a call");

      pLineCallStatus = (LPLINECALLSTATUS) LocalAlloc(LPTR, sizeof(LINECALLSTATUS) + 100);
      if (pLineCallStatus == NULL)  /* out of memory */
      {
         return TRUE;
      }
      pLineCallStatus->dwTotalSize = sizeof(LINECALLSTATUS) + 100;

      lineGetCallStatus(hCall, pLineCallStatus);

      /* Only drop the call when the line is not IDLE. */
      if (!((pLineCallStatus->dwCallState) & LINECALLSTATE_IDLE))
      {
         Tapi_WaitForReply(lineDrop(hCall, NULL, 0));
      }
      if (hCommFile != INVALID_HANDLE_VALUE)
         CloseHandle(hCommFile);

      hCommFile = INVALID_HANDLE_VALUE;

      /* The call is now idle.  Deallocate it! */
      Tapi_HandleLineErr(lineDeallocateCall(hCall));
   }
   hCall = NULL;
   bTapiInUse = FALSE;
   bStoppingCall = FALSE;           /* allow HangupCall to be called again. */

   /* Need to free buffer returned from lineGetCallStatus */
   if (pLineCallStatus)
      LocalFree(pLineCallStatus);

   return TRUE;
}

/*--------------------------------------------------------------------*/
/*  FUNCTION: BOOL Tapi_Shutdown()                                    */
/*                                                                    */
/*  PURPOSE: Shuts down all use of TAPI                               */
/*--------------------------------------------------------------------*/

void
Tapi_Shutdown(void)
{
   long            lReturn;

   /* If we aren't initialized, then Shutdown is unnecessary. */
   if (hLineApp == NULL)
      return;

   /* Prevent ShutdownTAPI re-entrancy problems. */
   if (bShuttingDown)
      return;

   bShuttingDown = TRUE;

   Tapi_HangupCall();

   /* if we have a line open, close it. */
   if (hLine)
   {
      lReturn = lineClose(hLine);
   }
   hLine = NULL;

   do {
      lReturn = lineShutdown(hLineApp);
      if (Tapi_HandleLineErr(lReturn))
         continue;
      else
         break;
   }

   while (lReturn != SUCCESS);

   bTapiInUse = FALSE;
   hLineApp = NULL;
   hCall = NULL;
   hLine = NULL;
   bShuttingDown = FALSE;

   printmsg(4, "Tapi: Shutdown complete");
   return;

} /* Tapi_Shutdown */
