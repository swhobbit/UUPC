/*--------------------------------------------------------------------*/
/*    h o s t r s e t . c                                             */
/*                                                                    */
/*    Clear host status information for UUPC/extended                 */
/*                                                                    */
/*    Copyright (c) 1992, Mitch Mitchell                              */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1995 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: hostrset.c 1.13 1994/12/22 00:09:02 ahd Exp $
 *
 *    Revision history:
 *    $Log: hostrset.c $
 *    Revision 1.13  1994/12/22 00:09:02  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.12  1994/02/26 17:17:32  ahd
 *    Change BINARY_MODE to IMAGE_MODE to avoid IBM C/SET 2 conflict
 *
 *     Revision 1.11  1994/02/20  19:05:02  ahd
 *     IBM C/Set 2 Conversion, memory leak cleanup
 *
 *     Revision 1.10  1994/02/19  04:42:36  ahd
 *     Use standard first header
 *
 *     Revision 1.9  1994/02/19  03:51:52  ahd
 *     Use standard first header
 *
 *     Revision 1.9  1994/02/19  03:51:52  ahd
 *     Use standard first header
 *
 *     Revision 1.8  1994/02/18  23:10:16  ahd
 *     Use standard first header
 *
 *     Revision 1.7  1994/01/24  03:08:10  ahd
 *     Annual Copyright Update
 *
 *     Revision 1.6  1994/01/01  19:02:38  ahd
 *     Annual Copyright Update
 *
 *     Revision 1.5  1993/10/12  00:47:57  ahd
 *     Normalize comments
 *
 *     Revision 1.4  1993/10/09  15:47:51  rhg
 *     ANSIify the source
 *
 *     Revision 1.3  1993/05/03  02:41:57  ahd
 *     Use correct directory for new status file
 *
 */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"

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
   KWBoolean firsthost = KWTrue;
   struct HostTable *host;
   char fname[FILENAME_MAX];
   FILE *stream;
   unsigned short len1 = (unsigned short) strlen(compilep );
   unsigned short len2 = (unsigned short) strlen(compilev );

/*--------------------------------------------------------------------*/
/*         Get the file name for the status file and open it          */
/*--------------------------------------------------------------------*/

   mkfilename( fname, E_confdir, DCSTATUS );

   if ((stream  = FOPEN(fname , "w", IMAGE_MODE)) == NULL)
   {
      printmsg(1,"HostReset: Unable to open host status file");
      printerr( fname );
      panic();                /* Critical error if unable to write    */
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
      len1 = (unsigned short) strlen( host->hostname );
      len2 = sizeof host->status;

      firsthost = KWFalse;

      fwrite( &len1, sizeof len1, 1, stream );
      fwrite( &len2, sizeof len2, 1, stream );
      fwrite( host->hostname , sizeof host->hostname[0], len1, stream);

/*--------------------------------------------------------------------*/
/*                    Clear this host if requested                    */
/*--------------------------------------------------------------------*/

      if ( (name == NULL) || equal(name,host->hostname) )
      {
          host->status.calls     = 0l;  /* Total number of calls to host */
          host->status.connect   = 0l;  /* Total length of connections to host */
          host->status.fsent     = 0l;  /* Total files sent to this host */
          host->status.freceived = 0l;  /* Total files received from this host */
          host->status.bsent     = 0l;  /* Total bytes sent to this host */
          host->status.breceived = 0l;  /* Total bytes received from this host */
          host->status.errors    = 0l;  /* Total transmission errors noted */
          host->status.packets   = 0l;  /* Total packets exchanged   */
      }

      fwrite( &host->status , len2, 1,  stream);

   }  /* while */

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
