#ifndef _SMTPDNS_H
#define _SMTPDNS_H

/*--------------------------------------------------------------------*/
/*       s m t p d n s . h                                            */
/*                                                                    */
/*       TCP/IP domain lookup for UUPC/extended                       */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1998 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: smtpdns.h 1.1 1997/11/28 23:13:35 ahd v1-12u $
 *
 *    $Log: smtpdns.h $
 *    Revision 1.1  1997/11/28 23:13:35  ahd
 *    Initial revision
 *
 */

#include "smtpconn.h"

KWBoolean
getHostNameFromSocket( SMTPConnection *connection );

#endif /* _SMTPDNS_H */
