/*--------------------------------------------------------------------*/
/*    r n e w s . c                                                   */
/*                                                                    */
/*    Receive incoming news into the news directory.                  */
/*                                                                    */
/*    Written by Mike Lipsie; modified for UUPC/extended 1.11s by     */
/*    Andrew H. Derbyshire.                                           */
/*                                                                    */
/*    1993/06/12:                                                     */
/*                                                                    */
/*    Rewritten by Mike McLagan (mmclagan@invlogic.com) to make code  */
/*    behave much more like a typical RNEWS.  Using a KWBoolean config  */
/*    option USESYSFILE causes RNEWS to read and interpret the sys    */
/*    file entries for redistributing news to other sites.  Each      */
/*    article is reviewed seperately for all systems listed in that   */
/*    file.  Later, using a KWBoolean config option BATCHNEWS and      */
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
 *       $Id: rnews.c 1.42 1995/01/07 16:21:38 ahd Exp $
 *
 *       $Log: rnews.c $
 *       Revision 1.42  1995/01/07 16:21:38  ahd
 *       Change KWBoolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *       Revision 1.41  1995/01/05 03:43:49  ahd
 *       rnews SYS file support
 *
 *       Revision 1.40  1995/01/03 05:32:26  ahd
 *       Further SYS file support cleanup
 *
 *       Revision 1.39  1995/01/02 05:03:27  ahd
 *       Pass 2 of integrating SYS file support from Mike McLagan
 *
 *       Revision 1.38  1994/12/31 03:49:18  ahd
 *       Mike McLagan's new version supporting SYS file for news routing
 *
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
 * Revision 1.26  1993/12/24  05:12:54  ahd
 * Kai Uwe Rommel's fixes for news
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
         "$Id: rnews.c 1.42 1995/01/07 16:21:38 ahd Exp $";

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include <ctype.h>
#include <fcntl.h>
#include <io.h>
#include <process.h>
#include <sys/stat.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "active.h"
#include "getopt.h"
#include "getseq.h"
#include "history.h"
#include "hostable.h"
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
#define LARGEBUF (BUFSIZ*2)
#else
#define LARGEBUF (BUFSIZ*4/3)
#endif

#define FROM         "From:"
#define PATH         "Path:"
#define DISTRIBUTION "Distribution:"
#define CONTROL      "Control:"
#define MESSAGEID    "Message-ID:"
#define NEWSGROUPS   "Newsgroups:"
#define XREF         "Xref:"

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

currentfile();

extern struct grp *group_list;   /* List of all groups */
static time_t now;
static void *history;

static int articles     = 0;
static int bad_articles = 0;
static int no_delivery  = 0;
static int junked       = 0;   /* counts SNEWS articles copied to JUNK */
static int ignored      = 0;
static int ourgroups    = 0;
static int loc_articles = 0;  /* How many articles were for me */
static int fwd_articles = 0;  /* How many articles were for others? */

/*--------------------------------------------------------------------*/
/*                    Header information structure                    */
/*--------------------------------------------------------------------*/

typedef struct _HEADERLIST
{
   char *name;
   char *data;
   char *defaultData;
   char *cache;
   size_t nameLength;
   size_t cacheLength;

} HEADERLIST;

/*--------------------------------------------------------------------*/
/*                       Functions in this file                       */
/*--------------------------------------------------------------------*/

static void deliver_article(const char *art_fname,
                            const long art_size);
                              /* Distribute the article to the
                                 proper newsgroups                    */

static void control_message(const char *control,
                            const char *filename );
                              /* process control message */

static int Single( const char *filename , FILE *stream );

static int Compressed( const char *filename ,
                       FILE *in_stream ,
                       const char *unpacker ,
                       const char *suffix );

static int Batched( const char *filename, FILE *stream);

static void shadow_news(const char *fname );

static KWBoolean deliver_local(FILE *tfile,
                             const long art_size,
                             const char *groups,
                             const char *msgID,
                             const char *control);

static KWBoolean deliver_remote(const struct sys *node,
                              FILE *tfile,
                              const char *fname,
                              const char *msgID,
                              const char *path);

static KWBoolean batch_remote(const struct sys *node,
                            FILE *tfile,
                            const char *msgID );

