/*--------------------------------------------------------------------*/
/*    r n e w s . c                                                   */
/*                                                                    */
/*    Receive incoming news into the news directory.                  */
/*                                                                    */
/*    Written by Mike Lipsie; modified for UUPC/extended 1.11s by     */
/*    Andrew H. Derbyshire.                                           */
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
/*                                                                    */
/*    1993/06/12:                                                     */
/*                                                                    */
/*    Rewritten by Mike McLagan (mmclagan@invlogic.com) to make code  */
/*    behave much more like a typical RNEWS.  Using a boolean config  */
/*    option USESYSFILE causes RNEWS to read and interpret the sys    */
/*    file entries for redistributing news to other sites.  Each      */
/*    article is reviewed seperately for all systems listed in that   */
/*    file.  Later, using a boolean config option BATCHNEWS and       */
/*    COMPRESSNEWS with optional BATCHSIZE=n, articles will be        */
/*    combined into batches for delivery to other systems.  This      */
/*    will be accomplished by using UUX, rather than generating       */
/*    the call files directly.  (it seems easier that way!)           */
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
 *       $Id: rnews.c 1.34 1994/05/09 02:19:31 ahd Exp $
 *
 *       $Log: rnews.c $
 * Revision 1.34  1994/05/09  02:19:31  ahd
 * Report command executed to unpack
 * Use final formatted command to execute
 *
 * Revision 1.33  1994/05/08  21:43:33  ahd
 * Use value of E_uncompress over default determination
 *
 * Revision 1.32  1994/03/07  06:09:51  ahd
 * Add additional error messages to error returns
 *
 * Revision 1.32  1994/03/07  06:09:51  ahd
 * Add additional error messages to error returns
 *
 * Revision 1.31  1994/03/05  21:12:05  ahd
 * Correct logging of control messages
 *
 * Revision 1.30  1994/02/26  17:20:16  ahd
 * Change BINARY_MODE to IMAGE_MODE to avoid IBM C/SET 2 conflict
 *
 * Revision 1.29  1994/02/22  04:18:46  rommel
 * Correct message ID for duplicate articles
 *
 * Revision 1.28  1994/02/20  19:11:18  ahd
 * IBM C/Set 2 Conversion, memory leak cleanup
 *
 * Revision 1.27  1994/02/19  04:22:37  ahd
 * Use standard first header
 *
 * Revision 1.27  1994/02/19  04:22:37  ahd
 * Use standard first header
 *
 * Revision 1.26  1993/12/24  05:12:54  ahd
 * Kai Uwe Rommel's fixes for news
 *
 * Revision 1.25  1993/11/20  13:47:06  rommel
 * Handle duplicate message ids caused by truncating keys at 80 characters
 *
 * Revision 1.25  1993/11/20  13:47:06  rommel
 * Handle duplicate message ids caused by truncating keys at 80 characters
 *
 * Revision 1.24  1993/11/06  17:56:09  rhg
 * Drive Drew nuts by submitting cosmetic changes mixed in with bug fixes
 *
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

#include "uupcmoah.h"

static const char rcsid[] =
         "$Id: rnews.c 1.34 1994/05/09 02:19:31 ahd Exp $";

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include <ctype.h>
#include <fcntl.h>
#include <io.h>
#include <process.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "active.h"
#include "getopt.h"
#include "getseq.h"
#include "history.h"
#include "import.h"
#include "importng.h"
#include "logger.h"
#include "timestmp.h"

#include "execute.h"

#include "batch.h"
#include "sys.h"

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
static time_t now;
static void *history;

FILE *sys_file = NULL;           /* C News SYS file */
static char buf[DISNEY];
static char temp[DISNEY];
static char groups[DISNEY];
static char message_buf[DISNEY];
static char subj_buf[DISNEY];
static char path_buf[DISNEY];
static char xref_buf[DISNEY];
static char xref_line[DISNEY];
static char distrib_buf[DISNEY];
static char control_buf[DISNEY];

static boolean posted   = FALSE;/* Used to determine if article goes to "JUNK" */

static int articles     = 0;
static int bad_articles = 0;
static int no_delivery  = 0;
static int junked       = 0;   /* counts SNEWS articles copied to JUNK */
static int ignored      = 0;
static int ourgroups    = 0;
static int loc_articles = 0;  /* How many articles were for me */
static int fwd_articles = 0;  /* How many articles were for others? */

/*--------------------------------------------------------------------*/
/*                       Functions in this file                       */
/*--------------------------------------------------------------------*/

