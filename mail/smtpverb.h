#ifndef _SMTPVERB_H
#define _SMTPVERB_H

/*--------------------------------------------------------------------*/
/*       s m t p v e r b . c                                          */
/*                                                                    */
/*       SMTP verb parser for UUPC/extended                           */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1997 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *       $Id: smtpverb.h 1.4 1997/11/25 05:05:36 ahd Exp $
 *
 *       Revision History:
 *       $Log: smtpverb.h $
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

   /* Following only used for POP3 server */
   POP_OKAY            = 1001,
   POP_ERROR,
   SR_ZZ_LAST
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
   const char name[5];              /* VERB issued by client         */
   KWBoolean trivial;               /* trivial command used in DoS?  */
   unsigned short validModes;       /* Modes we invoke comm proc for */
   SMTPMode newMode;                /* New mode if comm proc success */
   SR_VERB  successResponse;        /* Std resp if comm proc success */
   SR_VERB  modeErrorResponse;      /* Error code if wrong mode      */
   size_t minOperands;              /* Min operands needed for parse */
   const char *pattern;             /* Operand input pattern         */
} SMTPVerb;


void
SMTPInvokeCommand( SMTPClient *client );

/*--------------------------------------------------------------------*/
/*         Declarations for data/functions unique to procotol         */
/*--------------------------------------------------------------------*/

SMTPVerb verbTable[];

void
cleanupTransaction( SMTPClient *client );

#endif /* _SMTPVERB_H */
