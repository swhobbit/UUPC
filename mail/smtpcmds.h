#ifndef _SMTPCMDS_H
#define _SMTPCMDS_H

/*--------------------------------------------------------------------*/
/*       s m t p r e c v . h                                          */
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
 *       $Id: smtpcmds.h 1.1 1998/03/01 19:43:52 ahd Exp $
 *
 *       Revision History:
 *       $Log: smtpcmds.h $
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
