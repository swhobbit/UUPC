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
 *       $Id: rnews.c 1.22 1993/10/30 22:27:57 rommel Exp rommel $
 *
 *       $Log: rnews.c $
 * Revision 1.22  1993/10/30  22:27:57  rommel
 * News history support
 *
 * Revision 1.21  1993/10/28  00:18:10  ahd
 * Drop unneeded tzset() call
 *
 * Revision 1.20  1993/10/24  21:51:14  ahd
 * Use one token on Requestor line, per rhg
 *
 * Revision 1.19  1993/10/12  01:31:06  ahd
 * Normalize comments to PL/I style
 *
 * Revision 1.18  1993/10/09  13:15:43  rhg
 * Suppress compiler warnings
 *
 * Revision 1.17  1993/09/27  04:04:06  ahd
 * Reduce buffer sizes to avoid 16 bit stack over flows
 *
 * Revision 1.16  1993/09/24  03:43:27  ahd
 * Double buffers to avoid crashes during Disney Stock Flame War
 *
 * Revision 1.15  1993/09/21  01:42:13  ahd
 * Suppress changes to body of delivered news
 *
 * Revision 1.14  1993/09/20  04:41:54  ahd
 * OS/2 2.x support
 *
 * Revision 1.13  1993/07/31  16:26:01  ahd
 * Changes in support of Robert Denny's Windows support
 *
 * Revision 1.12  1993/07/22  23:19:50  ahd
 * First pass for Robert Denny's Windows 3.x support changes
 *
 * Revision 1.11  1993/05/03  02:41:57  ahd
 * Correct name of file to set into binary mode
 *
 * Revision 1.10  1993/04/19  13:16:20  ahd
 * Binary mode for snews
 *
 * Revision 1.9  1993/04/17  13:40:39  ahd
 * fix compile errors for snews fix
 *
 * Revision 1.8  1993/04/17  13:23:37  ahd
 * make snews option more compatible with snews (which is brain dead)
 *
 * Revision 1.7  1993/04/16  12:55:36  dmwatt
 * Bounds check group lengths
 *
 * Revision 1.5  1993/04/11  00:33:54  ahd
 * Global edits for year, TEXT, etc.
 *
 * Revision 1.4  1993/03/24  01:57:30  ahd
 * Corrections for short articles
 * Corrections for articles claimed to be zero length
 * Resync gracefully after incorrect length descriptor
 *
 * Revision 1.3  1993/03/06  23:04:54  ahd
 * Do not delete open files
 *
 * Revision 1.2  1992/11/22  21:14:21  ahd
 * Reformat selected sections of code
 * Check for premature end of articles in batched news
 *
 */

static const char rcsid[] =
         "$Id: rnews.c 1.22 1993/10/30 22:27:57 rommel Exp rommel $";

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

#include "execute.h"

/*--------------------------------------------------------------------*/
/*                           Global defines                           */
/*--------------------------------------------------------------------*/

#define MAGIC_FIRST       0x1F   /* Magic numbers for compressed batches */
#define MAGIC_COMPRESS    0x9D
#define MAGIC_FREEZE      0x9F
#define MAGIC_GZIP        0x8B

#ifdef BIT32ENV
#define DISNEY (BUFSIZ*2)
#else
#define DISNEY (BUFSIZ*4/3)
#endif

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

currentfile();

extern struct grp *group_list;   /* List of all groups */
time_t now;
void *history;

/*--------------------------------------------------------------------*/
/*                       Functions in this file                       */
/*--------------------------------------------------------------------*/

static boolean deliver_article(char *art_fname, long art_size);
                              /* Distribute the article to the
                                 proper newsgroups                   */

static boolean copy_file(FILE *f, char *group,
            char *xref);      /* Copy file (f) to newsgroup          */

static struct grp *find_newsgroup(const char *grp);
                              /* Get the grp struct for the newsgroup */

static void add_newsgroup(const char *grp, boolean moderated);
                              /* add new group to active file */

