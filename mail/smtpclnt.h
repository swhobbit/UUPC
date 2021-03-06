#ifndef _SMTPCLIENT_H
#define _SMTPCLIENT_H

/*--------------------------------------------------------------------*/
/*       s m t p c l n t . h                                          */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-2002 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: smtpclnt.h 1.20 2001/03/12 13:51:54 ahd v1-13k $
 *
 *    Revision history:
 *    $Log: smtpclnt.h $
 *    Revision 1.20  2001/03/12 13:51:54  ahd
 *    Annual copyright update
 *
 *    Revision 1.19  2000/05/25 03:43:59  ahd
 *    More conservative buffering to avoid aborts
 *
 *    Revision 1.18  2000/05/12 12:44:47  ahd
 *    Annual copyright update
 *
 *    Revision 1.17  1999/01/17 17:21:32  ahd
 *    Add test for one socket being ready
 *
 *    Revision 1.16  1999/01/04 03:56:09  ahd
 *    Annual copyright change
 *
 *    Revision 1.15  1998/11/04 02:01:17  ahd
 *    Add sanity check field to linked lists
 *    Add assertion macros
 *
 * Revision 1.14  1998/04/24  03:34:25  ahd
 * Use local buffers for output
 * Add flag bits to verb table
 * Add flag bytes to client structure
 *
 *    Revision 1.13  1998/04/08 11:36:31  ahd
 *    Alter socket error processing
 *
 *    Revision 1.12  1998/03/08 23:12:28  ahd
 *    Better supportfor UUXQT
 *
 *    Revision 1.11  1998/03/08 04:52:02  ahd
 *    Allowing setting per client timeout explicitly
 *
 *    Revision 1.10  1998/03/03 03:55:05  ahd
 *    Routines to handle messages within a POP3 mailbox
 *
 *    Revision 1.9  1998/03/01 19:43:52  ahd
 *    First compiling POP3 server which accepts user id/password
 *
 *    Revision 1.8  1997/11/29 13:06:52  ahd
 *    Correct compiler warnings under OS/2, copyright notice
 *
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
#include "smtpconn.h"

#ifndef SMTPTransaction
#include "smtptrns.h"
#endif


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
   SM_EXITING        = 0x0400,      /* Server is shutting down       */

   SM_PASSWORD       = 0x1000,      /* POP3 read password            */
   SM_LOAD_MBOX      = 0x2000,      /* POP3 load mailbox             */
   SM_SEND_DATA      = 0x4000       /* POP3 send data to client      */
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
   char    *NetworkBuffer;          /* Memory used by network I/O    */
   char    *DataBuffer;             /* Contents of one line          */

   size_t  NetworkUsed;             /* Valid data bytes in data      */
   size_t  DataUsed;                /* Length of current data line   */

   size_t  NetworkAllocated;        /* Total buffer length of data   */
   size_t  DataAllocated;           /* Total buffer length of data   */

   size_t  bytesTransferred;        /* Bytes via network conn        */
   size_t  linesTransferred;        /* CR/LF delimited lines via net */
} SMTPBuffer;

/*--------------------------------------------------------------------*/
/*                          Client flag bits                          */
/*--------------------------------------------------------------------*/

#define  SF_ALL_FLAG       0xffff   /* Used for clearing flags       */

#define  SF_NO_READ        0x0001   /* Do not read before proc       */
#define  SF_NO_TOKENIZE    0x0008   /* Do not tokenize next line     */

/*--------------------------------------------------------------------*/
/*       S M T P C l i e n t                                          */
/*                                                                    */
/*       The base structure for all client processing                 */
/*--------------------------------------------------------------------*/

typedef struct _SMTPClient
{
   long magic;                      /* Internal consistent check     */
   SMTPConnection connection;       /* Internal network information  */
   SMTPBuffer transmit;
   SMTPBuffer receive;
   SMTPTransaction *transaction;    /* Actual transaction client owns*/
   SMTPMode mode;
   unsigned long flag;

   char *clientName;                /* Name client *claims* to be    */

   size_t trivialTransactions;
   size_t majorTransactions;        /* Major commands processed      */
   size_t stalledReads;             /* Number of times read stalled  */

   time_t ignoreUntilTime;
   time_t lastTransactionTime;
   time_t connectTime;
   time_t timeoutPeriod;
   time_t terminationTime;
   time_t timeout;                  /* Overrides default mode timeout */

   size_t sequence;
   KWBoolean ready;                 /* Socket ready for read/accept  */
   KWBoolean process;               /* Client should be processed    */
   KWBoolean endOfTransmission;
   KWBoolean esmtp;                 /* SMTP specific, but not
                                       transaction specific          */
   KWBoolean listening;             /* Master client, only accepts
                                       connections                   */
   KWBoolean needQueueRun;          /* We have work to process       */
   struct _SMTPClient *next;
   struct _SMTPClient *previous;

} SMTPClient;

#define SMTPC_MAGIC     (0xdeadbeef + __LINE__ + (__LINE__ << 4))

#define assertSMTP(pointer) kwassert((pointer)->magic == SMTPC_MAGIC)

/* Initialize */
SMTPClient *initializeClient(SOCKET socket);
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

/* Set/retrieve queueRun flag */
void setClientQueueRun(SMTPClient *client, KWBoolean needQueueRun);
KWBoolean getClientQueueRun(const SMTPClient *client);

/* Mode (state) flag */
void setClientMode(SMTPClient *client, SMTPMode mode);
SMTPMode getClientMode(const SMTPClient *client);

/* Socket error flag */
void setClientSocketError(SMTPClient *client, int error);
int getClientSocketError(const SMTPClient *client);

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

/* Set override for mode time out */
void
setClientTimeout( SMTPClient *client, time_t timeout );

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

/* Query, set, and clear flag */
#define isClientFlag(_client, _flag)         \
      ((KWBoolean) (((_client)->flag & (_flag)) ? KWTrue : KWFalse))
#define setClientFlag(_client, _flag)   (_client)->flag |= (_flag)
#define clearClientFlag(_client, _flag)      \
      (_client)->flag &= (SF_ALL_FLAG - (_flag))
#endif  /* _SMTPCLIENT_H */
