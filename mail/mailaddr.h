#ifndef _MAILADDR_H
#define _MAILADDR_H

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1998 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1998 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: mailaddr.h 1.3 1998/03/08 23:12:28 ahd Exp $
 *
 *    $Log: mailaddr.h $
 *    Revision 1.3  1998/03/08 23:12:28  ahd
 *    Add new fields for local information
 *
 *    Revision 1.2  1998/03/01 01:30:57  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.1  1997/12/13 18:11:33  ahd
 *    Initial revision
 *
 */

typedef struct _MAIL_ADDR
{
   char *address;
   char *relay;
   char *host;
   char *user;                   /* Mailbox of sender address        */
   char *activeUser;             /* Local user executing program     */
   KWBoolean remote;             /* Address is non-local             */
   KWBoolean daemon;             /* Delivery by long running daemon  */
}  MAIL_ADDR;

#endif /* _MAILADDR_H */
