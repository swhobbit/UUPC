/*--------------------------------------------------------------------*/
/*    b a t c h . c                                                   */
/*                                                                    */
/*    News batch creation for UUPC/extended.                          */
/*                                                                    */
/*    Written by Mike McLagan <mmclagan@invlogic.com>                 */
/*                                                                    */
/*    Processes specified input list of news articles to send         */
/*    to the remote system, automatically handling compression        */
/*    and underlength batches.  Actual queuing for the remote         */
/*    system is handled via UUX.                                      */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1996 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: batch.c 1.16 1995/03/11 22:28:56 ahd v1-12q $
 *
 *    Revision history:
 *    $Log: batch.c $
 *    Revision 1.16  1995/03/11 22:28:56  ahd
 *    Use macro for file delete to allow special OS/2 processing
 *
 *    Revision 1.15  1995/03/11 12:39:24  ahd
 *    Add missing header for stater
 *
 *    Revision 1.14  1995/03/11 01:59:57  ahd
 *    Add batch length debugging information
 *    Delete uncompressed data before queuing compressed data via UUX
 *    thus requring less work space.
 *
 *    Revision 1.13  1995/02/22 01:32:17  ahd
 *    Correct check for non-existant files
 *
 *    Revision 1.12  1995/02/20 00:03:07  ahd
 *    Don't print message if unable to delete previously deleted batch
 *
 *    Revision 1.11  1995/02/12 23:37:04  ahd
 *    compiler cleanup, NNS C/news support, optimize dir processing
 *
 *    Revision 1.10  1995/01/29 16:43:03  ahd
 *    IBM C/Set compiler warnings
 *
 *    Revision 1.9  1995/01/29 13:57:34  ahd
 *    Only rewrite list of files to batch at end of processing
 *    Use imfile for rewrite of list of files
 *    Break functionality out of process_batch to reduce size
 *
 *    Revision 1.8  1995/01/22 04:16:52  ahd
 *    Add batching message
 *
 *    Revision 1.7  1995/01/07 20:48:21  ahd
 *    Correct 16 compile warnings
 *
 *    Revision 1.6  1995/01/07 16:20:33  ahd
 *    Change KWBoolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.5  1995/01/07 15:43:07  ahd
 *    Get the file length BEFORE we close the file.
 *
 *    Revision 1.4  1995/01/05 03:43:49  ahd
 *    rnews SYS file support
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

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include <fcntl.h>
#include <io.h>
#include <sys/stat.h>
#include <ctype.h>
#include <process.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "timestmp.h"
#include "import.h"
#include "getseq.h"
#include "batch.h"
#include "execute.h"
#include "imfile.h"
#include "stater.h"
#include "sys.h"

currentfile();

#define COMPRESS "compress"

/*--------------------------------------------------------------------*/
/*   q u e u e _ n e w s                                              */
/*                                                                    */
/*    Transmit news via UUX to news to other systems                  */
/*--------------------------------------------------------------------*/

static void queue_news( const char *sysName, const char *fname )
{
   char commandOptions[BUFSIZ];
   long length;
   int status = 0;

   sprintf(commandOptions, "-anews -p -g%c -n -x %d -C %s!rnews",
           E_newsGrade,
           debuglevel,
           sysName );

   status = execute( "UUX", commandOptions, fname, NULL, KWTrue, KWFalse );

   if ( status )
   {
      if ( status > 0 )
         printmsg(0, "UUX %s command exited with status %d",
                  commandOptions,
                  status );

      panic();

   }

/*--------------------------------------------------------------------*/
/*                         Report our results                         */
/*--------------------------------------------------------------------*/

   stater( fname, &length );

   printmsg(1,"queue_news: Queued %ld byte batch for %s",
              length,
              sysName );

} /* queue_news */

/*--------------------------------------------------------------------*/
/*    c o p y _ f i l e _ s 2 s                                       */
/*                                                                    */
/*    Copy from one stream to another.  Assumes both streams are at   */
/*    their respective locations.  Input is read till EOF.            */
/*--------------------------------------------------------------------*/

static void copy_file_s2s(FILE *output, FILE *input, const char *fileName)
{
   size_t len;
   char buf[BUFSIZ];

   while ( (len = fread( buf, 1, sizeof buf, input)) > 0)
   {
      if (fwrite( buf, 1, len, output ) != len)     /* I/O error? */
      {
         printerr(fileName);
         panic();
      } /* if */

   } /* while */

   if ( ferror( input ))
   {
      printerr("read");
      panic();
   }

} /* copy_file_s2s */

