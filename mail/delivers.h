#ifndef _DELIVERS_H
#define _DELIVERS_H
#define _DELIVERS_H

/*--------------------------------------------------------------------*/
/*       d e l i v e r s . h                                          */
/*                                                                    */
/*       Outbound SMTP delivery support for UUPC/extended             */
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
 *    $Id: delivers.h 1.7 2000/05/12 12:44:47 ahd v1-13g $
 *
 *    Revision history:
 *    $Log: delivers.h $
 *    Revision 1.7  2000/05/12 12:44:47  ahd
 *    Annual copyright update
 *
 *    Revision 1.6  1999/01/11 05:44:47  ahd
 *    Hide internal SMTP routines
 *
 *    Revision 1.5  1999/01/04 03:56:09  ahd
 *    Annual copyright change
 *
 *    Revision 1.4  1998/03/01 01:31:10  ahd
 *    Annual Copyright Update
 *
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

#endif /* _DELIVERS_H */