static void del_newsgroup(const char *grp);
                              /* delete group from active file */

static void control_message(const char *control);
                              /* process control message */

static int get_snum(const char *group, char *snum);
                                    /* Get (and format) the next article
                                       number in group                 */

static void fixEOF( char *buf, int bytes );

static int Single( char *filename , FILE *stream );

static int Compressed( char *filename , FILE *in_stream ,
		       char *unpacker , char *suffix );

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
   FILE *f;
   char in_filename[FILENAME_MAX];
   char filename[FILENAME_MAX];
   int c;
   int status;

   now = time(nil(time_t));

#if defined(__CORE__)
   copywrong = strdup(copyright);
   checkref(copywrong);
#endif

   logfile = stderr;             /* Prevent redirection of error      */
                                 /* messages during configuration     */

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

   setmode(fileno(stdin), O_BINARY);   /* Don't die on control-Z, etc */

/*--------------------------------------------------------------------*/
/*    If we are processing snews input, write it all out to the       */
/*    news directory as one file and return gracefully.               */
/*--------------------------------------------------------------------*/

   if ( bflag[F_SNEWS])
   {
      char *savetemp = E_tempdir;   /* Save the real temp directory  */

      if (bflag[F_HISTORY])
      {
         printmsg(0,
               "rnews: Conflicting options snews and history specified!");
         panic();
      } /* else if */


      E_tempdir = E_newsdir;        /* Generate this file in news    */
      mktempname(filename, "ART");  /* Get the file name             */
      E_tempdir = savetemp;         /* Restore true directory name   */
      exit (copy_snews( filename, stdin ));
                                    /* Dump news into NEWS directory */
   }
   else
      mktempname(filename, "tmp");  /* Make normal temp name         */

/*--------------------------------------------------------------------*/
/*             Load the active file and validate its data             */
/*--------------------------------------------------------------------*/

   get_active();           /* Get sequence numbers for groups
                              from active file                 */
   validate_newsgroups();  /* Make sure all directories exist  */

