/*--------------------------------------------------------------------*/
/*    s y s . c                                                       */
/*                                                                    */
/*    News sys file maintenance for UUPC/extended.                    */
/*                                                                    */
/*    Written by Mike McLagan <mmclagan@invlogic.com>                 */
/*                                                                    */
/*    The sys file is stored in [confdir]/sys.  The file is main-     */
/*    tained by the system administrator, and directs RNEWS where     */
/*    to send articles recieved from other systems.  Note that the    */
/*    implementation is C News style as follows:                      */
/*                                                                    */
/*    node/exclusions:groups/distributions:flags:tx command           */
/*                                                                    */
/*    Interpretation:                                                 */
/*                                                                    */
/*    node[/exclusions]:  A node in the systems file.  Also 'ME',     */
/*                        which is the same as the current node.      */
/*                        Exclusions specify sites which may appear   */
/*                        on the path line as this system.  News with */
/*                        any of the exclusions in the path line is   */
/*                        not sent to that site.                      */
/*                                                                    */
/*                                                                    */
/*    groups[/distribution]: A list of news groups, seperated by      */
/*                           commas.  The distribution field defaults */
/*                           to 'all' unless specified.  This controls*/
/*                           article transmission.  Newsgroup &       */
/*                           distribution must match for an article to*/
/*                           be sent.                                 */
/*                                                                    */
/*    flags:                                                          */
/*          Ln - Local only.  Means that only articles created        */
/*               on this system are sent to others.  This means       */
/*               that no outside articles are passed on to this       */
/*               system.  n specifies hops from this site.            */
/*                                                                    */
/*          B  - Never send underlength batches                       */
/*                                                                    */
/*         f, F - Data field contains a directory name to store       */
/*               articles for batching.  If this field is empty       */
/*               RNEWS will use [newsspooldir]/node/B to batch the    */
/*               files in.  F does not write size to the file.        */
/*                                                                    */
/*           I - Write message IDs instead of filenames.              */
/*                                                                    */
/*           n - Write message IDs after the filenames.               */
/*                                                                    */
/*           m - transmit only moderated groups                       */
/*                                                                    */
/*           u - transmit only unmoderated groups                     */
/*                                                                    */
/*    command: this field contains either a command thru which to     */
/*             pipe articles destined for this system, or is used     */
/*             based on the flags.                                    */
/*                                                                    */
/*                                                                    */
/*    For more information on SYS file, check with Managing UUCP and  */
/*    Usenet, pg177-180                                               */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1999 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: sys.c 1.29 1999/01/04 03:52:55 ahd Exp $
 *
 *    Revision history:
 *    $Log: sys.c $
 *    Revision 1.29  1999/01/04 03:52:55  ahd
 *    Annual copyright change
 *
 *    Revision 1.28  1998/03/01 01:30:40  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.27  1997/12/22 14:12:44  ahd
 *    Correct off-by-2 error in computing length of string to allocate
 *    Add debugging information for failure to backup/rename files
 *
 *    Revision 1.26  1997/04/24 01:39:48  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.25  1997/04/24 00:56:54  ahd
 *    Delete MAKEBUF/FREEBUF support
 *
 *    Revision 1.24  1996/11/18 04:46:49  ahd
 *    Normalize arguments to bugout
 *    Reset title after exec of sub-modules
 *    Normalize host status names to use HS_ prefix
 *
 *    Revision 1.23  1996/01/07 14:14:40  ahd
 *    Correct error messages referencing non-existent 'J' flag
 *
 *    Revision 1.22  1996/01/01 21:07:31  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.21  1995/09/11 00:20:45  ahd
 *    Correct compile warning
 *
 *    Revision 1.20  1995/07/21 13:25:31  ahd
 *    Check for lengths in additional to strings being non-null BEFORE
 *    copying them or otherwise processing them.
 *
 *    Revision 1.19  1995/03/12 16:42:24  ahd
 *    Comment match(), redo variable names to allow debugging
 *
 *    Revision 1.18  1995/03/07 23:38:22  ahd
 *    Add (missing) maximum hop support
 *
 *    Revision 1.17  1995/02/20 17:28:43  ahd
 *    in-memory file support, 16 bit compiler clean up
 *
 *    Revision 1.16  1995/02/15 02:03:39  ahd
 *    Treat mail server as default news server when generating a
 *    default SYS file.
 *
 *    Revision 1.15  1995/02/12 23:37:04  ahd
 *    compiler cleanup, NNS C/news support, optimize dir processing
 *
 *    Revision 1.14  1995/01/30 04:08:36  ahd
 *    Additional compiler warning fixes
 *
 *    Revision 1.13  1995/01/29 14:03:29  ahd
 *    Clean up IBM C/Set compiler warnings
 *
 *    Revision 1.12  1995/01/22 04:16:52  ahd
 *    Batching cleanup
 *
 *    Revision 1.11  1995/01/15 19:48:35  ahd
 *    Allow active file to be optional
 *    Delete fullbatch global option
 *    Add "local" and "batch" flags to SYS structure for news
 *
 *    Revision 1.10  1995/01/13 14:02:36  ahd
 *    News debugging fixes from Dave Watt
 *    Add new checks for possible I/O errors
 *
 *    Revision 1.9  1995/01/08 21:02:02  ahd
 *    Correct BC++ 3.1 compiler warnings
 *
 *    Revision 1.8  1995/01/08 19:52:44  ahd
 *    NNS support
 *
 *    Revision 1.7  1995/01/07 20:48:21  ahd
 *    Correct 16 compile warnings
 *
 *    Revision 1.6  1995/01/07 16:21:47  ahd
 *    Change KWBoolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.5  1995/01/07 15:43:07  ahd
 *    Use string pool, not disk, for various saved parameters from SYS file
 *
 *    Revision 1.4  1995/01/05 03:43:49  ahd
 *    rnews SYS file support
 *
 *    Revision 1.3  1995/01/03 05:32:26  ahd
 *    Further SYS file support cleanup
 *
 *    Revision 1.2  1995/01/02 05:03:27  ahd
 *    Pass 2 of integrating SYS file support from Mike McLagan
 *
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
#include <ctype.h>
#include <sys/stat.h>
#include <limits.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "timestmp.h"
#include "active.h"
#include "hostable.h"
#include "sys.h"
#include "stater.h"
#include "arpadate.h"

