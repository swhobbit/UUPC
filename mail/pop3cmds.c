/*--------------------------------------------------------------------*/
/*       p o p 3 c m d s . c                                          */
/*                                                                    */
/*       SMTP verb table for UUPC/extended                            */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-2000 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *       $Id: pop3cmds.c 1.6 1999/01/04 03:53:57 ahd Exp $
 *
 *       $Log: pop3cmds.c $
 *       Revision 1.6  1999/01/04 03:53:57  ahd
 *       Annual copyright change
 *
 *       Revision 1.5  1998/04/24 03:30:13  ahd
 *       Use local buffers, not client->transmit.buffer, for output
 *       Rename receive buffer, use pointer into buffer rather than
 *            moving buffered data to front of buffer every line
 *       Restructure main processing loop to give more priority
 *            to client processing data already buffered
 *       Add flag bits to client structure
 *       Add flag bits to verb tables
 *
 *       Revision 1.4  1998/03/06 06:51:28  ahd
 *       Improved POP3 support, including transaction counting
 *
 *       Revision 1.3  1998/03/03 07:36:28  ahd
 *       Delete unused commands, correct states for others
 *
 *       Revision 1.2  1998/03/03 03:51:53  ahd
 *       First POP3 which responses to data commands
 *
 *       Revision 1.1  1998/03/01 19:40:21  ahd
 *       Initial revision
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
#include "pop3clnt.h"
#include "pop3user.h"
#include "pop3lwc.h"
#include "smtpcmmn.h"
#include "smtpcmds.h"      /* Our name (actually, our SMTP sibling)*/

/*--------------------------------------------------------------------*/
/*                      Global defines/variables                      */
/*--------------------------------------------------------------------*/

RCSID("$Id: pop3cmds.c 1.6 1999/01/04 03:53:57 ahd Exp $");

/*--------------------------------------------------------------------*/
/*          External variables for used by various routines           */
/*--------------------------------------------------------------------*/

char *defaultPortName = "pop3";

int missingOperandError = PR_ERROR_GENERIC;

