/*--------------------------------------------------------------------*/
/*       d e l i v e r m . c                                          */
/*                                                                    */
/*       Misc Subroutines supporting outbound mail delivery           */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1997 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Embedded Japanese support provided by Kenji Rikitake            */
/*    28-AUG-1991                                                     */
/*                                                                    */
/*    On Japanese support:                                            */
/*                                                                    */
/*    Japanese MS-DOS uses a 2byte Kanji (Japanese ideogram) code     */
/*    called "Shift-JIS".  This cannot be delivered via SMTP since    */
/*    Shift-JIS maps its first byte from 0x80-0x9f and 0xe0-0xfc.     */
/*    JUNET requests all hosts to send Kanji in a 7bit subset of      */
/*    ISO2022.  This is commonly called "JIS 7bit".                   */
/*                                                                    */
/*    To provide Japanese functionality, you need to convert all      */
/*    remote delivery messages to JIS 7bit, and all local delivery    */
/*    messages to Shift-JIS.                                          */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: delivers.h 1.2 1997/11/29 13:06:52 ahd Exp $
 *
 *    Revision history:
 *    $Log: delivers.h $
 */

/*--------------------------------------------------------------------*/
/*                           Include files                            */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"
#include "arpadate.h"
#include "imfile.h"
#include "deliverm.h"
#include "kanjicnv.h"
#include "getseq.h"
#include "import.h"
#include "timestmp.h"
#include "address.h"

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

RCSID("$Id$");
currentfile();

/*--------------------------------------------------------------------*/
/*    f o r m a t F i l e S i z e                                     */
/*                                                                    */
/*    Report size of file in message, if desired                      */
/*--------------------------------------------------------------------*/

char
*formatFileSize( IMFILE *imf )
{
   if (bflag[ F_COLLECTSTATS ] )
   {
      static char buf[25];  /* "(nnnnnnn bytes) " */
                            /*  ....+....+....+.. */
      sprintf(buf,   "(%ld bytes) ", imlength( imf ));
      return buf;
   } /* if */
   else
      return "";              /* Pretend we were never here       */

} /* formatFileSize */

/*--------------------------------------------------------------------*/
/*       u n i q u e M a i l B o x N a m e                            */
/*                                                                    */
/*       Build a sequenced mailbox name                               */
/*--------------------------------------------------------------------*/

void
uniqueMailBoxName( const char *user,
                   char mboxname[FILENAME_MAX] )
{

   char *seq = jobNumber( getSeq(), 4, KWTrue );
   sprintf(mboxname,"%s%c%.8s%cuumx%s",
                     E_maildir,
                     SEPCHAR,
                     user,
                     SEPCHAR,
                     seq );

   if ( E_mailext != NULL )
   {
      strcat(mboxname, ".");
      strcat(mboxname, E_mailext );
   }

} /* uniqueMailBoxName */

/*--------------------------------------------------------------------*/
/*    q u e u e R e m o t e                                           */
/*                                                                    */
/*    Queue mail for delivery on another system via UUCP              */
/*--------------------------------------------------------------------*/