static void deliver_article(char *art_fname, long art_size);
                              /* Distribute the article to the
                                 proper newsgroups                   */

static struct grp *find_newsgroup(const char *grp);
                              /* Get the grp struct for the newsgroup */

static void add_newsgroup(const char *grp, boolean moderated);
                              /* add new group to active file */

static void del_newsgroup(const char *grp);
                              /* delete group from active file */

static void control_message(const char *control,
                            const char *filename );
                              /* process control message */

static int get_snum(const char *group, char *snum);
                                    /* Get (and format) the next article
                                       number in group                 */

static void fixEOF( char *buf, int bytes );

static int Single( char *filename , FILE *stream );

static int Compressed( char *filename ,
                       FILE *in_stream ,
                       char *unpacker ,
                       char *suffix );

static int Batched( char *filename, FILE *stream);

static int copy_snews( char *filename, FILE *stream );

static void shadow_news(FILE *in_stream);

static boolean deliver_local(FILE *tfile,
                             long art_size,
                             char *groups,
                             char *msgID,
                             char *control);

static boolean deliver_remote(struct sys *node, FILE *tfile, char *msgID);

static void copy_file(FILE *f,
                      char *group,
                      char *xref);      /* Copy file (f) to newsgroup */

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

   FILE *input;
   char in_filename[FILENAME_MAX];
   char filename[FILENAME_MAX];
   int c;
   int status;

   now = time(nil(time_t));

#if defined(__CORE__)
   copywrong = strdup(copyright);
   checkref(copywrong);
#endif

   setmode(0, O_BINARY);         /* don't die on control-Z, etc.      */
   input = fdopen(0, "rb");      /* Default to stdin, but             */

   logfile = stderr;             /* Prevent redirection of error      */
                                 /* messages during configuration     */

   banner( argv );

   if (!configure( B_NEWS ))
      exit(1);    /* system configuration failed */

   openlog( NULL );           /* Begin logging to disk            */

   checkname( E_nodename );      /* Fill in fdomain                  */

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
                  input = fopen(in_filename, "rb");
                  if (input == NULL)
                  {
                     printerr( in_filename );
                     panic();
                  }
                  else
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

/*--------------------------------------------------------------------*/
/* This is where we're going to shadow all news.  This insures that   */
/* all inbound articles get shadowed, whether or not they're com-     */
/* pressed.  The previous hack would only shadow compressed feeds.    */
/*--------------------------------------------------------------------*/

  if (!bflag[F_USESYSFILE])
    shadow_news(stdin);

/*--------------------------------------------------------------------*/
/* This is where we're going to check for conflicting options for     */
/* this program.  DISTRIBUTESNEWS and SNEWS can't both be set.
/*--------------------------------------------------------------------*/

   if (bflag[F_FULLBATCH] && (E_batchsize == 0))
   {
      E_batchsize = 60L * 1024L;    /* Provide reasonable default    */
      printmsg(0, "rnews: Conflicting options fullbatch and batchsize = 0, "
                   "using %ld for batch size",
                   E_batchsize );
   }
   if (bflag[F_SNEWS] && bflag[F_USESYSFILE])
   {
      printmsg(0, "rnews: Conflicting options snews and usesysfile");
      panic();
   }

/*--------------------------------------------------------------------*/
/*    If we are processing snews input, write it all out to the       */
/*    news directory as one file and return gracefully.               */
/*--------------------------------------------------------------------*/

   if ( bflag[F_SNEWS])
   {
      char *savetemp = E_tempdir;   /* Save the real temp directory  */

      E_tempdir = E_newsdir;        /* Generate this file in news    */
      mktempname(filename, "art"); /* Get the file name              */
      E_tempdir = savetemp;         /* Restore true directory name   */
      exit(copy_snews(filename, input));
                                    /* Dump news into NEWS directory */
   }
   else
      mktempname(filename, "tmp"); /* Make normal temp name          */

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
   {
     history = open_history("history");
     if ( history == NULL )
        panic();
   }

