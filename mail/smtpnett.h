#ifndef _SMTPNETT_H
#define _SMTPNETT_H

/*--------------------------------------------------------------------*/
/*       s m t p n e t t . h                                          */
/*                                                                    */
/*       TCP/IP specific support for UUPC/extended SMTP               */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-2001 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: smtpnett.h 1.2 2000/05/12 12:42:31 ahd v1-13g $
 *
 *    $Log: smtpnett.h $
 *    Revision 1.2  2000/05/12 12:42:31  ahd
 *    Annual copyright update
 *
 *    Revision 1.1  1999/02/21 04:11:15  ahd
 *    Initial revision
 *
 */

#include "smtpclnt.h"
#include "../uucico/uutcpip.h"

/*--------------------------------------------------------------------*/
/*                             Prototypes                             */
/*--------------------------------------------------------------------*/

size_t
SMTPRead(SMTPClient *client);

size_t
SMTPWrite(SMTPClient *client,
          const char UUFAR *data,
          unsigned int len);

KWBoolean
InitializeNetwork(void);

SOCKET
openMaster(const char *name);

SOCKET
openSlave(SOCKET pollingSock);

time_t
getModeTimeout( SMTPMode );

void closeSocket( SOCKET handle );

SOCKET
getDefaultHandle( void );

#define MAX_BUFFER_SIZE (1024 * 64)

#endif /* _SMTPNETT_H */
