/*--------------------------------------------------------------------*/
/*    g e t s e q . h                                                 */
/*                                                                    */
/*    Header file for get sequence and related functions              */
/*--------------------------------------------------------------------*/

#define SPOOLFMT "%c.%.8s%c%3.3s"
#define DATAFFMT "%c.%.8s%3.3s%c"

long getseq( void );

char *JobNumber( long sequence );
