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
 *       $Id: newsrun.c 1.5 1995/03/08 03:01:54 ahd Exp $
 *
 *       $Log: newsrun.c $
 *       Revision 1.5  1995/03/08 03:01:54  ahd
 *       Delete redundent buggy check for too many hops
 *
 *       Revision 1.4  1995/03/07 23:33:38  ahd
 *       Drop control messages into junk if no control group and junk exists
 *
 *       Revision 1.3  1995/03/06 18:27:23  rommel
 *       Correct count messages
 *       Correct handling of duplicate articles
 *       Correct cancel message processing
 *
 *       Revision 1.2  1995/02/20 00:03:07  ahd
 *       Delete improper adding of batching information
 *
 *       Revision 1.1  1995/02/12 23:37:04  ahd
 *       compiler cleanup, NNS C/news support, optimize dir processing
 *
 *       Revision 1.53  1995/02/05 00:36:38  ahd
 *       Don't report removal error for files which may not exist
 *
 *       Revision 1.52  1995/01/29 14:03:29  ahd
 *       Clean up IBM C/Set compiler warnings
 *
 *       Revision 1.51  1995/01/22 04:16:52  ahd
 *       Batching cleanup
 *
 *       Revision 1.50  1995/01/15 19:48:35  ahd
 *       Allow active file to be optional
 *       Delete fullbatch global option
 *       Add "local" and "batch" flags to SYS structure for news
 *
 *       Revision 1.49  1995/01/14 15:06:16  ahd
 *       Trap end of DOS headers as well as proper UNIX headers
 *
 *       Revision 1.48  1995/01/14 14:08:59  ahd
 *       Always reopen input stream in batched mode to prevent loss of
 *       information when running commands
 *
 *       Revision 1.47  1995/01/13 14:02:36  ahd
 *       News debugging fixes from Dave Watt
 *       Add new checks for possible I/O errors
 *
 *       Revision 1.46  1995/01/08 21:02:02  ahd
 *       Correct BC++ 3.1 compiler warnings
 *
 *       Revision 1.45  1995/01/08 19:52:44  ahd
 *       NNS support
 *
 *       Revision 1.44  1995/01/07 23:52:37  ahd
 *       Convert rnews to use in-memory files, debug associated functions
 *
 *       Revision 1.43  1995/01/07 20:48:21  ahd
 *       Correct 16 compile warnings
 *
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
         "$Id: newsrun.c 1.5 1995/03/08 03:01:54 ahd Exp $";

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
#include "imfile.h"
#include "importng.h"
#include "logger.h"
#include "timestmp.h"
#include "stater.h"

#include "execute.h"

#include "batch.h"
#include "sys.h"

/*--------------------------------------------------------------------*/
/*                           Global defines                           */
/*--------------------------------------------------------------------*/

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
static int junked       = 0;
static int retained     = 0;
static int duplicates   = 0;
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

static void deliver_article(IMFILE *imf );
                              /* Distribute the article to the
                                 proper newsgroups                    */

static void control_message(const char *control,
                            const char *filename );
                              /* process control message */

static int Single( FILE *stream );

static int Batched( FILE *stream);

static KWBoolean deliver_local(IMFILE *imf,
                             const char *groups,
                             const char *msgID,
                             const char *control);

static KWBoolean deliver_remote(const struct sys *node,
                              IMFILE *imf,
                              const char *msgID );

static KWBoolean batch_remote(const struct sys *node,
                            IMFILE *imf,
                            const char *msgID );

static KWBoolean copy_file(IMFILE *imf,
                      const char *group,
                      const char *xref);      /* Copy file (f) to newsgroup */

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

