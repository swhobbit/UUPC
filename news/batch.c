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
 *    $Id: lib.h 1.25 1994/12/27 20:50:28 ahd Exp $
 *
 *    Revision history:
 *    $Log: lib.h $
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

/*--------------------------------------------------------------------*/
/*    x m i t _ n e w s                                               */
/*                                                                    */
/*    A cruel hack to transmit news to other systems                  */
/*--------------------------------------------------------------------*/

void xmit_news( char *sysname, FILE *in_stream )
{
   static char *spool_fmt = SPOOLFMT;              /* spool file name */
   static char *dataf_fmt = DATAFFMT;
   static char *send_cmd  = "S %s %s %s - %s 0666\n";
   static long seqno = 0;
   FILE *out_stream;          /* For writing out data                */

   unsigned len;

   char msfile[FILENAME_MAX]; /* MS-DOS format name of files         */
   char msname[22];           /* MS-DOS format w/o path name         */

   char tmfile[15];           /* Call file, UNIX format name         */
   char ixfile[15];           /* eXecute file for remote system,
                                UNIX format name for local system   */
   char idfile[15];           /* Data file, UNIX format name         */
   char rdfile[15];           /* Data file name on remote system,
                                 UNIX format                         */
   char rxfile[15];           /* Remote system UNIX name of eXecute
                                 file                                */
   char *seq;

/*--------------------------------------------------------------------*/
/*          Create the UNIX format of the file names we need          */
/*--------------------------------------------------------------------*/

   seqno = getseq();
   seq = JobNumber( seqno );

   sprintf(tmfile, spool_fmt, 'C', sysname,  E_newsGrade, seq);
   sprintf(idfile, dataf_fmt, 'D', E_nodename , seq, 'd');
   sprintf(rdfile, dataf_fmt, 'D', E_nodename , seq, 'r');
   sprintf(ixfile, dataf_fmt, 'D', E_nodename , seq, 'e');
   sprintf(rxfile, dataf_fmt, 'X', E_nodename , seq, 'r');

/*--------------------------------------------------------------------*/
/*                     create remote X (xqt) file                     */
/*--------------------------------------------------------------------*/

   importpath( msname, ixfile, sysname);
   mkfilename( msfile, E_spooldir, msname);

   out_stream = FOPEN(msfile, "w", IMAGE_MODE);
   if ( out_stream == NULL )
   {
      printmsg(0, "xmit_news: cannot open X file %s", msfile);
      printerr(msfile);
      return ;
   } /* if */

   if (setvbuf( out_stream, NULL, _IONBF, 0))
   {
      printmsg(0, "xmit_news: Cannot unbuffer file %s (%s).",
                  ixfile, msfile);
      printerr(msfile);
      panic();
   } /* if */

   fprintf(out_stream, "U %s %s\n", "news", E_nodename );
                                 /* Actual user running command      */
   fprintf(out_stream, "F %s\n", rdfile );
                                 /* Required file for input          */
   fprintf(out_stream, "I %s\n", rdfile );
                                 /* stdin for command                */
   fprintf(out_stream, "C %s\n", "rnews");
                                 /* Command to execute using file    */
   fclose(out_stream);

/*--------------------------------------------------------------------*/
/*  Create the data file with the mail to send to the remote system   */
/*--------------------------------------------------------------------*/

   importpath(msname, idfile, sysname);
   mkfilename( msfile, E_spooldir, msname);

   out_stream = FOPEN(msfile, "w", IMAGE_MODE);
   if (out_stream == NULL )
   {
      printerr(msfile);
      printmsg(0,
               "xmit_news: Cannot open spool file \"%s\" for output",
                msfile);
      return;
   }

   if (setvbuf( out_stream, NULL, _IONBF, 0))
   {
      printmsg(0, "xmit_news: Cannot unbuffer file %s (%s).",
                  idfile, msfile);
      printerr(msfile);
      panic();
   } /* if */

/*--------------------------------------------------------------------*/
/*                       Loop to copy the data                        */
/*--------------------------------------------------------------------*/

   fseek(in_stream, 0L, SEEK_SET);      /* Back to the beginning       */

   while ( (len = fread( buf, 1, sizeof buf, in_stream)) != 0)
   {
      if (fwrite( buf, 1, len, out_stream ) != len)     /* I/O error?               */
      {
         printerr(msfile);
         fclose(out_stream);
         return;
      } /* if */
   } /* while */

   fclose( out_stream );

/*--------------------------------------------------------------------*/
/*                     create local C (call) file                     */
/*--------------------------------------------------------------------*/

   importpath( msname, tmfile, sysname);
   mkfilename( msfile, E_spooldir, msname);

   out_stream = FOPEN(msfile, "w",TEXT_MODE);
   if (out_stream == NULL)
   {
      printerr( msname );
      printmsg(0, "xmit_news: cannot open C file %s", msfile);
      return;
   }

   fprintf(out_stream, send_cmd, idfile, rdfile,
                  "uucp", idfile);
   fprintf(out_stream, send_cmd, ixfile, rxfile,
                  "uucp", ixfile);
   fclose(out_stream);

} /* xmit_news */

