#ifndef MLIB_H
#define MLIB_H

/*--------------------------------------------------------------------*/
/*       m l i b . h                                                  */
/*                                                                    */
/*       Interactive support functions for UUPC/extended mail         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-2002 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: mlib.h 1.14 2001/03/12 13:51:16 ahd v1-13k $
 *
 *    Revision history:
 *    $Log: mlib.h $
 *    Revision 1.14  2001/03/12 13:51:16  ahd
 *    Annual copyright update
 *
 *    Revision 1.13  2000/05/12 12:42:31  ahd
 *    Annual copyright update
 *
 *    Revision 1.12  1999/01/04 03:56:09  ahd
 *    Annual copyright change
 *
 *    Revision 1.11  1998/03/01 01:28:37  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.10  1997/04/24 01:10:57  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.9  1996/01/01 21:04:40  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.8  1995/01/07 16:19:52  ahd
 *    Change boolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.7  1994/12/22 00:20:28  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.6  1994/08/08 00:32:53  ahd
 *    Drop new session support from Invoke
 *
 * Revision 1.5  1994/01/01  19:13:40  ahd
 * Annual Copyright Update
 *
 * Revision 1.4  1993/11/13  17:52:31  ahd
 * Normalize external command processing
 *
 * Revision 1.3  1993/10/12  01:35:12  ahd
 * Normalize comments to PL/I style
 *
 * Revision 1.2  1993/07/31  16:28:59  ahd
 * Changes in support of Robert Denny's Windows Support
 *
 *
 *    01 Oct 89      Add prototypes for Console_fgets, Is_Console
 *    07 Sep 90      More prototypes
 */

extern int Get_One( void );

extern KWBoolean Console_fgets(char *buff,
                              int buflen, char *prompt);          /* ahd  */

int Invoke(const char *ecmd,
           const char *filename );

extern KWBoolean Is_Console(FILE *stream);                         /* ahd  */

extern void ClearScreen( void );                                  /* ahd  */

#endif