main( int argc, char **argv)
{

   FILE *input;
   char inputName[FILENAME_MAX];
   KWBoolean deleteInput = KWFalse;
   KWBoolean localNews;
   struct sys *localNode;
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

   if (!configure( B_NEWSRUN ))
      exit(1);    /* system configuration failed */

   openlog( NULL );           /* Begin logging to disk            */

   checkname( E_nodename );      /* Fill in fdomain                   */

/*--------------------------------------------------------------------*/
/*                      Parse arguments, if any.                      */
/*--------------------------------------------------------------------*/

   if (argc > 1)
   {
      int option;

       while ((option = getopt(argc, argv, "F:f:x:")) != EOF)
       {
           switch (option)
           {
               case 'F':
                  deleteInput = KWTrue;
                  /* Fall through to regular case for file input  */

               case 'f':
                  strcpy(inputName, optarg);

                  input = fopen(inputName, "rb");

                  if (input == NULL)
                  {
                     printerr( inputName );
                     panic();
                  }
                  else
                     printmsg(2, "Opened %s as input file", inputName);

                  break;

               case 'x':
                  debuglevel = atoi(optarg);
                  break;

               case '?':
                  fprintf(stderr,
                          "\nUsage:\t%s [-f newsfile] [-x debug]",
                          argv[0]);
                  exit(99);
           } /* break */

       } /* while */

/*--------------------------------------------------------------------*/
/*                Abort if any options were left over                 */
/*--------------------------------------------------------------------*/

       if (optind != argc)
       {
          puts("Extra parameter(s) at end.");
          exit(98);
       }

    } /* if (argc > 1) */

/*--------------------------------------------------------------------*/
/*                   Initialize sys file processing                   */
/*--------------------------------------------------------------------*/

   if ( ! init_sys() )
   {
     printmsg(0,"Cannot initialize from SYS file, program aborting");
     exit( 1 );
   }

   localNode = get_sys( canonical_news_name() );

   if ( localNode != NULL )
      localNews = localNode->flag.local;
   else
      localNews = KWFalse;

/*--------------------------------------------------------------------*/
/*       If the local news is to be processed normally, open up       */
/*       the history file.  Note if the file doesn't exist, it        */
/*       will be created automatically.                               */
/*--------------------------------------------------------------------*/

   if ( localNews )
   {
      history = open_history("history");

      if ( history == NULL )
         panic();
   }


/*--------------------------------------------------------------------*/
/*       Get sequence numbers for groups from active file.  The       */
/*       file is optional only if we are not updating it.             */
/*--------------------------------------------------------------------*/

   get_active( localNews );

/*--------------------------------------------------------------------*/
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
/*    (We don't actually see compressed batches here, they were       */
/*    made uncompressed by the rnews front-end.)                      */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Compressed files need to be read in BINARY mode so that         */
/*    "magic" characters (like ^Z) don't upset it.  Batched files     */
/*    need to be read in TEXT mode so that the character count is     */
/*    correct.  The other case doesn't matter.                        */
/*--------------------------------------------------------------------*/

   c = getc(input);
   ungetc(c, input);

   if (c == '#' )
      status = Batched( input );
   else
      status = Single( input );

/*--------------------------------------------------------------------*/
/*               Summarize the results of our processing              */
/*--------------------------------------------------------------------*/

   if ( ! articles )
   {
      printmsg(0,"%s No articles input to program", argv[0]);
      panic();
   }

   printmsg( ((articles == loc_articles) && ! no_delivery)  ? 2 : 1,
             "%s: Received %d articles, of which %d were bad and "
             "%d were undeliverable.",
             argv[0],
             articles,
             bad_articles,
             no_delivery);

   if ( retained || duplicates || junked )
      printmsg(1, "%s: Retained %d articles, "
                  "of which %d were duplicates and %d were junked.",
                  argv[0],
                  retained,
                  duplicates,
                  junked);

   if ( fwd_articles )
   {
      struct sys *node = sys_list;

      printmsg(4, "%s: Forwarded %d articles.", argv[0], fwd_articles);

      while( node )
      {
         if ( node->processed )
            printmsg(1,"%s: %ld articles queued for %s",
                       argv[0],
                       node->processed,
                       node->sysname );
         node = node->next;
      }

   } /* if ( fwd_articles ) */

/*--------------------------------------------------------------------*/
/*       Clean up shop and exit.  We only have to update the          */
/*       active and history files if processing local news into       */
/*       discrete files.                                              */
/*--------------------------------------------------------------------*/

   if ( localNews )
   {
      put_active();
      close_history(history);
   }

   exit_sys();

   if ( deleteInput && (status == 0 ))
      remove( inputName );

   exit(status);

   return 0;

} /* main */

