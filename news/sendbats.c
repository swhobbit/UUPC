/*--------------------------------------------------------------------*/
/*    s e n d b a t s . c                                             */
/*                                                                    */
/*    Collect outgoing news into batchs and send out to system.       */
/*                                                                    */
/*    Written by Mike McLagan <mmclagan@invlogic.com>                 */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1995 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: sendbats.c 1.4 1995/01/15 19:48:35 ahd Exp $
 *
 *    Revision history:
 *    $Log: sendbats.c $
 *    Revision 1.4  1995/01/15 19:48:35  ahd
 *    Allow active file to be optional
 *    Delete fullbatch global option
 *    Add "local" and "batch" flags to SYS structure for news
 *
 *    Revision 1.3  1995/01/03 05:32:26  ahd
 *    Further SYS file support cleanup
 *
 *    Revision 1.2  1995/01/02 05:03:27  ahd
 *    Pass 2 of integrating SYS file support from Mike McLagan
 *
 *    Revision 1.1  1994/12/31 03:41:08  ahd
 *    First pass of integrating Mike McLagan's news SYS file suuport
 *
 */

#include "uupcmoah.h"

static const char rcsid[] =
            "$Id: sendbats.c 1.4 1995/01/15 19:48:35 ahd Exp $";

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include <ctype.h>
#include <time.h>
#include <fcntl.h>
#include <io.h>
#include <process.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "active.h"
#include "getopt.h"
#include "getseq.h"
#include "history.h"
#include "import.h"
#include "importng.h"
#include "logger.h"
#include "timestmp.h"
#include "title.h"
#include "batch.h"
#include "sys.h"

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

currentfile();

/*--------------------------------------------------------------------*/
/*    m a i n                                                         */
/*                                                                    */
/*    Main program                                                    */
/*                                                                    */
/*    Exit conditions                                                 */
/*                                                                    */
/*    0 - Success                                                     */
/*    1 - System configuration failed                                 */
/*    2 - Unable to open working file                                 */
/*    4 - out of memory                                               */
/*    5 - Unable to create history dbm file                           */
/*    6 - Problem decompressing news batch                            */
/*    7 - Unable to create cmprssed directory                         */
/*--------------------------------------------------------------------*/

#ifdef __TURBOC__
#pragma argsused
#endif

int main( int argc, char **argv)
{

   struct sys *node;

#if defined(__CORE__)
   copywrong = strdup(copyright);
   checkref(copywrong);
#endif

   banner( argv );

   if (!configure( B_BATCH ))
      exit(1);    /* system configuration failed */

   openlog( NULL );                 /* Begin logging to disk         */


   if ( ! init_sys() )
   {
      printmsg(0,"Cannot load news configuration (SYS) file" );
      exit(2);
   }

   node = sys_list;

/*--------------------------------------------------------------------*/
/*       Walk the list of known systems looking for systems to        */
/*       batch.  We can't handle batching by message id (well,        */
/*       we're too lazy to), all others we pass along.                */
/*--------------------------------------------------------------------*/

   while (node != NULL)
   {

     if (node->flag.I)           /* Unsupported on this end?      */
     {
         printmsg(0,"Flag I is not supported for system %s",
                    node->sysname);
         panic();
     }

     if (node->flag.B && (E_batchsize == 0))
     {
        E_batchsize = 60L * 1024L;    /* Provide reasonable default    */

        printmsg(0, "%s: Conflicting options fullbatch (B) and batchsize = 0, "
                     "using %ld for batch size",
                     argv[0],
                     E_batchsize );
     }

     if (node->flag.batch)
     {
         setTitle( "Batching news for %s", node->sysname );
         process_batch(node, node->sysname, node->command);
     }

     node = node -> next;

   } /* while (node != NULL) */

/*--------------------------------------------------------------------*/
/*                 Clean up and terminate processing                  */
/*--------------------------------------------------------------------*/

   exit_sys();

   return 0;

} /*main*/
