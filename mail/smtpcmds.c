/*--------------------------------------------------------------------*/
/*       s m t p c m d s . c                                          */
/*                                                                    */
/*       SMTP verb table for UUPC/extended                            */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-2002 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *       $Id: smtpcmds.c 1.10 2001/03/12 13:55:29 ahd v1-13k $
 *
 *       $Log: smtpcmds.c $
 *       Revision 1.10  2001/03/12 13:55:29  ahd
 *       Annual copyright update
 *
 *       Revision 1.9  2000/05/12 12:35:45  ahd
 *       Annual copyright update
 *
 *       Revision 1.8  1999/01/04 03:54:27  ahd
 *       Annual copyright change
 *
 *       Revision 1.7  1998/04/24 03:30:13  ahd
 *       Use local buffers, not client->transmit.buffer, for output
 *       Rename receive buffer, use pointer into buffer rather than
 *            moving buffered data to front of buffer every line
 *       Restructure main processing loop to give more priority
 *            to client processing data already buffered
 *       Add flag bits to client structure
 *       Add flag bits to verb tables
 *
 *       Revision 1.6  1998/03/03 03:51:53  ahd
 *       Routines to handle messages within a POP3 mailbox
 *
 *       Revision 1.5  1998/03/01 19:42:17  ahd
 *       SMTP verb table
 *
 *       Revision 1.4  1998/03/01 01:31:39  ahd
 *       Annual Copyright Update
 *
 *       Revision 1.3  1998/01/03 05:24:17  ahd
 *       Correct name in header
 *
 *       Revision 1.2  1997/11/28 23:11:38  ahd
 *       Additional SMTP auditing, normalize formatting, more OS/2 SMTP fixes
 *
 *       Revision 1.1  1997/11/26 03:34:11  ahd
 *       Correct SMTP timeouts, break out protocol from rest of daemon
 *
 */

/*--------------------------------------------------------------------*/
/*                           Include files                            */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"
#include "smtprecv.h"
#include "smtpcmmn.h"
#include "smtplwc.h"

/*--------------------------------------------------------------------*/
/*                      Global defines/variables                      */
/*--------------------------------------------------------------------*/

RCSID("$Id: smtpcmds.c 1.10 2001/03/12 13:55:29 ahd v1-13k $");

/*--------------------------------------------------------------------*/
/*          External variables for used by various routines           */
/*--------------------------------------------------------------------*/

char *defaultPortName = "smtp";
int missingOperandError = SR_PE_OPER_MISS;

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
      VF_NO_READ,
      commandAccept,
      commandSequenceIgnore,
      "",
      SM_MASTER,
      SM_SAME_MODE,
   },
   {
      VF_EMPTY_FLAGS,
      commandInit,
      commandSequenceIgnore,
      "",
      SM_CONNECTED,
      SM_UNGREETED,

      SR_OK_CONNECT,
   },
   {
      VF_NO_READ,
      commandExiting,
      commandSequenceIgnore,
      "",
      SM_EXITING,
      SM_DELETE_PENDING,

      SR_TE_SHUTDOWN,
   },
   {
      VF_NO_READ,
      commandTimeout,
      commandSequenceIgnore,
      "",
      SM_TIMEOUT,
      SM_DELETE_PENDING,

      SR_TE_SHUTDOWN,
   },
   {
      VF_NO_READ,
      commandTerminated,
      commandSequenceIgnore,
      "",
      SM_ABORT,
      SM_DELETE_PENDING,
   },
   /* Period command has priority over generic SM_DATA mode */
   {
      VF_EMPTY_FLAGS,
      commandPeriod,
      commandSequenceIgnore,
      ".\0",                        /* Only match full line          */
      SM_DATA,
      SM_IDLE,

      SR_OK_MAIL_ACCEPTED,
      SR_PE_NEED_ADDR
   },
   {
      VF_NO_TOKENIZE,
      commandDataInput,
      commandSequenceIgnore,
      "",
      SM_DATA,
      SM_SAME_MODE,
   },
   {
      VF_TRIVIAL_CMD,
      commandHELO,
      commandSequenceIgnore,
      "HELO",
      SM_UNGREETED,
      SM_IDLE,

      SR_OK_GENERIC,
      SR_PE_DUPLICATE,
      1
   },
   {
      VF_TRIVIAL_CMD,
      commandHELO,
      commandSequenceIgnore,
      "EHLO",
      SM_UNGREETED,
      SM_IDLE,

      SR_OK_GENERIC,
      SR_PE_DUPLICATE,
      1
   },
   {
      VF_TRIVIAL_CMD,
      commandRSET,
      commandSequenceIgnore,
      "RSET",
      SMTP_MODES_AFTER_HELO,
      SM_IDLE,

      SR_OK_GENERIC,
      SR_PE_DUPLICATE,
   },
   {
      VF_TRIVIAL_CMD,
      commandNOOP,
      commandSequenceIgnore,
      "EXPN",
      SMTP_MODES_NONE,
      SM_SAME_MODE,

      SR_PE_NOT_POLICY
   },
   {
      VF_TRIVIAL_CMD,
      commandVRFY,
      commandSequenceIgnore,
      "VRFY",
      SMTP_MODES_AFTER_HELO,
      SM_SAME_MODE,

      SR_OK_VRFY_LOCAL,
      SR_PE_ORDERING,
      1
   },
   {
      VF_EMPTY_FLAGS,
      commandMAIL,
      commandSequenceIgnore,
      "MAIL",
      SM_IDLE,
      SM_ADDR_FIRST,

      SR_OK_SENDER,
      SR_PE_NOT_IDLE,
      1,
      "FROM:"
   },
   {
      VF_EMPTY_FLAGS,
      commandRCPT,
      commandSequenceIgnore,
      "RCPT",
      ( SM_ADDR_FIRST | SM_ADDR_SECOND ),
      SM_ADDR_SECOND,

      SR_OK_RECEIPT,
      SR_PE_NOT_MAIL,
      1,
      "TO:"
   },
   {
      VF_NO_TOKENIZE,
      commandDATA,
      commandSequenceIgnore,
      "DATA",
      SM_ADDR_SECOND,
      SM_DATA,

      SR_OK_SEND_DATA,
   },
   {
      VF_TRIVIAL_CMD,
      commandNOOP,
      commandSequenceIgnore,
      "NOOP",
      SMTP_MODES_AFTER_HELO,
      SM_SAME_MODE,

      SR_OK_GENERIC,
      0
   },
   {
      VF_NO_READ,
      commandQUIT,
      commandSequenceIgnore,
      "QUIT",
      SMTP_MODES_ALL,
      SM_DELETE_PENDING,

      SR_OK_QUIT,
      0
   },
   /* Command for syntax errors MUST BE LAST */
   {
      VF_TRIVIAL_CMD,
      commandSyntax,             /* Fall through for syntax errs  */
      commandSequenceIgnore,
      "",
      SMTP_MODES_ALL,
      SM_SAME_MODE,

      SR_PE_UNKNOWN,
      0
   }
}; /* SMTPVerb table[] */
