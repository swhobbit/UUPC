#ifndef _SMTPDNS_H
#define _SMTPDNS_H

/*--------------------------------------------------------------------*/
/*       s m t p d n s . h                                            */
/*                                                                    */
/*       TCP/IP domain lookup for UUPC/extended                       */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-2001 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: smtpdns.h 1.4 2000/05/12 12:42:31 ahd v1-13g $
 *
 *    $Log: smtpdns.h $
 *    Revision 1.4  2000/05/12 12:42:31  ahd
 *    Annual copyright update
 *
 *    Revision 1.3  1999/01/04 03:56:09  ahd
 *    Annual copyright change
 *
 *    Revision 1.2  1998/03/01 01:29:33  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.1  1997/11/28 23:13:35  ahd
 *    Initial revision
 *
 */

#include "smtpconn.h"

KWBoolean
getHostNameFromSocket( SMTPConnection *connection );

#endif /* _SMTPDNS_H */