/*--------------------------------------------------------------------*/
/*       Master command verb table for POP3                           */
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
      P3_MASTER,
      P3_SAME_MODE,
   },
   {
      VF_EMPTY_FLAGS,
      commandInit,
      commandSequenceIgnore,
      "",
      P3_CONNECTED,
      P3_AUTHORIZATION,

      PR_OK_GENERIC,
   },
   {
      VF_EMPTY_FLAGS,
      commandLoadMailbox,
      commandSequenceIgnore,
      "",
      P3_LOAD_MBOX,
      P3_TRANSACTION,

      PR_OK_GENERIC,
      PR_ERROR_GENERIC,
   },
   {
      VF_NO_READ,
      commandExiting,
      commandSequenceIgnore,
      "",
      P3_EXITING,
      P3_DELETE_PENDING,

      PR_ERROR_GENERIC,
   },
   {
      VF_NO_READ,
      commandTimeout,
      commandSequenceIgnore,
      "",
      P3_TIMEOUT,
      P3_DELETE_PENDING,

      PR_ERROR_GENERIC,
      PR_ERROR_GENERIC
   },
   {
      VF_NO_READ,
      commandTerminated,
      commandSequenceIgnore,
      "",
      P3_ABORT,
      P3_DELETE_PENDING,
   },
   {
      VF_TRIVIAL_CMD,
      commandAUTH,
      commandSequenceIgnore,
      "AUTH",
      P3_AUTHORIZATION,
      P3_SAME_MODE,

      PR_OK_GENERIC,
      PR_ERROR_GENERIC,
      SV_OPTIONAL_OPERANDS
   },
   {
      VF_TRIVIAL_CMD,
      commandDELE,
      commandSequenceIgnore,
      "DELE",
      P3_TRANSACTION,
      P3_SAME_MODE,

      PR_OK_GENERIC,
      PR_ERROR_GENERIC,
      1
   },
   {
      VF_EMPTY_FLAGS,
      commandLIST,
      commandSequenceIgnore,
      "LIST",
      P3_TRANSACTION,
      P3_SAME_MODE,

      PR_OK_GENERIC,
      PR_ERROR_GENERIC,
      SV_OPTIONAL_OPERANDS
   },
   {
      VF_EMPTY_FLAGS,
      commandLIST,
      commandSequenceIgnore,
      "XAUTHLIST",
      P3_TRANSACTION,
      P3_SAME_MODE,

      PR_OK_GENERIC,
      PR_ERROR_GENERIC,
      SV_OPTIONAL_OPERANDS
   },
   {
      VF_TRIVIAL_CMD,
      commandNOOP,
      commandSequenceIgnore,
      "NOOP",
      P3_TRANSACTION,
      P3_SAME_MODE,

      PR_OK_GENERIC,
      PR_ERROR_GENERIC,
      0
   },
   {
      VF_TRIVIAL_CMD | VF_NO_READ_SUCCESS,
      commandPASS,
      commandSequenceIgnore,
      "PASS",
      P3_PASSWORD,
      P3_LOAD_MBOX,

      PR_OK_GENERIC,
      PR_ERROR_GENERIC,
      1
   },
   {
      VF_EMPTY_FLAGS | VF_NO_READ,
      commandQUIT,
      commandSequenceIgnore,
      "QUIT",
      POP3_MODES_ALL,
      P3_DELETE_PENDING,

      PR_OK_GENERIC,
      PR_ERROR_GENERIC,
      0
   },
   {
      VF_EMPTY_FLAGS,
      commandRETR,
      commandSequenceIgnore,
      "RETR",
      P3_TRANSACTION,
      P3_SAME_MODE,

      PR_OK_GENERIC,
      PR_ERROR_GENERIC,
      1
   },
   {
      VF_EMPTY_FLAGS,
      commandRSET,
      commandSequenceIgnore,
      "RSET",
      P3_TRANSACTION,
      P3_SAME_MODE,

      PR_OK_GENERIC,
      PR_ERROR_GENERIC,
      0
   },
   {
      VF_EMPTY_FLAGS,
      commandSTAT,
      commandSequenceIgnore,
      "STAT",
      P3_TRANSACTION,
      P3_SAME_MODE,

      PR_OK_GENERIC,
      PR_ERROR_GENERIC,
      0
   },
   {
      VF_EMPTY_FLAGS,
      commandTOP,
      commandSequenceIgnore,
      "TOP",
      P3_TRANSACTION,
      P3_SAME_MODE,

      PR_OK_GENERIC,
      PR_ERROR_GENERIC,
      2
   },
   {
      VF_EMPTY_FLAGS,
      commandUIDL,
      commandSequenceIgnore,
      "UIDL",
      P3_TRANSACTION,
      P3_SAME_MODE,

      PR_OK_GENERIC,
      PR_ERROR_GENERIC,
      SV_OPTIONAL_OPERANDS
   },
   {
      VF_TRIVIAL_CMD,
      commandUSER,
      commandSequenceIgnore,
      "USER",
      P3_AUTHORIZATION,
      P3_PASSWORD,

      PR_OK_GENERIC,
      PR_ERROR_GENERIC,
      1
   },
   {
      VF_EMPTY_FLAGS,
      commandXSENDER,
      commandSequenceIgnore,
      "XSENDER",
      P3_TRANSACTION,
      P3_SAME_MODE,

      PR_OK_GENERIC,
      PR_ERROR_GENERIC,
      1,
   },

   /* Command for syntax errors MUST BE LAST */
   {
      VF_EMPTY_FLAGS,
      commandSyntax,             /* Fall through for syntax errs  */
      commandSequenceIgnore,
      "",
      POP3_MODES_ALL,
      P3_SAME_MODE,

      PR_ERROR_GENERIC,
      PR_ERROR_GENERIC,
      0
   }
}; /* POP3Verb table[] */
