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
 *    $Id: sendbats.c 1.1 1994/12/31 03:41:08 ahd Exp $
 *
 *    Revision history:
 *    $Log: sendbats.c $
 *    Revision 1.1  1994/12/31 03:41:08  ahd
 *    First pass of integrating Mike McLagan's news SYS file suuport
 *
 */

#include "uupcmoah.h"

static const char rcsid[] =
            "$Id: sendbats.c 1.1 1994/12/31 03:41:08 ahd Exp $";

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

   struct sys *sysnode;

#if defined(__CORE__)
   copywrong = strdup(copyright);
   checkref(copywrong);
#endif

   banner( argv );

   if (!configure( B_BATCH ))
      exit(1);    /* system configuration failed */

   openlog( argv[0] );        /* Begin logging to disk         */

   if (bflag[F_FULLBATCH] && (E_batchsize == 0))
   {
      E_batchsize = 60L * 1024L;    /* Provide reasonable default    */

      printmsg(0, "%s: Conflicting options fullbatch and batchsize = 0, "
                   "using %ld for batch size",
                   argv[0],
                   E_batchsize );
   }

   init_sys();

   sysnode = sys_list;

   while (sysnode != NULL)
   {

     printmsg(0,"SENDBATS: Batching news for system %s",sysnode->sysname);

     /* skip us! */

     if (equal(E_nodename,sysnode->sysname))
     {
       sysnode = sysnode -> next;
       continue;
     }

     /*
      * lets see...  check the flags, if there are none, we shouldn't be
      * here for this system, so continue! (news is either sent one
      * article at a time, or by batch!
      */

     /*
      * the only flags of interest are 'fFIn' and only 1 can be set, or
      * we wouldn't be here!!
      * the other flags 'muLn' are RNEWS's problem, not ours!
      */

     if (sysnode->flag.f || sysnode->flag.F || sysnode->flag.n)
       process_batch(sysnode, sysnode->sysname, sysnode->command);
     else if (sysnode->flag.I)   /* Unsupported on this end?         */
     {
            printmsg(0,"Flag I is not handled for system %s",sysnode->sysname);
            panic();
    }

     sysnode = sysnode -> next;

   } /* while (sysnode != NULL) */

/*--------------------------------------------------------------------*/
/*                 Clean up and terminate processing                  */
/*--------------------------------------------------------------------*/

   exit_sys();

   return 0;

} /*main*/
