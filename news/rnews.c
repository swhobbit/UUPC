/*--------------------------------------------------------------------*/
/*    r n e w s . c                                                   */
/*                                                                    */
/*    Receive incoming news into the news directory.                  */
/*                                                                    */
/*    Written by Mike Lipsie; modified for UUPC/extended 1.11s by     */
/*    Andrew H., Derbyshire.                                          */
/*                                                                    */
/*    Changes and Compilation Copyright (c) 1992 by Andrew H.         */
/*    Derbyshire.  All rights reserved except as granted by the       */
/*    general UUPC/extended license.                                  */
/*                                                                    */
/*    This package has been substantially modified.  It will now      */
/*    copy compressed articles (assumed batches but they could be     */
/*    single articles) to the CMPRSSED directory and it will          */
/*    unbatch (if necessary) and deliver articles to their            */
/*    appropriate newsgroup directories.  At the end, if the file     */
/*    was compressed it will invoke a batch file which will           */
/*    uncompress the file and then feed it back to rnews.             */
/*                                                                    */
/*    Appropriate is defined as                                       */
/*                                                                    */
/*       (a) the newsgroup is in the active file and                  */
/*       (b) the directory name is the group name with all            */
/*           the"."s replaced with "/"s and all of that under the     */
/*           news directory.                                          */
/*                                                                    */
/*    Names are insured to be valid by ImportNewsGroup, which maps    */
/*    invalid characters and truncates names as required.             */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *       $Id: RNEWS.C 1.3 1993/03/06 23:04:54 ahd Exp $
 *
 *       $Log: RNEWS.C $
 * Revision 1.3  1993/03/06  23:04:54  ahd
 * Do not delete open files
 *
 * Revision 1.2  1992/11/22  21:14:21  ahd
 * Reformat selected sections of code
 * Check for premature end of articles in batched news
 *
 */

static const char rcsid[] =
         "$Id: RNEWS.C 1.3 1993/03/06 23:04:54 ahd Exp $";

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <fcntl.h>
#include <io.h>
#include <process.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "active.h"
#include "getopt.h"
#include "getseq.h"
#include "history.h"
#include "hlib.h"
#include "import.h"
#include "importng.h"
#include "logger.h"
#include "timestmp.h"

/*--------------------------------------------------------------------*/
/*                           Global defines                           */
/*--------------------------------------------------------------------*/

#define MAXGRP 40             /* Max length of a news group name     */
#define UNCOMPRESS "uncompre"

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

currentfile();

extern struct grp *group_list;   /* List of all groups */

FILE *hfile = NULL;           /* History file */
char history_date[12];        /* dd/mm/yyyy + null + 1 for no good reason */
char hbuff[BUFSIZ];

/*--------------------------------------------------------------------*/
/*                       Functions in this file                       */
/*--------------------------------------------------------------------*/

static boolean deliver_article(char *art_fname);
                              /* Distribute the article to the
                                 proper newsgroups                   */

static void copy_file(FILE *f,
            char *group,
            char *xref);      /* Copy file (f) to newsgroup          */

static struct grp *find_newsgroup(const char *grp);
                              /* Get the grp struct for the newsgroup */

static void get_snum(const char *group, char *snum);
                                    /* Get (and format) the next article
                                       number in group                     */

static void fixEOF( char *buf, int bytes );

static int Single( char *filename , FILE *stream );

static int Compressed( char *filename , FILE *in_stream );

static int Batched( char *filename, FILE *stream);

static void xmit_news( char *sysname, FILE *in_stream );

static int copy_snews( char *filename, FILE *stream );

/*--------------------------------------------------------------------*/
/*    m a i n                                                         */
/*                                                                    */
/*    Main program                                                    */
/*                                                                    */
/*    Exit conditions                                                 */
/*                                                                    */
/*    0 - Success                                                     */
/*    1 - System configuration failed                                 */
/*    2 - Unable to open working file                                 */
/*    4 - out of memory                                               */
/*    5 - Unable to create history dbm file                           */
/*    6 - Problem decompressing news batch                            */
/*    7 - Unable to create cmprssed directory                         */
/*--------------------------------------------------------------------*/

