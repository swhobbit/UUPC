#ifndef _SMTPSERV_H
#define _SMTPSERV_H
/*--------------------------------------------------------------------*/
/*       s m t p s e r v . h                                          */
/*                                                                    */
/*       SMTP server support routines for clients                     */
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
 *    $Id: smtpserv.h 1.8 2000/05/12 12:44:47 ahd v1-13g $
 *
 *    $Log: smtpserv.h $
 *    Revision 1.8  2000/05/12 12:44:47  ahd
 *    Annual copyright update
 *
 *    Revision 1.7  1999/01/04 03:56:09  ahd
 *    Annual copyright change
 *
 *    Revision 1.6  1998/03/08 23:12:28  ahd
 *    Better UUXQT support
 *
 *    Revision 1.5  1998/03/01 01:28:11  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.4  1997/11/25 05:05:36  ahd
 *    More robust SMTP daemon
 *
 *    Revision 1.3  1997/11/24 02:53:26  ahd
 *    First working SMTP daemon which delivers mail
 *
 *    Revision 1.2  1997/11/21 18:16:32  ahd
 *    Command processing stub SMTP daemon
 *
 *    Revision 1.1  1997/06/03 03:26:38  ahd
 *    Initial revision
 *
 */

#include "smtpclnt.h"

KWBoolean
flagReadyClientList( SMTPClient *master );

KWBoolean
processReadyClientList( SMTPClient *current );

void
timeoutClientList( SMTPClient *current );

void dropTerminatedClientList( SMTPClient *master, KWBoolean runUUXQT);

void dropAllClientList( SMTPClient *master, KWBoolean runUUXQT);

void executeQueue( void );

#endif  /* _SMTPSERV_H */
