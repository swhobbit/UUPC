/*--------------------------------------------------------------------*/
/*    h o s t r s e t . c                                             */
/*                                                                    */
/*    Clear host status information for UUPC/extended                 */
/*                                                                    */
/*    Copyright (c) 1989-1993, Andrew H. Derbyshire                   */
/*    Copyright (c) 1992, Mitch Mitchell                              */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <time.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "hlib.h"
#include "hostable.h"
#include "hostatus.h"
#include "hostrset.h"
#include "security.h"
#include "timestmp.h"

/*--------------------------------------------------------------------*/
/*        Define current file name for panic() and printerr()         */
/*--------------------------------------------------------------------*/

currentfile();

/*--------------------------------------------------------------------*/
/*       H o s t R e s e t                                            */
/*                                                                    */
/*       Reset status information for one or more hosts               */
/*--------------------------------------------------------------------*/

void HostReset( const char *name )
{
   boolean firsthost = TRUE;
   struct HostTable *host;
   char fname[FILENAME_MAX];
   FILE *stream;
   unsigned short len1 = strlen(compilep );
   unsigned short len2 = strlen(compilev );

/*--------------------------------------------------------------------*/
/*         Get the file name for the status file and open it          */
/*--------------------------------------------------------------------*/

   mkfilename( fname, E_spooldir, DCSTATUS );


   if ((stream  = FOPEN(fname , "w", BINARY_MODE)) == NULL)
   {
      printmsg(1,"HostReset: Unable to open host status file");
      printerr( fname );
      panic();                /* Critical error if unable to write   */
      return;
   } /* if */

/*--------------------------------------------------------------------*/
/*                 Write header information for file                  */
/*--------------------------------------------------------------------*/

   fwrite( &len1, sizeof len1, 1, stream );
   fwrite( &len2, sizeof len2, 1, stream );
   fwrite( compilep , 1, len1, stream);
   fwrite( compilev , 1, len2, stream);
   fwrite( &start_stats , sizeof start_stats , 1,  stream);

/*--------------------------------------------------------------------*/
/*     Now spin through the hosts and write out their information     */
/*--------------------------------------------------------------------*/

   while  ((host = nexthost( firsthost )) != BADHOST)
   {
      len1 = strlen( host->hostname );
      len2 = sizeof *(host->hstats);

      firsthost = FALSE;

      fwrite( &len1, sizeof len1, 1, stream );
      fwrite( &len2, sizeof len2, 1, stream );
      fwrite( host->hostname , sizeof host->hostname[0], len1, stream);

/*--------------------------------------------------------------------*/
/*                    Clear this host if requested                    */
/*--------------------------------------------------------------------*/

      if ( (name == NULL) || equal(name,host->hostname) )
      {
          host->hstats->calls     = 0l;  /* Total number of calls to host       */
          host->hstats->connect   = 0l;  /* Total length of connections to host */
          host->hstats->fsent     = 0l;  /* Total files sent to this host       */
          host->hstats->freceived = 0l;  /* Total files received from this host */
          host->hstats->bsent     = 0l;  /* Total bytes sent to this host       */
          host->hstats->breceived = 0l;  /* Total bytes received from this host */
          host->hstats->errors    = 0l;  /* Total transmission errors noted     */
          host->hstats->packets   = 0l;  /* Total packets exchanged             */
      }

      host->hstats->save_hstatus = host->hstatus;
      fwrite( host->hstats , len2, 1,  stream);
   }

/*--------------------------------------------------------------------*/
/*         Make we sure got end of file and not an I/O error          */
/*--------------------------------------------------------------------*/

   if (ferror( stream ))
   {
      printerr( fname );
      clearerr( stream );
   }
   fclose( stream );

} /* HostReset */
