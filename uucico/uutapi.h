#ifndef _UUTAPI_H
#define _UUTAPI_H

/*--------------------------------------------------------------------*/
/*       u u t a p i . h                                              */
/*                                                                    */
/*       Define Win95 tapi funtions used in TAPI.C                    */
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
 *       $Id: uutapi.h 1.1 1998/04/20 02:48:54 ahd v1-13f ahd $
 *
 *       $Log: uutapi.h $
 *       Revision 1.1  1998/04/20 02:48:54  ahd
 *       Initial revision
 *
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