void main( int argc, char **argv)
{

   struct tm *local_now;

   long now = time(nil(long));
   FILE *f;
   char in_filename[FILENAME_MAX];
   char filename[FILENAME_MAX];
   int c;
   int status;

#if defined(__CORE__)
   copywrong = strdup(copyright);
   checkref(copywrong);
#endif

   banner( argv );

   if (!configure( B_NEWS ))
      exit(1);    /* system configuration failed */

   openlog( NULL );           /* Begin logging to disk            */

   if (argc > 1)
   {
      int option;

    /*------------------------------------------------------------*/
    /*                          parse arguments                   */
    /*------------------------------------------------------------*/

       while ((option = getopt(argc, argv, "f:x:")) != EOF)
       {
           switch (option)
           {
               case 'f':
                  strcpy(in_filename, optarg);
                  f = freopen(in_filename, "r", stdin);
                  if (f == NULL) {
                     printerr( in_filename );
                     panic();
                  } else
                     printmsg(2, "Opened %s as newsfile", in_filename);

                  break;

               case 'x':
                  debuglevel = atoi(optarg);
                  break;

               case '?':
                  puts("\nUsage:\trnews [-f newsfile] [-x debug]");
                  return;
           } /* break */

       } /* while */
    } /* if (argc > 1) */

   tzset();                   /* Set up time zone information  */

   if ( !bflag[F_SNEWS] )
   {
      get_active();           /* Get sequence numbers for groups
                                 from active file                 */
      validate_newsgroups();  /* Make sure all directories exist  */

   }
   else if (bflag[F_HISTORY])
   {
      printmsg(0,
            "rnews: Conflicting options snews and history specified!");
      panic();
   } /* else if */

   local_now = localtime(&now);

   sprintf(history_date, "%2.2d/%2.2d/%4d",  local_now->tm_mday,
                                 local_now->tm_mon+1,
                                 local_now->tm_year+1900);

/*--------------------------------------------------------------------*/
/*                 Open (or create) the history file                  */
/*--------------------------------------------------------------------*/

   if ( bflag[F_HISTORY] )
   {
      if (history_exists())
         hfile = open_history(history_date);
      else
         hfile = create_history(history_date);
   } /* if */

/*--------------------------------------------------------------------*/
/*                        Get output file name                        */
/*--------------------------------------------------------------------*/

   if ( bflag[F_SNEWS])
   {
      char *savetemp = E_tempdir;   /* Save the real temp directory  */

      E_tempdir = E_newsdir;        /* Generate this file in news    */
      mktempname(filename, "ART");  /* Get the file name             */
      E_tempdir = savetemp;         /* Restore true directory name   */
   }
   else
      mktempname(filename, "tmp");

/*--------------------------------------------------------------------*/
/*    This loop copies the file to the NEWS directory.                */
/*                                                                    */
/*    A news article/batch either has a '#' character as its first    */
/*    character or it does not.                                       */
/*                                                                    */
/*    If it does not it is a single (unbatched, uncompressed)         */
/*    article.                                                        */
/*                                                                    */
/*    If the first character _is_ a '#', the first line is #!         */
/*    cunbatch\n which means that it is a compressed batch; it        */
/*    will be uncompressed and then re-fed to rnews or the first      */
/*    line can be #! rnews nnn\n (where nnn is some number) which     */
/*    means that the next nnn characters are an article and that      */
/*    more might follow.                                              */
/*                                                                    */
/*    Due to a "feature" in the compressed batch, there may be        */
/*    _no_ articles batched (an article destined for transmittal      */
/*    is cancelled before being sent).                                */
/*                                                                    */
/*    Since this is a bit spread out, the various clauses of the      */
/*    if statement will be marked with boxed 1, 2, or 3 (and a        */
/*    brief explanation.                                              */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Compressed files need to be read in BINARY mode so that         */
/*    "magic" characters (like ^Z) don't upset it.  Batched files     */
/*    need to be read in TEXT mode so that the character count is     */
/*    correct.  The other case doesn't matter.                        */
/*--------------------------------------------------------------------*/

   c = getc(stdin);
   ungetc(c, stdin);

   if (c != '#') {

      /***********************************************/
      /* 1  single (unbatched, uncompressed) article */
      /***********************************************/

      status = Single(filename, stdin);

   }
   else {

      char buf[BUFSIZ];
      int fields = fscanf(stdin, "#! %s ", &buf);
      if ((fields == 1) && (strcmp(buf, "cunbatch") == 0))
      {

         /***********************************************/
         /*  2  Compressed batch                        */
         /***********************************************/

         status = Compressed(filename, stdin);

      }
      else {

         /***********************************************/
         /* 3  Uncompressed batch                       */
         /***********************************************/

         status = Batched( filename, stdin );
      } /* else */
   } /* else */

/*--------------------------------------------------------------------*/
/*                     Close open files and exit                      */
/*--------------------------------------------------------------------*/

   if ( !bflag[F_SNEWS] )
      put_active();

   if (hfile != NULL)
      fclose(hfile);

   exit(status);

} /*main*/