/*--------------------------------------------------------------------*/
/*    c o p y _ f i l e _ s 2 i                                       */
/*                                                                    */
/*    Copy a stream to an imfile.  Assumes both streams are at        */
/*    their respective locations.  Input is read till EOF.            */
/*--------------------------------------------------------------------*/

static void
copy_file_s2i(IMFILE *output, FILE *input )
{
   size_t len;
   char buf[BUFSIZ];

   while ( (len = fread( buf, 1, sizeof buf, input)) > 0)
   {
      if (imwrite( buf, 1, len, output ) != len)     /* I/O error? */
      {
         printerr("imwrite" );
         panic();
      } /* if */

   } /* while */

   if ( ferror( input ))
   {
      printerr("read");
      panic();
   }

} /* copy_file_s2i */

/*--------------------------------------------------------------------*/
/*       c o m p r e s s _ b a t c h                                  */
/*                                                                    */
/*       Apply user specified or default compression program          */
/*       to our outgoing batch.                                       */
/*--------------------------------------------------------------------*/

void compress_batch(const char *system, const char *batchName)
{
   char zfile[FILENAME_MAX];
   char finalName[FILENAME_MAX];
   char *p;

   FILE *zfile_stream;
   FILE *finalStream;
   int  status;

   char command[FILENAME_MAX * 4];

/*--------------------------------------------------------------------*/
/*                    Format the compressed file name                 */
/*--------------------------------------------------------------------*/

   strcpy(zfile, batchName);

   p = strrchr( zfile ,'\\' );   /* Get simple file name             */
   strtok(p ? p : zfile, "." );  /* Drop extension                   */
   strcat(zfile, ".Z");          /* Add new extension                */

/*--------------------------------------------------------------------*/
/*              Format the compress command and execute it            */
/*--------------------------------------------------------------------*/

   if ( E_compress == NULL )
      E_compress = COMPRESS " %s";

   sprintf( command, E_compress, batchName, zfile );

   status = executeCommand(command, NULL, NULL, KWTrue, KWFalse);

/*--------------------------------------------------------------------*/
/*                  Check the result of the compression               */
/*--------------------------------------------------------------------*/

   if (status != 0)
   {
      if ( status > 0 )
          printmsg(0, "%s command failed (exit code %d)",
                        command,
                        status);
      panic();

   } /* if status != 0 */

/*--------------------------------------------------------------------*/
/*               Open up our new file in order to copy it             */
/*--------------------------------------------------------------------*/

   zfile_stream = FOPEN(zfile, "r", IMAGE_MODE);

   if (zfile_stream == NULL)
   {
     printerr(zfile);
     panic();
   }

/*--------------------------------------------------------------------*/
/*                     Create a remote batch file                     */
/*--------------------------------------------------------------------*/

   mktempname( finalName, "TMP" );
   finalStream = FOPEN( finalName, "w", IMAGE_MODE );

   if ( finalStream == NULL )
   {
      printerr( finalName );
      panic();
   }

/*--------------------------------------------------------------------*/
/*            Create file to actually queue for the remote            */
/*--------------------------------------------------------------------*/

   fprintf(finalStream, "#! cunbatch\n");

   copy_file_s2s(finalStream, zfile_stream, finalName );

   fclose(finalStream);

/*--------------------------------------------------------------------*/
/*                  Close and dispose of input file                   */
/*--------------------------------------------------------------------*/

   fclose(zfile_stream);

   if (REMOVE(zfile))
      printerr( zfile );

/*--------------------------------------------------------------------*/
/*                queue the news, and drop output file                */
/*--------------------------------------------------------------------*/

   queue_news(system, finalName );

   if (REMOVE(finalName))
      printerr( finalName );

} /* compress_batch */

/*--------------------------------------------------------------------*/
/*       b u i l d _ b a t c h n a m e                                */
/*                                                                    */
/*       Build the file name for the batch, being aware it's          */
/*       also the base for the uncompressed variation                 */
/*--------------------------------------------------------------------*/

static void build_batchName(char *batchName,
                            const KWBoolean nocompress )
{
   char fileName[FILENAME_MAX];

   if (nocompress )
     mktempname( batchName, "TM1" );
   else {
      KWBoolean needtemp = KWTrue;

      while( needtemp )
      {
         mktempname(fileName, "Z"); /* Generate "compressed" file
                                       name                       */

         strcpy( batchName, fileName );
         batchName[ strlen(batchName)-2 ] = '\0';

         if ( access( batchName, 0 ))  /* Does the host file exist?       */
            needtemp = KWFalse;       /* No, we have a good pair         */
         else
            printmsg(3, "Had compressed name %s, found %s already exists!",
                     fileName, batchName );

      } /* while */

   }  /* else */

} /* build_batchName */

