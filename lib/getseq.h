/*--------------------------------------------------------------------*/
/*       g e t s e q . h                                              */
/*                                                                    */
/*       Header file for get sequence and related functions           */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1994 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: getseq.h 1.2 1993/12/02 01:37:33 ahd Exp $
 */

#define SPOOLFMT "%c.%.8s%c%3.3s"
#define DATAFFMT "%c.%.7s0%3.3s%c"

long getseq( void );

char *JobNumber( long sequence );