RCSID("$Id$");

struct sys *sys_list = NULL;

static char *cache = NULL;       /* Parsing string cache             */
static size_t cacheLength = 0;

#define ME "ME"
#define LINESIZ      BUFSIZ
#define INITSIZ      (BUFSIZ * 8)

static void bootStrap( const char *fileName );

/*--------------------------------------------------------------------*/
/*       t r i m                                                      */
/*                                                                    */
/*       Trim whitespace from a string                                */
/*--------------------------------------------------------------------*/

static char *trim( char *buf )
{
  /* compress out spaces, continuations and eoln's */

  char *t1 = strchr(buf, '\\');

  while (t1 != NULL)
  {
    memmove(t1, t1 + 1, strlen(t1));
    t1 = strchr(t1, '\\');
  }

  t1 = buf;                         /* rescan for whitespace from start */
  while( *t1 )
  {
    if ( isspace( *t1 ) )
      memmove(t1, t1 + 1, strlen(t1));
    else
      t1 ++;
  }

  return buf;

} /* trim */

/*--------------------------------------------------------------------*/
/*       s e t B o o l e a n O p t i o n                              */
/*                                                                    */
/*       Scan the character flags field to determine if an option     */
/*       is set, and clear the character if found.                    */
/*--------------------------------------------------------------------*/

static KWBoolean
setBooleanOption( char *s, const char flag)
{
   char *t = strchr(s, flag);       /* Locate flag, if it exists     */

   if ( t == NULL )                 /* No flag?                      */
      return KWFalse;               /* Correct --> Just return       */

   *t = ' ';                        /* Blank out flag for error check*/

   return KWTrue;                   /* Report flag exists            */

} /* setBooleanOption */

/*--------------------------------------------------------------------*/
/*       p r o c e s s _ s y s                                        */
/*                                                                    */
/*       Process a single SYS file entry which is already buffered    */
/*       into memory                                                  */
/*--------------------------------------------------------------------*/