/*--------------------------------------------------------------------*/
/*                   Initialize sys file processing                   */
/*--------------------------------------------------------------------*/

   if (bflag[F_USESYSFILE])
   {
     mkfilename(in_filename, E_confdir, "SYS");

     sys_file = fopen(in_filename, "rb");
     if ( sys_file == NULL )
     {
        printerr(in_filename);
        printmsg(0, "RNEWS: Must have SYS file if USESYSFILE option set");
        panic();
     }

     init_sys(sys_file);
   }

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

   c = getc(input);
   ungetc(c, input);

   if (c != '#' && c != MAGIC_FIRST)
   {

      /***********************************************/
      /* 1  single (unbatched, uncompressed) article */
      /***********************************************/

      status = Single(filename, input);

   }
   else {

      unsigned char buf[BUFSIZ];
      int bytes;
      char *unpacker = NULL, *suffix = NULL;

      bytes = fread(buf, 1, 12, input);

      if (bytes == 12 && memcmp(buf, "#! ", 3) == 0
                      && memcmp(buf + 4, "unbatch", 7) == 0 )
      {
        /* ignore headers like "#! cunbatch" where the 'c' can  *
         * also be one of "fgz" for frozen or [g]zipped batches */
        bytes = fread(buf, 2, 1, input);
        fseek(input, 12L, SEEK_SET);
      }
      else
        fseek(input, 0L, SEEK_SET);

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
             suffix = "gz";
             break;
        }

      if (unpacker != NULL)
      {

         /***********************************************/
         /*  2  Compressed batch                        */
         /***********************************************/

         status = Compressed(filename, input, unpacker, suffix);

      }
      else {

         /***********************************************/
         /* 3  Uncompressed batch                       */
         /***********************************************/

         status = Batched(filename, input);
      } /* else */
   } /* else */

/*--------------------------------------------------------------------*/
/*                     Close open files and exit                      */
/*--------------------------------------------------------------------*/

   put_active();

   if ( bflag[F_HISTORY] )
      close_history(history);

   if (bflag[F_USESYSFILE])
     exit_sys();

   if (sys_file != NULL)
     fclose(sys_file);

/*--------------------------------------------------------------------*/
/*                          Return to caller                          */
/*--------------------------------------------------------------------*/

   printmsg(1, "RNEWS: Received %4d articles.", articles);
   printmsg(1, "RNEWS: Of these %4d were bad, ", bad_articles);
   printmsg(1, "RNEWS:          %4d were undeliverable.\n", no_delivery);
   printmsg(1, "RNEWS: Forwarded %4d articles.\n", fwd_articles);
   printmsg(1, "RNEWS: Retained %4d articles.", loc_articles);
   printmsg(1, "RNEWS: Of these %4d were duplicated, ", ignored);
   printmsg(1, "RNEWS:          %4d were junked.", junked);

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

   tmpf = FOPEN(filename, "w", IMAGE_MODE);

   if ( tmpf == NULL )
   {
      printerr( tmp_fname );
      panic();
   }

/*--------------------------------------------------------------------*/
/*              Now copy the input into our holding bin               */
/*--------------------------------------------------------------------*/

   while ((chars_read = fread(buf, sizeof(char), BUFSIZ, stream)) != 0)
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

static int Compressed( char *filename ,
                       FILE *in_stream ,
                       char *unpacker ,
                       char *suffix )
{

   FILE *work_stream;

   char zfile[FILENAME_MAX];
   char unzfile[FILENAME_MAX];
   char buf[BUFSIZ];

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
         printmsg(0, "Had compressed name %s, found %s already exists!",
                  zfile, unzfile );

   } /* while */

/*--------------------------------------------------------------------*/
/*                 Open the file, with error recovery                 */
/*--------------------------------------------------------------------*/

   if ((work_stream = FOPEN(zfile, "w", IMAGE_MODE)) == nil(FILE))
   {
      printmsg(0, "Compressed: Can't open %s (%d)", zfile, errno);
      printerr(zfile);
      return 2;
   }

   printmsg(2, "Compressed: Copy to %s for later processing", zfile);

/*--------------------------------------------------------------------*/
/*                 Main loop to copy compressed file                  */
/*--------------------------------------------------------------------*/

   while ((chars_read = fread(buf, sizeof(char), BUFSIZ, in_stream)) != 0)
   {
      char *t_buf = buf;
      i = fwrite(t_buf, sizeof(char), chars_read, work_stream);

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
      printmsg(1, "Compressed: %s empty, deleted",
                   zfile);
      return status;
   }
   else
      printmsg(2, "Compressed: Copy to %s complete, %ld characters",
               zfile, cfile_size);

