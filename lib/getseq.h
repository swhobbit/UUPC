/*--------------------------------------------------------------------*/
/*       g e t s e q . h                                              */
/*                                                                    */
/*       Header file for get sequence and related functions           */
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
 *    $Id: getseq.h 1.13 2000/05/12 12:40:50 ahd v1-13g $
 *
 *    $Log: getseq.h $
 *    Revision 1.13  2000/05/12 12:40:50  ahd
 *    Annual copyright update
 *
 *    Revision 1.12  1999/01/04 03:55:03  ahd
 *    Annual copyright change
 *
 *    Revision 1.11  1998/03/01 01:26:34  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.10  1997/03/31 06:59:32  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.9  1996/01/01 20:57:09  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.8  1995/09/26 00:38:35  ahd
 *    Use unsigned sequence number for jobs to prevent mapping errors
 *
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
