#ifndef _SMTPTRANS_H
#define _SMTPTRANS_H

/*--------------------------------------------------------------------*/
/*       s m t p t r n s . h                                          */
/*                                                                    */
/*       Define variables used for SMTP transactions                  */
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
 *       $Id: smtptrns.h 1.1 1997/11/26 03:34:44 ahd v1-12t $
 *
 *       Revision history:
 *       $Log: smtptrns.h $
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
} SMTPTransaction;

#endif /* _SMTPTRANS_H */