/*--------------------------------------------------------------------*/
/*                 Open (or create) the history file                  */
/*--------------------------------------------------------------------*/

   if ( bflag[F_HISTORY] )
     history = open_history("history");

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

   if (c != '#' && c != MAGIC_FIRST) {

      /***********************************************/
      /* 1  single (unbatched, uncompressed) article */
      /***********************************************/

      status = Single(filename, stdin);

   }
   else {

      unsigned char buf[BUFSIZ];
      int bytes;
      char *unpacker = NULL, *suffix = NULL;

      bytes = fread(buf, 1, 12, stdin);

      if (bytes == 12 && strncmp(buf, "#! ", 3) == 0 
	              && strncmp(buf + 4, "unbatch", 7) == 0 )
      {
	/* ignore headers like "#! cunbatch" where the 'c' can  *
         * also be one of "fgz" for frozen or [g]zipped batches */
	bytes = fread(buf, 2, 1, stdin);
	fseek(stdin, 12L, SEEK_SET);
      }
      else
	fseek(stdin, 0L, SEEK_SET);

      if (buf[0] == MAGIC_FIRST)
	switch (buf[1])
	{
	case MAGIC_COMPRESS:
	  unpacker = "compress";
	  suffix = "Z";
	  break;
	case MAGIC_FREEZE:
	  unpacker = "freeze";
	  suffix = "F";
	  break;
	case MAGIC_GZIP:
	  unpacker = "gzip";
	  suffix = "z";
	  break;
	}

      if (unpacker != NULL)
      {

         /***********************************************/
         /*  2  Compressed batch                        */
         /***********************************************/

         status = Compressed(filename, stdin, unpacker, suffix);

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

   put_active();

   if ( bflag[F_HISTORY] )
      close_history(history);

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
   long article_size = 0;


/*--------------------------------------------------------------------*/
/* Make a file name and then open the file to write the article into  */
/*--------------------------------------------------------------------*/

   tmpf = FOPEN(filename, "w", BINARY_MODE);

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
      article_size += chars_written;
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

   deliver_article(filename, article_size);
   unlink( filename );
   return 0;

} /* Single */

/*--------------------------------------------------------------------*/
/*    C o m p r e s s e d                                             */
/*                                                                    */
/*    Decompress news                                                 */
/*--------------------------------------------------------------------*/

static int Compressed( char *filename , FILE *in_stream , 
		       char *unpacker , char *suffix )
{

   FILE *work_stream;

   char zfile[FILENAME_MAX];
   char unzfile[FILENAME_MAX];
   char buf[BUFSIZ];

   boolean first_time = TRUE;
   long cfile_size = 0L;
   size_t chars_read, i;
   int status = 0;
   boolean needtemp = TRUE;

   char *sysname;             /* For reading systems to process   */

/*--------------------------------------------------------------------*/
/*        Copy the compressed file to the "holding" directory         */
/*--------------------------------------------------------------------*/

   while( needtemp )
   {
      mktempname( zfile , suffix );    /* Generate "compressed" file
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

   if ((work_stream = FOPEN(zfile, "w", BINARY_MODE)) == nil(FILE))
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
      i = fwrite(t_buf,sizeof(char),chars_read,work_stream);

      if (i != chars_read)
      {
         fclose( work_stream );
         printerr( zfile );
         unlink( zfile );     /* Kill the compressed input file      */
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
         fseek(in_stream, 0L, SEEK_SET);      /* Back to the beginning */
         xmit_news( sysname, in_stream );
         sysname = strtok( NULL, WHITESPACE );
      }

   } /* if */

/*--------------------------------------------------------------------*/
/*          Uncompress the article and feed it back to rnews          */
/*--------------------------------------------------------------------*/

   sprintf(buf, "-d %s", zfile);
   status = execute( unpacker, buf, NULL, NULL, TRUE, FALSE);

   unlink( zfile );           /* Kill the compressed input file      */

   if (status != 0)
   {
      if (status == -1)
      {
          printmsg( 0, "Compress: spawn failed completely" );
          printerr( unpacker );
      }
      else
          printmsg(0, "%s command failed (exit code %d)",
                        unpacker, status);
      panic();
   } /* if status != 0 */

/*--------------------------------------------------------------------*/
/*            Now process the file as normal batched news             */
/*--------------------------------------------------------------------*/

                              /* Create uncompressed output file name  */

   work_stream = FOPEN( unzfile, "r", BINARY_MODE);
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
/*    This is an uncompressed batch.  Copy it to the working          */
/*    directory and then distribute the individual articles.          */
/*--------------------------------------------------------------------*/

   fseek(stream, 0L, SEEK_SET);        /* Back to the beginning       */

   while( ! feof( stream ) && ! ferror( stream ))
   {
      long article_left;
      int  max_read = (long) sizeof buf;
      long skipped_lines = 0;
      long skipped_bytes = 0;
      FILE *tmpf;

 /*--------------------------------------------------------------------*/
 /*    Handle next article (articles are separated by the line        */
 /*    indicating their size when they are batched.)                  */
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

      tmpf = FOPEN(filename, "w", BINARY_MODE);
      if ( tmpf == NULL )
      {
         printerr( filename );
         panic();
      }

 /*--------------------------------------------------------------------*/
 /*   Copy this article to the temp file (except for the last block)  */
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

         if ( article_left )     /* Premature EOF?                    */
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

	 article_size = actual_size;
         printmsg(2,"Batched: Article %d size %ld",
                     articles + 1,
                     actual_size );
      } /* else */

 /*--------------------------------------------------------------------*/
 /*      Close the file, deliver its contents, and get rid of it      */
 /*--------------------------------------------------------------------*/

      fclose(tmpf);
      if ( ! deliver_article(filename, article_size) )
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

static boolean deliver_article(char *art_fname, long art_size)
{

   char groupy[MAXGRP];

   char *newsgroups = NULL;   /* The Newsgroups: line */
   char *messageID  = NULL;   /* The Message-ID: line */

   char *gc_ptr;           /* Scratch pointers.  Mostly used to go */
   char *gc_ptr1;              /*   down the list of newsgroups.      */

   FILE *tfile;            /* The article file */

   int n_hdrs;             /* Number of desired headers seen */
   int b_xref = FALSE;
   int b_control = FALSE;
   int b_saved = FALSE;

   int line_len, groups_found;

   char hist_record[DISNEY];  /* buffer for history file
                                 (also used for article)             */
   char groups[DISNEY];
   char message_buf[DISNEY];
   char snum[10];

   tfile = FOPEN(art_fname, "r", BINARY_MODE);
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
   while (n_hdrs < 3)
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
               printmsg(0,
                    "Article %s has no Newsgroups: line, discarded",
                    messageID);
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
	    while (isspace(*gc_ptr))
	      gc_ptr++;
            newsgroups = strcpy(groups, gc_ptr);
            newsgroups[strlen(newsgroups)+1] = '\0';/* Guard char for rescan */
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
	    while (isspace(*gc_ptr))
	      gc_ptr++;
            messageID = message_buf;
            messageID[0] = '\0';
            if (*gc_ptr != '<') 
	      strcat(messageID, "<");
               strcat(messageID, gc_ptr);
            if (messageID[strlen(messageID)-1] != '>')
               strcat(messageID,">");
            n_hdrs++;
         } /* if (messageID == NULL) */
      }
      else if (equalni(hist_record, "Control:", strlen("Control:")))
      {
         /* Handle Control: line*/
         if (!b_control)
         {
	    control_message(hist_record);
            b_control = TRUE;
            n_hdrs++;
         }
         else
            printmsg(0, "Article has multiple Control: lines");
      }
   }  /* while getting Newsgroups: and Message-ID: */

   if (b_control)
   {
     memcpy(newsgroups, "control\0\0", 9);
     b_xref = FALSE;
   }