void xmit_news_file(char *system,char *filename)
{

  FILE *input;

  input = FOPEN(filename,"r",IMAGE_MODE);
  if (input == NULL)
  {
    printerr(filename);
    panic();
  }
  xmit_news(system,input);
  fclose(input);
}

/*--------------------------------------------------------------------*/
/*    c o p y _ f i l e _ s2s                                         */
/*                                                                    */
/*    Copy from one stream to another.  Assumes both streams are at   */
/*    their respective locations.  Input is read till EOF.            */
/*--------------------------------------------------------------------*/

void copy_file_s2s(FILE *output,FILE *input,char *filename)
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


void compress_batch(char *system, char *directory, char *batchname)
{
   char zfile[FILENAME_MAX];
   char filename[FILENAME_MAX];

   char *program;
   char *args;
   FILE *zfile_stream;
   FILE *batch;
   int  status;

   strcpy(zfile,batchname);
   strcat(zfile,".Z");

   if ( E_compress == NULL )
   {
      program = COMPRESS;
      args    = batchname;
      printmsg(2,"compress_batch: %s %s", program , args);
   }
   else {
      sprintf( buf, E_compress, batchname, zfile );
      printmsg(2,"compress_batch: %s", buf );

      program = strtok( buf, WHITESPACE );
      args = strtok( NULL, "");

      if ( args != NULL )
         while (isspace(*args))
            args++;
   }

   status = spawnlp(P_WAIT, program, program, args, NULL);

   if (status != 0)
   {
      if (status == -1)
      {
          printmsg( 0, "Compress: spawn failed completely" );
          printerr( program );
      }
      else
          printmsg(0, "%s command failed (exit code %d)",
                        COMPRESS, status);
      panic();
   } /* if status != 0 */

   zfile_stream = FOPEN(zfile,"r",IMAGE_MODE);
   if (zfile_stream == NULL)
   {
     printerr(zfile);
     panic();
   }

   /* our system should only come here if we're doing SYS file SNEWS
      delivery.  Other than that, everything should go thru deliver_local
      and never be batched! SNEWS wants it's batch in E_newsdir */

   mkdirfilename(filename,directory,"CMP");

   batch = FOPEN(filename,"w",IMAGE_MODE);
   if (batch == NULL)
   {
     printerr(filename);
     panic();
   }

   fprintf(batch,"#! cunbatch\n");

   copy_file_s2s(batch,zfile_stream,filename);

   fclose(batch);

   /* use xmit only if we're not the destination in snews mode */
   if (!equal(E_nodename,system))
   {
     xmit_news_file(system,filename);
     unlink(filename);
   }

   fclose(zfile_stream);
   unlink(zfile);

}

void build_batchname(FILE **batch,char *batchname,char *directory)
{
   char filename[FILENAME_MAX];
   boolean needtemp = TRUE;

   if (bflag[F_COMPRESSBATCH])
     while( needtemp )
     {
        mktempname( filename , "Z" );          /* Generate "compressed" file
                                            name                       */
        strcpy( batchname, filename );
        batchname[ strlen(batchname)-2 ] = '\0';

        if ( access( batchname, 0 ))  /* Does the host file exist?       */
           needtemp = FALSE;        /* No, we have a good pair         */
        else
           printmsg(3,"Had compressed name %s, found %s already exists!",
                    filename, batchname );

     } /* while */
   else
     mkdirfilename(batchname,directory,"BAT");

   *batch = FOPEN(batchname,"w+",IMAGE_MODE);
   if (*batch == NULL)
   {
     printerr(batchname);
     panic();
   }
}

