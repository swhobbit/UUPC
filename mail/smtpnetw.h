#ifndef _SMTPNETW_H
#define _SMTPNETW_H

/*--------------------------------------------------------------------*/
/*       s m t p n e t w . h                                          */
/*                                                                    */
/*       SMTP verb parser for UUPC/extended                           */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1997 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: smtpverb.c 1.1 1997/06/03 03:25:31 ahd Exp $
 *
 *    $Log$
 */

#include "smtpclnt.h"

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

#endif /* _SMTPNETW_H */