/*--------------------------------------------------------------------*/
/*    S i n g l e                                                     */
/*                                                                    */
/*    Deliver a single article to the proper news group(s)            */
/*--------------------------------------------------------------------*/

static int Single( char *filename , FILE *stream )
{
   char tmp_fname[FILENAME_MAX];
   FILE *tmpf;
   char buf[BUFSIZ];
   unsigned chars_read;
   unsigned chars_written;

/*--------------------------------------------------------------------*/
/*    If we are processing snews input, write it all out to the       */
/*    news directory as one file and return gracefully.               */
/*--------------------------------------------------------------------*/

   if ( bflag[F_SNEWS] )      /* Processing snews input?             */
      return copy_snews( filename, stream );

/*--------------------------------------------------------------------*/
/* Make a file name and then open the file to write the article into  */
/*--------------------------------------------------------------------*/

   tmpf = FOPEN(filename, "w", BINARY);

   if ( tmpf == NULL )
   {
      printerr( tmp_fname );
      panic();
   }

/*--------------------------------------------------------------------*/
/*              Now copy the input into our holding bin               */
/*--------------------------------------------------------------------*/

   while ((chars_read = fread(buf,sizeof(char), BUFSIZ, stream)) != 0)
   {

      chars_written = fwrite(buf, sizeof(char), chars_read, tmpf);
      if (chars_written != chars_read)
      {
         printerr( filename );
         printmsg(0, "rnews: Error writing single article to working file");
         fclose( tmpf );
         unlink( filename );
         panic();
      }
   }

/*--------------------------------------------------------------------*/
/*     Close the file, deliver the article, and return the caller     */
/*--------------------------------------------------------------------*/

   fclose(tmpf);

   deliver_article(filename);
   unlink( filename );
   return 0;

} /* Single */

/*--------------------------------------------------------------------*/
/*    C o m p r e s s e d                                             */
/*                                                                    */
/*    Decompress news                                                 */
/*--------------------------------------------------------------------*/

