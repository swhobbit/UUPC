/*--------------------------------------------------------------------*/
/*    d e l i v e r . h                                               */
/*                                                                    */
/*   Externally known functions in deliver.c, delivery subroutines    */
/*                in rmail component of UUPC/extended                 */
/*--------------------------------------------------------------------*/

#define POSTMASTER "postmaster"

size_t Deliver( const char *input, /* Input file name                */
             char *address,           /* Target address                 */
                   boolean validate); /* Validate/forward local mail    */

extern INTEGER hops;

extern boolean remoteMail;

extern char *now;

extern char fromuser[];
extern char fromnode[];

extern  char *ruser;
extern  char *rnode;
extern  char *uuser;
