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
 *    $Id: smtpclnt.h 1.3 1997/11/24 02:53:26 ahd Exp $
 *
 *    Revision history:
 *    $Log: smtpclnt.h $
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

#include "../uucico/uutcpip.h"
#include "imfile.h"

typedef enum
{
   SM_SAME_MODE   = 0x0000,         /* Don't go to a new mode        */
   SM_INVALID     = 0x0001,         /* Invalid client, to be dropped */
   SM_MASTER      = 0x0002,         /* Master (listening) socket     */
   SM_CONNECTED   = 0x0004,         /* Just connected, send own name */
   SM_UNGREETED   = 0x0008,         /* We need a HELO command        */
   SM_IDLE        = 0x0010,         /* Waiting for MAIL command      */
   SM_ADDR_FIRST  = 0x0020,         /* Have MAIL, need RCPT          */
   SM_ADDR_SECOND = 0x0040,         /* Have MAIL, need RCPT or DATA  */
   SM_DATA        = 0x0080,         /* Processing message body       */
   SM_ABORT       = 0x0100,         /* We unexpectedly lost client   */
   SM_TIMEOUT     = 0x0200,         /* Client idle too long          */
   SM_EXITING     = 0x0400          /* Server is shutting down       */
} SMTPMode;

#define SMTP_MODES_ALL        0xffff
#define SMTP_MODES_NONE       0x0000
#define SMTP_MODES_AFTER_HELO (SMTP_MODES_ALL - \
                               (SM_CONNECTED |  \
                                SM_UNGREETED |  \
                                SM_EXITING))

typedef struct _SMTPBuffer
{
   char *data;
   int  used;                       /* Valid data byes in data       */
   int  parsed;                     /* Valid data bytes processed    */
   int  length;                     /* Total buffer length of data   */
   int  bytesTransferred;           /* Bytes via network conn        */
   int  linesTransferred;           /* CR/LF delimited lines via net */
} SMTPBuffer;

typedef struct _SMTPClient
{
   int handle;                      /* TCP/IP socket handle          */
   SMTPBuffer receive;
   SMTPBuffer transmit;
   SMTPMode mode;

   char *SMTPName;                  /* Name client *claims* to be    */
   char *TrueName;                  /* Name DNS reports client to be */
   char *sender;                    /* RFC-822 address of sender     */
   char **address;                  /* List of addresses             */
   IMFILE *imf;

   size_t addressLength;            /* Size of address array         */
   size_t addressCount;             /* Number entries in array used  */
   size_t trivialTransactions;
   size_t stalledReads;             /* Number of times read stalled  */

   time_t ignoreUntilTime;
   time_t lastTransactionTime;
   time_t connectTime;
   time_t timeoutPeriod;

   long sequence;
   KWBoolean ready;                 /* Socket ready for read/accept  */
   KWBoolean process;               /* Client should be processed    */
   KWBoolean endOfTransmission;
   KWBoolean esmtp;
   struct _SMTPClient *next;
   struct _SMTPClient *previous;

} SMTPClient;

KWBoolean isClientValid( const SMTPClient *client );

KWBoolean isClientIgnored( const SMTPClient *client );

KWBoolean isClientEOF( const SMTPClient *client );

KWBoolean isClientTimedOut( const SMTPClient *client );

SMTPClient *initializeClient( SOCKET socket, KWBoolean needAccept );

SMTPClient *initializeMaster( const char *port, time_t exitTime );

void setClientMode(SMTPClient *client, SMTPMode mode );

SMTPMode getClientMode( const SMTPClient *client );

void setClientReady(SMTPClient *client, KWBoolean ready );

void setClientProcess(SMTPClient *client, KWBoolean process );

KWBoolean getClientReady( const SMTPClient *client );

KWBoolean getClientProcess( const SMTPClient *client );

time_t getClientTimeout( const SMTPClient *client );

void
setClientIgnore( SMTPClient *client, time_t delay );

SOCKET getClientHandle( const SMTPClient *client );

void setClientHandle( SMTPClient *client, SOCKET handle );

int getClientSequence( const SMTPClient *client );

void freeClient( SMTPClient *client );

void processClient( SMTPClient *client );

void setClientClosed( SMTPClient *client );

KWBoolean getClientBufferedData( const SMTPClient *client );

void incrementClientTrivialCount( SMTPClient *client );

int getClientTrivialCount( const SMTPClient *client );

/*--------------------------------------------------------------------*/
/*                     Transmitted data counters                      */
/*--------------------------------------------------------------------*/

void incrementClientLinesWritten( SMTPClient *client );

size_t getClientLinesWritten( SMTPClient *client );

void incrementClientBytesWritten( SMTPClient *client,
                               size_t count );

size_t getClientBytesWritten( SMTPClient *client );

/*--------------------------------------------------------------------*/
/*                       Received data counters                       */
/*--------------------------------------------------------------------*/

void incrementClientLinesRead( SMTPClient *client );

size_t getClientLinesRead( SMTPClient *client );

void incrementClientBytesRead( SMTPClient *client,
                               size_t count );

size_t getClientBytesRead( SMTPClient *client );

void
cleanupClientMail( SMTPClient *client );

#endif  /* _SMTPCLIENT_H */
