#ifndef _SMTPCONN_H
#define _SMTPCONN_H

/*--------------------------------------------------------------------*/
/*       s m t p d n s . c                                            */
/*                                                                    */
/*       TCP/IP domain lookup for UUPC/extended                       */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: SMTPCONN.H 1.2 1997/11/28 23:33:43 ahd v1-12v $
 *
 *    $Log: SMTPCONN.H $
 *    Revision 1.2  1997/11/28 23:33:43  ahd
 *    Delete recursive include
 *
 *    Revision 1.1  1997/11/28 23:13:35  ahd
 *    Initial revision
 *
 */

#include "../uucico/uutcpip.h"

typedef struct _SMTPConnection
{
   int handle;                      /* TCP/IP socket handle          */
   unsigned long IPAddr;            /* IP Address in binary          */
   char hostName[MAXADDR];          /* Name DNS reports client to be */
   char hostAddr[30];               /* Host IP address as string     */
   KWBoolean reverseLookup;         /* KWTrue = DNS could resolve adr*/
   int error;                       /* Last error code (zero if none)*/
} SMTPConnection;

#endif /* _SMTPCONN_H */