/*--------------------------------------------------------------------*/
/*          Uncompress the article and feed it back to rnews          */
/*--------------------------------------------------------------------*/

   if ( E_uncompress == NULL )
      sprintf(buf, "%s -d %s", unpacker, zfile);
   else
      sprintf(buf, E_uncompress, zfile, unzfile );

   printmsg(4, "Executing command: %s", buf );
   status = executeCommand ( buf, NULL, NULL, TRUE, FALSE);

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

   work_stream = FOPEN( unzfile, "r", IMAGE_MODE);
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

      tmpf = FOPEN(filename, "w", IMAGE_MODE);
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
               printmsg(0, "Batched: Read %d bytes, only wrote %d bytes of article %d",
                     chars_read, chars_written , articles + 1);
               printerr(filename);
            }

            article_left -= chars_read;

         } while (article_left > 0);

         if ( article_left )     /* Premature EOF?                    */
            printmsg(0, "Batched: Unexpected EOF for article %d, "
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
         printmsg(2, "Batched: Article %d size %ld",
                     articles + 1,
                     actual_size );
      } /* else */

 /*--------------------------------------------------------------------*/
 /*      Close the file, deliver its contents, and get rid of it      */
 /*--------------------------------------------------------------------*/

      fclose(tmpf);
      deliver_article(filename, article_size);
      unlink( filename );
   } /* while */

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
            printmsg(0, "Altered Cntl-Z to Z");
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

/*--------------------------------------------------------------------*/
/* This is where the main modifications are made.  This proc will     */
/* look up each entry in the sys file, and determine if it is to      */
/* be sent to a given system.  This includes our own system, which    */
/* is also controlled by the sys entry. The actual deliver into the   */
/* current implementation is done once that determination is made     */
/*                                                                    */
/*--------------------------------------------------------------------*/

static void deliver_article(char *art_fname, long art_size)
{

   struct sys *sysnode = NULL;
   char *path          = NULL;
   char *newsgroups    = NULL;   /* The Newsgroups: line */
   char *messageID     = NULL;   /* The Message-ID: line */
   char *distribution  = NULL;
   char *control       = NULL;

   char *gc_ptr;           /* Scratch pointers.  Mostly used to go */

   FILE *tfile;            /* The article file */

   int n_hdrs;             /* Number of desired headers seen */
   int line_len;

   char hist_record[DISNEY];  /* buffer for history file
                                 (also used for article)             */
   int delivered = FALSE;

   articles++;

   tfile = FOPEN(art_fname, "r", IMAGE_MODE);
   if ( tfile == NULL )
   {
      printerr( art_fname );
      panic();
   }

/*--------------------------------------------------------------------*/
/*    Get fields necessary for distribution (Newsgroups:)  and the    */
/*    history file (Message-ID:).  Also, if the article is going      */
/*    to more than one newsgroup, flag the creation of Xref: fields. */
/*    Also searches for PATH, DISTRIBUTION and SUBJECT headers for    */
/*    various purposes.                                                */
/*--------------------------------------------------------------------*/

   n_hdrs = 0;
   while (n_hdrs < 5)
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
         else if (newsgroups == NULL)
         {
            printmsg(0,
                 "Article %s has no Newsgroups: line, discarded",
                 messageID);
         } /* if */
         else if (distribution == NULL)
         {
            strcpy(distrib_buf, "world");
            distribution = distrib_buf;
            n_hdrs++;
            printmsg(3, "Article has no distribution, defaulting to world");
            break;
         } /* if */
         else if (path == NULL)
         {
           /* at this point, this had better be from us, and have a
            * brain dead article creator!
            */

              strcpy(path_buf, E_mailbox);
              path = path_buf;
              n_hdrs++;
              printmsg(3, "Article has no path, defaulting to %s", E_mailbox);
              break;
           } /* if */
         else
            break;

         fclose(tfile);
         bad_articles++; /* lets count this failure */
         return;

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
         }                     /* i.e. do we need to create a Xrefs: line ? */
         else {
            printmsg(0, "Article has multiple Newsgroups: lines, discarded");
            fclose(tfile);
            bad_articles++;
            return;
         }
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
               strcat(messageID, ">");
            n_hdrs++;
         } /* if (messageID == NULL) */
         else {
            printmsg(0, "Article has multiple Message-ID: lines, discarded");
            fclose(tfile);
            bad_articles++;
            return;
         }
      }
      else if (equalni(hist_record, "Path:", strlen("Path:")))
      {
         /* Handle Path: line */
         if (path == NULL)
         {
            gc_ptr += strlen("Path:") + 1;
            while (isspace(*gc_ptr))
              gc_ptr++;
            path = strcpy(path_buf, gc_ptr);
            path[strlen(path)+1] = '\0';  /* Guard char for rescan */
            n_hdrs++;
         } /* if (subject == NULL) */
         else {
            printmsg(0, "Article has multiple Path: lines, discarded");
            fclose(tfile);
            bad_articles++;
            return;
         }
      }
      else if (equalni(hist_record, "Distribution:", strlen("Distribution:")))
      {
         /* Handle Distribution: line*/
         if (distribution == NULL)
         {
            gc_ptr += strlen("Distribution:") + 1;
            while (isspace(*gc_ptr))
              gc_ptr++;
            distribution = strcpy(distrib_buf, gc_ptr);
            distribution[strlen(distribution)+1] = '\0';  /* Guard char for rescan */
            n_hdrs++;
         }
         else {
            printmsg(0, "Article has multiple Distribution: lines, discarded");
            fclose(tfile);
            bad_articles++;
            return;
         }
      }
      else if (equalni(hist_record, "Control:", strlen("Control:")))
      {
         /* Handle Control: line*/
         if (control == NULL)
         {
            gc_ptr += strlen("Controli:") + 1;
            while (isspace(*gc_ptr))
              gc_ptr++;
            control = strcpy(control_buf, gc_ptr);
            control[strlen(control)+1] = '\0';  /* Guard char for rescan */
            n_hdrs++;
         }
         else {
            printmsg(0, "Article has multiple control lines, discarded");
            fclose(tfile);
            bad_articles++;
            return;
         }
      }
   }  /* while getting Newsgroups: and Message-ID: */

