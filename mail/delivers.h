#ifndef _DELIVERS_H
#define _DELIVERS_H
#define _DELIVERS_H

/*--------------------------------------------------------------------*/
/*       d e v l i v e r . h                                          */
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
 *    $Id: lib.h 1.40 1997/06/03 03:26:38 ahd v1-12t $
 *
 *    Revision history:
 *    $Log: lib.h $
 */

size_t
ConnectSMTP(
   IMFILE *imf,                     /* Temporary input file          */
   const char *relay,               /* SMTP host to connect to       */
   const char *fromAddress,         /* Originating (error) address   */
   const char **toAddress,          /* List of target addressess     */
   int   count,                     /* Number of addresses           */
   const KWBoolean validate         /* Perform bounce on failure     */
);

#endif /* _DELIVERS_H */
