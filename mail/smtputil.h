#ifndef _SMTPUTIL_H
#define _SMTPUTIL_H

/*--------------------------------------------------------------------*/
/*       s m t p u t i l . c                                          */
/*                                                                    */
/*       SMTP commands utility functions                              */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1997 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *       $Id: smtprecv.c 1.1 1997/11/21 18:15:18 ahd Exp $
 *
 *       Revision History:
 *       $Log: smtprecv.c $
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
