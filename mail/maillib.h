/*--------------------------------------------------------------------*/
/*    m a i l l i b . h                                               */
/*                                                                    */
/*    descriptions of mail user agent subroutines                     */
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
 *    $Id: lib.h 1.20 1994/02/20 19:16:21 ahd Exp $
 *
 *    Revision history:
 *    $Log: lib.h $
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

extern struct ldesc *letters;

extern int letternum;