static int Compressed( char *filename , FILE *in_stream )
{

   FILE *work_stream;

   char zfile[FILENAME_MAX];
   char unzfile[FILENAME_MAX];
   char buf[BUFSIZ];

   boolean first_time = TRUE;
   char *program, *args;
   long cfile_size = 0L;
   size_t chars_read, i;
   int status = 0;
   boolean needtemp = TRUE;

   char *sysname;             /* For reading systems to process   */

/*--------------------------------------------------------------------*/
/*        Copy the compressed file to the "holding" directory         */
/*--------------------------------------------------------------------*/

   setmode(fileno(in_stream), O_BINARY);/* Don't die on control-Z, etc */
   fseek(in_stream, 0L, SEEK_SET);      /* Back to the beginning       */


   while( needtemp )
   {
      mktempname( zfile , "Z" );          /* Generate "compressed" file
                                          name                       */
      strcpy( unzfile, zfile );
      unzfile[ strlen(unzfile)-2 ] = '\0';

      if ( access( unzfile, 0 ))  /* Does the host file exist?       */
         needtemp = FALSE;        /* No, we have a good pair         */
      else
         printmsg(0,"Had compressed name %s, found %s already exists!",
                  zfile, unzfile );

   } /* while */

/*--------------------------------------------------------------------*/
/*                 Open the file, with error recovery                 */
/*--------------------------------------------------------------------*/

   if ((work_stream = FOPEN(zfile, "w", BINARY)) == nil(FILE))
   {
      printmsg(0, "Compressed: Can't open %s (%d)", zfile, errno);
      printerr(zfile);
      return 2;
   }

   printmsg(2, "Compressed: Copy to %s for later processing", zfile);

/*--------------------------------------------------------------------*/
/*                 Main loop to copy compressed file                  */
/*--------------------------------------------------------------------*/

   while ((chars_read = fread(buf,sizeof(char), BUFSIZ, in_stream)) != 0)
   {
      char *t_buf = buf;
      if (first_time)
      {
         t_buf += sizeof("#! cunbatch");

         first_time = FALSE;
         chars_read -= (t_buf - buf);

         while ((*t_buf == ' ') || (*t_buf == '\n') || (*t_buf == '\r'))
         {
            t_buf++;
            chars_read--;
         } /* while */

      } /* if */

      i = fwrite(t_buf,sizeof(char),chars_read,work_stream);

      if (i != chars_read)
      {
         fclose( work_stream );
         printerr( zfile );
         panic();
      }

      cfile_size += (long)chars_read;
   } /* while */

   fclose(work_stream);

/*--------------------------------------------------------------------*/
/*             If the file is empty, delete it gracefully             */
/*--------------------------------------------------------------------*/

   if (cfile_size == 3)
   {
      unlink(zfile); /* Get rid of null file */
      printmsg(1, "Compressed: %s empty, deleted", zfile);
      return status;
   }
   else
      printmsg(2,"Compressed: Copy to %s complete, %ld characters",
               zfile, cfile_size);

/*--------------------------------------------------------------------*/
/*      Special hack for creating mirror images of the news feed      */
/*--------------------------------------------------------------------*/

   sysname = getenv( "UUPCSHADOWS" );

   if ( sysname != NULL )
   {
      strcpy( buf, sysname);

      sysname = strtok( buf, WHITESPACE );

      while( sysname != NULL )
      {
         printmsg(1,"Compressed: Shadowing news to %s", sysname );
         fseek(in_stream, 0L, SEEK_SET);      /* Back to the beginning       */
         xmit_news( sysname, in_stream );
         sysname = strtok( NULL, WHITESPACE );
      }

   } /* if */

/*--------------------------------------------------------------------*/
/*          Uncompress the article and feed it back to rnews          */
/*--------------------------------------------------------------------*/

   if ( E_uncompress == NULL )
   {
      program = UNCOMPRESS;
      args    = zfile;
      printmsg(2,"Compressed: %s %s", program , args);
   }
   else {
      sprintf( buf, E_uncompress, zfile, unzfile );
      printmsg(2,"Compressed: %s", buf );

      program = strtok( buf, WHITESPACE );
      args = strtok( NULL, "");

      if ( args != NULL )
         while (isspace(*args))
            args++;

   }

   status = spawnlp(P_WAIT, program, program, args, NULL);

   unlink( zfile );           /* Kill the compressed input file      */

   if (status != 0)
   {
      if (status == -1)
      {
          printmsg( 0, "Compress: spawn failed completely" );
          printerr( program );
      }
      else
          printmsg(0, "%s command failed (exit code %d)",
                        UNCOMPRESS, status);
      panic();
   } /* if status != 0 */

/*--------------------------------------------------------------------*/
/*            Now process the file as normal batched news             */
/*--------------------------------------------------------------------*/

                              /* Create uncompressed output file name   */

   work_stream = FOPEN( unzfile, "r", BINARY );
   if ( work_stream == NULL )
   {
      printerr( unzfile );
      panic();
   }

   status = Batched( filename, work_stream );

/*--------------------------------------------------------------------*/
/*                   Clean up and return to caller                    */
/*--------------------------------------------------------------------*/

   fclose( work_stream );
   unlink( unzfile );

   return status;

} /* Compressed */

/*--------------------------------------------------------------------*/
/*    B a t c h e d                                                   */
/*                                                                    */
/*    Handle batched, uncompressed news                               */
/*--------------------------------------------------------------------*/

