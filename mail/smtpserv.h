#ifndef _SMTPSERV_H
#define _SMTPSERV_H
/*--------------------------------------------------------------------*/
/*       s m t p s e r v . h                                          */
/*                                                                    */
/*       SMTP server support routines for clients                     */
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
 *    $Id: uusmtpd.c 1.1 1997/05/20 03:55:46 ahd v1-12s $
 *
 *    $Log$
 */

#include "smtpclnt.h"

SMTPClient *initializeMaster( const char *port, time_t exitTime );

RemoteConnection *acceptClient( );

RemoteConnection *acceptHotClient( int handle );

KWBoolean flagReadySockets( SMTPClient *master );

KWBoolean processReadySockets( SMTPClient *first );

void addClient( SMTPClient *master, SMTPClient *client );

void dropTerminatedClients( SMTPClient *master );

void dropAllClients( SMTPClient *master );

time_t getClientPeriod( SMTPClient *client );

#endif  /* _SMTPSERV_H */
