#ifndef _SMTPCMDS_H
#define _SMTPCMDS_H

/*--------------------------------------------------------------------*/
/*       s m t p c m d s . h                                          */
/*                                                                    */
/*       SMTP commands which actually handle mail                     */
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
 *       $Id: smtpcmds.h 1.6 2001/03/12 13:51:54 ahd v1-13k $
 *
 *       Revision History:
 *       $Log: smtpcmds.h $
 *       Revision 1.6  2001/03/12 13:51:54  ahd
 *       Annual copyright update
 *
 *       Revision 1.5  2000/05/12 12:42:31  ahd
 *       Annual copyright update
 *
 *       Revision 1.4  1999/01/04 03:56:09  ahd
 *       Annual copyright change
 *
 *       Revision 1.3  1998/04/24 03:34:25  ahd
 *       Use local buffers for output
 *       Add flag bits to verb table
 *       Add flag bytes to client structure
 *
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