static int Batched( char *filename, FILE *stream)
{

   char buf[BUFSIZ * 2];
   int status = 0;
   long article_size;
   int articles = 0;
   int ignored  = 0;
   boolean gotsize = FALSE;
   int chars_read;
   int chars_written;

/*--------------------------------------------------------------------*/
/*    If we are processing snews input, write it all out to the       */
/*    news directory as one file and return gracefully.               */
/*--------------------------------------------------------------------*/

   if ( bflag[F_SNEWS] )      /* Processing snews input?             */
      return copy_snews( filename, stream );

/*--------------------------------------------------------------------*/
/*    This is an uncompressed batch.  Copy it to the working          */
/*    directory and then distribute the individual articles.          */
/*--------------------------------------------------------------------*/

   setmode(fileno(stream), O_BINARY ); /* Don't die on Cntrl Z, etc.  */
   fseek(stream, 0L, SEEK_SET);        /* Back to the beginning       */

   while( ! feof( stream ) && ! ferror( stream ))
   {
      long article_left;
      int  max_read = (long) sizeof buf;
      long skipped_lines = 0;
      long skipped_bytes = 0;
      FILE *tmpf;

 /*--------------------------------------------------------------------*/
 /*    Handle next article (articles are separated by the line         */
 /*    indicating their size when they are batched.)                   */
 /*--------------------------------------------------------------------*/

      while ( ! gotsize )
      {
         if (fgets( buf, sizeof buf, stream ) == NULL)
            break;

         if ( equaln( "#! rnews", buf, 8) )
         {
            article_size = 0;
            sscanf(buf, "#! rnews %ld \n", &article_size);
            gotsize = TRUE;
         }
         else {
            skipped_lines ++;
            skipped_bytes += strlen( buf );
         }
      } /* while */

      if ( skipped_lines )
         printmsg(0,
                  "Batched: Skipped %ld bytes in %ld "
                  "lines after article %d",
                  skipped_bytes,
                  skipped_lines,
                  articles );

/*--------------------------------------------------------------------*/
/*                          Trap end of file                          */
/*--------------------------------------------------------------------*/

      if ( ! gotsize )
      {
         if ( ferror( stream ))
            printerr( "stdin" );
         break;
      }

      article_left = article_size;
      gotsize = FALSE;

/*--------------------------------------------------------------------*/
/*                   Open up our next working file                    */
/*--------------------------------------------------------------------*/

      tmpf = FOPEN(filename, "w", BINARY);
      if ( tmpf == NULL )
      {
         printerr( filename );
         panic();
      }

 /*--------------------------------------------------------------------*/
 /*   Copy this article to the temp file (except for the last block)   */
 /*--------------------------------------------------------------------*/

      if ( article_size )
      {
         do {
            if ( article_left < max_read )
               max_read = (int) article_left;

            chars_read = fread(buf, sizeof(char), max_read, stream);

            if ( (chars_read < max_read) && ferror( stream ))
            {
               printerr("STDIN");
               panic();
            }

            if ( chars_read == 0)
               break;

            fixEOF( buf , chars_read );

            chars_written = fwrite(buf, sizeof(char), chars_read, tmpf);
            if (chars_read != chars_written)
            {
               printmsg(0,"Batched: Read %d bytes, only wrote %d bytes of article %d",
                     chars_read, chars_written , articles + 1);
               printerr(filename);
            }

            article_left -= chars_read;

         } while (article_left > 0);

         if ( article_left )     // Premature EOF?
            printmsg(0,"Batched: Unexpected EOF for article %d, "
                     "read %ld bytes of expected %ld",
                      articles + 1,
                      article_size - article_left, article_size );

      } /* if */
      else {

         long actual_size = 0;

         do {
            if (fgets( buf, sizeof buf, stream ) == NULL)
            {
               if ( ferror( stream ))
                  printerr( filename );
               break;
            }

            chars_read = strlen( buf );

            if ( equaln( "#! rnews", buf, 8) )
            {
               sscanf(buf, "#! rnews %ld \n", &article_size);
               gotsize = TRUE;
            }
            else if ( chars_read > 0 )
            {
               actual_size += chars_read;

               chars_written = fwrite(buf,
                                      sizeof(char),
                                      chars_read,
                                      tmpf);
               if (chars_read != chars_written)
               {
                  printmsg(0,
                       "Batched: Read %d bytes, only wrote %d bytes of article %d",
                        chars_read, chars_written , articles + 1);
                  printerr(filename);
               }
            } /* else */

         } while( ! gotsize );

         printmsg(2,"Batched: Article %d size %ld",
                     articles + 1,
                     actual_size );
      } /* else */

 /*--------------------------------------------------------------------*/
 /*      Close the file, deliver its contents, and get rid of it       */
 /*--------------------------------------------------------------------*/

      fclose(tmpf);
      if ( ! deliver_article(filename) )
         ignored ++;
      unlink( filename );
      articles ++;

   } /* while */

/*--------------------------------------------------------------------*/
/*                          Return to caller                          */
/*--------------------------------------------------------------------*/

   if ( ignored )
      printmsg(1,"Batched: Unbatched %d articles (discarded %d of these)",
               articles , ignored);
   else
      printmsg(1,"Batched: Unbatched %d articles", articles );
   return status;

} /* Batched */

/*--------------------------------------------------------------------*/
/*    f i x E O F                                                     */
/*                                                                    */
/*    Zap Cntrl-Z characters in the input stream                      */
/*--------------------------------------------------------------------*/

static void fixEOF( char *buf, int bytes )
{
   static warn = TRUE;

   while ( bytes-- )
   {
      if ( *buf == ('Z' - 'A'))
      {
         *buf = 'Z';
         if ( warn )
         {
            printmsg(0,"Altered Cntl-Z to Z");
            warn = FALSE;
         } /* if */
      } /* if */
   } /* while */

} /* fixEOF */

