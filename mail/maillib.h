/*--------------------------------------------------------------------*/
/*    m a i l l i b . h                                               */
/*                                                                    */
/*    descriptions of mail user agent subroutines                     */
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
 *    $Id: maillib.h 1.5 1995/01/07 16:19:44 ahd Exp $
 *
 *    Revision history:
 *    $Log: maillib.h $
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

void sayoptions( FLAGTABLE *flags);

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

extern IMFILE *imBox;

extern struct ldesc UUFAR *letters;

extern int letternum;
