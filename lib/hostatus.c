/*--------------------------------------------------------------------*/
/*    h o s t a t u s . c                                             */
/*                                                                    */
/*    Load host status information for UUPC/extended                  */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1994 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: hostatus.c 1.13 1994/02/19 04:42:27 ahd Exp $
 *
 *    Revision history:
 *    $Log: hostatus.c $
 *     Revision 1.13  1994/02/19  04:42:27  ahd
 *     Use standard first header
 *
 *     Revision 1.12  1994/02/19  03:51:38  ahd
 *     Use standard first header
 *
 *     Revision 1.12  1994/02/19  03:51:38  ahd
 *     Use standard first header
 *
 *     Revision 1.11  1994/02/18  23:10:07  ahd
 *     Use standard first header
 *
 *     Revision 1.10  1994/01/24  03:08:03  ahd
 *     Annual Copyright Update
 *
 *     Revision 1.9  1994/01/01  19:02:32  ahd
 *     Annual Copyright Update
 *
 *     Revision 1.8  1993/10/28  12:19:01  ahd
 *     Cosmetic time formatting twiddles and clean ups
 *
 *     Revision 1.7  1993/10/12  00:47:04  ahd
 *     Normalize comments
 *
 *     Revision 1.6  1993/10/09  15:46:15  rhg
 *     ANSIify the source
 *
 *     Revision 1.4  1993/07/06  10:56:27  ahd
 *     Always report bad record failures
 *
 */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"

#include <io.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "hostable.h"
#include "hostatus.h"
#include "security.h"
#include "timestmp.h"
#include "stater.h"

/*--------------------------------------------------------------------*/
/*        Define current file name for panic() and printerr()         */
/*--------------------------------------------------------------------*/

currentfile();

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

time_t start_stats = 0;
time_t hstatus_age = (time_t) -2L;

void HostStatus( void )
{

   char fname[FILENAME_MAX];
   char buf[BUFSIZ];
   struct HostTable *host;
   FILE *stream;
   unsigned short len1;
   unsigned short len2;
   long size;

   time_t age;

   mkfilename( fname, E_confdir, DCSTATUS );

/*--------------------------------------------------------------------*/
/*    If the file does not exist but does exist in the spool          */
/*    directory, attempt to move it, else use the spool directory.    */
/*    If the file doesn't exist at all, return quietly.               */
/*--------------------------------------------------------------------*/

   if ( access( fname, 0 ))
   {
      mkfilename( buf, E_spooldir, DCSTATUS );
      if ( access( buf, 0 ))
         return;

      if (rename( buf, fname ))
      {
         printerr( buf );
         strcpy( fname, buf );
      }
   }

/*--------------------------------------------------------------------*/
/*              Get the age of the file to avoid reloads              */
/*--------------------------------------------------------------------*/

   age = stater( fname , &size );

   if ( age == hstatus_age )  /* Unchanged file?                      */
      return;                 /* Yes --> Just return quietly          */

/*--------------------------------------------------------------------*/
/*                       Actually open the file                       */
/*--------------------------------------------------------------------*/

   if ((stream  = FOPEN(fname , "r", BINARY_MODE)) == NULL)
   {

      perror( fname );
      printmsg(1,"HostStatus: Unable to open host status file");
      time(&start_stats);
      return;

   } /* if */

/*--------------------------------------------------------------------*/
/*                    Read the header information                     */
/*--------------------------------------------------------------------*/

   fread( &len1, sizeof len1, 1, stream );
   fread( &len2, sizeof len2, 1, stream );
   fread( buf , 1, len1, stream);
   buf[len1++] = ' ';
   fread( buf + len1 , 1, len2, stream);
   buf[ len1 + len2 ] = '\0';
   fread( &start_stats , sizeof start_stats , 1,  stream);

   printmsg(5,"HostStatus: %s generated by %s beginning %.24s",
         fname,
         buf,
         ctime(&start_stats));

   if ( start_stats < 630720000L )  /* Invalid date?              */
      time(&start_stats);           /* Yes --> reset date to today*/

/*--------------------------------------------------------------------*/
/*                           Load each host                           */
/*--------------------------------------------------------------------*/

   while  (!feof( stream ) && !ferror(stream))
   {
      if (fread( &len1, sizeof len1, 1, stream ) < 1)
         break;

      fread( &len2, sizeof len2, 1, stream );
      fread( buf , sizeof host->hostname[0], len1, stream);
      buf[ len1 ] = '\0';

      printmsg(5,"HostStatus: Name length %d, status length %d, "
                 "host name \"%s\"",
                 len1, len2, buf );

      host = checkreal( buf );

      if ( host == BADHOST )
      {
         printmsg(1,"HostStatus: Host \"%s\" not found, purging entry",
                     buf );
         fread( buf , len2, 1,  stream);
      }
      else if (len2 <= (sizeof host->status))
      {
         struct HostStats stats;    /* host status, as defined by hostatus */

         fread( &stats , len2, 1,  stream);

/*--------------------------------------------------------------------*/
/*        Update the host status to the best known information        */
/*--------------------------------------------------------------------*/

         if ( host->status.lconnect > stats.lconnect )
            stats.lconnect = host->status.lconnect;

         if ( host->status.ltime > stats.ltime )
            stats.ltime = host->status.ltime;
         else if ( host->status.lconnect > stats.ltime )
            ; /* No operation */
         else if (( stats.hstatus >= nocall ) &&
                  ( stats.hstatus < last_status ))
            host->status.hstatus = stats.hstatus;
         else
            printmsg(0,"HostStatus: Invalid status (%d) ignored for \"%s\"",
               stats.hstatus,
               host->hostname ) ;

         memcpy( &host->status, &stats , sizeof stats );
      }
      else {
         printmsg(0,"HostStatus: Bad record length %d (wanted %d), "
                    "purging status for host %s",
                    len2,
                    (int) (sizeof host->status),
                    buf);
         fread( buf , len2, 1,  stream);
      } /* else */
   } /* while */

   if (ferror( stream ))
   {
      printerr( fname );
      clearerr( stream );
   }

   fclose( stream );

   hstatus_age = age;             /* Remember the age of the file     */

} /* HostStatus */
