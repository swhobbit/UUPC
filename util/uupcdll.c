/*--------------------------------------------------------------------*/
/*    u u p c d l l . c                                               */
/*                                                                    */
/*    A DLL for calling UUPC functions externally under Windows NT    */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) David M. Watt 1994, All Rights Reserved   */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-2002 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *       $Id: uupcdll.c 1.14 2001/03/12 13:54:49 ahd v1-13k $
 *       $Log: uupcdll.c $
 *       Revision 1.14  2001/03/12 13:54:49  ahd
 *       Annual copyright update
 *
 *       Revision 1.13  2000/05/12 12:32:00  ahd
 *       Annual copyright update
 *
 *       Revision 1.12  1999/01/08 02:20:52  ahd
 *       Convert currentfile() to RCSID()
 *
 *       Revision 1.11  1999/01/04 03:53:30  ahd
 *       Annual copyright change
 *
 *       Revision 1.10  1998/03/01 01:36:42  ahd
 *       Annual Copyright Update
 *
 *       Revision 1.9  1997/04/24 01:31:27  ahd
 *       Annual Copyright Update
 *
 *       Revision 1.8  1996/01/01 21:18:14  ahd
 *       Annual Copyright Update
 *
 *       Revision 1.7  1995/08/27 23:34:11  ahd
 *       Correct compile errors
 *
 *       Revision 1.6  1995/03/11 22:28:22  ahd
 *       Use macro for file delete to allow special OS/2 processing
 *
 *       Revision 1.5  1995/01/07 16:37:24  ahd
 *       Change boolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *       Revision 1.4  1995/01/07 16:22:53  ahd
 *       Change KWBoolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *       Revision 1.3  1994/12/22 00:31:20  ahd
 *       Annual Copyright Update
 *
 *       Revision 1.2  1994/05/30 02:20:00  ahd
 *       Add missing definition for Dllmain
 *
 * Revision 1.1  1994/05/23  21:42:10  dmwatt
 * Initial revision
 *
 *
 */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"
#include "hlib.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>

/*--------------------------------------------------------------------*/
/*                      Windows NT include files                      */
/*--------------------------------------------------------------------*/

#include <windows.h>

RCSID("$Id: uupcdll.c 1.14 2001/03/12 13:54:49 ahd v1-13k $");

/*--------------------------------------------------------------------*/
/*                      Global #defines                               */
/*--------------------------------------------------------------------*/

#define DllExport __declspec( dllexport )
#define UUPCHIVE "Software\\Kendra Electronic Wonderworks\\UUPC/extended"
#define UUPCSYSRC "UUPCSYSRC"
#define UUPCUSRRC "UUPCUSRRC"
#define TEMPLATENAME "UXXXXXX"

/*--------------------------------------------------------------------*/
/*      Function declarations and prototypes                          */
/*--------------------------------------------------------------------*/

// BOOL WINAPI DllMain(HANDLE hInst, ULONG reason, LPVOID reserved);
DllExport BOOL WINAPI DllMain(HANDLE hInst, ULONG reason, LPVOID reserved);
DllExport BOOL UUPCInit(void);
DllExport BOOL UUPCGetParm(char *parmName, char *buf, int len);
DllExport BOOL UUPCGetNewsSpoolSize(const char *system, long *count, long *bytes);
DllExport BOOL UUPCSendMail(char *message);

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

CRITICAL_SECTION UUPCCritSec;
static char tempDir[FILENAME_MAX];  /* Temp file name */
static BOOL initialized = KWFalse;

DllExport BOOL WINAPI DllMain(HANDLE hInst, ULONG reason, LPVOID reserved)
{
    switch (reason)
    {
    case DLL_PROCESS_ATTACH:
        InitializeCriticalSection(&UUPCCritSec);
    break;

    case DLL_THREAD_ATTACH:

    break;

    case DLL_THREAD_DETACH:

    break;

    case DLL_PROCESS_DETACH:
        DeleteCriticalSection(&UUPCCritSec);

    break;
    }

    return KWTrue;
}