/*--------------------------------------------------------------------*/
/*      SYS file entries control what we do with this article         */
/*--------------------------------------------------------------------*/

  if (bflag[F_USESYSFILE])
  {
    sysnode = sys_list;
    delivered = FALSE;

    while (sysnode != NULL)
    {
      if (check_sys(sysnode, newsgroups, distribution, path))
        if (equal(sysnode->sysname, E_nodename))
        {
          if (!deliver_local(tfile, art_size, newsgroups, messageID,control))
            ignored++;
          else
            delivered = TRUE;
        }
        else if (deliver_remote(sysnode, tfile, messageID))
            delivered = TRUE;

      sysnode = sysnode -> next;
    }

    if (!delivered)
    {
      no_delivery++;
      printmsg(0, "deliver_article: Article undeliverable");
      printmsg(0, "                ID: %s", messageID);
      printmsg(0, "                Path: %s", path);
      printmsg(0, "                Newsgroups: %s", newsgroups);
      printmsg(0, "                Distribution: %s", distribution);
    }
  }
  else /* this is where I deliver local stuff acording to how I found RNEWS */
    if (!deliver_local(tfile, art_size, newsgroups, messageID,control))
      ignored++;

  fclose(tfile);

  return;

} /* deliver_article */

/*--------------------------------------------------------------------*/
/*    f i n d _ n e w s g r o u p                                     */
/*                                                                    */
/*    Locate a news group in our list                                 */
/*--------------------------------------------------------------------*/

static struct grp *find_newsgroup(const char *grp)
{
   struct grp *cur = group_list;

