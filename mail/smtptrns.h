#ifndef _SMTPTRANS_H
#define _SMTPTRANS_H

/*--------------------------------------------------------------------*/
/*       s m t p t r n s . h                                          */
/*                                                                    */
/*       Define variables used for SMTP transactions                  */
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
 *       $Id: smtptrns.h 1.7 2001/03/12 13:51:54 ahd v1-13k $
 *
 *       Revision history:
 *       $Log: smtptrns.h $
 *       Revision 1.7  2001/03/12 13:51:54  ahd
 *       Annual copyright update
 *
 *       Revision 1.6  2000/05/12 12:44:47  ahd
 *       Annual copyright update
 *
 *       Revision 1.5  1999/01/04 03:56:09  ahd
 *       Annual copyright change
 *
 *       Revision 1.4  1998/05/11 01:25:40  ahd
 *       Support disallowing third-party SMTP relays
 *
 *       Revision 1.3  1998/03/01 01:28:04  ahd
 *       Annual Copyright Update
 *
 *       Revision 1.2  1997/12/14 02:42:06  ahd
 *       Don't use sender address as pointer, just alloc room for it
 *
 *       Revision 1.1  1997/11/26 03:34:44  ahd
 *       Correct SMTP timeouts, break out protocol from rest of daemon
 *
 */

typedef struct _SMTPTransaction
{
   char sender[MAXADDR];            /* RFC-822 address of sender     */
   char **address;                  /* List of addresses             */
   IMFILE *imf;
   size_t addressLength;            /* Size of address array         */
   size_t addressCount;             /* Number entries in array used  */
   KWBoolean localRelay;            /* True = DNS reports machine
                                       IP address in our local domain*/
   KWBoolean localSender;           /* True = Announced sender addr
                                       is known to us                */
} SMTPTransaction;

#endif /* _SMTPTRANS_H */
