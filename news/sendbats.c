/*--------------------------------------------------------------------*/
/*    s e n d b a t s . c                                             */
/*                                                                    */
/*    Collect outgoing news into batchs and send out to system.       */
/*                                                                    */
/*    Written by Mike McLagan <mmclagan@invlogic.com>                 */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-2001 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: sendbats.c 1.14 2000/05/12 12:26:40 ahd v1-13g $
 *
 *    Revision history:
 *    $Log: sendbats.c $
 *    Revision 1.14  2000/05/12 12:26:40  ahd
 *    Annual copyright update
 *
 *    Revision 1.13  1999/01/08 02:20:52  ahd
 *    Convert currentfile() to RCSID()
 *
 *    Revision 1.12  1999/01/04 03:52:55  ahd
 *    Annual copyright change
 *
 *    Revision 1.11  1998/03/01 01:30:36  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.10  1997/04/24 01:39:41  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.9  1996/11/18 04:46:49  ahd
 *    Normalize arguments to bugout
 *    Reset title after exec of sub-modules
 *    Normalize host status names to use HS_ prefix
 *
 *    Revision 1.8  1996/01/01 21:06:31  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.7  1995/02/12 23:37:04  ahd
 *    compiler cleanup, NNS C/news support, optimize dir processing
 *
 *    Revision 1.6  1995/01/29 14:03:29  ahd
 *    Clean up IBM C/Set compiler warnings
 *
 *    Revision 1.5  1995/01/22 04:16:52  ahd
 *    Batching cleanup
 *
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

RCSID("$Id: sendbats.c 1.14 2000/05/12 12:26:40 ahd v1-13g $");

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
#include "getopt.h"

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    m a i n                                                         */
/*                                                                    */
/*    Main program                                                    */
/*                                                                    */
/*    Exit conditions                                                 */
/*                                                                    */
/*    0  - Success                                                    */
/*    69 - We paniced, usually because of error in system call        */
/*--------------------------------------------------------------------*/

main( int argc, char **argv)
{

   struct sys *node;

#if defined(__CORE__)
   copywrong = strdup(copyright);
   checkref(copywrong);
#endif

   banner( argv );

   if (!configure( B_SENDBATS ))
      exit(1);    /* system configuration failed */

   openlog( NULL );                 /* Begin logging to disk         */

   if (argc > 1)
   {
      int option;

       while ((option = getopt(argc, argv, "x:")) != EOF)
       {
           switch (option)
           {
               case 'x':
                  debuglevel = atoi(optarg);
                  break;

               case '?':
                  fprintf(stderr, "\nUsage:\t%s [-x debug]", argv[0] );
                  exit(99);

           } /* break */

       } /* while */

/*--------------------------------------------------------------------*/
/*                Abort if any options were left over                 */
/*--------------------------------------------------------------------*/

       if (optind != argc)
       {
          puts("Extra parameter(s) at end.");
          exit(98);
       }

    } /* if (argc > 1) */

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

} /* main */
