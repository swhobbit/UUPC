/*--------------------------------------------------------------------*/
/*    s c r i p t . h                                                 */
/*                                                                    */
/*    Script processing routines used by UUPC/extended                */
/*                                                                    */
/*    Changes copyright (c) 1989, 1991, Andrew H. Derbyshire          */
/*--------------------------------------------------------------------*/

int expectstr(char *Search, unsigned int Timeout, char **failure);

boolean sendstr(char  *str, unsigned int timeout, char **failure);

void echoCheck( const unsigned int timeout );
