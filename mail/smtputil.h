#ifndef _SMTPUTIL_H
#define _SMTPUTIL_H

/*--------------------------------------------------------------------*/
/*       s m t p u t i l . c                                          */
/*                                                                    */
/*       SMTP commands utility functions                              */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1999 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *       $Id: smtputil.h 1.2 1998/03/01 01:27:58 ahd v1-13f ahd $
 *
 *       Revision History:
 *       $Log: smtputil.h $
 *       Revision 1.2  1998/03/01 01:27:58  ahd
 *       Annual Copyright Update
 *
 *       Revision 1.1  1997/11/24 02:53:26  ahd
 *       First working SMTP daemon which delivers mail
 *
 *
 */

KWBoolean
isValidAddress( const char *address,
                char buffer[MAXADDR],
                KWBoolean *ourProblem);

KWBoolean
isValidLocalAddress( const char *local );

KWBoolean
stripAddress( char *address, char *response );

#endif /* _SMTPUTIL_H */
