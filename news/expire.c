/*--------------------------------------------------------------------*/
/*       e x p i r e . c                                              */
/*                                                                    */
/*       Expire old news articles for UUPC/extended                   */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-2001 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/* new version, rewritten for history-based news database
 *
 * Author:  Kai Uwe Rommel <rommel@ars.muc.de>
 * Created: Sun Aug 15 1993
 *
 *    $Id: expire.c 1.29 2000/05/12 12:26:40 ahd v1-13g $
 *
 *    $Log: expire.c $
 *    Revision 1.29  2000/05/12 12:26:40  ahd
 *    Annual copyright update
 *
 *    Revision 1.28  1999/02/21 04:09:32  ahd
 *    Tighter checking for backup file errors
 *
 *    Revision 1.27  1999/01/08 02:20:52  ahd
 *    Convert currentfile() to RCSID()
 *
 *    Revision 1.26  1999/01/04 03:53:30  ahd
 *    Annual copyright change
 *
 *    Revision 1.25  1998/03/01 01:30:09  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.24  1997/12/22 18:29:49  ahd
 *    Don't issue stater() when file may not exist for backup
 *
 *    Revision 1.23  1997/12/22 14:12:44  ahd
 *    Correct off-by-2 error in computing length of string to allocate
 *    Add debugging information for failure to backup/rename files
 *
 *    Revision 1.22  1997/04/24 00:56:54  ahd
 *    Delete MAKEBUF/FREEBUF support
 *
 *    Revision 1.21  1996/11/18 04:46:49  ahd
 *    Normalize arguments to bugout
 *    Reset title after exec of sub-modules
 *    Normalize host status names to use HS_ prefix
 *
 *    Revision 1.20  1995/12/12 13:48:54  ahd
 *    Use binary tree for news group active file
 *    Use large buffers in news programs to avoid overflow of hist db recs
 *    Use true recursive function to walk entire active file
 *
 *    Revision 1.19  1995/12/03 13:51:44  ahd
 *    Additional debugging cleanup
 *
 *    Revision 1.17  1995/09/11 00:20:45  ahd
 *    Correct compile warning
 *
 *    Revision 1.16  1995/08/27 23:33:15  ahd
 *    Load and use ACTIVE file as tree structure
 *
 *    Revision 1.15  1995/03/11 22:29:13  ahd
 *    Use macro for file delete to allow special OS/2 processing
 *
 *    Revision 1.14  1995/02/20 00:03:07  ahd
 *    Drop previous work history file before creating new one
 *
 *    Revision 1.13  1995/02/12 23:37:04  ahd
 *    compiler cleanup, NNS C/news support, optimize dir processing
 *
 *    Revision 1.12  1995/01/30 04:08:36  ahd
 *    Additional compiler warning fixes
 *
 *    Revision 1.11  1995/01/29 14:03:29  ahd
 *    Clean up IBM C/Set compiler warnings
 *
 *    Revision 1.10  1995/01/15 19:48:35  ahd
 *    Allow active file to be optional
 *    Delete fullbatch global option
 *    Add "local" and "batch" flags to SYS structure for news
 *
 *    Revision 1.9  1995/01/07 16:21:07  ahd
 *    Change KWBoolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.8  1995/01/02 05:03:27  ahd
 *    Pass 2 of integrating SYS file support from Mike McLagan
 *
 *    Revision 1.7  1994/06/14 01:19:24  ahd
 *    Clean yp RCS information
 *    patches from Kai Uwe Rommel
 *
 * Revision 1.6  1994/02/19  04:21:11  ahd
 * Use standard first header
 *
 * Revision 1.5  1994/01/01  19:14:06  ahd
 * Annual Copyright Update
 *
 * Revision 1.4  1993/12/23  03:16:03  rommel
 * OS/2 32 bit support for additional compilers
 *
 * Revision 1.3  1993/12/09  04:51:21  ahd
 * restore ability to expire without having history database
 *
 */

#include "uupcmoah.h"

RCSID("$Id: expire.c 1.29 2000/05/12 12:26:40 ahd v1-13g $");

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include <ctype.h>
#include <limits.h>
#include <errno.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "active.h"
#include "dater.h"
#include "getopt.h"
#include "history.h"
#include "import.h"
#include "importng.h"
#include "logger.h"
#include "uundir.h"
#include "pushpop.h"
#include "stater.h"
#include "timestmp.h"
#include "hdbm.h"

/*--------------------------------------------------------------------*/
/*                          Global Variables                          */
/*--------------------------------------------------------------------*/

#define ONE_DAY (60L*60L*24L)

/*--------------------------------------------------------------------*/
/*                        Internal prototypes                         */
/*--------------------------------------------------------------------*/

