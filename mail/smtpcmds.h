#ifndef _SMTPCMDS_H
#define _SMTPCMDS_H

/*--------------------------------------------------------------------*/
/*       s m t p c m d s . h                                          */
/*                                                                    */
/*       SMTP commands which actually handle mail                     */
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
 *       $Id: SMTPCMDS.H 1.2 1998/03/03 03:55:05 ahd v1-12v $
 *
 *       Revision History:
 *       $Log: SMTPCMDS.H $
 *       Revision 1.2  1998/03/03 03:55:05  ahd
 *       Routines to handle messages within a POP3 mailbox
 *
 *       Revision 1.1  1998/03/01 19:43:52  ahd
 *       Initial revision
 *
 *       Revision 1.2  1998/03/01 01:28:37  ahd
 */

#include "smtpverb.h"

SMTPVerb verbTable[];

char *defaultPortName;           /* TCP/IP port to listen on */
int missingOperandError;         /* Error number for missing operands */

#endif /* _SMTPCMDS_H */