static KWBoolean copy_file(FILE *f,
                      const char *group,
                      const char *xref);      /* Copy file (f) to newsgroup */

static int copy_snews( const char *filename, FILE *stream );

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

   checkname( E_nodename );      /* Fill in fdomain                   */

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
                  strcpy(filename, optarg);

                  input = fopen(filename, "rb");

                  if (input == NULL)
                  {
                     printerr( filename );
                     panic();
                  }
                  else
                     printmsg(2, "Opened %s as input file", filename);

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
/*    If we are processing snews input, write it all out to the       */
/*    news directory as one file and return gracefully.               */
/*--------------------------------------------------------------------*/

   if ( bflag[F_SNEWS])
   {

      mkdirfilename(filename, E_newsdir, "art"); /* Get the file name   */
      status = copy_snews(filename, input);
                                    /* Dump news into NEWS directory */

      shadow_news(filename);        /* Shadow it via UUX             */

      exit( status );
   }
   else
      mktempname(filename, "tmp"); /* Make normal temp name          */

/*--------------------------------------------------------------------*/
/*             Load the active file and validate its data             */
/*--------------------------------------------------------------------*/

   get_active();           /* Get sequence numbers for groups
                              from active file                 */

/*--------------------------------------------------------------------*/
/*                 Open (or create) the history file                  */
/*--------------------------------------------------------------------*/

   history = open_history("history");
   if ( history == NULL )
      panic();

/*--------------------------------------------------------------------*/
/*                   Initialize sys file processing                   */
/*--------------------------------------------------------------------*/

  init_sys();                 /* Program aborts if init fails        */

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
/*               Summarize the results of our processing              */
/*--------------------------------------------------------------------*/

   printmsg( ((articles == loc_articles) && ! no_delivery)  ? 2 : 1,
             "%s: Received %d articles, of which %d were bad and "
             "%d were undeliverable.",
             argv[0],
             articles,
             bad_articles,
             no_delivery);

   if ( loc_articles || ignored || junked )
      printmsg(1, "%s: Retained %d articles, "
                  "of which %d were duplicates and %d were junked.",
                  argv[0],
                  loc_articles,
                  ignored,
                  junked);

   if ( fwd_articles )
   {
      struct sys *node = sys_list;

      printmsg(4, "%s: Forwarded %d articles.", argv[0], fwd_articles);

      while( node )
      {
         if ( node->processed )
            printmsg(1,"%s: %ld articles sent to %s",
                       argv[0],
                       node->processed,
                       node->sysname );
         node = node->next;
      }

   } /* if ( fwd_articles ) */

/*--------------------------------------------------------------------*/
/*                     Clean up and return to caller                  */
/*--------------------------------------------------------------------*/

   put_active();

   close_history(history);

   exit_sys();

   exit(status);

} /*main*/

/*--------------------------------------------------------------------*/
/*    S i n g l e                                                     */
/*                                                                    */
/*    Deliver a single article to the proper news group(s)            */
/*--------------------------------------------------------------------*/

static int Single( const char *filename , FILE *stream )
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

