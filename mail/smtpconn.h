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
 *    $Id: smtpnetw.c 1.5 1997/11/28 04:52:10 ahd Exp $
 *
 *    $Log: smtpnetw.c $
 */

#include "../uucico/uutcpip.h"
#include "smtpconn.h"

typedef struct _SMTPConnection
{
   int handle;                      /* TCP/IP socket handle          */
   unsigned long IPAddr;            /* IP Address in binary          */
   char hostName[MAXADDR];          /* Name DNS reports client to be */
   char hostAddr[30];               /* Host IP address as string     */
   KWBoolean reverseLookup;         /* KWTrue = DNS could resolve adr*/
} SMTPConnection;

#endif /* _SMTPCONN_H */
