/*--------------------------------------------------------------------*/
/*       p o p 3 c m d s . c                                          */
/*                                                                    */
/*       SMTP verb table for UUPC/extended                            */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1998 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *       $Id: pop3cmds.c 1.3 1998/03/03 07:36:28 ahd v1-12v $
 *
 *       $Log: pop3cmds.c $
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

RCSID("$Id: pop3cmds.c 1.3 1998/03/03 07:36:28 ahd v1-12v $");

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
      commandAccept,
      commandSequenceIgnore,
      "",
      KWFalse,
      P3_MASTER,
      P3_SAME_MODE,
   },
   {
      commandInit,
      commandSequenceIgnore,
      "",
      KWFalse,
      P3_CONNECTED,
      P3_AUTHORIZATION,

      PR_OK_GENERIC,
   },
   {
      commandLoadMailbox,
      commandSequenceIgnore,
      "",
      KWFalse,
      P3_LOAD_MBOX,
      P3_TRANSACTION,

      PR_OK_GENERIC,
      PR_ERROR_GENERIC,
   },
   {
      commandExiting,
      commandSequenceIgnore,
      "",
      KWFalse,
      P3_EXITING,
      P3_DELETE_PENDING,

      PR_ERROR_GENERIC,
   },
   {
      commandTimeout,
      commandSequenceIgnore,
      "",
      KWFalse,
      P3_TIMEOUT,
      P3_DELETE_PENDING,

      PR_ERROR_GENERIC,
      PR_ERROR_GENERIC
   },
   {
      commandTerminated,
      commandSequenceIgnore,
      "",
      KWFalse,
      P3_ABORT,
      P3_DELETE_PENDING,
   },
   {
      commandAUTH,
      commandSequenceIgnore,
      "AUTH",
      KWFalse,
      P3_AUTHORIZATION,
      P3_SAME_MODE,

      PR_OK_GENERIC,
      PR_ERROR_GENERIC,
      SV_OPTIONAL_OPERANDS
   },
   {
      commandDELE,
      commandSequenceIgnore,
      "DELE",
      KWFalse,
      P3_TRANSACTION,
      P3_SAME_MODE,

      PR_OK_GENERIC,
      PR_ERROR_GENERIC,
      1
   },
   {
      commandLIST,
      commandSequenceIgnore,
      "LIST",
      KWFalse,
      P3_TRANSACTION,
      P3_SAME_MODE,

      PR_OK_GENERIC,
      PR_ERROR_GENERIC,
      SV_OPTIONAL_OPERANDS
   },
   {
      commandLIST,
      commandSequenceIgnore,
      "XAUTHLIST",
      KWFalse,
      P3_TRANSACTION,
      P3_SAME_MODE,

      PR_OK_GENERIC,
      PR_ERROR_GENERIC,
      SV_OPTIONAL_OPERANDS
   },
   {
      commandNOOP,
      commandSequenceIgnore,
      "NOOP",
      KWFalse,
      P3_TRANSACTION,
      P3_SAME_MODE,

      PR_OK_GENERIC,
      PR_ERROR_GENERIC,
      0
   },
   {
      commandPASS,
      commandSequenceIgnore,
      "PASS",
      KWFalse,
      P3_PASSWORD,
      P3_LOAD_MBOX,

      PR_OK_GENERIC,
      PR_ERROR_GENERIC,
      1
   },
   {
      commandQUIT,
      commandSequenceIgnore,
      "QUIT",
      KWFalse,
      POP3_MODES_ALL,
      P3_DELETE_PENDING,

      PR_OK_GENERIC,
      PR_ERROR_GENERIC,
      0
   },
   {
      commandRETR,
      commandSequenceIgnore,
      "RETR",
      KWFalse,
      P3_TRANSACTION,
      P3_SAME_MODE,

      PR_OK_GENERIC,
      PR_ERROR_GENERIC,
      1
   },
   {
      commandRSET,
      commandSequenceIgnore,
      "RSET",
      KWFalse,
      P3_TRANSACTION,
      P3_SAME_MODE,

      PR_OK_GENERIC,
      PR_ERROR_GENERIC,
      0
   },
   {
      commandSTAT,
      commandSequenceIgnore,
      "STAT",
      KWFalse,
      P3_TRANSACTION,
      P3_SAME_MODE,

      PR_OK_GENERIC,
      PR_ERROR_GENERIC,
      0
   },
   {
      commandTOP,
      commandSequenceIgnore,
      "TOP",
      KWFalse,
      P3_TRANSACTION,
      P3_SAME_MODE,

      PR_OK_GENERIC,
      PR_ERROR_GENERIC,
      2
   },
   {
      commandUIDL,
      commandSequenceIgnore,
      "UIDL",
      KWFalse,
      P3_TRANSACTION,
      P3_SAME_MODE,

      PR_OK_GENERIC,
      PR_ERROR_GENERIC,
      SV_OPTIONAL_OPERANDS
   },
   {
      commandUSER,
      commandSequenceIgnore,
      "USER",
      KWFalse,
      P3_AUTHORIZATION,
      P3_PASSWORD,

      PR_OK_GENERIC,
      PR_ERROR_GENERIC,
      1
   },
   {
      commandXSENDER,
      commandSequenceIgnore,
      "XSENDER",
      KWFalse,
      P3_TRANSACTION,
      P3_SAME_MODE,

      PR_OK_GENERIC,
      PR_ERROR_GENERIC,
      1,
   },

   /* Command for syntax errors MUST BE LAST */
   {
      commandSyntax,             /* Fall through for syntax errs  */
      commandSequenceIgnore,
      "",
      KWTrue,
      POP3_MODES_ALL,
      P3_SAME_MODE,

      PR_ERROR_GENERIC,
      PR_ERROR_GENERIC,
      0
   }
}; /* POP3Verb table[] */