static int Compressed( const char *filename ,
                       FILE *in_stream ,
                       const char *unpacker ,
                       const char *suffix )
{

   FILE *work_stream;

   char zfile[FILENAME_MAX];
   char unzfile[FILENAME_MAX];
   char buf[BUFSIZ];

   long cfile_size = 0L;
   size_t chars_read, i;
   int status = 0;
   KWBoolean needtemp = KWTrue;

/*--------------------------------------------------------------------*/
/*        Copy the compressed file to the "holding" directory         */
/*--------------------------------------------------------------------*/

   while( needtemp )
   {
      mktempname( zfile , suffix );    /* Generate "compressed" file
                                          name                        */
      strcpy( unzfile, zfile );
      unzfile[ strlen(unzfile)-2 ] = '\0';

      if ( access( unzfile, 0 ))  /* Does the host file exist?        */
         needtemp = KWFalse;       /* No, we have a good pair          */
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
         unlink( zfile );     /* Kill the compressed input file       */
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
   status = executeCommand( buf, NULL, NULL, KWTrue, KWFalse);

   unlink( zfile );           /* Kill the compressed input file       */

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

                              /* Create uncompressed output file name */

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
/*    f i x E O F                                                     */
/*                                                                    */
/*    Zap Cntrl-Z characters in the input stream                      */
/*--------------------------------------------------------------------*/

static void fixEOF( char *buf, const int bytes )
{
   static warn = KWTrue;
   int left = bytes;

   while ( left-- )
   {
      if ( *buf == ('Z' - 'A'))
      {
         *buf = 'Z';
         if ( warn )
         {
            printmsg(0, "Altered Cntl-Z to Z");
            warn = KWFalse;
         } /* if */
      } /* if */
   } /* while */

} /* fixEOF */

/*--------------------------------------------------------------------*/
/*    B a t c h e d                                                   */
/*                                                                    */
/*    Handle batched, uncompressed news                               */
/*--------------------------------------------------------------------*/

static int Batched( const char *filename, FILE *stream)
{

   char buf[BUFSIZ * 2];
   int status = 0;
   long article_size;
   KWBoolean gotsize = KWFalse;
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
            gotsize = KWTrue;
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
      gotsize = KWFalse;

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
               gotsize = KWTrue;
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
/*      Close the file, deliver its contents, and get rid of it       */
/*--------------------------------------------------------------------*/

      fclose(tmpf);
      deliver_article(filename, article_size);
      unlink( filename );
   } /* while */

   return status;

} /* Batched */

/*--------------------------------------------------------------------*/
/*       g e t H e a d e r                                            */
/*                                                                    */
/*       Retrieve a header data field from our header table           */
/*--------------------------------------------------------------------*/

static char *getHeader( HEADERLIST table[],
                        const char *field,
                        char *defaultData )
{
   int subscript;

   for ( subscript = 0; table[subscript].name != NULL; subscript++ )
   {

      if ( equal(table[subscript].name, field ) )  /* Correct header?   */
      {                                      /* Yes --> process it      */

         if ( table[subscript].data )        /* Previously loaded data? */
            return table[subscript].data;    /* Yes --> Return it.      */

         if ( table[subscript].defaultData &&
              strlen( table[subscript].defaultData))  /* Table default? */
         {                                   /* Yes --> use it          */

            table[subscript].data = table[subscript].defaultData;

            return table[subscript].data;
         }

         return defaultData;        /* No table default, return caller
                                       supplied value instead.          */

      } /* if ( equal(table[subscript].name, field ) */

   }  /* for ( subscript = 0; table[subscript].name != NULL; subscript++ ) */

/*--------------------------------------------------------------------*/
/*      We never found the specified field, critical internal error   */
/*--------------------------------------------------------------------*/

   printmsg(0, "Unable find requested header %s in header table",
               field );

   panic();

} /* getHeader */

/*--------------------------------------------------------------------*/
/*       d e l i v e r _ a r t i c l e                                */
/*                                                                    */
/*       This function processes an article by looking up each        */
/*       entry in the SYS file, and determines if the article is      */
/*       to be sent to a given system.  This includes our own         */
/*       system, which is also controlled by a SYS entry.  Once       */
/*       delivery is decided on, we pass the article off to the       */
/*       actual local or remote delivery function as required.        */
/*--------------------------------------------------------------------*/

static void deliver_article(const char *art_fname, const long art_size)
{

   FILE *tfile = FOPEN(art_fname, "r", IMAGE_MODE);
   struct sys *sysnode = sys_list;

   static HEADERLIST table[] =
   {
      { PATH,            NULL, NULL,    NULL, 0, 0 },
      { NEWSGROUPS,      NULL, NULL,    NULL, 0, 0 },
      { MESSAGEID,       NULL, NULL,    NULL, 0, 0 },
      { FROM,            NULL, NULL,    NULL, 0, 0 },
      { DISTRIBUTION,    NULL, "world", NULL, 0, 0 },
      { CONTROL,         NULL, "",      NULL, 0, 0 },
      { NULL }
   };

   KWBoolean delivered = KWFalse;
   KWBoolean searchHeader = KWTrue;   /* Each article begins w/header  */
   KWBoolean error    = KWFalse;      /* Presume successful hdr scan   */
   int subscript;                   /* For walking header table      */

   articles++;

   if ( tfile == NULL )             /* Did the article file open?    */
   {                                /* No --> Now THAT's a problem!  */
      printerr( art_fname );
      panic();
   }

/*--------------------------------------------------------------------*/
/*                   Reinitialize our header table                    */
/*--------------------------------------------------------------------*/

   for ( subscript = 0; table[subscript].name != NULL; subscript++ )
      table[subscript].data = NULL;

/*--------------------------------------------------------------------*/
/*          Process the header, extracting all needed fields          */
/*--------------------------------------------------------------------*/

   while ( searchHeader )
   {
      char input[LARGEBUF];

      if ( fgets(input, sizeof input, tfile) == NULL )   /* eof ?    */
         searchHeader = KWFalse;     /* Yes --> Exit loop ...         */
      else if ( *input == '\n' )    /* Last of the red hot headers?  */
         searchHeader = KWFalse;     /* Yes --> Exit loop gracefully  */
      else for ( subscript = 0; table[subscript].name != NULL; subscript++ )
      {
         char *s;
         size_t stringLength;

         /* Initialize lookaside of entry length, if needed  */

         if ( ! table[subscript].nameLength )
            table[subscript].nameLength = strlen( table[subscript].name );

         if ( !equalni( table[subscript].name,
                       input,
                       table[subscript].nameLength )) /* Desired header? */
            continue;                     /* No --> Look at next one */

/*--------------------------------------------------------------------*/
/*          Trim trailing blanks off the data part of header          */
/*--------------------------------------------------------------------*/

         s = input + strlen( input ) - 1;

         while( s > input && isspace( *s ))
            *s-- = '\0';

/*--------------------------------------------------------------------*/
/*                Find first non-blank in header data                 */
/*--------------------------------------------------------------------*/

         s = input + strlen( table[subscript].name );

         while( *s && isspace( *s ) )
            s++;

/*--------------------------------------------------------------------*/
/*                          Validate header                           */
/*--------------------------------------------------------------------*/

         if ( table[subscript].data )  /* Duplicate field?           */
         {                             /* Yes --> Drop article       */

            printmsg(1,"Article %s has duplicate header for %s, discarded",
                        getHeader( table, MESSAGEID, "Unknown" ),
                        table[subscript].name );

            error = KWTrue;
            continue;

         } /* if ( table[subscript].data ) */

         if ( *s == '\0' )          /* Empty header field?        */
         {                          /* Yes --> Drop article       */

            printmsg(1,"Article %s has empty header for %s, discarded",
                        getHeader( table, MESSAGEID, "Unknown" ),
                        table[subscript].name );

            error = KWTrue;
            continue;

         } /* if ( s == '\0' ) */

/*--------------------------------------------------------------------*/
/*       Allocate a holding buffer for the string as needed, and      */
/*       save the string with an extra terminator byte for            */
/*       rescans.                                                     */
/*                                                                    */
/*       Note the header table includes a cache pointer for the       */
/*       last header of this type, which allows us to keep around     */
/*       the longest buffer needed for this header without either     */
/*       needing a fixed length buffer (which tends to overflow on    */
/*       alt.stupid.choke.choke.choke.  . . . .choke.rnews), and      */
/*       reallocating the buffers every pass, which can fragment      */
/*       the heap when using inferior compilers.                      */
/*--------------------------------------------------------------------*/

         stringLength = strlen( s ) + 2;  /* Plus string terminator
                                             and rescan terminator   */

         if ( table[subscript].cacheLength < stringLength )
         {
            if ( table[subscript].cache )       /* If old cache      */
               free( table[subscript].cache );  /* Drop it           */

            table[subscript].cache = malloc( stringLength );
            checkref( table[subscript].cache );

            table[subscript].cacheLength = stringLength;
         }

         memcpy( table[subscript].cache, s, stringLength - 1);
         table[subscript].data = table[subscript].cache; /* Now valid */

         /* Pad string with extra null for tokenizing  */

         table[subscript].data[ stringLength - 1] = '\0';

      } /* for ( subscript = 0; table[subscript].name != NULL; subscript++ ) */

   }  /* while ( header && fgets(input, sizeof input, tfile)) */

/*--------------------------------------------------------------------*/
/*                Verify we did not have a file error                 */
/*--------------------------------------------------------------------*/

   if ( ferror( tfile ) )        /* Exit loop from error?            */
   {
      printerr( art_fname );     /* Yes --> Report it ...            */
      panic();                   /* ... and die young                */
   }

/*--------------------------------------------------------------------*/
/*                  Verify we got our needed headers                  */
/*--------------------------------------------------------------------*/

   for ( subscript = 0; table[subscript].name != NULL; subscript++ )
   {
      if ( ! table[subscript].data && ! table[subscript].defaultData )
      {
            printmsg(1,"Article %s missing header for %s, discarded",
                        getHeader( table, MESSAGEID, "Unknown" ),
                        table[subscript].name );

            error = KWTrue;

      } /* if ( ! table[subscript].data && ! table[subscript].defaultData ) */

   } /* for ( subscript = 0; table[subscript].name != NULL; subscript++ ) */

/*--------------------------------------------------------------------*/
/*     Terminate processing of article if we had a logical error      */
/*--------------------------------------------------------------------*/

   if ( error )
   {
      fclose( tfile );
      bad_articles++;
      return;
   }

/*--------------------------------------------------------------------*/
/*           We have the header fields, deliver the article           */
/*--------------------------------------------------------------------*/

  while (sysnode != NULL)
  {
    if (check_sys(sysnode,
                  getHeader(table, NEWSGROUPS, NULL),
                  getHeader(table, DISTRIBUTION, NULL),
                  getHeader(table, PATH, NULL)))
    {
      if (equal(sysnode->sysname, E_domain))
      {
        if (deliver_local( tfile,
                           art_size,
                           getHeader(table, NEWSGROUPS, NULL),
                           getHeader(table, MESSAGEID, NULL),
                           getHeader(table, CONTROL, NULL)))
          delivered = KWTrue;
        else
          ignored++;
      }
      else if (deliver_remote(sysnode,
                              tfile,
                              art_fname,
                              getHeader(table, MESSAGEID, NULL),
                              getHeader(table, PATH, NULL )))
      {
         fwd_articles++;
         sysnode->processed ++;
         delivered = KWTrue;
      }

    }

    sysnode = sysnode -> next;

  }

  if (!delivered)
  {
    no_delivery++;
    printmsg(0, "deliver_article: Article %.40s undeliverable to "
                "groups %.80s, distribution %.40s",
                getHeader(table, MESSAGEID, NULL),
                getHeader(table, NEWSGROUPS, NULL),
                getHeader(table, DISTRIBUTION, NULL) );
  }

  fclose(tfile);

  return;

} /* deliver_article */

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
  KWBoolean moderated;
  char buf[200];
  char *operand;

  printmsg(1, "Control Message: %s", control);

/*--------------------------------------------------------------------*/
/*                     Parse the command verb off                     */
/*--------------------------------------------------------------------*/

  cmd = strtok(ctrl, WHITESPACE);      /* Discard first token         */

  if ( cmd != NULL )
     cmd = strtok(NULL, WHITESPACE);   /* Get second token, if any    */

  if ( cmd == NULL )
  {
      printmsg(0, "Control message missing verb, ignored");
      free( ctrl );
      return;
  }

/*--------------------------------------------------------------------*/
/*              Get the first operand, which we require               */
/*--------------------------------------------------------------------*/

  operand = strtok( NULL, WHITESPACE); /* Get first (required) op     */

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

  (void) execute( "rmail", buf, NULL, NULL, KWTrue, KWFalse);

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

static KWBoolean copy_file(FILE *input,
                         const char *group,
                         const char *xref)
{
   struct grp *cur;
   char filename[FILENAME_MAX];
   char buf[BUFSIZ];
   FILE *output;
   KWBoolean header = KWTrue;

/*--------------------------------------------------------------------*/
/*           Determine if the news has been already posted            */
/*--------------------------------------------------------------------*/

   cur = find_newsgroup(group);

   if (cur == NULL)
   {
      printmsg(3, "rnews: Article cross-posted to %s", group);
      return KWFalse;
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
      return KWFalse;
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
         header = KWFalse;
      else if (equalni(buf, PATH, strlen(PATH)))
      {
         fprintf(output,
                 PATH " %s!%s\n",
                 E_domain,
                 strtok(buf + strlen(PATH) + 1, WHITESPACE ));
         continue;
      }
      else if (equalni(buf, XREF, strlen(XREF)))
         continue; /* skip possibly old Xref: line */

      if (fputs(buf, output) == EOF)
      {
         printerr( filename );
         panic();
      }

   } /* while */

   fclose(output);

   return KWTrue;        /* Report the file is posted                  */

} /* copy_file */

/*--------------------------------------------------------------------*/
/*    c o p y _ s n e w s                                             */
/*                                                                    */
/*    Process news destined for the simple news reader                */
/*--------------------------------------------------------------------*/

static int copy_snews( const char *filename, FILE *stream )
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

void shadow_news( const char *fname )
{

  char *sysName = getenv( "UUPCSHADOWS" );

  if ( sysName != NULL )
  {
     char *buf = strdup( sysName );
     checkref(buf);

     sysName = strtok( buf, WHITESPACE );

     while( sysName != NULL )
     {
        char commandOptions[BUFSIZ];

        printmsg(1, "Shadowing news to %s", sysName );

        sprintf(commandOptions, "-p -g%c -n -x %d -C %s!rnews",
                E_newsGrade,
                debuglevel,
                sysName );

        execute( "UUX", commandOptions, fname, NULL, KWTrue, KWFalse );

        sysName = strtok( NULL, WHITESPACE );
     }

     free( buf );

  } /* if */

} /* shadow_news */

/*--------------------------------------------------------------------*/
/*       d e l i v e r _ l o c a l                                    */
/*                                                                    */
/*       Deliver an article locally to one or more news groups        */
/*--------------------------------------------------------------------*/

static KWBoolean deliver_local(FILE *tfile,
                             const long art_size,
                             const char *newsgroups_in,
                             const char *messageID,
                             const char *control)
{
  char hist_record[LARGEBUF];
  char groupy[MAXGRP];
  char *newsgroups = NULL;
  char *msgID = (char *) messageID;
  char idBuffer[FILENAME_MAX];
  int  newsgroups_len = strlen( newsgroups_in ) + 1;
  int  groups_found;
  char snum[10];

  char *gc_ptr;
  char *gc_ptr1;

  KWBoolean b_xref = KWFalse;
  KWBoolean posted = KWFalse;  /* Used to determine if article goes to "JUNK" */

  loc_articles++;

   if (control)
   {
      control_message(control, BIT_BUCKET );
      return KWTrue;
   }

/*--------------------------------------------------------------------*/
/*              Copy our news groups line for processing              */
/*--------------------------------------------------------------------*/

   newsgroups = malloc( newsgroups_len + 1 );
   checkref( newsgroups );
   memcpy( newsgroups, newsgroups, newsgroups_len );

/*--------------------------------------------------------------------*/
/*           Check whether article has been received before           */
/*--------------------------------------------------------------------*/

   if (get_histentry(history, messageID) != NULL)
   {
      printmsg(1, "rnews: Duplicate article %s", messageID);

      if (get_snum("duplicates", snum))
      {
         memcpy(newsgroups, "duplicates\0\0", 12);
         sprintf(idBuffer, "<%s.duplicate.%.10s@%.50s>",
                 snum,
                 E_nodename,
                 E_domain);         /* We need a new unique ID       */
         msgID = idBuffer;
         b_xref = KWFalse;
      }
      else {
         free( newsgroups );
         return KWFalse;
      }

      ignored++;

   } /* if (get_histentry(history, messageID) != NULL) */

   /* Start building the history record for this article */

   sprintf(hist_record, "%ld %ld ", now, art_size);
   groups_found = 0;

   for (gc_ptr = newsgroups; gc_ptr != NULL; gc_ptr = gc_ptr1)
   {
      if ((gc_ptr1 = strchr(gc_ptr, ',')) != NULL)
         *gc_ptr1++ = '\0';

      if (strlen(gc_ptr) > sizeof groupy  - 1)
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

   memcpy( newsgroups, newsgroups_in, newsgroups_len );

   if (groups_found == 0)
   {
     printmsg(2, "rnews: no group to deliver to: %s", messageID );
     memcpy(newsgroups, "junk\0\0", 6);
     b_xref = KWFalse;

     /* try "junk" group if none of the target groups is known here */

     if (get_snum("junk", snum))
       sprintf(hist_record, "%ld %ld junk:%s", now, art_size, snum);
     else {
       free( newsgroups );
       return KWFalse;
     }
   }

   /* Post the history record */

   add_histentry(history, msgID, hist_record);

/*--------------------------------------------------------------------*/
/*              Now build the Xref: line (if we need to)              */
/*--------------------------------------------------------------------*/

   if (b_xref)
   {
      strcpy(hist_record, "Xref: ");
      strcat(hist_record, E_domain);

      for (gc_ptr = newsgroups; gc_ptr != NULL; gc_ptr = gc_ptr1)
      {
         if ((gc_ptr1 = strchr(gc_ptr, ',')) != NULL)
            *gc_ptr1++ = '\0';

         if (strlen(gc_ptr) > sizeof groupy  - 1)
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

      printmsg(0, "reached" );
      strcat(hist_record, "\n");

      /* Restore the newsgroups line */

      memcpy( newsgroups, newsgroups_in, newsgroups_len );

   } /* if (b_xref) */

/*--------------------------------------------------------------------*/
/*       We now need to copy the file to each group in groupys        */
/*--------------------------------------------------------------------*/

   for (gc_ptr = newsgroups; gc_ptr != NULL; gc_ptr = gc_ptr1)
   {

      if ((gc_ptr1 = strchr(gc_ptr, ',')) != NULL)
         *gc_ptr1++ = '\0';

      strcpy(groupy, gc_ptr);

      if ( copy_file(tfile, groupy, b_xref ? hist_record : NULL))
         posted = KWTrue;

   } /* for (gc_ptr = newsgroups; gc_ptr != NULL; gc_ptr = gc_ptr1) */

   if ( ! posted)
   {
     junked++;

     if ( !copy_file(tfile, "junk", b_xref ? hist_record : NULL ) )
     {
        printmsg(0, "rnews: error, but no group is available for junk!");
     }

   } /* if ( ! posted) */

   free( newsgroups );

   return KWTrue;

} /* deliver_local */

/*--------------------------------------------------------------------*/
/*       c o p y _ r m t _ a r t i c l e                              */
/*                                                                    */
/*       Perform the low level copy for an article destined for       */
/*       a remote system                                              */
/*--------------------------------------------------------------------*/

static void copy_rmt_article(const char *filename, FILE *input)
{

  FILE *output;

  char buf[BUFSIZ];

  KWBoolean searchHeaders = KWTrue;
  KWBoolean skipHeader   = KWFalse;

  rewind( input );

  printmsg(2, "rnews: Saving remote article in %s", filename);

  output = FOPEN(filename, "w", IMAGE_MODE );

  if (output == NULL)
  {
    printerr( filename );
    panic();
  }

  fflush( input );
  rewind(input);

  while (fgets(buf, sizeof buf, input) != NULL)
  {

     if ( searchHeaders )
     {

         if ( *buf == '\n' )        /* End of header?                */
            searchHeaders = KWFalse;
         else if (equalni(buf, PATH, strlen(PATH)))
         {
           fprintf(output,
                   "%s %s!%s\n",
                   PATH,
                   E_domain,
                   strtok( buf + strlen(PATH) + 1, WHITESPACE ));

            searchHeaders = KWFalse;
            skipHeader = KWTrue;
         }
      }

     if (!skipHeader && (fputs(buf, output) == EOF))
     {
        printerr( filename );
        panic();
     }

     skipHeader = KWFalse;

  } /* while */

  fclose(output);

} /* copy_rmt_article */

/*--------------------------------------------------------------------*/
/*       b a t c h _ r e m o t e                                      */
/*                                                                    */
/*       Queue a file to be sent to a remote system                   */
/*--------------------------------------------------------------------*/

static KWBoolean batch_remote(const struct sys *node,
                            FILE *tfile,
                            const char *msgID )
{

  char fname[FILENAME_MAX];
  char dirname[FILENAME_MAX];
  FILE *batchListStream;

/*--------------------------------------------------------------------*/
/*               Generate a copy of the actual article                */
/*--------------------------------------------------------------------*/

   sprintf( dirname, "%s/%s/%.8s",
            E_newsdir,
            OUTGOING_NEWS,
            node->sysname );
   mkdirfilename(fname, dirname, "art");

   copy_rmt_article(fname, tfile);

/*--------------------------------------------------------------------*/
/*                   Open up the article list file                    */
/*--------------------------------------------------------------------*/

  batchListStream = FOPEN(node->command, "a+", TEXT_MODE );

  if (batchListStream == NULL)
  {
    printerr(node->command);
    panic();
  }

/*--------------------------------------------------------------------*/
/*       Add the filename to the article list unless we have the      */
/*       Message-ID only flag set.                                    */
/*--------------------------------------------------------------------*/

   if ( node->flag.I )
      fputs( msgID, batchListStream );
   else
      fputs( fname + strlen( E_newsdir ) + 1, batchListStream );
                              /* Print relative file name only */

/*--------------------------------------------------------------------*/
/*                     Add the Message-ID if needed                   */
/*--------------------------------------------------------------------*/

   if ( node->flag.n )
   {
      fputc(' ', batchListStream );
      fputs( msgID, batchListStream );
   }

/*--------------------------------------------------------------------*/
/*                   Add the file size, if desired                    */
/*--------------------------------------------------------------------*/

  if ( node->flag.f )
  {
      struct stat statBuf;

      if ( fstat( fileno( tfile ), &statBuf ))
      {
         printmsg(0,"Cannot determine size of input file!");
         printerr( "stat" );
         panic();
      }

      fprintf( batchListStream, " %lu",
               (unsigned long) statBuf.st_size );

  } /* if ( node->flag.f ) */

/*--------------------------------------------------------------------*/
/*            Close the file and return success to the caller         */
/*--------------------------------------------------------------------*/

  fputc( '\n', batchListStream );    /* Terminate the line of data    */
  fclose(batchListStream);

  return KWTrue;

} /* batch_remote */

/*--------------------------------------------------------------------*/
/*       x m i t _ r e m o t e                                        */
/*                                                                    */
/*       Special hack for creating mirror images of the news feed     */
/*--------------------------------------------------------------------*/

static KWBoolean xmit_remote( const char *sysname,
                            const char *commandIn,
                            const char *articleName)
{

   char command[BUFSIZ];
   int status;

   sprintf(command, commandIn, sysname );

   status = executeCommand( command, articleName, NULL, KWTrue, KWFalse );

   if ( status )
   {
      if ( status > 0 )
         printmsg(0, "%s command failed with status %d",
                  command,
                  status);
      panic();
   }

   return KWTrue;

} /* xmit_remote */

/*--------------------------------------------------------------------*/
/*       d e l i v e r _ r e m o t e                                  */
/*                                                                    */
/*       Queue a file to be sent to a remote system                   */
/*--------------------------------------------------------------------*/

static KWBoolean deliver_remote(const struct sys *node,
                              FILE *tfile,
                              const char *fname,
                              const char *msgID,
                              const char *path)
{

/*--------------------------------------------------------------------*/
/*          Verify we make the maximum number of hops limit           */
/*--------------------------------------------------------------------*/

   if ( strlen( path ) > (node->maximumHops/2) ) /* Near limit?      */
   {
      size_t hops = 0;
      char *p = (char *) path;

      while( (p = strchr( p++, '!' )) != NULL )
         hops++;

      if ( hops > node->maximumHops )
      {
         printmsg(4,"Article %s has too many hops (%ud) to deliver to %s",
                    msgID,
                    hops,
                    node->sysname );
      }
   }

/*--------------------------------------------------------------------*/
/*         Are we batching data or processing it immediately?         */
/*--------------------------------------------------------------------*/

  if ( node->flag.F || node->flag.f || node->flag.n || node->flag.I )
     return batch_remote( node, tfile, msgID );
  else {

     printmsg(5, "Transmitting article %s to %s via command %s",
                  node->sysname,
                  msgID,
                  node->command );

     return xmit_remote( node->sysname, node->command, fname );
   }

} /* deliver_remote */