void process_batch(char *system,char *directory,char *artnames)
{
   FILE    *article;
   FILE    *names;
   FILE    *batch;
   FILE    *temp;
   long    where;
   int     done;

   char batchname[FILENAME_MAX];
   char tempfilename[FILENAME_MAX];

   printmsg(2,"process_batch: batching for %s",system);
   printmsg(4,"process_batch: batching into %s",directory);
   printmsg(4,"process_batch: batching from %s",artnames);

   /* compressed batches are generated in the tempdir, with no extension */

   batch = NULL;

   names = FOPEN(artnames,"r+",IMAGE_MODE);
   if (names == NULL)  /* there are no article names to read */
     return;

   mktempname(tempfilename,"TMP");

   temp = FOPEN(tempfilename,"w+",IMAGE_MODE);
   if (temp == NULL)
   {
     printerr(tempfilename);
     panic();
   }

   done = FALSE;
   while ((filelength(fileno(names)) > 0) || done)
   {
     build_batchname(&batch,batchname,directory);

     while (((E_batchsize == 0) ||
             (filelength(fileno(batch)) < E_batchsize)) &&
            (fgets(buf,FILENAME_MAX - 1,names) != NULL))
     {
       char filename[FILENAME_MAX];

       strtok(buf," \n\r"); /* filename is first, dont need rest */
       printmsg(3,"Copying article %s",buf);
       strcpy(filename,E_newsdir);
       strcat(filename,"/");
       strcat(filename,buf);
       article = FOPEN(filename,"r",IMAGE_MODE);
       if (article == NULL)
       {
         printerr(buf);
         fclose(temp);
         unlink(tempfilename);
         fclose(batch);
         unlink(batchname);
         panic();
       }
       fprintf(batch,"#! rnews %ld\n",filelength(fileno(article)));
       copy_file_s2s(batch,article,batchname);
       fclose(article);
     } /* while names and batch small */

     fclose(batch);

     if ((!bflag[F_FULLBATCH]) ||
         (filelength(fileno(batch)) >= E_batchsize))
     {

       /* if we get back the compression happened ok */
       if (bflag[F_COMPRESSBATCH])
         compress_batch(system,directory,batchname);
       else
         xmit_news_file(system,batchname);

       /* keep the rest of the names in case of failure */
       rewind(temp);
       chsize(fileno(temp),0); /* truncate the temp file */
       where = ftell(names);
       copy_file_s2s(temp,names,tempfilename);

       /* remove the stuff we just copied to temp */
       fseek(names,0,SEEK_SET);
       chsize(fileno(names),where);

       rewind(names);
       while (fgets(buf,FILENAME_MAX - 1,names) != NULL)
       {
         char filename[FILENAME_MAX];

         strtok(buf," \n\r"); /* delete eoln chars */
         strcpy(filename,E_newsdir);
         strcat(filename,"/");
         strcat(filename,buf);

         /* only delete article files stored in outgoing directory */
         if (equaln(buf,"outgoing",strlen("outgoing")))
         {
           printmsg(3,"Deleting article %s",filename);
           unlink(filename);
         }
       }

       /* restore the list of names we have not yet batched */
       rewind(temp);
       rewind(names);
       chsize(fileno(names),0); /* trunc the file completely */
       copy_file_s2s(names,temp,artnames);
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

}

/* batch_news: build name for "togo" file, and spool directory name.
 * Call process_batch to actually do the batching
 */

void batch_news(char *system,char *artfile)
{
   char artnames[FILENAME_MAX];
   char directory[FILENAME_MAX];

   printmsg(2,"batching news for %s",system);

   strcpy(directory,E_newsdir);
   strcat(directory,"/outgoing/");

   /* the only time OUR system will be here is if we're compressing news
    * for SNEWS to deliver.  This means that output batches go into
    * E_newsdir.
    */
   if (equal(system,E_nodename))
   {
     strcpy(artnames,directory);
     strncat(artnames,system,8);
     strcat(artnames,"/togo");
     strcpy(directory,E_newsdir);
   }
   else /* get the filename for batching from SYS file here */
   {
     if ((artfile == NULL) || (*artfile == 0))
     {
       strcpy(artnames,directory);
       strncat(artnames,system,8);
       strcat(artnames,"/togo");
     }
     else
       if (*artfile == '/')
         strcpy(artnames,artfile);
       else
       {
         strcpy(artnames,directory);
         strcpy(artnames,artfile);
       }
   }

   strncat(directory,system,8);
   process_batch(system,directory,artnames);

}
