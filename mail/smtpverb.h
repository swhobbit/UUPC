#ifndef _SMTPVERB_H
#define _SMTPVERB_H

/*--------------------------------------------------------------------*/
/*       s m t p v e r b . c                                          */
/*                                                                    */
/*       SMTP verb parser for UUPC/extended                           */
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
 *       $Id: smtpverb.h 1.8 1998/03/01 19:45:01 ahd v1-12v $
 *
 *       Revision History:
 *       $Log: smtpverb.h $
 *       Revision 1.8  1998/03/01 19:45:01  ahd
 *       First compiling POP3 server which accepts user id/password
 *
 *       Revision 1.7  1997/11/28 23:13:35  ahd
 *       Additional auditing, including DNS support
 *
 *       Revision 1.6  1997/11/28 04:52:53  ahd
 *       Initial UUSMTPD OS/2 support
 *
 *       Revision 1.5  1997/11/26 03:34:44  ahd
 *       Correct SMTP timeouts, break out protocol from rest of daemon
 *
 *       Revision 1.4  1997/11/25 05:05:36  ahd
 *       More robust SMTP daemon
 *
 *       Revision 1.3  1997/11/24 02:53:26  ahd
 *       First working SMTP daemon which delivers mail
 *
 *       Revision 1.2  1997/11/21 18:16:32  ahd
 *       Command processing stub SMTP daemon
 *
 */

#include "smtpclnt.h"

typedef enum
{
   SR_AA_FIRST,

   /* SMTP return codes */
   SR_OK_CONNECT       = 220,
   SR_OK_QUIT          = 221,
   SR_OK_GENERIC       = 250,
   SR_OK_VRFY_REMOTE   = 251,
   SR_OK_SEND_DATA     = 354,
   SR_PE_TEMP_SYNTAX   = 401,
   SR_PE_NOT_IN_DNS    = 418,
   SR_TE_SHUTDOWN      = 421,
   SR_TE_SHORTAGE      = 452,
   SR_PE_UNKNOWN       = 500,
   SR_PE_SYNTAX        = 501,
   SR_PE_NOT_IMPL      = 502,
   SR_PE_ORDERING      = 503,
   SR_PE_TOO_MANY_ADDR = 552,
   SR_PE_BAD_MAILBOX   = 553,
   SR_PE_NOT_POLICY    = 571,
   SR_ZZ_LAST,

   /* Following only used for POP3 server */
   PR_AA_FIRST         = ( SR_ZZ_LAST / 100 + 1 ) * 100,
   PR_OK_GENERIC,
   PR_DATA,
   PR_ERROR_WARNING,          /* Doesn't print a message in the log */
   PR_ERROR_GENERIC,
   PR_ZZ_LAST,

   ZZ_LAST
} SR_VERB;

#define SR_OK_SENDER          SR_OK_GENERIC
#define SR_OK_RECEIPT         SR_OK_GENERIC
#define SR_OK_VRFY_LOCAL      SR_OK_GENERIC
#define SR_OK_MAIL_ACCEPTED   SR_OK_GENERIC

#define SR_PE_OPER_MISS       SR_PE_SYNTAX
#define SR_PE_OPER_PARSE      SR_PE_SYNTAX

#define SR_PE_DUPLICATE       SR_PE_ORDERING
#define SR_PE_MISSING         SR_PE_ORDERING
#define SR_PE_NEED_ADDR       SR_PE_ORDERING
#define SR_PE_NOT_IDLE        SR_PE_ORDERING
#define SR_PE_NOT_MAIL        SR_PE_ORDERING

/* Special operand for getOperand to denote possible operands */
#define SV_OPTIONAL_OPERANDS     9999


typedef KWBoolean(*ref_verbproc)(SMTPClient *client,
                                 struct _SMTPVerb* verb,
                                 char **operands );

/*--------------------------------------------------------------------*/
/*                    Layout of master verb table                     */
/*--------------------------------------------------------------------*/

typedef struct _SMTPVerb
{
   ref_verbproc processor;          /* Command processor             */
   ref_verbproc rejecter;           /* Processor for bad modes       */
   const char *name;                /* VERB issued by client         */
   KWBoolean trivial;               /* trivial command used in DoS?  */
   unsigned long validModes;        /* Modes we invoke comm proc for */
   SMTPMode newMode;                /* New mode if comm proc success */
   SR_VERB  successResponse;        /* Std resp if comm proc success */
   SR_VERB  modeErrorResponse;      /* Error code if wrong mode      */
   size_t minOperands;              /* Min operands needed for parse */
   const char *pattern;             /* Operand input pattern         */
} SMTPVerb;


void
SMTPInvokeCommand( SMTPClient *client );

/*--------------------------------------------------------------------*/
/*         Declarations for functions unique to procotol              */
/*--------------------------------------------------------------------*/

void
cleanupTransaction( SMTPClient *client );

#endif /* _SMTPVERB_H */
