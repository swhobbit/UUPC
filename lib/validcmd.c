/*--------------------------------------------------------------------*/
/*    v a l i d c m d . c                                             */
/*                                                                    */
/*    Security routines for UUPC/extended                             */
/*                                                                    */
/*    Copyright (c) 1991, Andrew H. Derbyshire                        */
/*    See README.PRN for additional copyrights and restrictions       */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>        /* Only really needed for MS C         */
#include <sys/stat.h>
#include <time.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "hostable.h"
#include "security.h"
#include "usertabl.h"
#include "expath.h"
#include "hlib.h"

currentfile();
/*--------------------------------------------------------------------*/
/*    V a l i d a t e C o m m a n d                                   */
/*                                                                    */
/*    Determine if a command is allowed for a host                    */
/*--------------------------------------------------------------------*/

boolean ValidateCommand( const char *command)
{
   char **p;

/*--------------------------------------------------------------------*/
/*                Validate the security table is okay                 */
/*--------------------------------------------------------------------*/

   if ( securep == NULL )
      panic();

/*--------------------------------------------------------------------*/
/*                        Handle local system                         */
/*--------------------------------------------------------------------*/

   if ( securep->local )      /* Local system?                       */
   {

#ifdef UDEBUG
      printmsg( 5, "ValidateCommand: Local system, command \"%s\" allowed",
            command );
#endif

      return TRUE;            /* Yes --> Bless the request           */
   }

/*--------------------------------------------------------------------*/
/*     Loop through security command table looking for the target     */
/*--------------------------------------------------------------------*/

   p = securep->commands;
   while (*p != NULL)
   {
      boolean global;

#ifdef UDEBUG
      printmsg(10,"ValidateCommand: Comparing \"%s\" to \"%s\"",
               *p, command );
#endif

      if equal(*p, ANY_COMMAND )
         global = TRUE;
      else
         global = FALSE;

      if (global || equali(*p, command ))
      {
         printmsg(5,"ValidateCommand: Command \"%s\" %splicitly allowed",
                  command, global ? "im" : "ex" );
         return TRUE;
      }
      p++ ;
   } /* while */

/*--------------------------------------------------------------------*/
/*               We didn't find the command; reject it                */
/*--------------------------------------------------------------------*/

   printmsg(5,"ValidateCommand: Command \"%s\" not allowed",
            command );
   return FALSE;

} /* ValidateCommand */