/*--------------------------------------------------------------------*/
/*    S i n g l e                                                     */
/*                                                                    */
/*    Deliver a single article to the proper news group(s)            */
/*--------------------------------------------------------------------*/

static int Single( FILE *stream )
{
   IMFILE *imf;
   char buf[BUFSIZ];
   unsigned chars_read;
   unsigned chars_written;

   imf = imopen( filelength( fileno( stream )));

   if ( imf == NULL )
   {
      printerr( "imopen" );
      panic();
   }

/*--------------------------------------------------------------------*/
/*              Now copy the input into our holding bin               */
/*--------------------------------------------------------------------*/

   while ((chars_read = fread(buf, sizeof(char), BUFSIZ, stream)) != 0)
   {

      chars_written = imwrite(buf, sizeof(char), chars_read, imf);
      if (chars_written != chars_read)
      {
         printerr( "imwrite" );
         printmsg(0, "Error writing single article to working file");
         imclose( imf );
         panic();
      }
   }

/*--------------------------------------------------------------------*/
/*     Close the file, deliver the article, and return the caller     */
/*--------------------------------------------------------------------*/

   deliver_article( imf );
   return 0;

} /* Single */

/*--------------------------------------------------------------------*/
/*    f i x E O F                                                     */
/*                                                                    */
/*    Zap Cntrl-Z characters in the input stream                      */
/*--------------------------------------------------------------------*/

