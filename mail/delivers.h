#ifndef _DELIVERS_H
#define _DELIVERS_H
#define _DELIVERS_H

/*--------------------------------------------------------------------*/
/*       d e l i v e r s . h                                          */
/*                                                                    */
/*       Outbound SMTP delivery support for UUPC/extended             */
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
 *    $Id: delivers.h 1.3 1997/12/13 18:11:33 ahd v1-12u $
 *
 *    Revision history:
 *    $Log: delivers.h $
 *    Revision 1.3  1997/12/13 18:11:33  ahd
 *    Change parsing and passing of sender address information
 *
 *    Revision 1.2  1997/11/29 13:06:52  ahd
 *    Correct compiler warnings under OS/2, copyright notice
 *
 */

size_t DeliverSMTP( IMFILE *imf,          /* Input file name          */
                    const MAIL_ADDR *sender,  /* Originating address  */
                    const char *address,  /* Target address           */
                    const char *path);

size_t
ConnectSMTP(
   IMFILE *imf,                     /* Temporary input file          */
   const MAIL_ADDR *sender,         /* Originating address           */
   const char *targetHost,          /* SMTP host to connect to       */
   const char **toAddress,          /* List of target addressess     */
   int   count,                     /* Number of addresses           */
   const KWBoolean validate         /* Perform bounce on failure     */
);

#endif /* _DELIVERS_H */