/*--------------------------------------------------------------------*/
/*    d e l i v e r _ a r t i c l e                                   */
/*                                                                    */
/*    Determine delivery of a posting                                 */
/*--------------------------------------------------------------------*/

static boolean deliver_article(char *art_fname)
{

   char groupy[MAXGRP];

   char *newsgroups = NULL;   /* The Newsgroups: line */
   char *messageID  = NULL;   /* The Message-ID: line */

   char *gc_ptr;           /* Scratch pointers.  Mostly used to go */
   char *gc_ptr1;              /*   down the list of newsgroups.      */

   FILE *tfile;            /* The article file */

   int n_hdrs;             /* Number of desired headers seen */
   int b_xref;

   int line_len;

   char hist_record[BUFSIZ];  /* buffer for history file
                                 (also used for article)             */
   char groups[BUFSIZ];
   char message_buf[BUFSIZ];
   char snum[10];

   tfile = FOPEN(art_fname, "r", BINARY);
   if ( tfile == NULL )
   {
      printerr( art_fname );
      panic();
   }

/*--------------------------------------------------------------------*/
/*    Get fields necessary for distribution (Newsgroups:)  and the    */
/*    history file (Message-ID:).  Also, if the article is going      */
/*    to more than one newsgroup, flag the creation of Xref: fields.  */
/*--------------------------------------------------------------------*/

   n_hdrs = b_xref = 0;
   while (n_hdrs < 2)
   {
      /* Get the next line */
      gc_ptr = fgets(hist_record, sizeof(hist_record), tfile);

/*--------------------------------------------------------------------*/
/*                    Check for end of the headers                    */
/*--------------------------------------------------------------------*/

      if ((gc_ptr == NULL) || (strlen(hist_record) == 1))
      {
         /* Ooops.  Missing Message-ID: or Newsgroups: */
         if (messageID == NULL)
            printmsg(0, "Article has no Message-ID:, discarded");
         else {
            if (newsgroups == NULL)
            {
               printmsg(0,
                    "Article %s has no Newsgroups: line, discarded",
                    messageID);
            } /* if */
            else
               break;
         } /* else */

         fclose(tfile);

         return FALSE;
      } /* if ((gc_ptr == NULL) || (strlen(hist_record) == 1)) */

      line_len = strlen(hist_record);

      if (hist_record[line_len-1] == '\n')
         hist_record[(line_len--)-1] = '\0';

      if (hist_record[line_len-1] == '\r')
         hist_record[(line_len--)-1] = '\0';

      if (equalni(hist_record, "Newsgroups:", strlen("Newsgroups:")))
      {
         /* Handle Newsgroups: line*/
         if (newsgroups == NULL)
         {
            gc_ptr += strlen("Newsgroups:") + 1;
            newsgroups = strcpy(groups, gc_ptr);
            newsgroups[strlen(newsgroups)+1] = '\0';  /* Guard char for rescan */
            n_hdrs++;
            b_xref = (strchr(newsgroups, ',') != NULL); /* more than 1 group */
     }                     /* i.e. do we need to create a Xrefs: line ? */
         else
            printmsg(0, "Article has multiple Newsgroups: lines");
      }
      else if (equalni(hist_record, "Message-ID:", strlen("Message-ID:")))
      {
         /* Handle Message-ID: line */
         if (messageID == NULL)
         {
            gc_ptr += strlen("Message-ID:") + 1;
            messageID = message_buf;
            messageID[0] = '\0';
            if (*gc_ptr != '<') strcat(messageID, "<");
               strcat(messageID, gc_ptr);
            if (messageID[strlen(messageID)-1] != '>')
               strcat(messageID,">");
            n_hdrs++;
         } /* if (messageID == NULL) */
      }
   }  /* while getting Newsgroups: and Message-ID: */

/*--------------------------------------------------------------------*/
/*           Check whether article has been received before           */
/*--------------------------------------------------------------------*/

   if ( bflag[ F_HISTORY ] )
   {
      if (is_in_history(hfile, messageID))
      {
         printmsg(2, "rnews: Duplicate article %s", messageID);
         fclose(tfile);
         return FALSE;
      }

      /* Start building the history record for this article */
      strcpy(hist_record, messageID);
      strcat(hist_record, " ");
      strcat(hist_record, history_date);
      strcat(hist_record, " ");

      gc_ptr1 = newsgroups;
      while ((gc_ptr = strchr(gc_ptr1, ',')) != NULL) {
         gc_ptr[0] = '\0';
         strcpy(groupy, gc_ptr1);
         strcat(hist_record, groupy);
         strcat(hist_record, ":");
         gc_ptr1 = gc_ptr + 1;
         get_snum(groupy,snum);
         strcat(hist_record, snum);
           strcat(hist_record, ",");
      }
      strcpy(groupy, gc_ptr1);
      strcat(hist_record, groupy);
      strcat(hist_record, ":");
      get_snum(groupy,snum);
      strcat(hist_record, snum);
      strcat(hist_record, "\n");

      /* Restore the newsgroups line */
      while (newsgroups[strlen(newsgroups)+1] != '\0') {
         newsgroups[strlen(newsgroups)] = ',';
      }

      /* Post the history record */
      fseek(hfile, 0L, SEEK_END);
      fwrite(hist_record, sizeof(char), strlen(hist_record), hfile);
   } /* if ( bflag[ F_HISTORY ] ) */

/*--------------------------------------------------------------------*/
/*              Now build the Xref: line (if we need to)              */
/*--------------------------------------------------------------------*/

   if (b_xref) {
      strcpy(hist_record, "Xref: ");
      strcat(hist_record, E_nodename);
      strcat(hist_record, " ");

      gc_ptr1 = newsgroups;
      while ((gc_ptr = strchr(gc_ptr1, ',')) != NULL)
      {
         gc_ptr[0] = '\0';
         strcpy(groupy, gc_ptr1);
         strcat(hist_record, groupy);
         strcat(hist_record, ":");
         gc_ptr1 = gc_ptr + 1;
         get_snum(groupy,snum);
         strcat(hist_record, snum);
         strcat(hist_record, " ");
      }

      strcpy(groupy, gc_ptr1);
      strcat(hist_record, groupy);
      strcat(hist_record, ":");
      get_snum(groupy,snum);
      strcat(hist_record, snum);
      strcat(hist_record, "\n");

        /* Restore the newsgroups line */
      while (newsgroups[strlen(newsgroups)+1] != '\0') {
         newsgroups[strlen(newsgroups)] = ',';
      }

    }

/*--------------------------------------------------------------------*/
/*       We now need to copy the file to each group in groupys        */
/*--------------------------------------------------------------------*/

   gc_ptr1 = newsgroups;
   while ((gc_ptr = strchr(gc_ptr1, ',')) != NULL)
   {
      gc_ptr[0] = '\0';
      strcpy(groupy, gc_ptr1);
      gc_ptr1 = gc_ptr + 1;
      copy_file(tfile, groupy, b_xref ? hist_record : NULL);
   }

   strcpy(groupy, gc_ptr1);
   copy_file(tfile, groupy, b_xref ? hist_record : NULL);
   fclose(tfile);

   return TRUE;
} /* deliver_article */