KWBoolean
process_sys( char *buf)
{
  static NEWS_SYS *previous = NULL;

  NEWS_SYS *node = (NEWS_SYS *) malloc(sizeof(NEWS_SYS));
  char       *f1, *f2, *f3, *f4, *s1, *s2, *t;
  size_t tempLen;

  KWBoolean success = KWTrue;
  int batchOptions = 0;

  checkref( node );
  memset(node, 0, sizeof(NEWS_SYS));

/*--------------------------------------------------------------------*/
/*               Check this entry to the previous entry               */
/*--------------------------------------------------------------------*/

  if ( sys_list == NULL )
     sys_list = node;
  else
     previous->next = node;

  previous = node;                  /* Save this node for next pass  */

  node->maximumHops = USHRT_MAX;

/*--------------------------------------------------------------------*/
/*                    Begin processing the buffer                     */
/*--------------------------------------------------------------------*/

  f1 = buf;                   /* node field    */
  f2 = strchr(f1, ':');       /* groups field  */

  if (f2 != NULL)
  {
    *f2++ = 0;
    f3 = strchr(f2, ':');     /* flags field   */
  }
  else
    f3 = NULL;

  if (f3 != NULL)
  {
    *f3++ = 0;
     f4 = strchr(f3, ':');     /* command field */
   }
   else
      f4 = NULL;

  if (f4 != NULL)
    *f4++ = 0;

  s1 = strchr(f1, '/'); /* exclusions subfield    */
  if (s1 != NULL)
    *s1++ = 0;

  if ( f2 != NULL )
  {
     s2 = strchr(f2, '/'); /* distributions subfield */

     if (s2 != NULL)
       *s2++ = 0;
  }
  else
     s2 = NULL;

  if ((f4 != NULL) && strlen( f4 )) /* Command or batch file name */
    node->command = newstr(f4);

  /*
   * if the node field has a subfield, it's been stripped off already,
   * leaving the node name all by itself.  Note that ME is a special
   * case, and refers to this machine, and is replaced by
   * canonical_news_name() (our node name).
   */

  if ( equal(f1, ME ) )
     node->sysname = canonical_news_name();
  else
     node->sysname = newstr( trim(f1) );

   if ( equal( node->sysname, canonical_news_name() ) )
      node->flag.local = KWTrue;

  if ((s1 != NULL) && strlen(s1))
  {
    node->exclude = newstr(trim(s1) );
    tempLen = strlen( node->exclude );

    if ( tempLen >= cacheLength )
      cacheLength = tempLen + 1;
  }

  if ((s2 != NULL) && strlen(s2))
  {
      node->distribution = newstr( trim(s2) );
      tempLen = strlen( node->distribution );

      if ( tempLen >= cacheLength )
         cacheLength = tempLen + 1;
  }

  if ((f2 != NULL) && strlen( f2 ))
  {
      node->groups = newstr( trim(f2) );
      tempLen = strlen( node->groups );

      if ( tempLen >= cacheLength )
         cacheLength = tempLen + 1;
  }

  printmsg(4, "process_sys: Node %s, exclude %s, distribution %s, groups %s",
               node->sysname,
               node->exclude        ? node->exclude : "(none)",
               node->distribution   ? node->distribution : "(none)",
               node->groups         ? node->groups: "(none)" );

  if ((f3 != NULL) && strlen(f3))
  {
     printmsg(4, "Flags = %s, length = %i", f3, strlen(f3));

/*--------------------------------------------------------------------*/
/*                    UUPC/extended specific options.                 */
/*--------------------------------------------------------------------*/

    node->flag.c = setBooleanOption(f3, 'c' );
                                    /* Do _not_ compress batches     */

    node->flag.B = setBooleanOption(f3, 'B' );
                                    /* Do not send undersized batches   */

/*--------------------------------------------------------------------*/
/*                     Normal UNIX (C news) options                   */
/*--------------------------------------------------------------------*/

    node->flag.f = setBooleanOption(f3, 'f' );
    node->flag.F = setBooleanOption(f3, 'F' );
    node->flag.I = setBooleanOption(f3, 'I' );
    node->flag.m = setBooleanOption(f3, 'm' );
    node->flag.n = setBooleanOption(f3, 'n' );
    node->flag.u = setBooleanOption(f3, 'u' );

/*--------------------------------------------------------------------*/
/*       Determine the maximum number of hops this news can           */
/*       deliver and we still forward it.                             */
/*--------------------------------------------------------------------*/

    t = strchr(f3, 'L');

    if (t != NULL)
    {
      node->maximumHops = 0;
      *t = ' ';

      while (isdigit(*++t)) /* strip off hops value) */
      {
        node->maximumHops *= 10;
        node->maximumHops += (size_t) (*t - '0');
        *t = ' ';
      }

    }

    t = f3;
    while (isspace(*t))
      t++;

    if (*t != 0)
    {
      printmsg(0, "process_sys: Invalid flags field %s for system %s",
                  f3,
                  f1);
      success = KWFalse;
    }

    if ( node->flag.f )
      batchOptions++;

    if ( node->flag.F )
      batchOptions++;

    if ( node->flag.I )
      batchOptions++;

    if ( node->flag.n )
      batchOptions++;

    if ( batchOptions > 1 )
    {
      printmsg(0, "process_sys: Can't specify more than one of 'fFIJn' "
                  "flags in system %s", f1);
      success = KWFalse;
    }

    if ( batchOptions )
       node->flag.batch = KWTrue;

/*--------------------------------------------------------------------*/
/*       If we are batching, then the default "command" is            */
/*       actually the file name which lists the articles to be        */
/*       processed.                                                   */
/*--------------------------------------------------------------------*/

    if (node->flag.batch && (node->command == NULL))
    {
      char dirname[FILENAME_MAX];

      sprintf( dirname,
               "%s/%s/%.8s/togo",
               E_newsdir,
               OUTGOING_NEWS,
               node->sysname );

      node->command = newstr( dirname );
   }

  } /* flags */

/*--------------------------------------------------------------------*/
/*           Validate options versus the type of the system           */
/*--------------------------------------------------------------------*/

    if ( node->flag.local )
    {

      if ( node->flag.batch )
      {
         printmsg(0,"Batching requested for local system %s, "
                    "which is not allowed",
                    canonical_news_name() );
         success = KWFalse;
      }

      if ( node->command != NULL )
      {

         printmsg(0,"Command/file/directory \"%s\""
                    "specified for local system %s, which is not allowed.",
                    node->command,
                    canonical_news_name() );
         success = KWFalse;
      }

    } /* else if ( node->flag.local ) */
    else if ( (node->flag.batch || (node->command == NULL )) &&
             (checkreal( node->sysname ) == BADHOST))
    {
       printmsg(0,"Invalid host %s listed for news batching in SYS file",
                  node->sysname );
       success = KWFalse;
    }

/*--------------------------------------------------------------------*/
/*       If the command field was not filled in by the user or        */
/*       defaulted (via batching mode), provide our default           */
/*       command to send news onto the next system.                   */
/*--------------------------------------------------------------------*/

   if (( node->command == NULL ) && ! node->flag.local)
   {
      static const char format[] = "uux -anews -p -g%c -n -x %d -C %%s!rnews";
      char command[ sizeof format + FILENAME_MAX ];

      sprintf(command, format ,
              E_newsGrade,
              debuglevel );

      node->command = newstr( command );

   }

   return success;

} /* process_sys */

