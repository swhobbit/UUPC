/*--------------------------------------------------------------------*/
/*       d e l i v e r . h                                            */
/*                                                                    */
/*       Externally known functions in deliver.c, delivery            */
/*       subroutines in rmail component of UUPC/extended              */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-2000 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: deliver.h 1.26 1999/01/11 05:44:47 ahd Exp $
 *
 *    $Log: deliver.h $
 *    Revision 1.26  1999/01/11 05:44:47  ahd
 *    Delete obsolete SMTP retry routine
 *
 *    Revision 1.25  1999/01/04 03:56:09  ahd
 *    Annual copyright change
 *
 *    Revision 1.24  1998/03/01 01:31:21  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.23  1997/12/13 18:11:33  ahd
 *    Change parsing and passing of sender address information
 *
 *    Revision 1.22  1997/11/24 02:53:26  ahd
 *    First working SMTP daemon which delivers mail
 *
 *    Revision 1.21  1997/05/11 18:17:27  ahd
 *    Add flag for fastsmtp delivery
 *    Move TCP/IP dependent delivery code from rmail to deliver
 *
 *    Revision 1.20  1997/05/11 04:28:53  ahd
 *    SMTP client support for RMAIL/UUXQT
 *
 *    Revision 1.19  1997/04/24 01:10:40  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.18  1996/11/19 00:25:20  ahd
 *    Externalize functions
 *
 *    Revision 1.17  1996/01/01 21:04:46  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.16  1995/09/11 00:24:49  ahd
 *    Delete non-existent variable
 *
 *    Revision 1.15  1995/01/09 01:43:38  ahd
 *    Optimize UUCP remote queuing
 *
 *    Revision 1.14  1995/01/08 19:54:01  ahd
 *    Add in-memory files to RMAIL
 *
 *    Revision 1.13  1995/01/07 16:19:37  ahd
 *    Change Boolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 */

#include "imfile.h"
#include "mailaddr.h"

#define POSTMASTER "postmaster"

size_t Deliver( IMFILE *imf,           /* Input file name            */
                const MAIL_ADDR *sender,
                const char *address,   /* Target address             */
                KWBoolean validate);   /* Validate/forward local mail*/

size_t
DeliverRemote( IMFILE *imf,               /* Input file name      */
               const MAIL_ADDR *sender,
               const char *address,       /* Target address       */
               const char *path);

size_t Bounce( IMFILE *imf,
               const MAIL_ADDR *sender,
               const char *text,
               const char *data,
               const char *address,
               const KWBoolean validate );

void
flushQueues( IMFILE *imf,
             const MAIL_ADDR *sender );

void
setDeliveryGrade( const char inGrade );

extern KEWSHORT hops;

extern char *myProgramName;
