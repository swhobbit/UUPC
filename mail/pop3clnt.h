#ifndef _POP3CLNT_H
#define _POP3CLNT_H

/*--------------------------------------------------------------------*/
/*       p o p 3 c l n t . h                                          */
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
 *    $Id: pop3clnt.h 1.2 1999/01/04 03:56:09 ahd Exp $
 *
 *    Revision history:
 *    $Log: pop3clnt.h $
 *    Revision 1.2  1999/01/04 03:56:09  ahd
 *    Annual copyright change
 *
 *    Revision 1.1  1998/03/01 19:43:52  ahd
 *    Initial revision
 *
 */

#include "pop3trns.h"
#include "smtpclnt.h"

/* POP3 Aliases, to map more closely to RFC 1939 state descriptions */
#define  P3_AUTHORIZATION     SM_UNGREETED
#define  P3_TRANSACTION       SM_IDLE

/* Non-standard aliases, for our own states */
#define  P3_PASSWORD          SM_PASSWORD    /* Password after USER  */
#define  P3_LOAD_MBOX         SM_LOAD_MBOX   /* Load mbox after PASS */
#define  P3_SEND_DATA         SM_SEND_DATA   /* Sending text of message */

/* Startup shutdown, same as SMTP versions */
#define  P3_DELETE_PENDING    SM_DELETE_PENDING
#define  P3_MASTER            SM_MASTER
#define  P3_CONNECTED         SM_CONNECTED
#define  P3_UNGREETED         SM_UNGREETED
#define  P3_ABORT             SM_ABORT
#define  P3_TIMEOUT           SM_TIMEOUT
#define  P3_EXITING           SM_EXITING
#define  P3_SAME_MODE         SM_SAME_MODE

#define  POP3_MODES_ALL       SMTP_MODES_ALL
#define  POP3_MODES_NONE      SMTP_MODES_NONE

#endif /* _POP3CLNT_H */