   while (!equal(grp, cur->grp_name))
   {
      if (cur->grp_next != NULL)
      {
         cur = cur->grp_next;
      }
      else {
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

   while ((strcmp(grp, cur->grp_name) != 0))
   {
      if (cur->grp_next != NULL)
      {
         cur = cur->grp_next;
      }
      else {
         cur->grp_next = (struct grp *) malloc(sizeof(struct grp));
         cur = cur->grp_next;
         checkref(cur);
         cur->grp_next = NULL;
         cur->grp_name = newstr(grp);
         cur->grp_high = 1;
         cur->grp_low  = 0;
         cur->grp_can_post = (char) (moderated ? 'm' : 'y');
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

   while ((strcmp(grp, cur->grp_name) != 0))
   {
      if (cur->grp_next != NULL)
      {
         prev = cur;
         cur = cur->grp_next;
      }
      else {
         return;
      }
   }

   if (prev == NULL)
     group_list = cur->grp_next;
   else
     prev->grp_next = cur->grp_next;

   free(cur);

   /* name string is not free'ed because it's in the string pool */

} /* del_newsgroup */

/*--------------------------------------------------------------------*/
/*    c o n t r o l _ m e s s a g e                                   */
/*                                                                    */
/*    Handle control message                                          */
/*--------------------------------------------------------------------*/

static void control_message(const char *control,
                            const char *filename )
{
  char *ctrl = strdup(control);
  char *cmd, *mod;
  boolean moderated;
  char buf[200];
  char *operand;

  printmsg(1, "Control Message: %s", control);

/*--------------------------------------------------------------------*/
/*                     Parse the command verb off                     */
/*--------------------------------------------------------------------*/

  cmd = strtok(ctrl, WHITESPACE);      /* Discard first token        */
  if ( cmd != NULL )
     cmd = strtok(NULL, WHITESPACE);   /* Get second token, if any   */

  if ( cmd == NULL )
  {
      printmsg(0, "Control message missing verb, ignored");
      free( ctrl );
      return;
  }

/*--------------------------------------------------------------------*/
/*              Get the first operand, which we require               */
/*--------------------------------------------------------------------*/

  operand = strtok( NULL, WHITESPACE); /* Get first (required) op    */

  if ( operand == NULL )
  {
      printmsg(0, "Control message %s missing operand, ignored",
                 cmd );
      free( ctrl );
      return;
  }

/*--------------------------------------------------------------------*/
/*    We always honor cancel messages, since they are pretty          */
/*    frequent and trivial.                                           */
/*--------------------------------------------------------------------*/

  if (equali(cmd, "cancel"))
  {
    printmsg(1, "Canceling article %s", operand );
    cancel_article(history, operand);
    free( ctrl );
    return;
  }

/*--------------------------------------------------------------------*/
/*                       Log all other commands                       */
/*--------------------------------------------------------------------*/

  sprintf( buf,
           "-wf %s -s\"(%s) %.100s\" %s",
           filename,
           (const char *) (bflag[F_HONORCTRL] ? "executed" : "suppressed"),
           control,
           E_postmaster );          /* Do we need newsmaster as well? */

  (void) execute( "rmail", buf, NULL, NULL, TRUE, FALSE);

/*--------------------------------------------------------------------*/
/*         Other messages require user authorize the commands         */
/*--------------------------------------------------------------------*/

  if ( ! bflag[F_HONORCTRL] )
  {
     free( ctrl );
     return;
  }

/*--------------------------------------------------------------------*/
/*              The command is authorized, let's do it.               */
/*--------------------------------------------------------------------*/

  if (equali(cmd, "newgroup"))
  {
    mod = strtok(NULL, WHITESPACE);
    moderated = (mod != NULL) && equal(mod, "moderated");
    add_newsgroup(operand, moderated);
    printmsg(1, "rnews: newsgroup added: %s", operand);
  }
  else if (equali(cmd, "rmgroup"))
  {
    del_newsgroup(operand);
    printmsg(1, "rnews: newsgroup removed: %s", operand);
  }
  else if (equali(cmd, "ihave") ||
           equali(cmd, "sendme") ||
           equali(cmd, "sendsys") ||
           equali(cmd, "version") ||
           equali(cmd, "checkgroups"))
  {
    printmsg(1, "rnews: control message not implemented: %s", cmd);
  }
  else {
    printmsg(1, "rnews: control message unknown: %s", cmd);
  }

  free(ctrl);

} /* control_message */

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

   if ((output = FOPEN(filename, "w", TEXT_MODE)) == nil(FILE))
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

   return TRUE;         /* Report the file is posted                 */

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
/*    c o p y _ s n e w s                                             */
/*                                                                    */
/*    Process news destined for the simple news reader                */
/*--------------------------------------------------------------------*/

static int copy_snews( char *filename, FILE *stream )
{
   char buf[BUFSIZ];
   size_t len;

   FILE *out_stream = FOPEN(filename, "w", IMAGE_MODE);

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

/*--------------------------------------------------------------------*/
/*       s h a d o w _ n e w s                                        */
/*                                                                    */
/*       Special hack for creating mirror images of the news feed     */
/*--------------------------------------------------------------------*/

void shadow_news(FILE *in_stream)
{
  char *sysname;

  sysname = getenv( "UUPCSHADOWS" );

  if ( sysname != NULL )
  {
     strcpy( buf, sysname);
     sysname = strtok( buf, WHITESPACE );
     while( sysname != NULL )
     {
        printmsg(1, "Shadowing news to %s", sysname );
        xmit_news( sysname, in_stream );
        sysname = strtok( NULL, WHITESPACE );
     }
  } /* if */

} /* shadow_news */

/* note PATH is not changed, SNEWS places us in the path itself */
static void copy_article(char *filename,
                         FILE *input)
{
  FILE *output;

  printmsg(2, "rnews: Saving SNEWS article in %s", filename);

  output = fopen(filename, "wb");
  if (output == NULL)
  {
    printerr( filename );
    panic();
  }

  rewind(input);

  while (fgets(buf, sizeof buf, input) != NULL)
  {
    if (fputs(buf, output) == EOF)
    {
       printerr( filename );
       panic();
    }
  } /* while */

  fclose(output);

} /* copy_article */

/*--------------------------------------------------------------------*/
/*       d e l i v e r _ l o c a l                                    */
/*                                                                    */
/*       Deliver an article locally to one or more news groups        */
/*--------------------------------------------------------------------*/

static boolean deliver_local(FILE *tfile,
                             long art_size,
                             char *newsgroups,
                             char *messageID,
                             char *control)
{
  char hist_record[DISNEY];
  char groupy[MAXGRP];
  int  groups_found;
  char snum[10];

  char *gc_ptr;
  char *gc_ptr1;

  boolean b_xref = FALSE;
  boolean posted = FALSE;  /* Used to determine if article goes to "JUNK" */

  loc_articles++;

/*--------------------------------------------------------------------*/
/*       SNEWS direct file copy.  No checks are done                  */
/*                                                                    */
/*       We're feeding SNEWS style system, but they don't want it     */
/*       put in directly, so we'll store it in temp for batching      */
/*--------------------------------------------------------------------*/

   if (bflag[F_SNEWS])
   {

     char fname[FILENAME_MAX];
     char dirname[FILENAME_MAX];
     char artlist[FILENAME_MAX];
     FILE *artfiles;

     strcpy(dirname, E_newsdir);
     strcat(dirname, "/outgoing/");
     strncat(dirname, E_nodename, 8);    /* eight chars of significance */

     strcpy(artlist, dirname);
     strcat(artlist, "/togo");

     artfiles = fopen(artlist, "a+b");
     if (artfiles == NULL)
     {
       printerr(artlist);
       panic();
     }

     mkdirfilename(fname, dirname, "art");
     strcpy(artlist, fname+strlen(dirname)+1); /* get bare filename */

     fprintf(artfiles, "outgoing/%.8s/%s\n", E_nodename, artlist);
     fclose(artfiles);

     copy_article(fname, tfile);

     return TRUE;

   } /* if (bflag[F_SNEWS]) */

   if (control)
   {
      control_message(control, art_fname );
      return TRUE;
   }

/*--------------------------------------------------------------------*/
/*           Check whether article has been received before           */
/*--------------------------------------------------------------------*/

   if ( bflag[ F_HISTORY ] )
   {
      if (get_histentry(history, messageID) != NULL)
      {
         printmsg(2, "rnews: Duplicate article %s", messageID);

         if (get_snum("duplicates", snum))
         {
            memcpy(newsgroups, "duplicates\0\0", 12);
            sprintf(messageID, "<%s.duplicate.%s@%s>",
                    snum, E_nodename, E_domain); /* we need a new unique ID */
            b_xref = NULL;
         }
         else {
            fclose(tfile);
            return FALSE;
         }

      } /* if (get_histentry(history, messageID) != NULL) */

      /* Start building the history record for this article */

      sprintf(hist_record, "%ld %ld ", now, art_size);
      groups_found = 0;

      for (gc_ptr = newsgroups; gc_ptr != NULL; gc_ptr = gc_ptr1)
      {
         if ((gc_ptr1 = strchr(gc_ptr, ', ')) != NULL)
            *gc_ptr1++ = '\0';

         if (strlen(gc_ptr) > MAXGRP - 1)
         {
            /* Bounds check the newsgroup length */
            printmsg(0, "rnews: newsgroup name too long -- %s", gc_ptr1);
            continue; /* Punt the newsgroup history record */
         }

         strcpy(groupy, gc_ptr);

         if (get_snum(groupy, snum))
         {
           if (groups_found)
           strcat(hist_record, ", ");
           strcat(hist_record, groupy);
           strcat(hist_record, ":");
           strcat(hist_record, snum);
           groups_found++;
         }

      }  /* for (gc_ptr = newsgroups; gc_ptr != NULL; gc_ptr = gc_ptr1) */

      /* Restore the newsgroups line */

      while (newsgroups[strlen(newsgroups)+1] != '\0')
         newsgroups[strlen(newsgroups)] = ', ';

      if (groups_found == 0)
      {
        printmsg(2, "rnews: no group to deliver to: %s", messageID);
        memcpy(newsgroups, "junk\0\0", 6);
        b_xref = FALSE;

        /* try "junk" group if none of the target groups is known here */

        if (get_snum("junk", snum))
          sprintf(hist_record, "%ld %ld junk:%s", now, art_size, snum);
        else {
          fclose(tfile);
          return FALSE;
        }
      }

      /* Post the history record */

      add_histentry(history, messageID, hist_record);

   } /* if ( bflag[ F_HISTORY ] ) */

/*--------------------------------------------------------------------*/
/*              Now build the Xref: line (if we need to)              */
/*--------------------------------------------------------------------*/

   if (b_xref)
   {
      strcpy(hist_record, "Xref: ");
      strcat(hist_record, E_nodename);

      for (gc_ptr = newsgroups; gc_ptr != NULL; gc_ptr = gc_ptr1)
      {
         if ((gc_ptr1 = strchr(gc_ptr, ', ')) != NULL)
            *gc_ptr1++ = '\0';
         if (strlen(gc_ptr) > MAXGRP - 1)
         {
            /* Bounds check the newsgroup length */
            printmsg(0, "rnews: newsgroup name too long -- %s", gc_ptr);
            continue; /* Punt the newsgroup history record */
         }
         strcpy(groupy, gc_ptr);
         if (get_snum(groupy, snum))
         {
            strcat(hist_record, " ");
            strcat(hist_record, groupy);
            strcat(hist_record, ":");
            strcat(hist_record, snum);
         }
      } /* for (gc_ptr = newsgroups; gc_ptr != NULL; gc_ptr = gc_ptr1) */

      strcat(hist_record, "\n");

      /* Restore the newsgroups line */

      while (newsgroups[strlen(newsgroups)+1] != '\0')
         newsgroups[strlen(newsgroups)] = ', ';

   } /* if (b_xref) */

/*--------------------------------------------------------------------*/
/*       We now need to copy the file to each group in groupys        */
/*--------------------------------------------------------------------*/

   for (gc_ptr = newsgroups; gc_ptr != NULL; gc_ptr = gc_ptr1)
   {

      if ((gc_ptr1 = strchr(gc_ptr, ', ')) != NULL)
         *gc_ptr1++ = '\0';

      strcpy(groupy, gc_ptr);

      if ( copy_file(tfile, groupy, b_xref ? hist_record : NULL))
         posted = TRUE;

   } /* for (gc_ptr = newsgroups; gc_ptr != NULL; gc_ptr = gc_ptr1) */

   if ( ! posted)
   {
     junked++;

     if ( !copy_file(tfile, "junk", b_xref ? hist_record : NULL ) )
     {
        printmsg(0, "rnews: error, but no group is available for junk!");
     }

   } /* if ( ! posted) */

   fclose(tfile);

   return TRUE;

} /* deliver_local */

/*--------------------------------------------------------------------*/
/*       c o p y _ r m t _ a r t i c l e                              */
/*                                                                    */
/*       Perform the low level copy for an article destined for       */
/*       a remote system                                              */
/*--------------------------------------------------------------------*/

static void copy_rmt_article(char *filename, FILE *input)
{

  FILE *output;

  printmsg(2, "rnews: Saving remote article in %s", filename);

  output = fopen(filename, "wb");
  if (output == NULL)
  {
    printerr( filename );
    panic();
  }
  rewind(input);

  while (fgets(buf, sizeof buf, input) != NULL)
  {

     if (equalni(buf, "Path:", strlen("Path:")))
     {
       sprintf(temp, "Path: %s!%s",
                     E_fdomain
                     buf + strlen("Path:") + 1);
       strcpy(buf, temp);
     }

     if (fputs(buf, output) == EOF)
     {
        printerr( filename );
        panic();
     }
  } /* while */

  fclose(output);

} /* copy_rmt_article */

/*--------------------------------------------------------------------*/
/*       d e l i v e r _ r e m o t e                                  */
/*                                                                    */
/*       Queue a file to be sent to a remote system                   */
/*--------------------------------------------------------------------*/

static boolean deliver_remote(struct sys *node, FILE *tfile, char *msgID)
{

  char fname[FILENAME_MAX];
  char dirname[FILENAME_MAX];
  char artlist[FILENAME_MAX];
  FILE *artfiles;

  fwd_articles++; /* lets count this one */

  strcpy(dirname, E_newsdir);
  strcat(dirname, "/outgoing/");
  strncat(dirname, node->sysname, 8);    /* eight chars of significance */

  strcpy(artlist, dirname);
  strcat(artlist, "/togo");

  artfiles = fopen(artlist, "a+b");
  if (artfiles == NULL)
  {
    printerr(artlist);
    panic();
  }

  mkdirfilename(fname, dirname, "art");
  strcpy(artlist, fname+strlen(dirname)+1); /* get bare filename */

  fprintf(artfiles, "outgoing/%.8s/%s\n", node->sysname, artlist);
  fclose(artfiles);

  copy_rmt_article(fname, tfile);

  return TRUE;

} /* deliver_remote */
