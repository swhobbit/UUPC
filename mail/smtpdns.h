#ifndef _SMTPDNS_H
#define _SMTPDNS_H

/*--------------------------------------------------------------------*/
/*       s m t p d n s . h                                            */
/*                                                                    */
/*       TCP/IP domain lookup for UUPC/extended                       */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1997 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: smtpnetw.c 1.5 1997/11/28 04:52:10 ahd Exp $
 *
 *    $Log: smtpnetw.c $
 */

#include "smtpconn.h"

KWBoolean
getHostNameFromSocket( SMTPConnection *connection );

#endif /* _SMTPDNS_H */
