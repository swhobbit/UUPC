/*--------------------------------------------------------------------*/
/*     t e s t s e c . c                                              */
/*                                                                    */
/*    Test permissions file for UUPC/extended                         */
/*                                                                    */
/*    Copyright (c) 1991, Andrew H. Derbyshire                        */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "lib.h"
#include "expath.h"
#include "hlib.h"
#include "hostable.h"
#include "usertabl.h"
#include "security.h"

currentfile();

extern struct HostTable *hostp = NULL;
extern struct UserTable *userp = NULL;

void main( int argc, char **argv)
{
   configure( B_UUIO );
   checkuser( E_homedir );
   checkreal( E_nodename );

   debuglevel = 10;

   if (!LoadSecurity())
      panic();

   hostp = checkreal( *(++argv) );
   securep = GetSecurity( hostp );

   while( --argc > 1 )
   {
      char path[FILENAME_MAX];

      ValidateFile( expand_path( strcpy( path, *(++argv)), ".", E_pubdir , NULL),
                    argc % 2 );
   }
} /* main */
