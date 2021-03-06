/*--------------------------------------------------------------------*/
/*    File transfer information for UUPC/extended                     */
/*                                                                    */
/*    Copyright (c) 1991, Andrew H. Derbyshire                        */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-2002 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*
 *       $Id: dcpstats.c 1.23 2001/03/12 13:55:29 ahd v1-13k $
 *
 *       $Log: dcpstats.c $
 *       Revision 1.23  2001/03/12 13:55:29  ahd
 *       Annual copyright update
 *
 *       Revision 1.22  2000/05/12 12:32:55  ahd
 *       Annual copyright update
 *
 *       Revision 1.21  1999/01/08 02:20:56  ahd
 *       Convert currentfile() to RCSID()
 *
 *       Revision 1.20  1999/01/04 03:53:57  ahd
 *       Annual copyright change
 *
 *       Revision 1.19  1998/03/01 01:39:34  ahd
 *       Annual Copyright Update
 *
 *       Revision 1.18  1997/04/24 01:33:46  ahd
 *       Annual Copyright Update
 *
 *       Revision 1.17  1996/11/18 04:46:49  ahd
 *       Normalize arguments to bugout
 *       Reset title after exec of sub-modules
 *       Normalize host status names to use HS_ prefix
 *
 *       Revision 1.16  1996/03/18 03:55:08  ahd
 *       Use character which doesn't need mapping lock name
 *
 *       Revision 1.15  1996/01/01 21:21:10  ahd
 *       Annual Copyright Update
 *
 *       Revision 1.14  1995/01/07 16:38:32  ahd
 *       Change boolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *       Revision 1.13  1994/12/22 00:35:06  ahd
 *       Annual Copyright Update
 *
 *       Revision 1.12  1994/03/11 01:49:45  ahd
 *       Don't backup the host status file before updates
 *
 * Revision 1.11  1994/02/26  17:22:04  ahd
 * Change BINARY_MODE to IMAGE_MODE to avoid IBM C/SET 2 conflict
 *
 * Revision 1.10  1994/02/20  19:11:18  ahd
 * IBM C/Set 2 Conversion, memory leak cleanup
 *
 * Revision 1.9  1994/02/19  05:07:18  ahd
 * Use standard first header
 *
 * Revision 1.8  1994/01/01  19:19:13  ahd
 * Annual Copyright Update
 *
 * Revision 1.7  1993/10/09  22:21:55  rhg
 * ANSIfy source
 *
 * Revision 1.7  1993/10/09  22:21:55  rhg
 * ANSIfy source
 *
 * Revision 1.6  1993/09/20  04:41:54  ahd
 * OS/2 2.x support
 *
 * Revision 1.5  1993/04/11  00:34:11  ahd
 * Global edits for year, TEXT, etc.
 *
 * Revision 1.4  1993/03/06  23:04:54  ahd
 * Lock host status file before updating
 *
 * Revision 1.3  1992/12/30  13:17:12  ahd
 * Windows/NT changes
 *
 */

/*--------------------------------------------------------------------*/
/*                       standard include files                       */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"

#include "dcp.h"
#include "dcpstats.h"
#include "stater.h"
#include "hostable.h"
#include "hostatus.h"
#include "security.h"
#include "timestmp.h"
#include "ssleep.h"
#include "lock.h"

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

RCSID("$Id: dcpstats.c 1.23 2001/03/12 13:55:29 ahd v1-13k $");

/*--------------------------------------------------------------------*/
/*    d c s t a t s                                                   */
/*                                                                    */
/*    Report transmission information for a connection                */
/*--------------------------------------------------------------------*/