/*--------------------------------------------------------------------*/
/*           Check whether article has been received before           */
/*--------------------------------------------------------------------*/

   if ( bflag[ F_HISTORY ] )
   {
      if (get_histentry(history, messageID) != NULL)
      {
         printmsg(2, "rnews: Duplicate article %s", messageID);
         fclose(tfile);
         return FALSE;
      }

      /* Start building the history record for this article */
      sprintf(hist_record, "%ld %ld ", now, art_size);
      groups_found = 0;

      for (gc_ptr = newsgroups; gc_ptr != NULL; gc_ptr = gc_ptr1)
      {
	 if ((gc_ptr1 = strchr(gc_ptr, ',')) != NULL)
	    *gc_ptr1++ = '\0';
         if (strlen(gc_ptr) > MAXGRP - 1) { 
	    /* Bounds check the newsgroup length */
            printmsg(0, "rnews: newsgroup name too long -- %s", gc_ptr1);
            continue; /* Punt the newsgroup history record */
         }
         strcpy(groupy, gc_ptr);
         if (get_snum(groupy,snum)) {
	   if (groups_found)
           strcat(hist_record, ",");
      strcat(hist_record, groupy);
      strcat(hist_record, ":");
      strcat(hist_record, snum);
	   groups_found++;
	 }
      }

      /* Restore the newsgroups line */
      while (newsgroups[strlen(newsgroups)+1] != '\0')
         newsgroups[strlen(newsgroups)] = ',';

      if (groups_found == 0) { 
	printmsg(2, "rnews: no group to deliver to: %s", messageID);
	strcpy(newsgroups, "junk");
	/* try "junk" group if none of the target groups is known here */
	if (get_snum("junk",snum))
	  sprintf(hist_record, "%ld %ld junk:%s", now, art_size, snum);
	else
	  return fclose(tfile), FALSE;
      }

      /* Post the history record */
      add_histentry(history, messageID, hist_record);
   } /* if ( bflag[ F_HISTORY ] ) */