/*--------------------------------------------------------------------*/
/*       b o o t S t r a p                                            */
/*                                                                    */
/*       Generate a new SYS for a site missing one                    */
/*--------------------------------------------------------------------*/

static void bootStrap( const char *fileName )
{

   FILE *stream = FOPEN( fileName, "w", TEXT_MODE );
   char *sysname = getenv( "UUPCSHADOWS" );

   if ( stream == NULL )
   {
      printmsg(0, "Cannot generate new SYS file for news processing.");
      printerr( fileName );
      panic();
   }

   fprintf( stream, "# News configuration file, automatically generated by "
                    "%s %s\n# at %s\n",
                     compilep,
                     compilev,
                     arpadate() );

/*--------------------------------------------------------------------*/
/*     We get everything, like we did before the SYS file existed     */
/*--------------------------------------------------------------------*/

   fprintf( stream, "# The local system, %s (%s)\n",
            E_domain,
            E_nodename );

   fprintf( stream, "ME:all\n\n" );

/*--------------------------------------------------------------------*/
/*         Everyone else gets our full feed sans-local stuff.         */
/*--------------------------------------------------------------------*/

   fprintf( stream, "# Our news feed, not batched to speed our posts\n");
   fprintf( stream, "%s:all/!local:%s:\n\n",
                    E_newsserv ? E_newsserv : E_mailserv,
                    ( sysname == NULL ) ? "L" :"" );
                           /* Uncompressed feed for speedy posts     */

   if ( sysname != NULL )
   {

      char *buf = strdup( sysname );
      checkref( buf );

      fprintf( stream,
               "# Systems we feed, batched/compressed for high throughput\n" );

      sysname = strtok( buf, WHITESPACE );

      while( sysname != NULL )
      {
         fprintf( stream, "%s:all/!local:F:\n", sysname );
         sysname = strtok( NULL, WHITESPACE );
      }

      free( buf );

   } /* if */

/*--------------------------------------------------------------------*/
/*      Check all our I/O worked, then close up shop and return       */
/*--------------------------------------------------------------------*/

   if ( ferror( stream ) )
   {
      printerr( fileName );
      panic();
   }

   fclose( stream );

   printmsg(0, "Generated new %s file for routing news",
              fileName );

} /* bootStrap */