static void HistoryExpireAll( char **groups, const time_t expire_date );

static void usage( void );

static long total_articles_purged = 0;
static long total_articles_kept   = 0;
static long total_cross_purged    = 0;
static long total_cross_kept      = 0;
static long total_bytes_purged    = 0;
static long total_bytes_kept      = 0;

static DBM *history;
static DBM *new_history;

/*--------------------------------------------------------------------*/
/*       b a c k u p N e w s F i l e                                  */
/*                                                                    */
/*       Delete an old news database file, and optionally rename a    */
/*       newer file to the name of the just deleted file.             */
/*--------------------------------------------------------------------*/

static void
backupNewsFile(  const char *nextGeneration, const char *previous )
{
   static const char mName[] = "backupNewsFile";
   char file_previous[FILENAME_MAX];
   char file_new[FILENAME_MAX];

   mkfilename(file_previous, E_newsdir, previous);

/*--------------------------------------------------------------------*/
/*       Punt the old file, and return if there is no new file        */
/*--------------------------------------------------------------------*/

   if (REMOVE(file_previous) &&
       ((errno == ENOENT) || (debuglevel > 1)))
   {
      printmsg(0,"%s: Unable to delete backup file %s",
                 mName,
                 file_previous );
      printerr( file_previous );

      if (errno != ENOENT)
         panic();
   }

   if ( nextGeneration == NULL )
      return;

/*--------------------------------------------------------------------*/
/*         Build the new file name and move into the old name         */
/*--------------------------------------------------------------------*/

   mkfilename(file_new, E_newsdir, nextGeneration);

   if ( rename(file_new, file_previous) )
   {
      printmsg(0, "%s: Backup of %s to %s failed",
                  mName,
                  file_new,
                  file_previous );
      printerr( file_new );
   }
#ifdef UDEBUG
   else
      printmsg(2, "%s: Renamed %s to %s",
                  mName,
                  file_new,
                  file_previous );
#endif

} /* backupNewsFile */

/*--------------------------------------------------------------------*/
/*       r e s e t O n e A r t i c l e G r o u p                      */
/*                                                                    */
/*       Set oldest and newest article numbers to same                */
/*--------------------------------------------------------------------*/

void
resetOneArticleGroup( const char *group, void *dummy )
{
   setArticleOldest( group, getArticleNewest( group ));
} /* resetOneArticleGroup */

/*--------------------------------------------------------------------*/
/*    m a i n                                                         */
/*                                                                    */
/*    Main program                                                    */
/*--------------------------------------------------------------------*/

main( int argc, char **argv)
{
   int c;
   extern char *optarg;
   extern int   optind;
   char **groups = NULL;

   time_t expire_period  = 7; /* Seven days visible to users         */
   time_t expire_date;

/*--------------------------------------------------------------------*/
/*     Report our version number and date/time compiled               */
/*--------------------------------------------------------------------*/

   debuglevel = 1;
   banner( argv );

#if defined(__CORE__)
   copywrong = strdup(copyright);
   checkref(copywrong);
#endif

/*--------------------------------------------------------------------*/
/*        Process our arguments                                       */
/*--------------------------------------------------------------------*/

   while ((c = getopt(argc, argv, "e:x:")) !=  EOF)
      switch(c) {

      case 'e':
         expire_period = atoi( optarg );
         break;

      case 'x':
         debuglevel = atoi( optarg );
         break;

      case '?':
         usage();
         exit(1);
         break;

      default:
         printmsg(0, "expire - invalid option -%c", c);
         usage();
         exit(2);
         break;
   }

   if (optind != argc)
      groups = argv + optind;

/*--------------------------------------------------------------------*/
/*                             Initialize                             */
/*--------------------------------------------------------------------*/

   if (!configure( B_EXPIRE ))
      exit(1);   /* system configuration failed */

/*--------------------------------------------------------------------*/
/*                    Switch to the news directory                    */
/*--------------------------------------------------------------------*/

   PushDir( E_newsdir );
   atexit( PopDir );

/*--------------------------------------------------------------------*/
/*                     Initialize logging file                        */
/*--------------------------------------------------------------------*/

   openlog( NULL );

/*--------------------------------------------------------------------*/
/*                 Load the history and active files                  */
/*--------------------------------------------------------------------*/

  backupNewsFile( NULL, "newhist.dir" );  /* Delete any junk history */
  backupNewsFile( NULL, "newhist.pag" );  /* Delete any junk history */

   history = open_history("history");
   new_history = open_history("newhist");

   loadActive( KWTrue );      /* Get sequence numbers for groups from
                                 active file                      */

/*--------------------------------------------------------------------*/
/*                       Reset article numbers                        */
/*--------------------------------------------------------------------*/

   startActiveWalk( resetOneArticleGroup, 0);

/*--------------------------------------------------------------------*/
/*                  Compute times for expiring files                  */
/*--------------------------------------------------------------------*/

   time( &expire_date );
   expire_date  -= (expire_period * ONE_DAY);

   printmsg(1,"expire: Purging news older than %s (%ld days)",
            dater( expire_date , NULL), (long) expire_period );

/*--------------------------------------------------------------------*/
/*    Process groups as requested, otherwise process the entire       */
/*    history database                                                */
/*--------------------------------------------------------------------*/

   HistoryExpireAll(groups, expire_date );

/*--------------------------------------------------------------------*/
/*       Backup our existing history files, and then move the new     */
/*       history databases into place.                                */
/*--------------------------------------------------------------------*/

   close_history(history);
   close_history(new_history);

   backupNewsFile( "history.dir", "oldhist.dir" );
   backupNewsFile( "history.pag", "oldhist.pag" );

   backupNewsFile( "newhist.dir", "history.dir" );
   backupNewsFile( "newhist.pag", "history.pag" );

/*--------------------------------------------------------------------*/
/*                         Clean up and exit                          */
/*--------------------------------------------------------------------*/

   writeActive();

   if ( total_articles_purged)
      printmsg(1,"Purged %ld articles, %ld cross postings (%ld bytes).",
               total_articles_purged, total_cross_purged, total_bytes_purged );

   printmsg(1,"Total of %ld articles, %ld cross postings (%ld bytes)." ,
            total_articles_kept, total_cross_kept, total_bytes_kept );

   exit(0);
   return 0;                        /* For brain dead IBM C/Set      */

} /* main */

