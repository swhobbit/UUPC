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
 *       $Id: smtpverb.c 1.1 1997/06/03 03:25:31 ahd Exp $
 *
 *       Revision History:
 *       $Log$
 */

#include "smtpclnt.h"

typedef enum
{
   SR_AA_FIRST,
   SR_OK_CONNECT     = 220,
   SR_OK_QUIT        = 221,
   SR_OK_GENERIC     = 250,
   SR_OK_VRFY_REMOTE = 252,
   SR_OK_SEND_DATA   = 354,
   SR_PE_NOT_IN_DNS  = 418,
   SR_TE_SHUTDOWN    = 421,
   SR_PE_UNKNOWN     = 500,
   SR_PE_SYNTAX      = 501,
   SR_PE_NOT_IMPL    = 502,
   SR_PE_ORDERING    = 503,
   SR_PE_VRFY_REMOTE = 553,
   SR_PE_NOT_POLICY  = 571,
   SR_ZZ_LAST
} SR_VERB;

#define SR_OK_SENDER          SR_OK_GENERIC
#define SR_OK_RECEIPT         SR_OK_GENERIC
#define SR_OK_VRFY_LOCAL      SR_OK_GENERIC
#define SR_OK_END_DATA        SR_OK_GENERIC

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

typedef struct _SMTPVerb
{
   const char name[5];              /* VERB issued by client         */
   ref_verbproc processor;          /* Command processor             */
   ref_verbproc rejecter;           /* Processor for bad modes       */
   long validModes;                 /* Modes we invoke comm proc for */
   SMTPMode newMode;                /* New mode if comm proc success */
   SR_VERB  successResponse;        /* Std resp if comm proc success */
   SR_VERB  modeErrorResponse;      /* Error code if wrong mode      */
   size_t minOperands;              /* Min operands needed for parse */
   const char *pattern;             /* Operand input pattern         */
} SMTPVerb;

void
SMTPInvokeCommand( SMTPClient *client );

#endif /* _SMTPVERB_H */
