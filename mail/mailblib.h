
/*--------------------------------------------------------------------*/
/*          Function prototypes for UUPC/extended mailblib.c          */
/*--------------------------------------------------------------------*/

void ShowAlias( const char *alias);

KWBoolean DeliverMail( char *addresses , int item);

KWBoolean Reply( const int current );

int Position(int absolute, int relative, int letter);

KWBoolean SaveItem( const int letter,
               const KWBoolean remove,
               copyopt headers,
               char *fname,
               const ACTION verb);

KWBoolean ForwardItem( const int item , const char *string );

void subshell( char *command );

KWBoolean SetItem( int item );

KWBoolean SelectItems( char **input, int current , unsigned int bits);

KWBoolean Get_Operand( int *item,
                           char **token,
                           unsigned int bits,
                           KWBoolean first_pass );

KWBoolean SetTrailing( char **input, unsigned int bits );

int PushItemList( int **save_list );

void PopItemList( int *save_list, int save_item );