/*--------------------------------------------------------------------*/
/*       i n i t _ s y s                                              */
/*                                                                    */
/*       Primary entry point for initializing SYS list                */
/*--------------------------------------------------------------------*/

KWBoolean
init_sys( void )
{

  FILE       *sysFileStream = NULL;
  char        sysFileName[ FILENAME_MAX ];
  char        line[ LINESIZ ];
  char       *t;
  char        buf[ INITSIZ ];
  KWBoolean   wantMore = KWTrue;
  KWBoolean   success  = KWTrue;

  mkfilename(sysFileName, E_confdir, "SYS");

/*--------------------------------------------------------------------*/
/*               Generate a new SYS file if we need to                */
/*--------------------------------------------------------------------*/

   if ( stater( sysFileName, 0 ) == -1 )
      bootStrap( sysFileName );

  sysFileStream = fopen(sysFileName, "rb");

  if ( sysFileStream == NULL )
  {
     printerr(sysFileName);
     panic();
  }

  printmsg(3, "init_sys: reading system file %s", sysFileName);

  memset(buf, 0, INITSIZ);

   while (fgets(line, LINESIZ, sysFileStream) != NULL)
   {

/*--------------------------------------------------------------------*/
/*       Trim trailing spaces and cr/lf.  (Prevents empty lines in    */
/*       the log file).                                               */
/*--------------------------------------------------------------------*/

      t = line + strlen(line) - 1;

      while ((t >= line) && isspace(*t))
         *t-- = '\0';

/*--------------------------------------------------------------------*/
/*                      Also trim leading spaces                      */
/*--------------------------------------------------------------------*/

      t = line;

      while (t && isspace(*t))
         t++;

      printmsg(6, "init_sys: read line length %u, \"%s\"",
                  strlen(t),
                  t);

/*--------------------------------------------------------------------*/
/*       Process any buffered data if this line is empty (which       */
/*       terminates the entry)                                        */
/*--------------------------------------------------------------------*/

      if ( *buf && (! strlen(t) || ! wantMore ))
                                    /* Previous entry complete?      */
      {
         if (! process_sys( buf ))  /* Yes --> end of entry, process */
            success = KWFalse;

         *buf = '\0';               /* Also, reset buffer to empty   */
      }

/*--------------------------------------------------------------------*/
/*           Buffer the new data if this is not a comment line        */
/*--------------------------------------------------------------------*/

      if (*t != '#')                /* Comment line?                 */
      {                             /* No --> Add it to our buffer   */

         if ( t[ strlen(t) - 1 ] == '\\' )
            wantMore = KWTrue;
         else
            wantMore = KWFalse;
         strcat(buf, t);

      }  /* else if (*t != '#') */

   } /* while (fgets(line, LINESIZ, sysFileStream) != NULL) */

/*--------------------------------------------------------------------*/
/*                Process the final system entry, if any              */
/*--------------------------------------------------------------------*/

   if (( *buf ) && ! process_sys( buf ) )
      success = KWFalse;

   fclose( sysFileStream );

/*--------------------------------------------------------------------*/
/*              Create a cache buffer for use by check_sys            */
/*--------------------------------------------------------------------*/

   if ( cacheLength )
   {
      cache = (char *) malloc( cacheLength );
      checkref( cache );
   }

/*--------------------------------------------------------------------*/
/*       Report if we had any problems processing the SYS file to     */
/*       our caller                                                   */
/*--------------------------------------------------------------------*/

   return success;

} /* init_sys */

/*
 * this could also be done by copying path into a temp buffer, and
 * tokenizing it on '!' and '\n'.  Then compare the two tokens, if
 * they match, you're outta there!  While slower, this is less memory
 * intensive.
 */

KWBoolean excluded(char *list, char *path)
{
  char    *t1, *t2, *t3;
  int     temp;

  printmsg(5, "exclude: checking %s against %s", list, path);

  t1 = strtok(list, ", ");

  while (t1 != NULL)
  {
    t2 = strstr(path, t1);

    while (t2 != NULL)
    {
      /* MUST be start of string or preceded by '!' */

      if ((t2 == path) ||
          (*(t2 - 1) == '!'))
      {
        /* is this the only entry left ? */

        if (equal(t1, t2))
          return KWTrue;

        /* must be directly followed by a '!' */

        t3 = strchr(t2, '!');

        if (t3 != NULL)
        {
          *t3 = 0;
          temp = strcmp(t1, t2);
          *t3 = '!';
          if (temp == 0)
            return KWTrue;
        }
      }

      /* search for another occurence */
      t2 = strstr(t2 + 1, t1);

    }

    t1 = strtok(NULL, ", ");

  } /* while (t1 != NULL) */

  printmsg(5, "exclude: results in KWFalse");
  return KWFalse;

} /* excluded */

