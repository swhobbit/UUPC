/*
   router.h

   routines included in router.c

   Copyright (c) 1989, Andrew H. Derbyshire

   Update History:

   30Sep89     Convert selected functions to type boolean            ahd
   18Mar90     Split out host name routines to hostable.h            ahd
 */

static struct AliasTable {
      char *anick;
      char *anode;
      char *auser;
      char *afull;
   };

void ExtractName( char *result, char *column );

void BuildAddress(char *result, const char *input );

char *AliasByNick(const char *input);

char *AliasByAddr(const char *node, const char *user);

boolean InitRouter( void );
