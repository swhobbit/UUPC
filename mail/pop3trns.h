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
 *       $Id: smtptrns.h 1.3 1998/03/01 01:28:04 ahd Exp $
 *
 *       Revision history:
 *       $Log: smtptrns.h $
 */

#include "usertabl.h"
#include "imfile.h"

typedef struct _POP3Transaction
{

   struct UserTable *userp;
   IMFILE *imf;                     /* Mailbox temporary copy        */

} POP3Transaction;

/* Silly hack so pointer type is correct for our purposes */
#define SMTPTransaction POP3Transaction
#define _SMTPTransaction _POP3Transaction

#endif /* _POP3TRANS_H */