/*--------------------------------------------------------------------*/
/*       d i s t r i b u t i o n s                                    */
/*                                                                    */
/*       These are not as simple as above, there are matching/non     */
/*       matching issues.  !distrib is not a match, and "all"         */
/*       matches all unless an negation exists!  :(                   */
/*--------------------------------------------------------------------*/

KWBoolean distributions(char *list, const char *distrib)
{

  char *listPtr;

  KWBoolean bAll  = KWFalse;    /* We saw "all" or "world"             */
  KWBoolean bRet  = KWFalse;    /* Non-global forward status           */
  KWBoolean bFail = KWFalse;    /* Punted by at least one distribution */
  KWBoolean bDef  = KWTrue;     /* We have yet to see anything but
                                 negations                           */

  size_t distribLength = strlen( distrib ) + 1;
  char * tempDistribP  = malloc( distribLength );

  while (isspace(*distrib))
    distrib++;

  printmsg(5, "distributions: checking %s against %s", list, distrib);

/*--------------------------------------------------------------------*/
/*       Outer loop steps through SYS file list, inner loop steps     */
/*       through list from the article to be processed                */
/*--------------------------------------------------------------------*/

  while ((listPtr = strtok(list, ", ")) != NULL)
  {

    char  *distribPtr = tempDistribP;
    char  *nextDistrib = tempDistribP;

    const KWBoolean bNot = (KWBoolean) ((*listPtr == '!') ?
                                          KWTrue : KWFalse);

    list = strtok( NULL, "" );      /* Save rest of list             */

    if ( bNot )
      listPtr++;                    /* Step to beginning of word     */
    else
      bDef  = KWFalse;              /* Have inclusive distribution   */

    if ( ! bAll && ! bNot )
    {

       if ( equali(listPtr, "all") || equali(listPtr, "world") )
          bAll = KWTrue;

    } /* if ( ! bAll ) */

    strcpy( tempDistribP , distrib );

/*--------------------------------------------------------------------*/
/*       Scan the article's distribution list against this entry      */
/*       in our SYS file.                                             */
/*--------------------------------------------------------------------*/

    while ((distribPtr = strtok(nextDistrib, ", ")) != NULL )
    {
      nextDistrib = NULL;     /* Continue scan with next token
                                 on following pass                */

      if (equali(listPtr, distribPtr ))
      {

         if ( bNot )
            bFail = KWTrue;
         else
            bRet = KWTrue;
      }

#ifdef UDEBUG
      printmsg(6,"Comparing %s and %s, "
                 "bAll = %s, bFail = %s, bRet = %s, bDef = %s",
                 listPtr,
                 distrib,
                 bAll  ? "True" : "False",
                 bFail ? "True" : "False",
                 bRet  ? "True" : "False",
                 bDef  ? "True" : "False" );
#endif

    } /* while ((distribPtr = strtok(nextDistrib, ", ")) != NULL ) */

    list    = strtok( list, "" );

  } /* while ((listPtr = strtok(list, ", ")) != NULL) */

/*--------------------------------------------------------------------*/
/*       If no distributions were found to send to (as opposed to     */
/*       exclude), we assume the implicit distribution all.           */
/*                                                                    */
/*       If we found a specific exclusion, then we approve the        */
/*       distribution only if another distribtion allows it (but      */
/*       ignore a global distribution).                               */
/*                                                                    */
/*       If no exclusions were found, we accept any distribution,     */
/*       including all/world.                                         */
/*--------------------------------------------------------------------*/

  if ( ! bFail )
  {
     if ( bAll || bDef )
        bRet = KWTrue;
  }

  printmsg(5, "distributions: results %s", bRet ? "True" : "False");

  free( tempDistribP );
  return bRet;

} /* distributions */

/*--------------------------------------------------------------------*/
/*       m a t c h                                                    */
/*                                                                    */
/*       Perform matching on news groups                              */
/*--------------------------------------------------------------------*/

