
/*--------------------------------------------------------------------*/
/*          Function prototypes for UUPC/extended mailblib.c          */
/*--------------------------------------------------------------------*/

void ShowAlias( const char *alias);

boolean DeliverMail( char *addresses , int item);

boolean Reply( const int current );

int Position(int absolute, int relative, int letter);

boolean SaveItem( const int letter,
               const boolean delete,
               const copyopt headers,
               char *fname,
               const ACTION verb);

boolean ForwardItem( const int item , const char *string );

void subshell( char *command );

boolean SetItem( int item );

boolean SelectItems( char **input, int current , int bits);

boolean Get_Operand( int *item,
                           char **token,
                           int bits,
                           boolean first_pass );

boolean SetTrailing( char **input, int bits );

int PushItemList( int **save_list );

void PopItemList( int *save_list, int save_item );
