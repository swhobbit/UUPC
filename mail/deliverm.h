#ifndef _DELIVERM_H
#define _DELIVERM_H

/*--------------------------------------------------------------------*/
/*       d e l i v e r m . h                                          */
/*                                                                    */
/*       Misc Subroutines supporting outbound mail delivery           */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1999 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: deliverm.h 1.2 1998/03/01 01:31:15 ahd v1-13f ahd $
 *
 *    Revision history:
 *    $Log: deliverm.h $
 *    Revision 1.2  1998/03/01 01:31:15  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.1  1997/12/13 18:11:33  ahd
 *    Initial revision
 *
 */

#include "mailaddr.h"
#include "imfile.h"

char
*formatFileSize( IMFILE *imf );

void
uniqueMailBoxName( const char *user,
                   char mboxname[FILENAME_MAX] );

KWBoolean
putFromLine( const MAIL_ADDR *sender,
             const KWBoolean remoteDelivery,
             FILE *dataOut);

KWBoolean
CopyData( IMFILE *imf,              /* Input temporary file          */
          const MAIL_ADDR *sender,
          const KWBoolean remoteDelivery,
          FILE *dataOut);           /* Output file - spool or mbox   */

size_t
queueRemote( IMFILE *imf,           /* Input file                    */
             const MAIL_ADDR *sender,  /* Originating user@host      */
             const char *command,   /* Target address                */
             const char *path,      /* Node to queue for             */
             const char grade );    /* UUCP call grade for message   */

#endif /* _DELIVERM_H */