/*--------------------------------------------------------------------*/
/*              Now build the Xref: line (if we need to)              */
/*--------------------------------------------------------------------*/

   if (b_xref) {
      strcpy(hist_record, "Xref: ");
      strcat(hist_record, E_nodename);

      for (gc_ptr = newsgroups; gc_ptr != NULL; gc_ptr = gc_ptr1)
      {
	 if ((gc_ptr1 = strchr(gc_ptr, ',')) != NULL)
	    *gc_ptr1++ = '\0';
         if (strlen(gc_ptr) > MAXGRP - 1) { 
	    /* Bounds check the newsgroup length */
            printmsg(0, "rnews: newsgroup name too long -- %s", gc_ptr);
            continue; /* Punt the newsgroup history record */
         }
         strcpy(groupy, gc_ptr);
         if (get_snum(groupy,snum)) {
	   strcat(hist_record, " ");
         strcat(hist_record, groupy);
         strcat(hist_record, ":");
         strcat(hist_record, snum);
	 }
      }

      strcat(hist_record, "\n");

        /* Restore the newsgroups line */
      while (newsgroups[strlen(newsgroups)+1] != '\0')
         newsgroups[strlen(newsgroups)] = ',';
      }

/*--------------------------------------------------------------------*/
/*       We now need to copy the file to each group in groupys        */
/*--------------------------------------------------------------------*/

   for (gc_ptr = newsgroups; gc_ptr != NULL; gc_ptr = gc_ptr1)
   {
      if ((gc_ptr1 = strchr(gc_ptr, ',')) != NULL)
	 *gc_ptr1++ = '\0';
      strcpy(groupy, gc_ptr);
      b_saved |= copy_file(tfile, groupy, b_xref ? hist_record : NULL);
   }

   fclose(tfile);

   return b_saved;
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
/*    a d d _ n e w s g r o u p                                       */
/*                                                                    */
/*    Add a news group to our list                                    */
/*--------------------------------------------------------------------*/

static void add_newsgroup(const char *grp, boolean moderated)
{
   struct grp *cur = group_list;

   while ((strcmp(grp,cur->grp_name) != 0)) {
      if (cur->grp_next != NULL) {
         cur = cur->grp_next;
      } else {
	 cur->grp_next = (struct grp *) malloc(sizeof(struct grp));
         cur = cur->grp_next;
	 checkref(cur);
	 cur->grp_next = NULL;
	 cur->grp_name = newstr(grp);
	 cur->grp_high = 1;
	 cur->grp_low  = 0;
	 cur->grp_can_post = (char) (moderated ? 'n' : 'y');
	 break;
      }
   }
}

/*--------------------------------------------------------------------*/
/*    d e l _ n e w s g r o u p                                       */
/*                                                                    */
/*    Remove a news group from our list                               */
/*--------------------------------------------------------------------*/

static void del_newsgroup(const char *grp)
{
   struct grp *cur = group_list;
   struct grp *prev = NULL;

   while ((strcmp(grp,cur->grp_name) != 0)) {
      if (cur->grp_next != NULL) {
	 prev = cur;
         cur = cur->grp_next;
      } else {
         return;
      }
   }

   if (prev == NULL)
     group_list = cur->grp_next;
   else
     prev->grp_next = cur->grp_next;

   free(cur);
   /* name string is not free'ed because it's in the string pool */
}

/*--------------------------------------------------------------------*/
/*    c o n t r o l _ m e s s a g e                                   */
/*                                                                    */
/*    Handle control message                                          */
/*--------------------------------------------------------------------*/

