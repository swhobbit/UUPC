#ifndef _UUTAPI_H
#define _UUTAPI_H

/*--------------------------------------------------------------------*/
/*       u u t a p i . h                                              */
/*                                                                    */
/*       Define Win95 tapi funtions used in TAPI.C                    */
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
 *       $Id: LIB.H 1.42 1998/03/01 01:26:54 ahd v1-12v $
 *
 *       $Log$
 */

#ifndef INVALID_HANDLE_VALUE
#define INVALID_HANDLE_VALUE -1
#endif

#include <windef.h>

HANDLE          Tapi_AnswerCall(char *szDeviceName, UINT interval, long *TapiShutdown);
void            Tapi_Shutdown(void);
BOOL            Tapi_Init(char *szDeviceName);
HANDLE          Tapi_DialCall(char *szDeviceName, char *szPhoneNumber, int Timeout);
BOOL            Tapi_MsgCheck();

int Tapi_HangupCall();
int Tapi_MsgCheck();
int Tapi_GetCallRate();

extern char *TapiMsg;
extern char TapiLineName[150];

#endif  /* _UUTAPI_H */
