#ifndef POP3MBOX_H
#define POP3MBOX_H

/*--------------------------------------------------------------------*/
/*       p o p 3 m b o x . h                                          */
/*                                                                    */
/*       Definitions for processing the working copy of the POP3      */
/*       mailbox.                                                     */
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
 *       $Id: lib.h 1.42 1998/03/01 01:26:54 ahd Exp $
 *
 *       Revision history:
 *       $Log: lib.h $
 */

#include "pop3clnt.h"

KWBoolean
popBoxLoad(SMTPClient *client);

long
getMessageOctetCount( MailMessage *current,
                      long *messageCountPtr );

void
cleanupMailbox( MailMessage *current );

int
popBoxUndelete( MailMessage *current );

MailMessage *
popBoxGet(MailMessage *current,  const long sequence );

char *
popBoxUIDL( MailMessage *current );

MailMessage *
getPopMessageNext( MailMessage *current );

#define popBoxIsDeleted( current ) (current->deleted)
#define popBoxDelete( current ) { current->deleted = KWTrue; }

#endif  /* POP3MBOX_H */