/*--------------------------------------------------------------------*/
/*       b a t c h T o g e t h e r                                    */
/*                                                                    */
/*       Given an open stream with a list of file names to batch,     */
/*       and a limit, batch the files togther onto the specified      */
/*       output stream                                                */
/*--------------------------------------------------------------------*/

static long
batchTogether( FILE *batch,
               FILE *names,
               const long batchSize,
               const char *batchName )
{
   long articleCount = 0;

   while (filelength(fileno(batch)) < batchSize)
   {

     char articleName[FILENAME_MAX];
     char fileNameBuf[FILENAME_MAX];
     long length;

     FILE *article;

     if (fgets(fileNameBuf, sizeof fileNameBuf, names) == NULL)
        break;

     strtok(fileNameBuf, WHITESPACE);
                                  /* fileName is first, dont need rest */

     if ( isAbsolutePath( fileNameBuf ))
        strcpy( articleName, fileNameBuf );   /* Yes --> Just copy    */
     else
        mkfilename(articleName, E_newsdir, fileNameBuf);

/*--------------------------------------------------------------------*/
/*                      Process a specific article                    */
/*--------------------------------------------------------------------*/

     article = FOPEN(articleName, "r", IMAGE_MODE);

     if (article == NULL)
     {
       printmsg(0,"batchTogether: Unable to open %s", articleName );
       printerr(articleName);

       continue;                  /* Non-fatal, since the file is
                                     not going to magically
                                     reappear                      */
     }

     length = (long) filelength(fileno( article ));

     if ( length > 0 )
     {
        printmsg(3, "Copying article %s (%ld) to %s (%ld)",
                     articleName,
                     length,
                     batchName,
                     (long) filelength(fileno( batch )));

        fprintf(batch, "#! rnews %ld\n", length );
        copy_file_s2s(batch, article, batchName);

        articleCount++;
     }
     else
        printmsg(0, "batchTogether: Ignored empty article %s",
                    articleName );

     fclose(article);

     fflush( batch );

   } /* while (filelength(fileno(batch)) < batchSize) */

   return articleCount;

} /* batchTogether */

/*--------------------------------------------------------------------*/
/*       d e l e t e B a t c h e d F i l e s                          */
/*                                                                    */
/*       Read a list of names up to the specified file position       */
/*       and delete each specified file.                              */
/*--------------------------------------------------------------------*/

static void
deleteBatchedFiles( FILE *names, const long lastPosition )
{

   while ( ftell( names ) < lastPosition )
   {
     char fileName[FILENAME_MAX];
     char fileNameBuf[FILENAME_MAX];

     if ( fgets(fileNameBuf, sizeof fileNameBuf, names) == NULL)
        break;

     strtok(fileNameBuf, WHITESPACE);    /* Delete eoln chars    */

     if (isAbsolutePath( fileNameBuf ))
        strcpy( fileName, fileNameBuf ); /* Yes --> Just copy    */
     else
        mkfilename(fileName, E_newsdir, fileNameBuf);

     /* Only delete article files stored in outgoing directory */

     if ((strlen(fileName) > strlen(E_newsdir)) &&
          strstr(fileName + strlen(E_newsdir), OUTGOING_NEWS ))
     {
       printmsg(3, "deleteBatchedFiles: Deleting article %s",
               fileName);

       if ( REMOVE(fileName) )
          printerr( fileName );
     }

   } /* while ( ftell( names ) < lastPosition ) */

} /* deleteBatchedFiles */

/*--------------------------------------------------------------------*/
/*       s a v e U n b a t c h e d N a m e s                          */
/*                                                                    */
/*       Given an open stream positioned midway through a file,       */
/*       delete all data previous to the current file position by     */
/*       copying the data after the current position to a work        */
/*       file and then copy it back into the beginning of the         */
/*       original file                                                */
/*--------------------------------------------------------------------*/

static void
saveUnbatchedNames( FILE *streamIn )
{
   IMFILE *saveStream = imopen( filelength( fileno( streamIn ) ) );

   if ( saveStream == NULL )
   {
      printerr("imopen");
      panic();
   }

   /* Copy the names in */

   copy_file_s2i(saveStream, streamIn );

   /* Trunate the output file */

   rewind( streamIn );
   chsize( fileno( streamIn ), 0 );      /* Empty original file  */

   /* Now copy the names back into the shorter file */

   imrewind( saveStream );
   imunload( streamIn, saveStream );

   imclose( saveStream );

} /* saveUnbatchedNames */

