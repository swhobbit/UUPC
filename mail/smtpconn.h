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
 *    $Id: smtpconn.h 1.4 1999/01/17 17:21:32 ahd Exp $
 *
 *    $Log: smtpconn.h $
 *    Revision 1.4  1999/01/17 17:21:32  ahd
 *    Add test for one socket being ready
 *
 *    Revision 1.3  1998/04/08 11:36:31  ahd
 *    Alter socket error processing
 *
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
   KWBoolean localhost;             /* KWTrue = host is local        */
   int error;                       /* Last error code (zero if none)*/
   FILE *stream;                    /* Batch mode file input         */
} SMTPConnection;

#define BATCH_HANDLE -2

#endif /* _SMTPCONN_H */
