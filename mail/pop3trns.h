#ifndef _POP3TRANS_H
#define _POP3TRANS_H

/*--------------------------------------------------------------------*/
/*       p o p 3 t r n s . h                                          */
/*                                                                    */
/*       Define variables used for POP3 transactions                  */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1998 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *       $Id: pop3trns.h 1.1 1998/03/01 19:45:01 ahd Exp $
 *
 *       Revision history:
 *       $Log: pop3trns.h $
 *       Revision 1.1  1998/03/01 19:45:01  ahd
 *       Initial revision
 *
 */

#include "usertabl.h"
#include "imfile.h"

typedef struct _MailMessage
{
   long sequence;                   /* Message num, counting from 1  */
   long octets;                     /* Size in bytes                 */
   long startPosition;
   long startBodyPosition;
   long endPosition;
   char *uidl;                      /* unique string to indent msg   */
   KWBoolean deleted;
   KWBoolean fakeUIDL;              /* UIDL was generated on the fly */
   struct _MailMessage *previous;
   struct _MailMessage *next;

} MailMessage;

typedef struct _POP3Transaction
{

   long messageCount;               /* Mailbox message count         */
   long octets;                     /* Mailbox size in bytes         */
   char mailboxName[FILENAME_MAX];  /* Box we move data into         */
   struct UserTable *userp;
   IMFILE *imf;                     /* Mailbox temporary copy        */
   FILE *mailboxStream;             /* Stream used to access mbox    */
   MailMessage *top;                /* Our message queue             */

} POP3Transaction;

/* Silly hack so pointer type is correct for our purposes */
#define SMTPTransaction POP3Transaction
#define _SMTPTransaction _POP3Transaction

#endif /* _POP3TRANS_H */
