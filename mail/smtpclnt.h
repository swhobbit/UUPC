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
 *    $Id: smtpclnt.h 1.7 1997/11/28 23:13:35 ahd Exp $
 *
 *    Revision history:
 *    $Log: smtpclnt.h $
 *    Revision 1.7  1997/11/28 23:13:35  ahd
 *    Additional auditing, including DNS support
 *
 *    Revision 1.6  1997/11/28 04:52:53  ahd
 *    Initial UUSMTPD OS/2 support
 *
 *    Revision 1.5  1997/11/26 03:34:44  ahd
 *    Correct SMTP timeouts, break out protocol from rest of daemon
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

#include "../uucico/uutcpip.h"
#include "imfile.h"
#include "smtptrns.h"
#include "smtpconn.h"

typedef enum
{
   SM_SAME_MODE      = 0x0000,      /* Don't go to a new mode        */
   SM_DELETE_PENDING = 0x0001,      /* Invalid client, to be dropped */
   SM_MASTER         = 0x0002,      /* Master (listening) socket     */
   SM_CONNECTED      = 0x0004,      /* Just connected, send own name */
   SM_UNGREETED      = 0x0008,      /* We need a HELO command        */
   SM_IDLE           = 0x0010,      /* Waiting for MAIL command      */
   SM_ADDR_FIRST     = 0x0020,      /* Have MAIL, need RCPT          */
   SM_ADDR_SECOND    = 0x0040,      /* Have MAIL, need RCPT or DATA  */
   SM_DATA           = 0x0080,      /* Processing message body       */
   SM_ABORT          = 0x0100,      /* We unexpectedly lost client   */
   SM_TIMEOUT        = 0x0200,      /* Client idle too long          */
   SM_EXITING        = 0x0400       /* Server is shutting down       */
};

typedef unsigned long SMTPMode;

#define SMTP_MODES_ALL        0xffff
#define SMTP_MODES_NONE       0x0000
#define SMTP_MODES_AFTER_HELO (SMTP_MODES_ALL - \
                               (SM_CONNECTED |  \
                                SM_UNGREETED |  \
                                SM_EXITING))

typedef struct _SMTPBuffer
{
   char    *data;
   size_t  used;                    /* Valid data byes in data       */
   size_t  parsed;                  /* Valid data bytes processed    */
   size_t  length;                  /* Total buffer length of data   */
   size_t  bytesTransferred;        /* Bytes via network conn        */
   size_t  linesTransferred;        /* CR/LF delimited lines via net */
} SMTPBuffer;

typedef struct _SMTPClient
{
   SMTPConnection connection;       /* Internal network information  */
   SMTPBuffer receive;
   SMTPBuffer transmit;
   SMTPTransaction *transaction;    /* Actual transaction client owns*/
   SMTPMode mode;

   char *SMTPName;                  /* Name client *claims* to be    */

   size_t trivialTransactions;
   size_t majorTransactions;        /* Major commands processed      */
   size_t stalledReads;             /* Number of times read stalled  */

   time_t ignoreUntilTime;
   time_t lastTransactionTime;
   time_t connectTime;
   time_t timeoutPeriod;
   time_t terminationTime;

   size_t sequence;
   KWBoolean ready;                 /* Socket ready for read/accept  */
   KWBoolean process;               /* Client should be processed    */
   KWBoolean endOfTransmission;
   KWBoolean esmtp;                 /* SMTP specific, but not
                                       transaction specific          */
   struct _SMTPClient *next;
   struct _SMTPClient *previous;

} SMTPClient;

/* Initialize */
SMTPClient *initializeClient(SOCKET socket, KWBoolean needAccept);
SMTPClient *initializeMaster(const char *port, time_t exitTime);

/* Clean up */
void freeClient(SMTPClient *client);

/* Boolean queries, usually of derived values */
KWBoolean isClientValid(const SMTPClient *client);
KWBoolean isClientIgnored(const SMTPClient *client);
KWBoolean isClientEOF(const SMTPClient *client);
KWBoolean isClientTimedOut(const SMTPClient *client);

/* Termination time (set only by initialization, so no method) */
time_t getClientTerminationTime(const SMTPClient *client);

/* Close client socket (opened in initialize functions */
void setClientClosed(SMTPClient *client);

/* Alter network socket id, used only when opening/closing it  */
SOCKET getClientHandle(const SMTPClient *client);
void setClientHandle(SMTPClient *client, SOCKET handle);

/* Mode (state) flag */
void setClientMode(SMTPClient *client, SMTPMode mode);
SMTPMode getClientMode(const SMTPClient *client);

/* Client network socket ready for processing */
void setClientReady(SMTPClient *client, KWBoolean ready);
KWBoolean getClientReady(const SMTPClient *client);

/* Client genericly ready for processing (explicitly set true
   by high level functions if if socket is ready) */
void setClientProcess(SMTPClient *client, KWBoolean process);
KWBoolean getClientProcess(const SMTPClient *client);

/* Return count of bytes already in network input buffer */
KWBoolean getClientBufferedData(const SMTPClient *client);

/* Set client to not be flagged ready for delay seconds from present */
void setClientIgnore(SMTPClient *client, time_t delay);

/* Report timeout based on various flags, including mode and ignore */
time_t getClientTimeout(const SMTPClient *client);

/* Get unique sequence number for this client, used in debug msgs */
size_t getClientSequence(const SMTPClient *client);

/* Major tranaction (mail) counter */
void incrementClientMajorTransaction(SMTPClient *client );
size_t getClientMajorTransaction(SMTPClient *client);

/* Read data if needed and call command processor for client */
void processClient(SMTPClient *client);

/* Transaction Counter to detect Denial of Sevice Attacks */
void incrementClientTrivialCount(SMTPClient *client);
size_t getClientTrivialCount(const SMTPClient *client);

/* Transmitted lines counters */
void incrementClientLinesWritten(SMTPClient *client);
size_t getClientLinesWritten(SMTPClient *client);

/* Transmitted bytes counters */
void incrementClientBytesWritten(SMTPClient *client, size_t count);
size_t getClientBytesWritten(SMTPClient *client);

/* Received lines counters */
void incrementClientLinesRead(SMTPClient *client);
size_t getClientLinesRead(SMTPClient *client);

/* Received bytes counters */
void incrementClientBytesRead(SMTPClient *client, size_t count);
size_t getClientBytesRead(SMTPClient *client);

#endif  /* _SMTPCLIENT_H */