KWBoolean match(char *group, char *pattern, int *iSize)
{

  KWBoolean bMatch;
  char *t1, *t2, *t3, *t4;

  bMatch = KWTrue;
  *iSize = 0;
  t1 = group;
  t3 = pattern;

  while (bMatch && (t1 != NULL) && (t3 != NULL))
  {

    t2 = strchr(t1, '.');

    if (t2 != NULL)
      *t2 = 0;

    t4 = strchr(t3, '.');

    if (t4 != NULL)
      *t4 = 0;

    /*
     * "all" as a word in pattern matches everything, counts less
     * than one word.  For these purposes, a word counts as 10, "all"
     * counts as 8.
     */

    if (equal(t3, "all"))
      *iSize += 8;
    else {
      if (equal(t1, t3))
        *iSize += 10;
      else
        bMatch = KWFalse;
    }

    t3 = t4;

    if (t3 != NULL)
      *t3++ = '.';

    t1 = t2;

    if (t1 != NULL)
      *t1++ = '.';
  }

  /*
   * if either part group or pattern has words left, match is true only
   * if there are more group words than pattern words.
   */

  if (bMatch && (((t1 == NULL) && (t3 != NULL)) ||
                 ((t1 != NULL) && (t3 == NULL))
                )
     )
    bMatch = (KWBoolean) ((t3 == NULL) ? KWTrue : KWFalse );

  printmsg(5, "match: matching %s to %s resulting in %s with size %i",
              group,
              pattern,
              bMatch ? "True" : "False",
              *iSize);

  return bMatch;

} /* match */

/*--------------------------------------------------------------------*/
/*       n e w s g r o u p s                                          */
/*                                                                    */
/*       Determine if we want to send the news groups are listed      */
/*       in the supplied allowed groups.                              */
/*--------------------------------------------------------------------*/

KWBoolean newsgroups(char *list, char *groups)
{
  char *currentGroup = groups;
  KWBoolean bMatch, bNoMatch, bNot, success;
  int     iMatch, iNoMatch, iSize;

  printmsg(5, "newsgroups: checking %s against %s", list, groups);

  iMatch   = 0;
  iNoMatch = 0;
  bMatch = KWFalse;
  bNoMatch = KWFalse;

/*--------------------------------------------------------------------*/
/*          Outer loop to walk list of provided news groups           */
/*--------------------------------------------------------------------*/

  while (currentGroup != NULL)
  {
    char *sysGroup = list;          /* Group(s) in SYS file to match */

    char *next = strchr(currentGroup, ',');
                                    /* Actually, comma preceding
                                       next group, if any            */

    if (next != NULL)
      *next = '\0';                 /* Terminate current group name  */

    while (isspace(*currentGroup))
      currentGroup++;               /* Trim leading space            */

/*--------------------------------------------------------------------*/
/*       Inner loop to walk our SYS file list of groups for single    */
/*       group out of provided list                                   */
/*--------------------------------------------------------------------*/

    while (sysGroup != NULL)
    {

      char *sysNext = strchr(sysGroup, ',');
                                    /* Actually, comma preceding
                                       next SYS file group, if any   */

      if (sysNext != NULL)
        *sysNext = 0;

      if (*sysGroup == '!')
      {
         bNot = KWTrue;
         sysGroup++;
      }
      else
         bNot = KWFalse;

      if (match(currentGroup, sysGroup, &iSize))
      {
        if (bNot)
        {
          bNoMatch = KWTrue;

          if (iSize > iNoMatch)
            iNoMatch = iSize;
        }
        else {
          bMatch = KWTrue;

          if (iSize > iMatch)
            iMatch = iSize;
        }

#ifdef UDEBUG
        printmsg(7, "newsgroups: %smatch %s to %s, "
                    "iSize =%d, iMatch = %d, iNoMatch = %d",
                    (char *) ((bNot) ? "mis" : "" ),
                    currentGroup,
                    sysGroup,
                    iSize,
                    iMatch,
                    iNoMatch );
#endif

      } /* if (match(currentGroup, sysGroup, &iSize)) */

      sysGroup = sysNext;

      if (sysGroup != NULL)
        *sysGroup++ = ',';

    } /* while (sysGroup != NULL) */

    currentGroup = next;

    if (currentGroup != NULL)
      *currentGroup++ = ',';

  } /* while (currentGroup != NULL) */

/*--------------------------------------------------------------------*/
/*                  Summarize results and it to caller                */
/*--------------------------------------------------------------------*/

#ifdef UDEBUG

  if (bMatch)
    printmsg(7, "newsgroups: match found, size is %d", iMatch);

  if (bNoMatch)
    printmsg(7, "newsgroups: mismatch found, size is %d", iNoMatch);

#endif

  if ( bMatch && (!bNoMatch || (iMatch > iNoMatch)) )
     success = KWTrue;
  else
     success = KWFalse;

  printmsg(5, "newsgroups: results in %s", success ? "True" : "False");

  return success;

} /* newsgroups */

