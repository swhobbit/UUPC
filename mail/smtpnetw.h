#ifndef _SMTPNETW_H
#define _SMTPNETW_H

/*--------------------------------------------------------------------*/
/*       s m t p n e t w . h                                          */
/*                                                                    */
/*       SMTP verb parser for UUPC/extended                           */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1999 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: smtpnetw.h 1.7 1999/01/17 17:21:32 ahd Exp $
 *
 *    $Log: smtpnetw.h $
 *    Revision 1.7  1999/01/17 17:21:32  ahd
 *    Add test for one socket being ready
 *
 *    Revision 1.6  1999/01/04 03:56:09  ahd
 *    Annual copyright change
 *
 *    Revision 1.5  1998/04/24 03:34:25  ahd
 *    Use local buffers for output
 *    Add flag bits to verb table
 *    Add flag bytes to client structure
 *
 *    Revision 1.4  1998/03/01 01:28:19  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.3  1997/11/29 13:06:52  ahd
 *    Allow explicit invocation of network initialization
 *
 *    Revision 1.2  1997/11/24 02:53:26  ahd
 *    First working SMTP daemon which delivers mail
 *
 *    Revision 1.1  1997/11/21 18:16:32  ahd
 *    Command processing stub SMTP daemon
 *
 */

#include "smtpclnt.h"

KWBoolean
selectReadySockets( SMTPClient *master );

KWBoolean
isSocketReady(SMTPClient *current, time_t timeout);

KWBoolean
SMTPGetLine( SMTPClient *client );

KWBoolean
SMTPResponse( SMTPClient *client, int code, const char *text );

#define XMIT_LENGTH  KW_BUFSIZ

#endif /* _SMTPNETW_H */