static void fixEOF( char *buf, const unsigned bytes )
{
   static warn = KWTrue;
   unsigned left = bytes;

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

static int Batched( FILE *streamIn)
{

   char buf[BUFSIZ * 2];
   int status = 0;
   unsigned long article_size;
   KWBoolean gotsize = KWFalse;
   size_t chars_read;
   size_t chars_written;
   int handle;
   FILE *stream;

/*--------------------------------------------------------------------*/
/*       This is uncompressed batch.  We will be distributing the     */
/*       articles as we read the batch, so use a unique stream        */
/*       (not stdin) to process the data                              */
/*--------------------------------------------------------------------*/

   fseek(streamIn, 0L, SEEK_SET);     /* Back to the beginning       */

   handle = dup(fileno( streamIn ));

   if ( handle == -1 )
   {
      printerr( "Batched: dup:" );
      panic();
   }

   stream = fdopen( handle, "r" );

   if ( stream == NULL)
   {
      printerr( "Batched: fdopen:" );
      panic();
   }

   streamIn = freopen( BIT_BUCKET, "r",  streamIn );

/*--------------------------------------------------------------------*/
/*                    Main loop to read our stream                    */
/*--------------------------------------------------------------------*/

   while( ! feof( stream ) && ! ferror( stream ))
   {
      unsigned long article_left;
      size_t max_read = sizeof buf;
      unsigned skipped_lines = 0;
      unsigned skipped_bytes = 0;
      IMFILE *imf;

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

      imf = imopen( (long) article_size );

      if ( imf == NULL )
      {
         printerr( "imopen");
         panic();
      }

/*--------------------------------------------------------------------*/
/*   Copy this article to the temp file (except for the last block)   */
/*--------------------------------------------------------------------*/

      if ( article_size )
      {
         do {

            if ( article_left < max_read )
               max_read = (unsigned) article_left;

            chars_read = fread(buf, sizeof(char), max_read, stream);

            if ( (chars_read < max_read) && ferror( stream ))
            {
               printerr("STDIN");
               panic();
            }

            if ( chars_read == 0)
               break;

            fixEOF( buf , chars_read );

            chars_written = imwrite(buf, sizeof(char), chars_read, imf);
            if (chars_read != chars_written)
            {
               printmsg(0, "Batched: Read %d bytes, only wrote %d bytes of article %d",
                     chars_read, chars_written , articles + 1);
               printerr("imwrite");
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

         unsigned long actual_size = 0;

         do {
            if (fgets( buf, sizeof buf, stream ) == NULL)
            {
               if ( ferror( stream ))
                  printerr( "fgets" );
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

               chars_written = imwrite(buf,
                                      sizeof(char),
                                      chars_read,
                                      imf);
               if (chars_read != chars_written)
               {
                  printmsg(0,
                       "Batched: Read %d bytes, only wrote %d bytes "
                       "of article %d",
                        chars_read, chars_written , articles + 1);
                  printerr("imwrite");

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

      deliver_article( imf );
      imclose( imf );

   } /* while */

   fclose( stream );

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

      } /* if ( equal(table[subscript].name, field ) )*/

   }  /* for ( subscript = 0; table[subscript].name != NULL; subscript++ ) */

/*--------------------------------------------------------------------*/
/*      We never found the specified field, critical internal error   */
/*--------------------------------------------------------------------*/

   printmsg(0, "Unable find requested header %s in header table",
               field );

   panic();

   return NULL;                     /* Make compiler happy           */

} /* getHeader */

/*--------------------------------------------------------------------*/
/*       v a l i d a t e I D                                          */
/*                                                                    */
/*       Validate a message id; it must have no spaces, or a          */
/*       trailing angle bracket after last space.                     */
/*--------------------------------------------------------------------*/

static KWBoolean
validateID( char *s )
{
   char *space = strrchr( s, ' ');
   char *bracket;

   if ( space == NULL )             /* No spaces in message id?      */
      return KWTrue;                /* Correct --> Good message id   */

   bracket = strchr( s, '>' );      /* Find first (only?) bracket    */

   if (( bracket == NULL ) || (bracket < space))
   {
      printmsg(0,"Cannot process article with invalid message id \"%s\"",
                 s );
      return KWFalse;
   }
   else
      return KWTrue;

} /* validateID */

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

static void deliver_article( IMFILE *imf )
{

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
   KWBoolean searchHeader = KWTrue; /* Each article begins w/header  */
   KWBoolean error    = KWFalse;    /* Presume successful hdr scan   */
   int subscript;                   /* For walking header table      */

   imrewind( imf );                 /* Begin at top of article       */
   articles++;

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

      if ( imgets(input, sizeof input, imf) == NULL )   /* eof ?    */
         searchHeader = KWFalse;     /* Yes --> Exit loop ...         */
      else if ( *input == '\n' )     /* Last of the red hot headers?  */
         searchHeader = KWFalse;     /* Yes --> Exit loop gracefully  */
      else if ( equal(input, "\r\n"))/* Last of the DOS hot headers?  */
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
/*                     Validate message id field                      */
/*--------------------------------------------------------------------*/

         if ( equal( table[subscript].name, MESSAGEID) )
         {
            error = validateID( s );

            if ( error )
               continue;
         }

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

   }  /* while ( header && fgets(input, sizeof input, imf )) */

/*--------------------------------------------------------------------*/
/*                Verify we did not have a file error                 */
/*--------------------------------------------------------------------*/

   if ( imerror( imf ) )         /* Exit loop from error?            */
   {
      printerr( "imgets" );      /* Yes --> Report it ...            */
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
      bad_articles++;
      return;
   }

/*--------------------------------------------------------------------*/
/*       Loop through all defined systems to deliver the article      */
/*       as desired                                                   */
/*--------------------------------------------------------------------*/

  while (sysnode != NULL)
  {

    imrewind( imf );             /* Deliver from top of article      */

    if (check_sys(sysnode,
                  getHeader(table, NEWSGROUPS, NULL),
                  getHeader(table, DISTRIBUTION, NULL),
                  getHeader(table, PATH, NULL)))
    {

      if (sysnode->flag.local )
      {
        if (deliver_local( imf,
                           getHeader(table, NEWSGROUPS, NULL),
                           getHeader(table, MESSAGEID, NULL),
                           getHeader(table, CONTROL, NULL)))
        {
          delivered = KWTrue;
          retained++;
          sysnode->processed ++;
        }
      }
      else if (deliver_remote(sysnode,
                              imf,
                              getHeader(table, MESSAGEID, NULL)))
      {
         fwd_articles++;
         sysnode->processed ++;
         delivered = KWTrue;
      }

    } /* if (checksys(...)) */

    sysnode = sysnode -> next;

  } /* while (sysnode != NULL) */

/*--------------------------------------------------------------------*/
/*                  Report on any dead-end articles                   */
/*--------------------------------------------------------------------*/

  if (!delivered)
  {
    no_delivery++;
    printmsg(0, "deliver_article: Article %.40s undeliverable to "
                "groups %.80s, distribution %.40s",
                getHeader(table, MESSAGEID, NULL),
                getHeader(table, NEWSGROUPS, NULL),
                getHeader(table, DISTRIBUTION, NULL) );
  }

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
    if (cancel_article(history, operand))
    printmsg(1, "Canceling article %s", operand );
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

    if ((mod != NULL) && equal(mod, "moderated"))
      moderated = KWTrue;
    else
      moderated = KWFalse;

    add_newsgroup(operand, moderated);
    printmsg(1, "newsgroup added: %s", operand);

  } /* if (equali(cmd, "newgroup")) */
  else if (equali(cmd, "rmgroup"))
  {
    del_newsgroup(operand);
    printmsg(1, "newsgroup removed: %s", operand);
  }
  else if (equali(cmd, "ihave") ||
           equali(cmd, "sendme") ||
           equali(cmd, "sendsys") ||
           equali(cmd, "version") ||
           equali(cmd, "checkgroups"))
  {
    printmsg(1, "control message not implemented: %s", cmd);
  }
  else {
    printmsg(1, "control message unknown: %s", cmd);
  }

  free(ctrl);

} /* control_message */

/*--------------------------------------------------------------------*/
/*    c o p y _ f i l e                                               */
/*                                                                    */
/*    Write an article to it's final resting place                    */
/*--------------------------------------------------------------------*/

static KWBoolean copy_file(IMFILE *imf,
                         const char *group,
                         const char *xref)
{
   struct grp *cur = find_newsgroup(group);
   char filename[FILENAME_MAX];
   char buf[LARGEBUF];
   FILE *output;
   KWBoolean header = KWTrue;

/*--------------------------------------------------------------------*/
/*           Determine if the news has been already posted            */
/*--------------------------------------------------------------------*/

   if (cur == NULL)
   {
      printmsg(3, "Article cross-posted to %s", group);
      return KWFalse;
   }

/*--------------------------------------------------------------------*/
/*                       Now build a file name                        */
/*--------------------------------------------------------------------*/

   ImportNewsGroup( filename, cur->grp_name, cur->grp_high++);

/*--------------------------------------------------------------------*/
/*                 We have a file name, open the file                 */
/*--------------------------------------------------------------------*/

   printmsg(2, "Saving %s article in %s",
               cur->grp_name, filename);

   if ((output = FOPEN(filename, "w", TEXT_MODE)) == nil(FILE))
   {
      printerr( filename );
      printmsg(0, "Unable to save article");
      return KWFalse;
   }

   imrewind(imf);

   if (xref) /* write new Xref: line first */
   {
      if (fputs(xref, output) == EOF)
      {
         printerr( filename );
         panic();
      }
   }

/*--------------------------------------------------------------------*/
/*                Loop to actually write out the article              */
/*--------------------------------------------------------------------*/

   while (imgets(buf, sizeof buf, imf ) != NULL)
   {
      KWBoolean skipHeader = KWFalse;

      if ( ! header )
         ;                          /* No op after end of header     */
      else if ( *buf == '\n' )
         header = KWFalse;
      else if ( equal(buf, "\r\n")) /* Trap DOS headers as well      */
         header = KWFalse;
      else if (equalni(buf, PATH, strlen(PATH)))
      {
         fprintf(output,
                 PATH " %s!%s\n",
                 canonical_news_name(),
                 strtok(buf + strlen(PATH) + 1, WHITESPACE ));

         skipHeader = KWTrue;
      }
      else if (equalni(buf, XREF, strlen(XREF)))
         skipHeader = KWTrue;       /* Skip old Xref: line           */

      if ( ! skipHeader )
         fputs(buf, output);        /* Write normal line line out    */

      if ( ferror( output ))
      {
         printerr( filename );
         panic();
      }

   } /* while */

   if (fclose(output))
   {
      printerr( filename );
      panic();
   }

   return KWTrue;        /* Report the file is posted                  */

} /* copy_file */

/*--------------------------------------------------------------------*/
/*       d e l i v e r _ l o c a l                                    */
/*                                                                    */
/*       Deliver an article locally to one or more news groups        */
/*--------------------------------------------------------------------*/

static KWBoolean deliver_local(IMFILE *imf,
                             const char *newsgroups_in,
                             const char *messageID,
                             const char *control)
{

  char hist_record[LARGEBUF];
  char groupy[MAXGRP];
  char *newsgroups = NULL;
  char *msgID = (char *) messageID;
  char idBuffer[FILENAME_MAX];
  size_t newsgroups_len;
  int  groups_found;
  char snum[10];

  char *gc_ptr;
  char *gc_ptr1;

  KWBoolean b_xref;               /* Xref line if multiple groups     */
  KWBoolean posted = KWFalse;

  loc_articles++;

   if (control)
   {
      control_message(control, BIT_BUCKET );

      if (get_snum("control", snum))
         newsgroups_in = "control";
      else if (get_snum("junk", snum))
         newsgroups_in = "junk";
      else
         return KWFalse;
   }

/*--------------------------------------------------------------------*/
/*           Check whether article has been received before           */
/*--------------------------------------------------------------------*/

   if (get_histentry(history, messageID) != NULL)
   {
      duplicates++;

      if (control)               /* Don't save control message twice */
         return KWFalse;

      printmsg(1, "Duplicate article %s", messageID);

      if (get_snum("duplicates", snum))
      {
         newsgroups_in = "duplicates";
         sprintf(idBuffer, "<%s.duplicate.%.10s@%.50s>",
                 snum,
                 E_nodename,
                 E_domain );         /* We need a new unique ID       */
         msgID = idBuffer;
      }
      else {
         return KWFalse;
      }

   } /* if (get_histentry(history, messageID) != NULL) */

/*--------------------------------------------------------------------*/
/*              Copy our news groups line for processing              */
/*--------------------------------------------------------------------*/

   newsgroups_len = strlen( newsgroups_in ) + 1;
   b_xref = (KWBoolean) ((strchr(newsgroups_in,',') == NULL) ?
                                                        KWFalse : KWTrue);

   newsgroups = malloc( newsgroups_len + 1 );
   checkref( newsgroups );
   memcpy( newsgroups, newsgroups_in, newsgroups_len );
   newsgroups[newsgroups_len] = '\0';     /* Terminate for rescan    */

/*--------------------------------------------------------------------*/
/*             Build the history record for this article              */
/*--------------------------------------------------------------------*/

   sprintf(hist_record, "%ld %ld ", now, imlength( imf ));
   groups_found = 0;

   for (gc_ptr = newsgroups; gc_ptr != NULL; gc_ptr = gc_ptr1)
   {
      if ((gc_ptr1 = strchr(gc_ptr, ',')) != NULL)
         *gc_ptr1++ = '\0';

      if (strlen(gc_ptr) > sizeof groupy  - 1)
      {
         /* Bounds check the newsgroup length */

         printmsg(0, "newsgroup name too long -- %s", gc_ptr1);
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

   memcpy( newsgroups, newsgroups_in, newsgroups_len );
                                       /* Restore the newsgroups line   */

/*--------------------------------------------------------------------*/
/*       Reroute the article to deliver to junk (if available) if     */
/*       we have no real local group to send it to.                   */
/*--------------------------------------------------------------------*/

   if (groups_found == 0)
   {

     printmsg(2, "no group to deliver to: %s", messageID );
     memcpy(newsgroups, "junk\0\0", 6);
     b_xref = KWFalse;

     if (!get_snum("junk", snum))      /* Do we maintain junk group? */
     {                                 /* No --> Throw article away  */
       free( newsgroups );
       return KWFalse;
     }

     sprintf(hist_record, "%ld %ld junk:%s",
             now,
             imlength( imf ),
             snum);

     junked++;

   } /* if (groups_found == 0) */

   /* Post the history record */

   add_histentry(history, msgID, hist_record);

/*--------------------------------------------------------------------*/
/*              Now build the Xref: line (if we need to)              */
/*--------------------------------------------------------------------*/

   if (b_xref)
   {
      strcpy(hist_record, "Xref: ");
      strcat(hist_record, canonical_news_name());

      for (gc_ptr = newsgroups; gc_ptr != NULL; gc_ptr = gc_ptr1)
      {
         if ((gc_ptr1 = strchr(gc_ptr, ',')) != NULL)
            *gc_ptr1++ = '\0';

         if (strlen(gc_ptr) > sizeof groupy  - 1)
         {
            /* Bounds check the newsgroup length */
            printmsg(0, "newsgroup name too long -- %s", gc_ptr);
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

      if ( copy_file( imf, groupy, b_xref ? hist_record : NULL))
         posted = KWTrue;

   } /* for (gc_ptr = newsgroups; gc_ptr != NULL; gc_ptr = gc_ptr1) */

   free( newsgroups );

   return posted;

} /* deliver_local */

/*--------------------------------------------------------------------*/
/*       c o p y _ r m t _ a r t i c l e                              */
/*                                                                    */
/*       Perform the low level copy for an article destined for       */
/*       a remote system                                              */
/*--------------------------------------------------------------------*/

static void copy_rmt_article( const char *filename, IMFILE *imf )
{

  FILE *output;

  char buf[BUFSIZ*2];

  KWBoolean skipHeader   = KWFalse;
  KWBoolean searchHeaders   = KWTrue;

  printmsg(2, "Saving remote article in %s", filename);

/*--------------------------------------------------------------------*/
/*          Open up the output file and verify the open worked        */
/*--------------------------------------------------------------------*/

  output = FOPEN(filename, "w", IMAGE_MODE );

  if (output == NULL)
  {
    printerr( filename );
    panic();
  }

/*--------------------------------------------------------------------*/
/*                     Main loop to process the data                  */
/*--------------------------------------------------------------------*/

  while (imgets(buf, sizeof buf, imf) != NULL)
  {

     if ( searchHeaders )
     {

         if ( *buf == '\n' )        /* End of header?                */
            searchHeaders = KWFalse;
         else if ( equal( buf, "\r\n" ))  /* End of DOS header?      */
            searchHeaders = KWFalse;
         else if (equalni(buf, PATH, strlen(PATH)))
         {
           fprintf(output,
                   "%s %s!%s\n",
                   PATH,
                   canonical_news_name(),
                   strtok( buf + strlen(PATH) + 1, WHITESPACE ));

            searchHeaders = KWFalse;
            skipHeader = KWTrue;
         }
      }

     if (!skipHeader)
       fputs(buf, output);

     skipHeader = KWFalse;

     if ( ferror( output ))
     {
        printerr( filename );
        panic();
     }

  } /* while */

  if ( fclose(output) )
  {
     printerr( filename );
     panic();
  }

} /* copy_rmt_article */

/*--------------------------------------------------------------------*/
/*       b a t c h _ r e m o t e                                      */
/*                                                                    */
/*       Queue a file to be sent to a remote system                   */
/*--------------------------------------------------------------------*/

static KWBoolean batch_remote(const struct sys *node,
                            IMFILE *imf,
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

   copy_rmt_article(fname, imf );

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
      long length;

      stater( fname, &length );

      fprintf( batchListStream, " %ld", &length );

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
                              IMFILE *imf,
                              const char *msgID )
{

/*--------------------------------------------------------------------*/
/*         Are we batching data or processing it immediately?         */
/*--------------------------------------------------------------------*/

  if ( node->flag.batch )
     return batch_remote( node, imf, msgID );
  else {

     char fname[FILENAME_MAX];
     KWBoolean result;

     printmsg(5, "Transmitting article %s to %s via command %s",
                  node->sysname,
                  msgID,
                  node->command );

      mktempname( fname, "tmp" );

      copy_rmt_article(fname, imf );

      result = xmit_remote( node->sysname, node->command, fname );
      if ( unlink( fname ) )
         printerr( fname );

      return result;

   }  /* else */

} /* deliver_remote */