DllExport BOOL UUPCInit()
{
    BOOL result;
    char *envptr;
    char *ptr;
    int buflen;         /* Length of the tempname buffer, sometimes */
    HKEY hSystemKey = INVALID_HANDLE_VALUE;
    HKEY hUserKey = INVALID_HANDLE_VALUE;
    char tempFileName[FILENAME_MAX];

/*
    Perform some checks to see if UUPC/extended is really around:
    1) Check to see if the registry hive is there
    2) Check to see if rmail is on the path

    If those are both true, then initialize tempDir and return KWTrue.
    Otherwise, return KWFalse.
*/

/*
    1) Check for the registry hives
*/

    result = RegOpenKeyEx(HKEY_CURRENT_USER, UUPCHIVE, 0, KEY_ALL_ACCESS, &hUserKey);

    if (result != ERROR_SUCCESS)
        result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, UUPCHIVE, 0, KEY_ALL_ACCESS, &hSystemKey);

    if (result != ERROR_SUCCESS)
        return KWFalse;

    if (hUserKey != INVALID_HANDLE_VALUE)
        RegCloseKey(hUserKey);

    if (hSystemKey != INVALID_HANDLE_VALUE)
        RegCloseKey(hSystemKey);

/*
    2) Check for RMAIL on the path
*/

    result = SearchPath(NULL, "RMAIL.EXE", NULL, FILENAME_MAX, tempFileName, &ptr);

    if (result == 0)
        return KWFalse;

    /* Construct initial tempName here */

    result = UUPCGetParm("TempDir", tempDir, FILENAME_MAX);
    if (!result) {
        envptr = getenv("TMP");
        if (!envptr)
            envptr = getenv("TEMP");

        if (!envptr)
            strcpy(tempDir, "C:");          /* Try something, anything */
        else
            strcpy(tempDir, envptr);
    }

    /* Convert forward to back slashes */
    denormalize(tempDir);

    buflen = strlen(tempDir);

    if (tempDir[buflen - 1] == '\\')        /* Strip off trailing slashes, if any */
        tempDir[buflen - 1] = '\0';

    strcat(tempDir, "\\");                  /* Then add one of my own */

    initialized = KWTrue;

    return KWTrue;
}

BOOL UUPCGetSysRCHive(PHKEY hSysKey)
{
    LONG result;
    HKEY hSystemKey = INVALID_HANDLE_VALUE;
    HKEY hUserKey = INVALID_HANDLE_VALUE;
    HKEY hResultKey = INVALID_HANDLE_VALUE;
    char keyValue[BUFSIZ];
    DWORD keySize = BUFSIZ;
    DWORD keyType;

    result = RegOpenKeyEx(HKEY_CURRENT_USER, UUPCHIVE, 0, KEY_ALL_ACCESS, &hUserKey);

    if (result == ERROR_SUCCESS) {
        result = RegQueryValueEx(hUserKey, UUPCSYSRC, 0, &keyType, keyValue, &keySize);
        if (result == ERROR_SUCCESS) {
            result = RegOpenKeyEx(hUserKey, keyValue, 0, KEY_ALL_ACCESS, &hResultKey);
            if (result == ERROR_SUCCESS) {
                RegCloseKey(hUserKey);
                *hSysKey = hResultKey;
                return KWTrue;
            }
        }
    }

    if (hUserKey != INVALID_HANDLE_VALUE)
        RegCloseKey(hUserKey);

    result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, UUPCHIVE, 0, KEY_ALL_ACCESS, &hSystemKey);

    if (result == ERROR_SUCCESS) {
        result = RegQueryValueEx(hSystemKey, UUPCSYSRC, NULL, &keyType, keyValue, &keySize);
        if (result == ERROR_SUCCESS) {
            result = RegOpenKeyEx(hSystemKey, keyValue, 0, KEY_ALL_ACCESS, &hResultKey);
            if (result == ERROR_SUCCESS) {
                RegCloseKey(hSystemKey);
                *hSysKey = hResultKey;
                return KWTrue;
            }
        }
    }

    if (hSystemKey != INVALID_HANDLE_VALUE)
        RegCloseKey(hSystemKey);

    return KWFalse;
}

DllExport BOOL UUPCGetParm(char *parmName, char *resultBuf, int bufSize)
{
    LONG result;
    HKEY sysKey;
    DWORD type;

    if (!initialized)
        return KWFalse;

    if (!UUPCGetSysRCHive(&sysKey))
      return KWFalse;

    result = RegQueryValueEx(sysKey, parmName, NULL, &type, resultBuf, &bufSize);

#if defined(_DEBUG)
    printf("Size returned from RegQueryValueEx is %d\n", bufSize);
#endif

    RegCloseKey(sysKey);

    resultBuf[bufSize] = '\0';  /* Paranoia:  put in the '\0' at the end of the string */
    if (result != ERROR_SUCCESS)
        return KWFalse;
    return KWTrue;
}

