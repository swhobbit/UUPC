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
 *    $Id: getseq.h 1.7 1995/09/24 19:11:38 ahd Exp $
 *
 *    $Log: getseq.h $
 *    Revision 1.7  1995/09/24 19:11:38  ahd
 *    Use const parameters where possible
 *
 *    Revision 1.6  1995/02/12 23:39:45  ahd
 *    compiler cleanup, NNS C/news support, optimize dir processing
 *
 *    Revision 1.5  1994/12/22 00:12:52  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.4  1994/04/26 03:34:33  ahd
 *    Add log
 *
 */

#define SPOOLFMT "%c.%.8s%c%3.3s"
#define DATAFFMT "%c.%.7s0%3.3s%c"

unsigned long getSeq( void );

char *jobNumber( const unsigned long sequence,
                 const size_t length,
                 const KWBoolean monocase );
