#ifndef _SMTPNETW_H
#define _SMTPNETW_H

/*--------------------------------------------------------------------*/
/*       s m t p n e t w . h                                          */
/*                                                                    */
/*       SMTP verb parser for UUPC/extended                           */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1998 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: SMTPNETW.H 1.4 1998/03/01 01:28:19 ahd v1-12v $
 *
 *    $Log: SMTPNETW.H $
 *    Revision 1.4  1998/03/01 01:28:19  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.3  1997/11/29 13:06:52  ahd
 *    Allow explicit invocation of network initialization
 *
 *    Revision 1.2  1997/11/24 02:53:26  ahd
 *    First working SMTP daemon which delivers mail
 *
 *    Revision 1.1  1997/11/21 18:16:32  ahd
 *    Command processing stub SMTP daemon
 *
 */

#include "smtpclnt.h"

KWBoolean
InitWinsock(void);

KWBoolean
SMTPGetLine( SMTPClient *client );

KWBoolean
SMTPResponse( SMTPClient *client, int code, const char *text );

SOCKET
openMaster(const char *name );

SOCKET
openSlave(SOCKET pollingSock);

time_t
getModeTimeout( SMTPMode );

void closeSocket( SOCKET handle );

KWBoolean
selectReadySockets( SMTPClient *master );

#define XMIT_LENGTH  512

#endif /* _SMTPNETW_H */