static void control_message(const char *control)
{
  char *ctrl = strdup(control);
  char *cmd, *group, *mod, *msg;
  boolean moderated;
  
  strtok(ctrl, " \t");
  cmd = strtok(NULL, " \t");

  if (stricmp(cmd, "newgroup") == 0) {
    group = strtok(NULL, " \t");
    mod = strtok(NULL, " \t");
    moderated = (mod != NULL) && (strcmp(mod, "moderated") == 0);
    add_newsgroup(group, moderated);
    printmsg(1, "rnews: newsgroup added: %s", group);
  } else if (stricmp(cmd, "rmgroup") == 0) {
    group = strtok(NULL, " \t");
    del_newsgroup(group);
    printmsg(1, "rnews: newsgroup removed: %s", group);
  } else if (stricmp(cmd, "cancel") == 0) {
    msg = strtok(NULL, " \t");
    cancel_article(history, msg);
  } else if (stricmp(cmd, "ihave") == 0 || stricmp(cmd, "sendme") == 0 ||
	     stricmp(cmd, "sendsys") == 0 || stricmp(cmd, "version") == 0 ||
	     stricmp(cmd, "checkgroups") == 0) {
    printmsg(1, "rnews: control message not implemented: %s", cmd);
  } else {
    printmsg(1, "rnews: control message unknown: %s", cmd);
  }

  free(ctrl);
}

/*--------------------------------------------------------------------*/
/*    c o p y _ f i l e                                               */
/*                                                                    */
/*    Write an article to it's final resting place                    */
/*--------------------------------------------------------------------*/

static boolean copy_file(FILE *input,
                      char *group,
                      char *xref)
{
   struct grp *cur;
   char filename[FILENAME_MAX];
   char buf[BUFSIZ];
   FILE *output;
   boolean header = TRUE;

/*--------------------------------------------------------------------*/
/*           Determine if the news has been already posted            */
/*--------------------------------------------------------------------*/

   cur = find_newsgroup(group);
   if (cur == NULL)
   {
      printmsg(3, "rnews: Article cross-posted to %s", group);
      return FALSE;
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

   if ((output = FOPEN(filename, "w",TEXT_MODE)) == nil(FILE))
   {
      printerr( filename );
      printmsg(0, "rnews: Unable to save article");
      return FALSE;
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

      if ( ! header )
         ;                 /* No operation after end of header        */
      else if ( *buf == '\n' )
         header = FALSE;
      else if (equalni(buf, "Path:", strlen("Path:")))
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

   return TRUE;

} /* copy_file */

/*--------------------------------------------------------------------*/
/*    g e t _ s n u m                                                 */
/*                                                                    */
/*    Get highest article number of newsgroup                         */
/*--------------------------------------------------------------------*/

static int get_snum(const char *group, char *snum)
{
   struct grp *cur;

   strcpy(snum, "0");
   cur = find_newsgroup(group);
   if (cur == NULL)
   return FALSE;

   sprintf(snum, "%ld", cur->grp_high);
   return TRUE;

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

   out_stream = FOPEN(msfile, "w", BINARY_MODE);
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

   fprintf(out_stream, "R %s@%s\nU %s %s\nF %s\nI %s\nC rnews\n",
               "uucp", E_domain,
               "uucp", E_nodename,
               rdfile, rdfile);
   fclose(out_stream);

/*--------------------------------------------------------------------*/
/*  Create the data file with the mail to send to the remote system   */
/*--------------------------------------------------------------------*/

   importpath(msname, idfile, sysname);
   mkfilename( msfile, E_spooldir, msname);

   out_stream = FOPEN(msfile, "w", BINARY_MODE);
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
      if (fwrite( buf, 1, len, out_stream ) != len)     /* I/O error?  */
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

/*--------------------------------------------------------------------*/
/*    c o p y _ s n e w s                                             */
/*                                                                    */
/*    Process news destined for the simple news reader                */
/*--------------------------------------------------------------------*/

static int copy_snews( char *filename, FILE *stream )
{
   char buf[BUFSIZ];
   size_t len;

   FILE *out_stream = FOPEN(filename, "w", BINARY_MODE);

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
