#ifndef _SMTPCLIENT_H
#define _SMTPCLIENT_H

/*--------------------------------------------------------------------*/
/*       s m t p c l n t . h                                          */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1997 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: lib.h 1.39 1997/05/11 04:28:53 ahd v1-12s $
 *
 *    Revision history:
 *    $Log: lib.h $
 */

#include "imfile.h"
#include "../uucico/commlib.h"

typedef enum
{
   SM_INVALID     = 0x0001,         /* Invalid client, to be dropped */
   SM_MASTER      = 0x0002,         /* Master (listening) socket     */
   SM_CONNECTED   = 0x0004,         /* Just connected                */
   SM_UNGREETED   = 0x0008,         /* We need a HELO command        */
   SM_IDLE        = 0x0010,         /* Waiting for MAIL command      */
   SM_ADDR        = 0x0020,         /* Have mail, getting addresses  */
   SM_DATA        = 0x0040,         /* Processing message body       */
   SM_PERIOD      = 0x0080,         /* End of data, we're sending    */
   SM_ABORT       = 0x0100,         /* We unexpectedly lost client   */
   SM_TERMINATED  = 0x0200          /* Client sent QUIT command      */
} SMTPMode;

typedef struct _SMTPClient
{
   RemoteConnection *connection;
   char *receiveBuffer;
   char *sender;
   char **address;
   IMFILE *imf;
   SMTPMode mode;

   size_t addressCount;
   size_t receiveBufferCount;
   size_t trivialTransactions;
   size_t receiveBufferLength;
   size_t senderLength;

   time_t ignoreUntilTime;
   time_t lastTransactionTime;
   time_t timeoutPeriod;
   time_t expirationTime;

   long bytesRead;
   long bytesWritten;
   long linesRead;
   long linesWritten;
   KWBoolean ready;
   struct _SMTPClient *next;

} SMTPClient;

KWBoolean isClientValid( const SMTPClient *client );

KWBoolean isClientReady( const SMTPClient *client );

KWBoolean isClientIgnored( const SMTPClient *client );

SMTPClient *initializeClient( RemoteConnection *connection );

void setClientMode(SMTPClient *client, SMTPMode mode );

SMTPMode getClientMode( const SMTPClient *client );

void setClientReady(SMTPClient *client, KWBoolean ready );

KWBoolean getClientReady( const SMTPClient *client );

time_t getClientTimeout( const SMTPClient *client );

int getClientHandle( SMTPClient *client );

void freeClient( SMTPClient *client );

void processClient( SMTPClient *client );

#endif  _SMTPCLIENT_H
