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
 *    $Id: expire.c 1.6 1994/02/19 04:21:11 ahd Exp $
 *
 *    $Log: expire.c $
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
      "$Id: expire.c 1.6 1994/02/19 04:21:11 ahd Exp $";

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

static void ExpireAll( const time_t expire_date );

static void  ExpireGroup( const char *group,
                      const time_t expire_date );

static void ExpireOneGroup( struct grp *cur_grp,
                      const time_t expire_date );

static void ExpireDirectory( struct grp *cur_grp,
                      const time_t expire_date,
                      const char *directory );

static boolean numeric( char *start);

static void usage( void );

long total_articles_purged = 0;
long total_articles_kept   = 0;
long total_cross_purged    = 0;
long total_cross_kept      = 0;
long total_bytes_purged    = 0;
long total_bytes_kept      = 0;

void *history;
void *new_history;

/*--------------------------------------------------------------------*/
/*    m a i n                                                         */
/*                                                                    */
/*    Main program                                                    */
/*--------------------------------------------------------------------*/

void main( int argc, char **argv)
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

   tzset();                      /* Set up time zone information  */

   if (!configure( B_NEWS ))
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
/*                       Load the active file                         */
/*--------------------------------------------------------------------*/

   if ( bflag[F_HISTORY] )
   {
      history = open_history("history");
      new_history = open_history("newhist");
   }

   get_active();              /* Get sequence numbers for groups from
                                 active file                      */

/*--------------------------------------------------------------------*/
/*                  Chain together groups to process                  */
/*--------------------------------------------------------------------*/

   if ( bflag[F_HISTORY] )
   {
      for ( cur_grp = group_list; cur_grp != NULL;
            cur_grp = cur_grp->grp_next )
            cur_grp->grp_low = cur_grp->grp_high;
   } /* if */

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

   if ( bflag[F_HISTORY] )
      HistoryExpireAll(groups, expire_date );
   else if ( groups != NULL )
   {
      while( argc-- > optind )
         ExpireGroup( *groups++, expire_date );
   }
   else
      ExpireAll( expire_date );

/*--------------------------------------------------------------------*/
/*                         Clean up and exit                          */
/*--------------------------------------------------------------------*/

   put_active();

   if ( bflag[F_HISTORY] )
   {
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
   }

   if ( total_articles_purged)
      printmsg(1,"Purged %ld articles, %ld cross postings (%ld bytes).",
               total_articles_purged, total_cross_purged, total_bytes_purged );

   printmsg(1,"Total of %ld articles, %ld cross postings (%ld bytes)." ,
            total_articles_kept, total_cross_kept, total_bytes_kept );

   exit(0);

} /* main */

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
/*    E x p i r e A l l                                               */
/*                                                                    */
/*    Expire all defined news groups                                  */
/*--------------------------------------------------------------------*/

static void ExpireAll( const time_t expire_date )
{
   struct grp *cur_grp = group_list;

   while ( cur_grp != NULL )
   {
      ExpireOneGroup( cur_grp, expire_date  );
                                    /* Clean up this group           */

      cur_grp = cur_grp->grp_next;  /* Then clean up the next group  */
   }
} /* Expire_All */

/*--------------------------------------------------------------------*/
/*    E x p i r e G r o u p                                           */
/*                                                                    */
/*    Clean up one group by name                                      */
/*--------------------------------------------------------------------*/

static void  ExpireGroup( const char *group,
                      const time_t expire_date )
{
   struct grp *cur_grp = group_list;
   struct grp *target = NULL;

/*--------------------------------------------------------------------*/
/*         Search the list of groups for the requested group          */
/*--------------------------------------------------------------------*/

   while ( (cur_grp != NULL) && (target == NULL))
   {
      if ( equal( cur_grp->grp_name, group ))
         target = cur_grp;

      cur_grp = cur_grp->grp_next;  /* Then clean up the next group  */
   }

/*--------------------------------------------------------------------*/
/*   If we found the group, process it, otherwise report the error    */
/*--------------------------------------------------------------------*/

   if ( target == NULL )
      printmsg(0,"Unable to locate active group %s", group );
   else
      ExpireOneGroup( target, expire_date );
                                    /* Clean up this group           */

} /* ExpireGroup */

/*--------------------------------------------------------------------*/
/*    E x p i r e O n e G r o u p                                     */
/*                                                                    */
/*    Clean up one group by name                                      */
/*--------------------------------------------------------------------*/