DllExport BOOL UUPCGetNewsSpoolSize(const char *system, long *count, long *bytes)
{
    char spoolDirBuf[BUFSIZ];
    char nameBuf[BUFSIZ];
    char *str;
    WIN32_FIND_DATA fData;
    HANDLE searchHandle = INVALID_HANDLE_VALUE;
    LONG totalSize = 0;
    LONG fileCount = 0;

    if (!initialized)
        return KWFalse;

    if (!UUPCGetParm("SpoolDir", spoolDirBuf, BUFSIZ))
        return KWFalse;

    /*  Convert to backslashes  */

    denormalize(spoolDirBuf);

    sprintf(nameBuf, "%s\\%.8s\\D\\*.*", spoolDirBuf, system);

    searchHandle = FindFirstFile(nameBuf, &fData);

    if (searchHandle == INVALID_HANDLE_VALUE)
        return KWFalse;

    do {
        if (!(fData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            fileCount++;
            totalSize += fData.nFileSizeLow;
        }
    } while (FindNextFile(searchHandle, &fData));

    *count = fileCount;
    *bytes = totalSize;
    return KWTrue;
}

BOOL UUPCCheckMessage(char *message)
{
    char *headerEnd;
    BOOL gotTo = KWFalse, gotFrom = KWFalse;

    headerEnd = strstr(message, "\n\n");

    if (!headerEnd)
        return KWFalse;

    *headerEnd = '\0';

    if (strstr(message, "\nTo:") || (strncmp(message, "To:", 3) == 0))
        gotTo = KWTrue;

    if (strstr(message, "\nFrom:") || (strncmp(message, "From:", 5) == 0))
        gotFrom = KWTrue;

    *headerEnd = '\n';

    if (gotTo && gotFrom)
        return KWTrue;

    return KWFalse;
}

/*

The UUPCSendMail function blithely assumes that the program calling it
has created a message that contains a To: line and a From: line (it can
have a subject, or other fields, if you want), followed by a couple of
\n\n's, followed by the message.  Anything else will cause rmail to
barf.  It does some checking, but it's only cursory.  Caveat user.

*/

DllExport BOOL UUPCSendMail(char *message)
{
    HANDLE hFile;       /* Temp file for mail message handle */
    LONG msgLen;
    BOOL result;
    DWORD written;
    char tempFileName[FILENAME_MAX];
    char templateName[32];
    char cmdLine[BUFSIZ];
    STARTUPINFO si;     /* Used with CreateProcess for rmail */
    PROCESS_INFORMATION pi;
    char *ptr;          /* Pointer to environment strings, and temp ptr */

    if (!initialized)
        return KWFalse;

    if (!UUPCCheckMessage(message))
        return KWFalse;

    msgLen = strlen(message);

/* Need to use a critical section here because _mktemp() isn't thread safe */

    EnterCriticalSection(&UUPCCritSec);

    strcpy(templateName, TEMPLATENAME);

    ptr = _mktemp(templateName);

    if (!ptr) {
        LeaveCriticalSection(&UUPCCritSec);
        return KWFalse;
    }

    strcpy(tempFileName, tempDir);
    strcat(tempFileName, ptr);

    hFile = CreateFile(tempFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL, NULL);

    LeaveCriticalSection(&UUPCCritSec);

    if (hFile == INVALID_HANDLE_VALUE) {
        return KWFalse;
    }

    result = WriteFile(hFile, message, msgLen, &written, NULL);

    if (!result) {
        CloseHandle(hFile);
        LeaveCriticalSection(&UUPCCritSec);
        return KWFalse;
    }

    CloseHandle(hFile);

    sprintf(cmdLine, "rmail.exe -t -f %s", tempFileName);

    memset(&si, 0, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    si.lpReserved = NULL;
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_MINIMIZE;
    si.lpTitle = "UUPC/Extended RMail";

    result = CreateProcess(NULL, cmdLine, NULL, NULL, KWFalse, DETACHED_PROCESS,
        NULL, NULL, &si, &pi);

    if (!result) {
        _unlink(tempFileName);
        return KWFalse;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    GetExitCodeProcess(pi.hProcess, &result);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    _unlink(tempFileName);

    if (result != 0)
        return KWFalse;

    return KWTrue;
}

BOOL UUPCSendNews(const char *remoteSystem, const char *newsFileName, const char *compressCmd)
{

    if (!initialized)
        return KWFalse;

    return KWFalse;
}
