/*--------------------------------------------------------------------*/
/*    b a t c h . c                                                   */
/*                                                                    */
/*    News batch creation for UUPC/extended.                          */
/*                                                                    */
/*    Written by Mike McLagan <mmclagan@invlogic.com>                 */
/*                                                                    */
/*    Scans directory given for files named *.ART which are assumed   */
/*    to be news articles.  These are batched together into a *.BAT   */
/*    file in the same directory.  If the F_COMPRESS flag is set,     */
/*    the batch file is then compressed and the compressed file is    */
/*    given the fileName *.CMP.  Intermediate files as necessary.     */
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
 *    $Id: batch.c 1.6 1995/01/07 16:20:33 ahd Exp $
 *
 *    Revision history:
 *    $Log: batch.c $
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
#include "sys.h"

currentfile();

#define COMPRESS "compress"

static void queue_news( const char *sysName, const char *fname );

/*--------------------------------------------------------------------*/
/*   q u e u e _ n e w s                                              */
/*                                                                    */
/*    Transmit news via UUX to news to other systems                  */
/*--------------------------------------------------------------------*/

static void queue_news( const char *sysName, const char *fname )
{
   char commandOptions[BUFSIZ];
   int status = 0;

   sprintf(commandOptions, "-p -g%c -n -x %d -C %s!rnews",
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

   fprintf(finalStream, "#! cunbatch\n");

   copy_file_s2s(finalStream, zfile_stream, finalName );

   fclose(finalStream);

   queue_news(system, finalName );
   unlink(finalName);

   fclose(zfile_stream);
   unlink(zfile);

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
     mktempname( batchName, "TMP" );
   else {
      KWBoolean needtemp = KWTrue;

      while( needtemp )
      {
         mktempname( fileName , "Z" );          /* Generate "compressed" file
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
/*       p r o c e s s _ b a t c h                                    */
/*                                                                    */
/*       Drive the batching of news for one system.  Zero             */
/*       or more batches may be generated for the system.             */
/*--------------------------------------------------------------------*/

void process_batch(const struct sys *node,
                   const char *system,
                   const char *articleListName)
{

   FILE    *article;
   FILE    *names;
   FILE    *listCopy;
   long    where;
   int     done = KWFalse;

   char batchName[FILENAME_MAX];
   char listCopyName[FILENAME_MAX];

   printmsg(2, "process_batch: batching for %s from %s, batch size %ld",
                system,
                articleListName,
                E_batchsize );

   /* compressed batches are generated in the tempdir, with no extension */

   names = FOPEN(articleListName, "r+", IMAGE_MODE);

   if (names == NULL)  /* there are no article names to read */
     return;

   mktempname( listCopyName, "TMP");

   listCopy = FOPEN( listCopyName, "w+", IMAGE_MODE);

   if (listCopy == NULL)
   {
     printerr(listCopyName);
     panic();
   }

   while ((filelength(fileno(names)) > 0) || done)
   {
     FILE    *batch = NULL;
     long    batchLength;
     char fileNameBuf[FILENAME_MAX];

     build_batchName( batchName, node->flag.c );
     denormalize( batchName );
     batch = FOPEN(batchName, "w", IMAGE_MODE);

     if (batch == NULL)
     {
       printerr(batchName);
       panic();
     }

     while (filelength(fileno(batch)) < E_batchsize)
     {
       char articleName[FILENAME_MAX];
       long length;

      if (fgets(fileNameBuf, sizeof fileNameBuf, names) == NULL)
         break;

       strtok(fileNameBuf, WHITESPACE);
                                    /* fileName is first, dont need rest */

       denormalize( fileNameBuf );

       if (( *fileNameBuf == '\\' ) ||    /* Full path name?         */
           ( isalpha(*fileNameBuf) &&
             equalni( fileNameBuf + 1, ":\\", 2 )))
          strcpy( articleName, fileNameBuf );   /* Yes --> Just copy    */
       else
          mkfilename(articleName, E_newsdir, fileNameBuf);

/*--------------------------------------------------------------------*/
/*                      Process a specific article                    */
/*--------------------------------------------------------------------*/

       article = FOPEN(articleName, "r", IMAGE_MODE);

       if (article == NULL)
       {
         printmsg(0,"process_batch: Unable to open %s", articleName );
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
       }
       else
          printmsg(0, "process_batch: Ignored empty article %s",
                      articleName );

       fclose(article);

       fflush( batch );

     } /* while names and batch small */

     batchLength = filelength(fileno(batch));

     fclose(batch);

/*--------------------------------------------------------------------*/
/*       Send off the file unless it's too small and we're            */
/*       refusing to send underlength batches                         */
/*--------------------------------------------------------------------*/

     if ((!node->flag.B) || (batchLength >= E_batchsize))
     {

/*--------------------------------------------------------------------*/
/*         Only queue up a file to send if we have data in it         */
/*--------------------------------------------------------------------*/

       if ( batchLength )
       {
          if (!node->flag.c)
            compress_batch(system, batchName);
          else
            queue_news(system, batchName);
       }

       /* keep the rest of the names in case of failure */

       fflush(listCopy);
       rewind(listCopy);
       chsize(fileno(listCopy), 0); /* truncate the listCopy file */
       where = ftell(names);
       copy_file_s2s(listCopy, names, listCopyName);

       /* remove the stuff we just copied to listCopy */

       fseek(names, 0, SEEK_SET);
       chsize(fileno(names), where);

       rewind(names);

       while (fgets(fileNameBuf, sizeof fileNameBuf, names) != NULL)
       {
         char fileName[FILENAME_MAX];

         strtok(fileNameBuf, WHITESPACE);     /* delete eoln chars    */

         denormalize( fileNameBuf );

         if (( *fileNameBuf == '\\' ) ||     /* Full path name?      */
             ( isalpha(*fileNameBuf) &&
               equalni( fileNameBuf+ 1, ":\\", 2 )))
            strcpy( fileName, fileNameBuf );    /* Yes --> Just copy    */
         else
            mkfilename(fileName, E_newsdir, fileNameBuf);

         /* only delete article files stored in outgoing directory */

         if ((strlen(fileName) > strlen(E_newsdir)) &&
              strstr(fileName + strlen(E_newsdir), OUTGOING_NEWS ))
         {
           printmsg(3, "Deleting article %s", fileName);
           unlink(fileName);
         }

       }

       /* restore the list of names we have not yet batched */

       rewind(listCopy);
       rewind(names);

       chsize(fileno(names), 0); /* trunc the file completely */

       copy_file_s2s(names, listCopy, articleListName);

       rewind(names);
     }
     else
        done = KWTrue;

     unlink(batchName);

   } /* while are names */

   fclose(listCopy);
   unlink(listCopyName);

   fclose(names);

   /* done is only tripped if the batch was too small to send out.  Otherwise
    * all batches were sent, so we don't need the article list anymore
    */

   if (!done)
     unlink(articleListName);

} /* process_batch */
