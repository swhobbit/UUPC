/*--------------------------------------------------------------------*/
/*    m a i l l i b . h                                               */
/*                                                                    */
/*    descriptions of mail user agent subroutines                     */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1999 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: maillib.h 1.11 1998/03/01 01:30:38 ahd v1-13f ahd $
 *
 *    Revision history:
 *    $Log: maillib.h $
 *    Revision 1.11  1998/03/01 01:30:38  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.10  1997/04/24 01:10:51  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.9  1996/11/19 00:25:20  ahd
 *    Revise LDESC declare
 *
 *    Revision 1.8  1996/01/04 04:01:44  ahd
 *    Use sorted table for boolean options with binary search
 *
 *    Revision 1.7  1996/01/01 21:04:52  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.6  1995/02/20 17:30:32  ahd
 *    16 bit compiler warning cleanup
 *
 *    Revision 1.5  1995/01/07 16:19:44  ahd
 *    Change KWBoolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.4  1994/12/22 00:20:22  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.3  1994/03/11 01:50:58  ahd
 *    Make mail box description array UUFAR
 *
 * Revision 1.2  1994/03/05  21:15:49  ahd
 * Revamp header copyrights, line buffer lengths
 *
 */

#include "imfile.h"

/*--------------------------------------------------------------------*/
/*                        Function prototypes                         */
/*--------------------------------------------------------------------*/

KWBoolean Pager(const int msgnum,
              KWBoolean external,
              copyopt received,
              const KWBoolean reset);

void PageReset( void );

KWBoolean PageLine(char *line);

KWBoolean CopyMsg(const int msgnum,
                FILE *f,
                const copyopt headers,
                const KWBoolean indent);

 void Sub_Pager(const char *tmailbag,
                        KWBoolean builtin );

 KWBoolean Numeric( const char *number);
                                 /* return 1 if number all digits,
                                    else return 0                    */

 KWBoolean RetrieveLine(const long adr,
                      char *line,
                      const size_t len);

 void    ReturnAddress(char *line,
                       const long adr);

void sayOptions( FLAGTABLE *flags, const size_t size);

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

extern IMFILE *imBox;

extern LDESC UUFAR  *letters;

extern int letternum;