/*--------------------------------------------------------------------*/
/*       h o p s                                                      */
/*                                                                    */
/*       Return number of entries in supplied path                    */
/*--------------------------------------------------------------------*/

static unsigned short
hops( const char *path )
{
   char *p = strchr( path , '!' );
   unsigned short count = 0;

   while( p != NULL )
   {
      count++;
      p = strchr( p + 1, '!' );
   }

   return count;

} /* hops */

/*--------------------------------------------------------------------*/
/*       c h e c k _ s y s                                            */
/*                                                                    */
/*       Process posting criteria for a given article                 */
/*--------------------------------------------------------------------*/

KWBoolean check_sys(NEWS_SYS *entry, char *groups, char *distrib, char *path)
{

  printmsg(5, "check_sys: node: %s", entry->sysname);
  printmsg(5, "check_sys: groups: %s", groups);
  printmsg(5, "check_sys: distrib: %s", distrib);
  printmsg(5, "check_sys: path: %s", path);

/*--------------------------------------------------------------------*/
/*       Through out this processing, we keep the copies of the       */
/*       strings used for this system clean by copying them before    */
/*       passing them to the routines which want to tokenize them.    */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Never send a post from where it came                         */
/*--------------------------------------------------------------------*/

  if (excluded(strcpy( cache, entry->sysname ), path))
    return KWFalse;

/*--------------------------------------------------------------------*/
/*       Never send to a post to a system which explicitly listed     */
/*       in the system exclusion list (normally because it's an       */
/*       alias of the system name we know the system by.              */
/*--------------------------------------------------------------------*/

  if (entry->exclude )
  {
    printmsg(3, "check_sys: checking exclusions");
    if (excluded(strcpy( cache, entry->exclude ), path))
       return KWFalse;
  }

/*--------------------------------------------------------------------*/
/*       Determine if the message has gone too hops to be forwarded   */
/*--------------------------------------------------------------------*/

  if ( (entry->maximumHops < USHRT_MAX) &&
       (hops( path ) > entry->maximumHops ))
  {
     printmsg(3, "check_sys: Too many hops to be accepted" );
     return KWFalse;
  }

/*--------------------------------------------------------------------*/
/*        Determine if we accept the distribution for this system     */
/*--------------------------------------------------------------------*/

  if (entry->distribution)
  {
    printmsg(3, "check_sys: checking distributions");
    if (!distributions(strcpy( cache, entry->distribution ), distrib))
       return KWFalse;
  }

/*--------------------------------------------------------------------*/
/*            Determine if the remote system wants this group         */
/*--------------------------------------------------------------------*/

  if (entry->groups)
  {
    printmsg(3, "check_sys: checking groups");
    if (!newsgroups(strcpy( cache, entry->groups ), groups))
       return KWFalse;
  }

/*--------------------------------------------------------------------*/
/*       The article passed our filters, report acceptance to caller  */
/*--------------------------------------------------------------------*/

  return KWTrue;

} /* check_sys */

/*--------------------------------------------------------------------*/
/*       g e t _ s y s                                                */
/*                                                                    */
/*       Given a system name, return the SYS structure for it.        */
/*       Returns NULL on system not found or error.                   */
/*--------------------------------------------------------------------*/

NEWS_SYS *
get_sys( const char *name )
{
   NEWS_SYS *current = sys_list;

   while( current != NULL )
   {
      if ( equal( current->sysname, name ) )
         return current;
      else
         current = current->next;
   }

   return NULL;                     /* We didn't find the entry      */

} /* get_sys */

/*--------------------------------------------------------------------*/
/*       e x i t _ s y s                                              */
/*                                                                    */
/*       Terminate SYS file processing                                */
/*--------------------------------------------------------------------*/

void exit_sys(void)
{

  NEWS_SYS *sys_entry;

  while (sys_list != NULL)
  {
    sys_entry = sys_list;
    sys_list = sys_list -> next;
    free(sys_entry);
  }

  if ( cache )
  {
      free( cache );
      cacheLength = 0;
      cache = NULL;
   }

} /* exit_sys */