/*--------------------------------------------------------------------*/
/*    f i n d _ n e w s g r o u p                                     */
/*                                                                    */
/*    Locate a news group in our list                                 */
/*--------------------------------------------------------------------*/

static struct grp *find_newsgroup(const char *grp)
{
   struct grp *cur = group_list;

   while ((strcmp(grp,cur->grp_name) != 0)) {
      if (cur->grp_next != NULL) {
         cur = cur->grp_next;
      } else {
         return NULL;
      }
   }

   return cur;
}

/*--------------------------------------------------------------------*/
/*    c o p y _ f i l e                                               */
/*                                                                    */
/*    Write an article to it's final resting place                    */
/*--------------------------------------------------------------------*/

static void copy_file(FILE *input,
                      char *group,
                      char *xref)
{
   struct grp *cur;
   char filename[FILENAME_MAX];
   char buf[BUFSIZ];
   FILE *output;

/*--------------------------------------------------------------------*/
/*           Determine if the news has been already posted            */
/*--------------------------------------------------------------------*/

   cur = find_newsgroup(group);
   if (cur == NULL)
   {
      printmsg(3, "rnews: Article cross-posted to %s", group);
      return;
   }

/*--------------------------------------------------------------------*/
/*                       Now build a file name                        */
/*--------------------------------------------------------------------*/

   ImportNewsGroup( filename, cur->grp_name, cur->grp_high++);

/*--------------------------------------------------------------------*/
/*                 We have a file name, open the file                 */
/*--------------------------------------------------------------------*/

   printmsg(2, "rnews: Saving %s article in %s",
               cur->grp_name, filename);

   if ((output = FOPEN(filename, "w", TEXT)) == nil(FILE))
   {
      printerr( filename );
      printmsg(0, "rnews: Unable to save article");
      return;
   }

   rewind(input);

   if (xref) /* write new Xref: line first */
   {
      if (fputs(xref, output) == EOF)
      {
         printerr( filename );
         panic();
      }
   }

   while (fgets(buf, sizeof buf, input) != NULL)
   {

      if (equalni(buf, "Path:", strlen("Path:")))
      {
         fprintf(output, "Path: %s!%s", E_nodename,
                         buf + strlen("Path:") + 1);
         continue;
      }
      else if (equalni(buf, "Xref:", strlen("Xref:")))
         continue; /* skip possibly old Xref: line */

      if (fputs(buf, output) == EOF)
      {
         printerr( filename );
         panic();
      }
   } /* while */

   fclose(output);

} /* copy_file */

