/*--------------------------------------------------------------------*/
/*       g e t s e q . h                                              */
/*                                                                    */
/*       Header file for get sequence and related functions           */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1993 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: lib.h 1.16 1993/11/06 17:57:46 rhg Exp $
 */

#define SPOOLFMT "%c.%.8s%c%3.3s"
#define DATAFFMT "%c.%.7s0%3.3s%c"

long getseq( void );

char *JobNumber( long sequence );
