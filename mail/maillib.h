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
 *    $Id: maillib.h 1.3 1994/03/11 01:50:58 ahd v1-12k $
 *
 *    Revision history:
 *    $Log: maillib.h $
 *    Revision 1.3  1994/03/11 01:50:58  ahd
 *    Make mail box description array UUFAR
 *
 * Revision 1.2  1994/03/05  21:15:49  ahd
 * Revamp header copyrights, line buffer lengths
 *
 */

/*--------------------------------------------------------------------*/
/*                        Function prototypes                         */
/*--------------------------------------------------------------------*/

boolean Pager(const int msgnum,
              boolean external,
              copyopt received,
              const boolean reset);

void PageReset( void );

boolean PageLine(char *line);

boolean CopyMsg(const int msgnum,
                FILE *f,
                const copyopt headers,
                const boolean indent);

 void Sub_Pager(const char *tmailbag,
                        boolean builtin );

 boolean Numeric( const char *number);
                                 /* return 1 if number all digits,
                                    else return 0                    */

 boolean RetrieveLine(const long adr,
                      char *line,
                      const size_t len);

 void    ReturnAddress(char *line,
                       const long adr);

void sayoptions( FLAGTABLE *flags);

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

extern FILE* fmailbox;

extern struct ldesc UUFAR *letters;

extern int letternum;