void dcstats(void)
{
   if (hostp == BADHOST)
   {
      printmsg(0,"dcstats: host structure pointer is NULL");
      panic();
   }

   if (!equal(rmtname , hostp->hostname))
      return;

   if (remote_stats.lconnect > 0)
   {
      time_t connected;
      unsigned long bytes;
      unsigned long bps;

      HostStatus();

      connected = time(NULL) - remote_stats.lconnect;
      remote_stats.connect += connected;
      bytes = remote_stats.bsent + remote_stats.breceived;

      if ( connected > 0 )
         bps = bytes / (long unsigned) connected;
      else
         bps = 0;

      printmsg(0,"%ld files sent, %ld files received, "
                 "%ld bytes sent, %ld bytes received",
            remote_stats.fsent, remote_stats.freceived ,
            remote_stats.bsent, remote_stats.breceived);
      printmsg(0, "%ld packets transferred, %ld errors, "
                  "connection time %ld:%02ld, %ld bytes/second",
            (long) remote_stats.packets,
            (long) remote_stats.errors,
            (long) connected / 60L, (long) connected % 60L, bps);

   } /*if */
   else
      printmsg(0,"Unable to print information for host %s (%s)",
            rmtname,
            hostp->hostname);

   if (remote_stats.lconnect > hostp->status.lconnect)
      hostp->status.lconnect = remote_stats.lconnect;

   if (remote_stats.ltime > hostp->status.ltime)
      hostp->status.lconnect = remote_stats.lconnect;

   hostp->status.connect   += remote_stats.connect;
   hostp->status.calls     += remote_stats.calls;
   hostp->status.fsent     += remote_stats.fsent;
   hostp->status.freceived += remote_stats.freceived;
   hostp->status.bsent     += remote_stats.bsent;
   hostp->status.breceived += remote_stats.breceived;
   hostp->status.errors    += remote_stats.errors;
   hostp->status.packets   += remote_stats.packets;

} /* dcstats */

/*--------------------------------------------------------------------*/
/*    d c u p d a t e                                                 */
/*                                                                    */
/*    Update the status of all known hosts                            */
/*--------------------------------------------------------------------*/

void dcupdate( void )
{
   KWBoolean firsthost = KWTrue;
   struct HostTable *host;
   FILE *stream;
   char fname[FILENAME_MAX];
   long size;
   unsigned short len1 = (unsigned short) strlen(compilep);
   unsigned short len2 = (unsigned short) strlen(compilev);
   KWBoolean gotlock;
   short retries = 30;
   LOCKSTACK savelock;

   mkfilename( fname, E_confdir, DCSTATUS );

/*--------------------------------------------------------------------*/
/*            Save lock status, then lock host status file            */
/*--------------------------------------------------------------------*/

   PushLock( &savelock );

   do {
      gotlock = LockSystem( "_status", B_UUSTAT );
      if ( ! gotlock )
         ssleep(2);
   } while ( ! gotlock && retries-- );

   if ( ! gotlock )
   {
      printmsg(0,"Cannot obtain lock for %s", fname );
      PopLock( &savelock );
      return;
   }

/*--------------------------------------------------------------------*/
/*                  Old previous status as required                   */
/*--------------------------------------------------------------------*/

   HostStatus();              /* Get new data, if needed          */

   if ((stream  = FOPEN(fname, "w", IMAGE_MODE)) == NULL)
   {
      printerr( fname );
      return;
   }

   fwrite( &len1, sizeof len1, 1, stream );
   fwrite( &len2, sizeof len2, 1, stream );
   fwrite( compilep , 1, len1, stream);
   fwrite( compilev , 1, len2, stream);
   fwrite( &start_stats , sizeof start_stats , 1,  stream);

   while  ((host = nexthost( firsthost )) != BADHOST)
   {
      unsigned short saveStatus = host->status.hstatus;

      len1 = (unsigned short) strlen( host->hostname );
      len2 = (unsigned short) sizeof host->status;

      firsthost = KWFalse;

      fwrite( &len1, sizeof len1, 1, stream );
      fwrite( &len2, sizeof len2, 1, stream );
      fwrite( host->hostname , sizeof hostp->hostname[0], len1, stream);

      if ( host->status.hstatus == HS_CALLED )
         host->status.hstatus = HS_SUCCEEDED;

      fwrite( &host->status, len2, 1,  stream);

      host->status.hstatus = saveStatus;
                              /* Restore to insure we don't call
                                 host again this invocation of UUCICO */
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

   hstatus_age = stater( fname , &size );

/*--------------------------------------------------------------------*/
/*                      Restore locks and return                      */
/*--------------------------------------------------------------------*/

   UnlockSystem( );
   PopLock( &savelock );

} /* dcupdate */
