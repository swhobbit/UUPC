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
 *       $Id: smtpverb.c 1.4 1997/11/25 05:05:06 ahd Exp $
 *
 *       $Log$
 */

/*--------------------------------------------------------------------*/
/*                           Include files                            */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"
#include "smtprecv.h"
#include "smtplwc.h"

/*--------------------------------------------------------------------*/
/*                      Global defines/variables                      */
/*--------------------------------------------------------------------*/

RCSID("$Id: smtpverb.c 1.4 1997/11/25 05:05:06 ahd Exp $");

/*--------------------------------------------------------------------*/
/*       Master command verb table for SMTP                           */
/*                                                                    */
/*       Empty verbs names are completely state driven and are        */
/*       skipped if client is not in proper state, all others         */
/*       (with name) are processed as an out of sequence command.     */
/*--------------------------------------------------------------------*/

SMTPVerb verbTable[] =
{
   /* commandAccept only used by master socket to create clients */
   {
      commandAccept,
      commandSequenceIgnore,
      "",
      KWFalse,
      SM_MASTER,
      SM_SAME_MODE,
   },
   {
      commandInit,
      commandSequenceIgnore,
      "",
      KWFalse,
      SM_CONNECTED,
      SM_UNGREETED,

      SR_OK_CONNECT,
   },
   {
      commandExiting,
      commandSequenceIgnore,
      "",
      KWFalse,
      SM_EXITING,
      SM_DELETE_PENDING,

      SR_TE_SHUTDOWN,
   },
   {
      commandTimeout,
      commandSequenceIgnore,
      "",
      KWFalse,
      SM_TIMEOUT,
      SM_DELETE_PENDING,

      SR_TE_SHUTDOWN,
   },
   {
      commandTerminated,
      commandSequenceIgnore,
      "",
      KWFalse,
      SM_ABORT,
      SM_DELETE_PENDING,
   },
   /* Period command has priority over generic SM_DATA mode */
   {
      commandPeriod,
      commandSequenceIgnore,
      ".",
      KWFalse,
      SM_DATA,
      SM_IDLE,

      SR_OK_MAIL_ACCEPTED,
      SR_PE_NEED_ADDR
   },
   {
      commandDataInput,
      commandSequenceIgnore,
      "",
      KWFalse,
      SM_DATA,
      SM_SAME_MODE,
   },
   {
      commandHELO,
      commandSequenceIgnore,
      "HELO",
      KWFalse,
      SM_UNGREETED,
      SM_IDLE,

      SR_OK_GENERIC,
      SR_PE_DUPLICATE,
      1
   },
   {
      commandHELO,
      commandSequenceIgnore,
      "EHLO",
      KWFalse,
      SM_UNGREETED,
      SM_IDLE,

      SR_OK_GENERIC,
      SR_PE_DUPLICATE,
      1
   },
   {
      commandRSET,
      commandSequenceIgnore,
      "RSET",
      KWTrue,
      SMTP_MODES_AFTER_HELO,
      SM_IDLE,

      SR_OK_GENERIC,
      SR_PE_DUPLICATE,
   },
   {
      commandNOOP,
      commandSequenceIgnore,
      "EXPN",
      KWTrue,
      SMTP_MODES_NONE,
      SM_SAME_MODE,

      SR_PE_NOT_POLICY
   },
   {
      commandVRFY,
      commandSequenceIgnore,
      "VRFY",
      KWTrue,
      SMTP_MODES_AFTER_HELO,
      SM_SAME_MODE,

      SR_OK_VRFY_LOCAL,
      SR_PE_ORDERING,
      1
   },
   {
      commandMAIL,
      commandSequenceIgnore,
      "MAIL",
      KWFalse,
      SM_IDLE,
      SM_ADDR_FIRST,

      SR_OK_SENDER,
      SR_PE_NOT_IDLE,
      1,
      "FROM:"
   },
   {
      commandRCPT,
      commandSequenceIgnore,
      "RCPT",
      KWFalse,
      ( SM_ADDR_FIRST | SM_ADDR_SECOND ),
      SM_ADDR_SECOND,

      SR_OK_RECEIPT,
      SR_PE_NOT_MAIL,
      1,
      "TO:"
   },
   {
      commandDATA,
      commandSequenceIgnore,
      "DATA",
      KWFalse,
      SM_ADDR_SECOND,
      SM_DATA,

      SR_OK_SEND_DATA,
   },
   {
      commandNOOP,
      commandSequenceIgnore,
      "NOOP",
      KWTrue,
      SMTP_MODES_AFTER_HELO,
      SM_SAME_MODE,

      SR_OK_GENERIC,
      0
   },
   {
      commandQUIT,
      commandSequenceIgnore,
      "QUIT",
      KWFalse,
      SMTP_MODES_ALL,
      SM_DELETE_PENDING,

      SR_OK_QUIT,
      0
   },
   /* Command for syntax errors MUST BE LAST */
   {
      commandSyntax,             /* Fall through for syntax errs  */
      commandSequenceIgnore,
      "",
      KWTrue,
      SMTP_MODES_ALL,
      SM_SAME_MODE,

      SR_PE_UNKNOWN,
      0
   }
}; /* SMTPVerb table[] */
