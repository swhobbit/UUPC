/*--------------------------------------------------------------------*/
/*    a d d r e s s . h                                               */
/*                                                                    */
/*    Routines in address.c                                           */
/*--------------------------------------------------------------------*/

void user_at_node( const char *address,
                  char *hispath,
                  char *hisnode,
                  char *hisuser );

char *HostAlias( char *host);

char *HostPath( char *host, char *best);

typedef enum {
      ADDRESSONLY = FALSE,
      FULLNAMEONLY = TRUE,
      FULLADDRESS }
      FULLNAME;

char *ExtractAddress( char *result, const char *column, FULLNAME fullname );
