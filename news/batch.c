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
/*    given the filename *.CMP.  Intermediate files as necessary.     */
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
 *    $Id: batch.c 1.1 1994/12/31 03:41:08 ahd Exp $
 *
 *    Revision history:
 *    $Log: batch.c $
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
#include "sys.h"

currentfile();
static char buf[BUFSIZ];

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
   int status;

   printmsg(1, "Transmitting news to %s", sysName );

   sprintf(commandOptions, "-p -g%c -n -x %d -C %s!rnews",
           E_newsGrade,
           debuglevel,
           sysName );

   status = execute( "UUX", commandOptions, fname, NULL, TRUE, FALSE );

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

static void copy_file_s2s(FILE *output, FILE *input, const char *filename)
{
   size_t len;

   while ( (len = fread( buf, 1, sizeof buf, input)) != 0)
   {
      if (fwrite( buf, 1, len, output ) != len)     /* I/O error? */
      {
         printerr(filename);
         panic();
      } /* if */
   } /* while */

} /* copy_file_s2s */

/*--------------------------------------------------------------------*/
/*       c o m p r e s s _ b a t c h                                  */
/*                                                                    */
/*       Apply user specified or default compression program          */
/*       to our outgoing batch.                                       */
/*--------------------------------------------------------------------*/

void compress_batch(const char *system, const char *batchname)
{
   char zfile[FILENAME_MAX];
   char filename[FILENAME_MAX];
   char *p;

   FILE *zfile_stream;
   FILE *batch;
   int  status;

   char command[FILENAME_MAX * 4];

/*--------------------------------------------------------------------*/
/*                    Format the compressed file name                 */
/*--------------------------------------------------------------------*/

   strcpy(zfile, batchname);

   p = strrchr( zfile ,'\\' );   /* Get simple file name             */
   strtok(p ? p : zfile, "." );  /* Drop extension                   */
   strcat(zfile, ".Z");          /* Add new extension                */

/*--------------------------------------------------------------------*/
/*              Format the compress command and execute it            */
/*--------------------------------------------------------------------*/

   if ( E_compress == NULL )
      E_compress = COMPRESS "%s %s";

   sprintf( command, E_compress, batchname, zfile );

   status = executeCommand(command, NULL, NULL, TRUE, FALSE);

/*--------------------------------------------------------------------*/
/*                  Check the result of the compression               */
/*--------------------------------------------------------------------*/

   if (status != 0)
   {
      if ( status > 0 )
          printmsg(0, "%s command failed (exit code %d)",
                        buf,
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

   fprintf(batch, "#! cunbatch\n");

   copy_file_s2s(batch, zfile_stream, filename);

   fclose(batch);

   /* use xmit only if we're not the destination in snews mode */

   if (!equal(E_nodename, system))
   {
     queue_news(system, filename);
     unlink(filename);
   }

   fclose(zfile_stream);
   unlink(zfile);

} /* compress_batch */

/*--------------------------------------------------------------------*/
/*       b u i l d _ b a t c h n a m e                                */
/*                                                                    */
/*       Build the file name for the batch, being aware it's          */
/*       also the base for the uncompressed variation                 */
/*--------------------------------------------------------------------*/

static void build_batchname(FILE **batch,
                            char *batchname,
                            const boolean nocompress )
{
   char filename[FILENAME_MAX];
   boolean needtemp = TRUE;

   if (! nocompress )
   {
      while( needtemp )
      {
         mktempname( filename , "Z" );          /* Generate "compressed" file
                                                   name                       */
         strcpy( batchname, filename );
         batchname[ strlen(batchname)-2 ] = '\0';

         if ( access( batchname, 0 ))  /* Does the host file exist?       */
            needtemp = FALSE;        /* No, we have a good pair         */
         else
            printmsg(3, "Had compressed name %s, found %s already exists!",
                     filename, batchname );

      } /* while */
   }
   else
     mktempname( filename, "TMP" );

   denormalize( batchname );
   *batch = FOPEN(batchname, "w", IMAGE_MODE);

   if (*batch == NULL)
   {
     printerr(batchname);
     panic();
   }

} /* build_batchname */

/*--------------------------------------------------------------------*/
/*       p r o c e s s _ b a t c h                                    */
/*                                                                    */
/*       Drive the batching of news for one system.  Zero             */
/*       or more batches may be generated for the system.             */
/*--------------------------------------------------------------------*/

void process_batch(const struct sys *node,
                   const char *system,
                   const char *artnames)
{

   FILE    *article;
   FILE    *names;
   FILE    *batch = NULL;
   FILE    *temp;
   long    where;
   int     done = FALSE;

   char batchname[FILENAME_MAX];
   char tempfilename[FILENAME_MAX];

   printmsg(2, "process_batch: batching for %s", system);
   printmsg(4, "process_batch: batching from %s", artnames);

   /* compressed batches are generated in the tempdir, with no extension */

   names = FOPEN(artnames, "r", IMAGE_MODE);

   if (names == NULL)  /* there are no article names to read */
     return;

   mktempname(tempfilename, "TMP");

   temp = FOPEN(tempfilename, "w+", IMAGE_MODE);

   if (temp == NULL)
   {
     printerr(tempfilename);
     panic();
   }

   while ((filelength(fileno(names)) > 0) || done)
   {

     build_batchname(&batch, batchname, node->flag.c );

     while (((E_batchsize == 0) ||
             (filelength(fileno(batch)) < E_batchsize)) &&
            (fgets(buf, FILENAME_MAX - 1, names) != NULL))
     {
       char filename[FILENAME_MAX];

       strtok(buf, WHITESPACE); /* filename is first, dont need rest */

       printmsg(3, "Copying article %s", buf);

       denormalize( buf );

       if (( *buf == '\\' ) ||               /* Full path name?      */
           ( isalpha(*buf) && equalni( buf + 1, ":\\", 2 )))
          strcpy( filename, buf );           /* Yes --> Just copy    */
       else
          mkfilename(filename, E_newsdir, buf);

/*--------------------------------------------------------------------*/
/*                      Process a specific article                    */
/*--------------------------------------------------------------------*/

       article = FOPEN(filename, "r", IMAGE_MODE);

       if (article == NULL)
       {
         printerr(buf);
         fclose(temp);
         unlink(tempfilename);
         fclose(batch);
         unlink(batchname);
         panic();
       }

       fprintf(batch, "#! rnews %ld\n", filelength(fileno(article)));
       copy_file_s2s(batch, article, batchname);
       fclose(article);

     } /* while names and batch small */

     fclose(batch);

/*--------------------------------------------------------------------*/
/*       Send off the file unless it's too small and we're            */
/*       refusing to send underlength batches                         */
/*--------------------------------------------------------------------*/

     if ((!node->flag.B) ||
         (filelength(fileno(batch)) >= E_batchsize))
     {

       /* if we get back the compression happened ok */

       if (!node->flag.c)
         compress_batch(system, batchname);
       else
         queue_news(system, batchname);

       /* keep the rest of the names in case of failure */

       rewind(temp);
       chsize(fileno(temp), 0); /* truncate the temp file */
       where = ftell(names);
       copy_file_s2s(temp, names, tempfilename);

       /* remove the stuff we just copied to temp */

       fseek(names, 0, SEEK_SET);
       chsize(fileno(names), where);

       rewind(names);

       while (fgets(buf, FILENAME_MAX - 1, names) != NULL)
       {
         char filename[FILENAME_MAX];

         strtok(buf, WHITESPACE);   /* delete eoln chars             */

         denormalize( buf );

         if (( *buf == '\\' ) ||               /* Full path name?      */
             ( isalpha(*buf) && equalni( buf + 1, ":\\", 2 )))
            strcpy( filename, buf );           /* Yes --> Just copy    */
         else
            mkfilename(filename, E_newsdir, buf);

         /* only delete article files stored in outgoing directory */

         if ((strlen(buf) > strlen(E_newsdir)) &&
              strstr(buf + strlen(E_newsdir), OUTGOING_NEWS ))
         {
           printmsg(3, "Deleting article %s", filename);
           unlink(filename);
         }

       }

       /* restore the list of names we have not yet batched */

       rewind(temp);
       rewind(names);

       chsize(fileno(names), 0); /* trunc the file completely */

       copy_file_s2s(names, temp, artnames);

       rewind(names);
     }
     else
        done = TRUE;

     unlink(batchname);

   } /* while are names */

   fclose(temp);
   unlink(tempfilename);

   fclose(names);

   /* done is only tripped if the batch was too small to send out.  Otherwise
    * all batches were sent, so we don't need the article list anymore
    */

   if (!done)
     unlink(artnames);

} /* process_batch */