/*--------------------------------------------------------------------*/
/*    g e t _ s n u m                                                 */
/*                                                                    */
/*    Get highest article number of newsgroup                         */
/*--------------------------------------------------------------------*/

static void get_snum(const char *group, char *snum)
{
   struct grp *cur;

   strcpy(snum, "0");
   cur = find_newsgroup(group);
   if (cur == NULL)
   return;

   sprintf(snum, "%d", cur->grp_high);

} /* snum */

/*--------------------------------------------------------------------*/
/*    x m i t _ n e w s                                               */
/*                                                                    */
/*    A cruel hack to transmit news to other systems                  */
/*--------------------------------------------------------------------*/

static void xmit_news( char *sysname, FILE *in_stream )
{
   static char *spool_fmt = SPOOLFMT;              /* spool file name */
   static char *dataf_fmt = DATAFFMT;
   static char *send_cmd  = "S %s %s %s - %s 0666\n";
   static long seqno = 0;
   FILE *out_stream;          /* For writing out data                */

   char buf[BUFSIZ];
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

   sprintf(tmfile, spool_fmt, 'C', sysname,  'd' , seq);
   sprintf(idfile, dataf_fmt, 'D', E_nodename , seq, 'd');
   sprintf(rdfile, dataf_fmt, 'D', E_nodename , seq, 'r');
   sprintf(ixfile, dataf_fmt, 'D', E_nodename , seq, 'e');
   sprintf(rxfile, dataf_fmt, 'X', E_nodename , seq, 'r');

/*--------------------------------------------------------------------*/
/*                     create remote X (xqt) file                     */
/*--------------------------------------------------------------------*/

   importpath( msname, ixfile, sysname);
   mkfilename( msfile, E_spooldir, msname);

   out_stream = FOPEN(msfile, "w", BINARY);
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

   fprintf(out_stream, "R %s %s\nU %s %s\nF %s\nI %s\nC rnews\n",
               "uucp", E_domain,
               "uucp", E_nodename,
               rdfile, rdfile);
   fclose(out_stream);

/*--------------------------------------------------------------------*/
/*  Create the data file with the mail to send to the remote system   */
/*--------------------------------------------------------------------*/

   importpath(msname, idfile, sysname);
   mkfilename( msfile, E_spooldir, msname);

   out_stream = FOPEN(msfile, "w", BINARY);
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

   out_stream = FOPEN(msfile, "w", TEXT);
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

/*--------------------------------------------------------------------*/
/*    c o p y _ s n e w s                                             */
/*                                                                    */
/*    Process news destined for the simple news reader                */
/*--------------------------------------------------------------------*/

static int copy_snews( char *filename, FILE *stream )
{
   char buf[BUFSIZ];
   size_t len;

   FILE *out_stream = FOPEN(filename, "w", BINARY);

   if ( out_stream == NULL )
   {
      printerr(filename);
      panic();
   } /* if */

   if (setvbuf( out_stream, NULL, _IONBF, 0))
   {
      printmsg(0, "copy_snews: Cannot unbuffer file %s.", filename);
      printerr(filename);
      panic();
   } /* if */

/*--------------------------------------------------------------------*/
/*                          Perform the copy                          */
/*--------------------------------------------------------------------*/

   while ( (len = fread( buf, 1, sizeof buf, stream)) != 0)
   {
      if (fwrite( buf, 1, len, out_stream ) != len)     /* I/O error? */
      {
         printerr(filename);
         panic();
      } /* if */
   } /* while */

/*--------------------------------------------------------------------*/
/*                           Close the file                           */
/*--------------------------------------------------------------------*/

   fclose( out_stream );
   fclose( stream );
   return 0;

} /* copy_snews */
