#ifndef _SMTPVERB_H
#define _SMTPVERB_H

/*--------------------------------------------------------------------*/
/*       s m t p v e r b . h                                          */
/*                                                                    */
/*       SMTP verb parser for UUPC/extended                           */
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
 *       $Id: smtpverb.h 1.14 2001/03/12 13:51:54 ahd v1-13k $
 *
 *       Revision History:
 *       $Log: smtpverb.h $
 *       Revision 1.14  2001/03/12 13:51:54  ahd
 *       Annual copyright update
 *
 *       Revision 1.13  2000/05/12 12:44:47  ahd
 *       Annual copyright update
 *
 *       Revision 1.12  1999/01/04 03:56:09  ahd
 *       Annual copyright change
 *
 *       Revision 1.11  1998/04/24 03:34:25  ahd
 *       Use local buffers for output
 *       Add flag bits to verb table
 *       Add flag bytes to client structure
 *
 *       Revision 1.10  1998/03/08 23:12:28  ahd
 *       Support for sending raw text to POP clients
 *
 *       Revision 1.9  1998/03/06 06:53:00  ahd
 *       Add dummy commands to make Netscape happy
 *
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
   PR_OK_GENERIC,             /* Standard okay response              */
   PR_DATA,                   /* Generic multi-responses             */
   PR_TEXT,                   /* Message text responses              */
   PR_ERROR_WARNING,          /* Doesn't print a message in the log  */
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

#define VF_EMPTY_FLAGS     0x0000   /* Dummy place holder            */

#define VF_DATA_REDRIVE    0x0001   /* Run next cms if data avail    */
#define VF_NO_TOKENIZE     0x0002   /* Do not tokenize next command  */
#define VF_TRIVIAL_CMD     0x0004   /* Command is trivial            */

#define VF_NO_READ         0x0010   /* Immediately run next command  */
#define VF_NO_READ_SUCCESS 0x0020   /* Immediately run next command
                                       if previous cmd succeeds      */

/*--------------------------------------------------------------------*/
/*                    Layout of master verb table                     */
/*--------------------------------------------------------------------*/

typedef struct _SMTPVerb
{
   const unsigned long flag;        /* Flag bits                     */
   ref_verbproc processor;          /* Command processor             */
   ref_verbproc rejecter;           /* Processor for bad modes       */
   const char *name;                /* VERB issued by client         */
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
