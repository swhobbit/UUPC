/*--------------------------------------------------------------------*/
/*    e x p i r e . c                                                 */
/*                                                                    */
/*    Expire old news articles for UUPC/extended                      */
/*                                                                    */
/*    Copyright (c) 1992-1994 by Kendra Electronic Wonderworks, all   */
/*    rights reserved except those explicitly granted by the UUPC/    */
/*    extended license.                                               */
/*--------------------------------------------------------------------*/

/* new version, rewritten for history-based news database
 *
 * Author:  Kai Uwe Rommel <rommel@ars.muc.de>
 * Created: Sun Aug 15 1993
 *
 *    $Id: expire.c 1.13 1995/02/12 23:37:04 ahd Exp $
 *
 *    $Log: expire.c $
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

static const char rcsid[] =
      "$Id: expire.c 1.13 1995/02/12 23:37:04 ahd Exp $";

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include <ctype.h>
#include <limits.h>

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

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

extern struct grp *group_list;   /* List of all groups */

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

void *history;
void *new_history;

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
   struct grp *cur_grp;
   char file_old[FILENAME_MAX], file_new[FILENAME_MAX];

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
/*                  Switch to the spooling directory                  */
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

  mkfilename(file_old, E_newsdir, "newhist.dir");
  unlink(file_old);                 /* Delete any junk history       */
  mkfilename(file_old, E_newsdir, "newhist.pag");
  unlink(file_old);                 /* Delete any junk history       */

   history = open_history("history");
   new_history = open_history("newhist");

   get_active( KWTrue );      /* Get sequence numbers for groups from
                                 active file                      */

/*--------------------------------------------------------------------*/
/*                  Chain together groups to process                  */
/*--------------------------------------------------------------------*/

   for ( cur_grp = group_list; cur_grp != NULL;
         cur_grp = cur_grp->grp_next )
         cur_grp->grp_low = cur_grp->grp_high;

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
/*                         Clean up and exit                          */
/*--------------------------------------------------------------------*/

   put_active();

   close_history(history);
   close_history(new_history);

   mkfilename(file_old, E_newsdir, "oldhist.dir");
   mkfilename(file_new, E_newsdir, "history.dir");
   unlink(file_old);
   rename(file_new, file_old);
   mkfilename(file_old, E_newsdir, "newhist.dir");
   rename(file_old, file_new);

   mkfilename(file_old, E_newsdir, "oldhist.pag");
   mkfilename(file_new, E_newsdir, "history.pag");
   unlink(file_old);
   rename(file_new, file_old);
   mkfilename(file_old, E_newsdir, "newhist.pag");
   rename(file_old, file_new);

   if ( total_articles_purged)
      printmsg(1,"Purged %ld articles, %ld cross postings (%ld bytes).",
               total_articles_purged, total_cross_purged, total_bytes_purged );

   printmsg(1,"Total of %ld articles, %ld cross postings (%ld bytes)." ,
            total_articles_kept, total_cross_kept, total_bytes_kept );

   return 0;                        /* For brain dead IBM C/Set      */

} /* main */

/*--------------------------------------------------------------------*/
/*    S e t G r o u p L o w e r                                       */
/*                                                                    */
/*    Set the lower bounds of all groups an article is posted to      */
/*--------------------------------------------------------------------*/

static void SetGroupLower(char *histentry)
{
  char value[BUFSIZ];
  char *group, *num;
  struct grp *cur_grp;
  long article;

  strcpy(value, histentry);
  strtok(value, " ");   /* strip off date */
  strtok(NULL, " ");    /* strip off size */

  while ((group = strtok(NULL, " ,\n")) != NULL)
  {
    num = strchr(group, ':');
    *num++ = 0;
    article = atol(num);

    if ( (cur_grp = find_newsgroup(group)) != NULL && article > 0 )
      if ( cur_grp->grp_low > article )
        cur_grp->grp_low = article;
  }
}

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