size_t
queueRemote( IMFILE *imf,           /* Input file                    */
             const MAIL_ADDR *sender,   /* Originating user@host   */
             const char *command,   /* Target address                */
             const char *path,      /* Node to queue for             */
             const char grade )     /* UUCP call grade for message   */
{

   static const char mName[] = "queueRemote";
   static const char spool_fmt[] = SPOOLFMT;  /* spool file name */
   static const char dataf_fmt[] = DATAFFMT;
   static const char send_cmd[]  = "S %s %s %s - %s 0666\n";
   char *effectiveUserId = (sender->relay == NULL) ? E_mailbox : "uucp";

   char *seq = jobNumber( getSeq(), 3, bflag[F_ONECASE] );
   FILE *stream;              /* For writing out data                 */

   char msfile[FILENAME_MAX]; /* MS-DOS format name of files          */
   char msname[22];           /* MS-DOS format w/o path name          */

   char tmfile[15];           /* Call file, UNIX format name          */

   char ixfile[15];           /* eXecute file for remote system,
                                 UNIX format name for local system    */
   char rxfile[15];           /* Remote system UNIX name of eXecute
                                 file                                 */

   char idfile[15];           /* Data file, UNIX format name          */
   char rdfile[15];           /* Data file name on remote system,
                                 UNIX format                          */

   char *callFile = equal( E_nodename , path ) ? BIT_BUCKET : tmfile;
   char *dataFile = equal( E_nodename , path ) ? rdfile     : idfile;
   char *exqtFile = equal( E_nodename , path ) ? rxfile     : ixfile;

   sprintf(tmfile, spool_fmt, 'C', path,     grade , seq);
   sprintf(idfile, dataf_fmt, 'D', E_nodename , seq, 'd');
   sprintf(rdfile, dataf_fmt, 'D', E_nodename , seq, 'r');
   sprintf(ixfile, dataf_fmt, 'D', E_nodename , seq, 'e');
   sprintf(rxfile, dataf_fmt, 'X', E_nodename , seq, 'r');

/*--------------------------------------------------------------------*/
/*                     create remote X (xqt) file                     */
/*--------------------------------------------------------------------*/

   importpath( msname, exqtFile, path);
   mkfilename( msfile, E_spooldir, msname);

   stream = FOPEN(msfile, "w", IMAGE_MODE);

   if ( stream == NULL )
   {
      printerr(msfile);
      printmsg(0, "DeliverRemote: cannot open X file %s", msfile);
      return 0;
   } /* if */

   fprintf(stream, "U %s %s\n", effectiveUserId , E_nodename );
                                 /* Actual user running command      */
   fprintf(stream, "R %s\n", sender->address );
                                 /* Original requestor of command    */
   fprintf(stream, "F %s\n", rdfile );
                                 /* Required file for input          */
   fprintf(stream, "I %s\n", rdfile );
                                 /* stdin for command                */
   fprintf(stream, "C %s\n", command );
                                 /* Command to execute using file    */

/*--------------------------------------------------------------------*/
/*               Add some self-documenting information                */
/*--------------------------------------------------------------------*/

   fprintf(stream, "# Generated on %s by %s %s (built on %s %s)\n"
                   "# at %s\n",
                      E_nodename,
                      compilep,
                      compilev,
                      compiled,
                      compilet,
                      arpadate() );
   fprintf(stream, "# Call file    %s\n",    callFile );
   fprintf(stream, "# Execute file %s %s\n", idfile, rdfile );
   fprintf(stream, "# Data file    %s %s\n", ixfile, rxfile );

   fclose(stream);

/*--------------------------------------------------------------------*/
/*  Create the data file with the mail to send to the remote system   */
/*--------------------------------------------------------------------*/

   importpath(msname, dataFile, path);
   mkfilename( msfile, E_spooldir, msname);

   stream = FOPEN(msfile, "w", IMAGE_MODE);

   if (stream == NULL )
   {
      printerr(msfile);
      printmsg(0,
               "%s: Cannot open spool file \"%s\" for output",
                mName,
                msfile);
      return 0;
   }

/*--------------------------------------------------------------------*/
/*         Write the UUCP From line and the rest of the data          */
/*--------------------------------------------------------------------*/

   if (!putFromLine( sender,
                     equal( E_nodename , path ) ? KWFalse : KWTrue,
                     stream ) ||
       !CopyData( imf, sender, KWTrue, stream ))
   {
      REMOVE( msfile );
      return 0;
   }

/*--------------------------------------------------------------------*/
/*                     create local C (call) file                     */
/*--------------------------------------------------------------------*/

   if ( equal( callFile, BIT_BUCKET ))
      return 1;

   importpath( msname, callFile, path);
   mkfilename( msfile, E_spooldir, msname);

   stream = FOPEN(msfile, "w", TEXT_MODE);

   if (stream == NULL)
   {
      printerr( msname );
      printmsg(0, "%s: cannot open C file %s",
                  mName,
                  msfile);
      return 0;
   }

   fprintf(stream, send_cmd, idfile, rdfile, effectiveUserId, idfile);
   fprintf(stream, send_cmd, ixfile, rxfile, effectiveUserId, ixfile);

   fclose(stream);

   return 1;

} /* queueRemote */

/*--------------------------------------------------------------------*/
/*       p u t F r o m L i n e                                        */
/*                                                                    */
/*       Put a UUCP From line into the specified data file;           */
/*       returns KWTrue for success, else KWFalse for I/O error       */
/*--------------------------------------------------------------------*/