/*--------------------------------------------------------------------*/
/*    S e t G r o u p L o w e r                                       */
/*                                                                    */
/*    Set the lower bounds of all groups an article is posted to      */
/*--------------------------------------------------------------------*/

static void
SetGroupLower(char *histentry)
{
  char *value = (char *) malloc( strlen( histentry ) + 1 );
  char *group, *num;
  long article;

  checkref( value );
  strcpy(value, histentry);
  strtok(value, " ");   /* strip off date */
  strtok(NULL, " ");    /* strip off size */

  while ((group = strtok(NULL, "," WHITESPACE )) != NULL)
  {

    long lowest;

    num = strchr(group, ':');
    *num++ = 0;
    article = atol(num);

    lowest = getArticleOldest( group );

    if (article && ( lowest > article ))
      setArticleOldest( group, article );

  } /* while ((group = strtok(NULL, "," WHITESPACE )) != NULL) */

  free( value );

} /* SetGroupLower */

/*--------------------------------------------------------------------*/
/*    H i s t o r y E x p i r e A l l                                 */
/*                                                                    */
/*    Expire all requested news groups                                */
/*--------------------------------------------------------------------*/

static void HistoryExpireAll( char **groups, const time_t expire_date )
{
   char *messageID, *histentry;
   time_t article_date;
   long article_size;
   int total, remaining, deleted;
   int found;

   for (found = get_first_histentry(history, &messageID, &histentry); found;
        found = get_next_histentry(history, &messageID, &histentry))
   {

     sscanf(histentry, "%ld %ld",
            &article_date, &article_size);

     remaining = total = count_postings(histentry);

#ifdef UDEBUG
      if ( debuglevel > 8 )
         printmsg(8,"Article %s expires %.24s, %d bytes, %d copies",
                     messageID,
                     ctime( &article_date ),
                     article_size,
                     total );
#endif

     if (article_date < expire_date)
     {
       if ((histentry = purge_article(histentry, groups)) == NULL)
         remaining = 0;
       else
         remaining = count_postings(histentry);
     }

     if ( remaining )
     {
       add_histentry(new_history, messageID, histentry);
       SetGroupLower(histentry);
     }

     deleted = (total - remaining);

     if (deleted > 1)
     {
       total_articles_purged += 1;
       total_cross_purged    += (deleted - 1);
     }
     else
       total_articles_purged += deleted;

     if (remaining > 1)
     {
       total_articles_kept   += 1;
       total_cross_kept      += (remaining - 1);
     }
     else
       total_articles_kept   += remaining;

     total_bytes_purged    += deleted * article_size;
     total_bytes_kept      += remaining * article_size;
   }

} /* HistoryExpireAll */

/*--------------------------------------------------------------------*/
/*    u s a g e                                                       */
/*                                                                    */
/*    Print usage of program                                          */
/*--------------------------------------------------------------------*/

static void usage( void )
{
   printf( "Usage:   expire [-edays] [group ...]\n");
   exit(1);
} /* usage */
