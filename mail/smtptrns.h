#ifndef _SMTPTRANS_H
#define _SMTPTRANS_H

/*--------------------------------------------------------------------*/
/*       s m t p t r n s . h                                          */
/*                                                                    */
/*       Define variables used for SMTP transactions                  */
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
 *       $Id: smtptrns.h 1.2 1997/12/14 02:42:06 ahd v1-12u $
 *
 *       Revision history:
 *       $Log: smtptrns.h $
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
} SMTPTransaction;

#endif /* _SMTPTRANS_H */