KWBoolean
putFromLine( const MAIL_ADDR *sender,
             const KWBoolean remoteDelivery,
             FILE *dataOut)
{
   static const char mName[] = "putFromLine";
   const char *relay;
   time_t now;

/*--------------------------------------------------------------------*/
/*         If the entire line is moot, we're out of here quietly      */
/*--------------------------------------------------------------------*/

   if ( bflag[ F_SUPPRESSFROM ] )
      return KWTrue;

/*--------------------------------------------------------------------*/
/*       Put our first information out, and check for errors;         */
/*       we'll also check for errors after the entire line is         */
/*       output.                                                      */
/*--------------------------------------------------------------------*/

   fputs( "From ", dataOut );

   if ( ferror( dataOut ))
   {
      printerr( mName );
      return KWFalse;
   }

/*--------------------------------------------------------------------*/
/*              Now the core of the From line, the address            */
/*--------------------------------------------------------------------*/

   if ( bflag[ F_DOMAINFROM ] || (!remoteDelivery))
   {
      fputs( sender->address, dataOut );   /* Yes --> Put FQDN addr out */
      relay = NULL;                 /* Never report relay             */
   }
   else {
      relay = sender->relay;

      /* Just user id if relay is in address, else full address */
      if ((relay != NULL) &&
           equal(HostAlias(sender->host), HostAlias(relay)))
         fputs(sender->user, dataOut );
      else
         fprintf( dataOut, "%s!%s", sender->host, sender->user );

   } /* else */

/*--------------------------------------------------------------------*/
/*                Now add the time stamp all formats need             */
/*--------------------------------------------------------------------*/

   time( &now );
   fprintf(dataOut, " %.24s", ctime( &now ));

/*--------------------------------------------------------------------*/
/*            Print the "remote from <hostname>", if desired          */
/*--------------------------------------------------------------------*/

   if ((relay != NULL ) && ! bflag[F_SHORTFROM])
   {
      fputs( " remote from " , dataOut );
      fputs( relay, dataOut );
   }

/*--------------------------------------------------------------------*/
/*                        Terminate our adventure                     */
/*--------------------------------------------------------------------*/

   fputc( '\n', dataOut  );

   if ( ferror( dataOut ))
   {
      printerr( mName );
      return KWFalse;
   }
   else
      return KWTrue;

} /* putFromLine */

/*--------------------------------------------------------------------*/
/*       C o p y D a t a                                              */
/*                                                                    */
/*       Copy data into its final resting spot                        */
/*--------------------------------------------------------------------*/

KWBoolean
CopyData( IMFILE *imf,              /* Input temporary file          */
          const MAIL_ADDR *sender,  /* NULL if originated locally    */
          const KWBoolean remoteDelivery,
          FILE *dataOut)            /* Output file - spool or mbox   */
{
   char buf[BUFSIZ];
   KWBoolean remoteMail;
   size_t deliveryMode = ((size_t) remoteMail) * 2 +
                         ((size_t) remoteDelivery);
   KWBoolean success = KWTrue;

   int (*put_string) (char *, FILE *) = (int (*)(char *, FILE *)) fputs;
                              /* Assume no Kanji translation needed   */

   if (sender->relay == NULL)
      remoteMail = KWFalse;
   else
      remoteMail = KWTrue;

   imrewind( imf );

/*--------------------------------------------------------------------*/
/*       Note that the default is initialized to fputs() above,       */
/*       which performs no translation.                               */
/*                                                                    */
/*       If Kanji is not enabled, don't translate it                  */
/*                                                                    */
/*       If local mail queued for local delivery, the data is         */
/*       already in Shift JIS, so don't translate it.                 */
/*                                                                    */
/*       If remote mail is queued for remote delivery, the data is    */
/*       already in JIS 7bit, so don't translate it.                  */
/*                                                                    */
/*       If delivering remote mail locally, translate to Shift JIS    */
/*                                                                    */
/*       If delivering local mail remotely, translate to JIS 7 bit    */
/*--------------------------------------------------------------------*/

   if ( bflag[ F_KANJI ] )
   switch( deliveryMode )
   {
      case 2:                 /* Remote sender, local delivery        */
            put_string = (int (*)(char *, FILE *)) fputs_shiftjis;
                              /* Yes --> Translate it                 */
         break;

      case 1:                 /* Local sender, remote delivery        */
         if ( bflag[F_KANJI]) /* Translation enabled?                 */
            put_string = (int (*)(char *, FILE *)) fputs_jis7bit;
                              /* Translate into 7 bit Kanji           */

      default:
         break;
   }

/*--------------------------------------------------------------------*/
/*                       Loop to copy the data                        */
/*--------------------------------------------------------------------*/

   while (imgets(buf, BUFSIZ, imf) != NULL)
   {

      if ((*put_string)(buf, dataOut) == EOF)     /* I/O error? */
      {

         printerr("output");
         printmsg(0,"I/O error on \"%s\"", "output");
         fclose(dataOut);
         return KWFalse;

      } /* if */

   } /* while */

/*--------------------------------------------------------------------*/
/*                      Close up shop and return                      */
/*--------------------------------------------------------------------*/

   if (imerror(imf))          /* Clean end of file on input?          */
   {
      printerr("imgets");
      panic();
   }

   fclose(dataOut);
   return success;

} /* CopyData */