/*--------------------------------------------------------------------*/
/*       p r o c e s s _ b a t c h                                    */
/*                                                                    */
/*       Drive the batching of news for one system.  Zero             */
/*       or more batches may be generated for the system.             */
/*--------------------------------------------------------------------*/

void process_batch(const struct sys *node,
                   const char *system,
                   const char *articleListName)
{

   FILE    *names;
   long    firstPosition = 0;
   long    lastPosition  = 0;
   long    articleCount = 0;
   int     done = KWFalse;

   names = FOPEN(articleListName, "r+", IMAGE_MODE);

   if (names == NULL)  /* there are no article names to read */
     return;

   printmsg(2, "process_batch: batching for %s from %s, batch size %ld",
                system,
                articleListName,
                E_batchsize );

/*--------------------------------------------------------------------*/
/*       Outer loop to generate 0 or more batches for a specific      */
/*       system                                                       */
/*--------------------------------------------------------------------*/

   do {

     FILE    *batch = NULL;
     char    batchName[FILENAME_MAX];

     long    batchLength;

     /* compressed batches are generated in the tempdir, with no extension */

     build_batchName( batchName, node->flag.c );
     batch = FOPEN(batchName, "w", IMAGE_MODE);

     if (batch == NULL)
     {
       printerr(batchName);
       panic();
     }

     firstPosition = ftell(names);  /* Remember start of list        */

/*--------------------------------------------------------------------*/
/*             Generate a single batch for the remote system          */
/*--------------------------------------------------------------------*/

     articleCount = batchTogether( batch, names, E_batchsize, batchName );

     batchLength = filelength(fileno(batch));

     fclose(batch);

/*--------------------------------------------------------------------*/
/*       Send off the file unless it's too small and we're            */
/*       refusing to send underlength batches.  We also handle        */
/*       empty batches, for which processing will just delete the     */
/*       list of input files.                                         */
/*--------------------------------------------------------------------*/

     if ((!node->flag.B) ||
         (batchLength == 0 ) ||
         (batchLength >= E_batchsize))
     {

/*--------------------------------------------------------------------*/
/*         Only queue up a file to send if we have data in it         */
/*--------------------------------------------------------------------*/

       if ( batchLength )
       {
          printmsg(1, "process_batch: Sending %ld articles "
                      "in %ld byte batch to %s",
                       articleCount,
                       batchLength,
                       system );

          if (!node->flag.c)
            compress_batch(system, batchName);
          else
            queue_news(system, batchName);
       }
       else
          printmsg(3,"Deleting batch with no data (%ld empty articles)",
                     articleCount );

       /* Remember end of list processed to allow further processing */

       lastPosition = ftell(names);

       /* remove the stuff we just copied to listCopy */

       fseek(names, firstPosition, SEEK_SET);
       deleteBatchedFiles( names, lastPosition );

     } /* if ((!node->flag.B) || (batchLength >= E_batchsize)) */
     else {

        printmsg(2,"process_batch: Underlength batch "
                   "of %ld bytes in %ld articles deferred for %s",
                   batchLength,
                   articleCount,
                   system );

/*--------------------------------------------------------------------*/
/*       We save any unbatched names if we actually batched data.     */
/*       Otherwise, we leave the file untouched, which implicitly     */
/*       saves all the input files names.                             */
/*--------------------------------------------------------------------*/

       if ( firstPosition > 0 )
       {
          fseek(names, firstPosition, SEEK_SET);
          saveUnbatchedNames( names );
       }

       done = KWTrue;

     } /* else */

/*--------------------------------------------------------------------*/
/*       Delete the input batch file if it still exists.  (It may     */
/*       have been deleted by the compress program if we sent the     */
/*       batch compressed.                                            */
/*--------------------------------------------------------------------*/

     if ( !access( batchName, 0) && REMOVE(batchName) )
        printerr( batchName );

   } while (articleCount && ! done);

/*--------------------------------------------------------------------*/
/*       We're done processing batches for this system, clean up.     */
/*--------------------------------------------------------------------*/

   fclose(names);

/*--------------------------------------------------------------------*/
/*       Done is only tripped if the batch was too small to send      */
/*       out.  Otherwise all batches were sent, so we don't need      */
/*       the article list anymore                                     */
/*--------------------------------------------------------------------*/

   if (!done)
   {

#ifdef UDEBUG
      filebkup( articleListName );  /* Mostly for debugging          */
#endif

      if ( !access( articleListName, 0 ) && REMOVE(articleListName) )
         printerr( articleListName );

   } /* if (!done) */

} /* process_batch */
