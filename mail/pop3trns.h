#ifndef _POP3TRANS_H
#define _POP3TRANS_H

/*--------------------------------------------------------------------*/
/*       p o p 3 t r n s . h                                          */
/*                                                                    */
/*       Define variables used for POP3 transactions                  */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-2000 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *       $Id: pop3trns.h 1.6 1999/01/04 03:56:09 ahd Exp $
 *
 *       Revision history:
 *       $Log: pop3trns.h $
 *       Revision 1.6  1999/01/04 03:56:09  ahd
 *       Annual copyright change
 *
 *       Revision 1.5  1998/11/04 02:01:17  ahd
 *       Add sanity check field to linked lists
 *       Add assertion macros
 *
 * Revision 1.4  1998/04/19  15:33:53  ahd
 * *** empty log message ***
 *
 *       Revision 1.3  1998/03/03 07:37:36  ahd
 *       Add flags for requiring mbox update
 *
 *       Revision 1.2  1998/03/03 03:55:26  ahd
 *       First POP3 server to handle most data commands
 *
 *       Revision 1.1  1998/03/01 19:45:01  ahd
 *       Initial revision
 *
 */

#include "usertabl.h"
#include "imfile.h"

typedef struct _MailMessage
{
   long magic;                      /* Flag for valid item           */
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
   long magic;                      /* Flag for valid item           */
   long messageCount;               /* Mailbox message count         */
   long octets;                     /* Mailbox size in bytes         */
   char mailboxName[FILENAME_MAX];  /* Box we move data into         */
   struct UserTable *userp;
   IMFILE *imf;                     /* Mailbox temporary copy        */
   FILE *mailboxStream;             /* Stream used to access mbox    */
   MailMessage *top;                /* Our message queue             */
   KWBoolean rewrite;               /* At least one message updated  */

} POP3Transaction;

#define POP3T_MAGIC     (0xdeadbeef + __LINE__ + (__LINE__ << 4))
#define POP3M_MAGIC     (0xdeadbeef + __LINE__ + (__LINE__ << 4))

#define assertPOP3Transaction(pointer) kwassert((pointer)->magic == POP3T_MAGIC)
#define assertPOP3Message(pointer)     kwassert((pointer)->magic == POP3M_MAGIC)

/* Silly hack so pointer type is correct for our purposes */
#define SMTPTransaction POP3Transaction
#define _SMTPTransaction _POP3Transaction

#endif /* _POP3TRANS_H */
