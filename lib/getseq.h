/*--------------------------------------------------------------------*/
/*       g e t s e q . h                                              */
/*                                                                    */
/*       Header file for get sequence and related functions           */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1995 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: getseq.h 1.5 1994/12/22 00:12:52 ahd Exp $
 *
 *    $Log: getseq.h $
 *    Revision 1.5  1994/12/22 00:12:52  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.4  1994/04/26 03:34:33  ahd
 *    Add log
 *
 */

#define SPOOLFMT "%c.%.8s%c%3.3s"
#define DATAFFMT "%c.%.7s0%3.3s%c"

long getSeq( void );

char *jobNumber( long sequence,
                 size_t length,
                 const KWBoolean monocase );