static void ExpireOneGroup( struct grp *cur_grp,
                      const time_t expire_date )
{
   char groupdir[FILENAME_MAX];

   printmsg(3,"Processing news group %s", cur_grp->grp_name );

/*--------------------------------------------------------------------*/
/*                     Set up the directory names                     */
/*--------------------------------------------------------------------*/

   ImportNewsGroup( groupdir, cur_grp->grp_name, 0 );

/*--------------------------------------------------------------------*/
/*                      Process the directories                       */
/*--------------------------------------------------------------------*/

   ExpireDirectory( cur_grp, expire_date, groupdir );

} /* ExpireOneGroup */

/*--------------------------------------------------------------------*/
/*    E x p i r e D i r e c t o r y                                   */
/*                                                                    */
/*    Clean up one group by name                                      */
/*--------------------------------------------------------------------*/

static void ExpireDirectory( struct grp *cur_grp,
                      const time_t expire,
                      const char *directory )
{
   int articles_purged   = 0;/* Count of files actually deleted        */
   int articles_kept     = 0;/* Count of files actually deleted        */
   long bytes_purged = 0;    /* Bytes freed on disk from deletions     */
   long bytes_kept   = 0;    /* Bytes left on disk total               */

   long low = LONG_MAX;  /* Oldest article number left             */

   DIR *dirp;
   struct direct *dp;

/*--------------------------------------------------------------------*/
/*                Open up the directory for processing                */
/*--------------------------------------------------------------------*/

   if ((dirp = opendirx(directory,"*.*")) == nil(DIR))
   {
      printmsg(3, "ExpireDirectory: couldn't opendir() %s", directory);
      cur_grp->grp_low = cur_grp->grp_high;
      return;
   } /* if */

/*--------------------------------------------------------------------*/
/*                 Switch to directory for processing                 */
/*--------------------------------------------------------------------*/

   CHDIR( directory );

/*--------------------------------------------------------------------*/
/*              Look for the next file in the directory               */
/*--------------------------------------------------------------------*/

   while((dp = readdir(dirp)) != nil(struct direct))
   {

/*--------------------------------------------------------------------*/
/*                      Archive/expire this file?                     */
/*--------------------------------------------------------------------*/

      if ( numeric( dp->d_name ))/* Article format name?             */
      {                          /* Yes --> Examine it closer        */

         printmsg(6,"Processing file %s from %s",
                  dp->d_name, dater( dp->d_modified, NULL));

         if ( dp->d_modified < expire )   /* Long in the tooth?      */
         {                       /* Yes --> Move it on out           */

            printmsg( 4,"Purging file %s from %s", dp->d_name, directory );
            unlink( dp->d_name );
            articles_purged++;
            bytes_purged += dp->d_size;

         } /* if ( dp->d_modified < expire ) */

/*--------------------------------------------------------------------*/
/*    If the article is valid and still in the main news              */
/*    directory, determine if it is the lowest article left           */
/*--------------------------------------------------------------------*/

         else {
            long article = 0;
            char *digit = dp->d_name;

            while( *digit )
               article = article * 10 + (*digit++ - '0');

            low = min( article, low );

            bytes_kept += dp->d_size;
            articles_kept ++;

         } /* else if ( archive != NULL ) */
      } /* if ( numeric( dp->d_name ) */

   } /* while */

/*--------------------------------------------------------------------*/
/*            Update lowest article available to the users            */
/*--------------------------------------------------------------------*/

   if ( low == LONG_MAX )
       cur_grp->grp_low = cur_grp->grp_high;
   else
       cur_grp->grp_low = low;

/*--------------------------------------------------------------------*/
/*           Close up the directory and report what we did            */
/*--------------------------------------------------------------------*/

   closedir(dirp);

   if ( articles_purged )
      printmsg(2,"%s: Purged %d articles (%ld bytes),"
                    " left alone %d articles (%ld bytes).",
                  cur_grp->grp_name,
                  articles_purged, bytes_purged,
                  articles_kept, bytes_kept );
   else if ( articles_kept )
      printmsg(2,"%s: Left alone %d articles (%ld bytes).",
                  cur_grp->grp_name,
                  articles_kept, bytes_kept );

   total_articles_kept     += articles_kept;
   total_articles_purged   += articles_purged;
   total_bytes_kept        += bytes_kept;
   total_bytes_purged      += bytes_purged;

} /* ExpireDirectory */

/*--------------------------------------------------------------------*/
/*    n u m e r i c                                                   */
/*                                                                    */
/*    Examines string, returns true if numeric with period            */
/*--------------------------------------------------------------------*/

 static boolean numeric( char *start)
 {
   char *number = start;

   while (*number != '\0')
   {
      if (!isdigit(*number) && (*number != '.'))
         return FALSE;

      number++;
   }

   return TRUE;
 } /* numeric */

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
