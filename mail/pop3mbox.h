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
 *       $Id: pop3mbox.h 1.2 1998/03/03 07:37:36 ahd v1-12v $
 *
 *       Revision history:
 *       $Log: pop3mbox.h $
 *       Revision 1.2  1998/03/03 07:37:36  ahd
 *       Add flags for requiring mbox update
 *
 *       Revision 1.1  1998/03/03 03:55:26  ahd
 *       Initial revision
 *
 */

#include "pop3clnt.h"

KWBoolean
popBoxLoad(SMTPClient *client);

KWBoolean
popBoxUnload( SMTPClient *client );

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
getBoxPopNext( MailMessage *current );

#define popBoxIsUpdated( current ) (current->deleted || current->fakeUIDL)

#define popBoxIsDeleted( current ) (current->deleted)

#define popBoxDelete( current ) { current->deleted = KWTrue; }

#endif  /* POP3MBOX_H */
