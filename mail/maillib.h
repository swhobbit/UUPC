/*--------------------------------------------------------------------*/
/*    maillib.h                                                       */
/*                                                                    */
/*    descriptions of mail user agent subroutines                     */
/*                                                                    */
/*    Changes copyright 1990, Andrew H. Derbyshire                    */
/*                                                                    */
/*    Change History:                                                 */
/*                                                                    */
/*       3 May 90 Create from mail.c                                  */
/*       8 Jul 90 Make LSIZE BUFSIZ                                   */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                        Function prototypes                         */
/*--------------------------------------------------------------------*/

boolean Pager(const int msgnum,
              boolean external,
              copyopt received,
              const boolean reset);

void PageReset( void );

boolean PageLine(char *line);

boolean CopyMsg(int msgnum, FILE *f, copyopt headers, boolean indent);

 void Sub_Pager(const char *tmailbag,
                        boolean builtin );

 boolean Numeric( const char *number);
                                 /* return 1 if number all digits,
                                    else return 0                    */

 boolean RetrieveLine(long adr, char *line, const size_t len);

 void    ReturnAddress(char *line, struct ldesc *ld);

void sayoptions( FLAGTABLE *flags);

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

extern FILE* fmailbox;

extern struct ldesc *letters;

extern int letternum;
