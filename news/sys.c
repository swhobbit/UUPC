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
/*           J - Batch news for NNS                                   */
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
 *    $Id: sys.c 1.12 1995/01/22 04:16:52 ahd Exp $
 *
 *    Revision history:
 *    $Log: sys.c $
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

currentfile();

struct sys *sys_list = NULL;

static char *cache = NULL;       /* Parsing string cache             */
static size_t cacheLength = 0;

#define ME "ME"

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
  static struct sys *previous = NULL;

  struct sys *node = malloc(sizeof(struct sys));
  char       *f1, *f2, *f3, *f4, *s1, *s2, *t;
  size_t tempLen;

  KWBoolean success = KWTrue;
  int batchOptions = 0;

  memset(node, 0, sizeof(struct sys));

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

  if (s1 != NULL)
  {
    node->exclude = newstr(trim(s1) );
    tempLen = strlen( node->exclude );

    if ( tempLen >= cacheLength )
      cacheLength = tempLen + 1;
  }

  if (s2 != NULL)
  {
      node->distribution = newstr( trim(s2) );
      tempLen = strlen( node->distribution );

      if ( tempLen >= cacheLength )
         cacheLength = tempLen + 1;
  }

  if (f2 != NULL)
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

  if (f3 != NULL)
  {
     printmsg(4, "Flags = %s, length = %i", f3, strlen(f3));

/*--------------------------------------------------------------------*/
/*                    UUPC/extended specific options.                 */
/*--------------------------------------------------------------------*/


    node->flag.c = setBooleanOption(f3, 'c' );
                                    /* Do _not_ compress batches     */

    node->flag.B = setBooleanOption(f3, 'B' );
                                    /* Do not send undersized batches   */

    node->flag.J = setBooleanOption(f3, 'J' );
                                    /* NNS mode - gen local batches     */

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
        node->maximumHops += (size_t) (t - '0' );
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

    if ( node->flag.J )    /* Not a normal batch, but okay for       */
      batchOptions++;      /* conflict management                    */

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

    if ( node->flag.J )             /* Now ignore for true batch     */
       batchOptions--;

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

    if ( node->flag.J )
    {
       if ( node->command == NULL )
          node->command = newstr( E_newsdir );

      node->flag.local = KWFalse;      /* Not treated as local system   */
    }
    else if ( node->flag.local )
    {

      if ( node->flag.batch )
      {
         printmsg(0,"Invalid batching requested for local system %s; only "
                    "NNS batching (J flag) is allowed",
                    canonical_news_name() );
         success = KWFalse;
      }

      if ( node->command != NULL )
      {

         printmsg(0,"Command/file/directory \"%s\""
                    "specified for local system %s; only "
                    "allowed if NNS batching (J flag)"
                    " is specified" ,
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
      char command[ FILENAME_MAX * 4 ];

      sprintf(command, "uux -anews -p -g%c -n -x %d -C %%s!rnews",
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

   if ( E_newsserv )
   {
      fprintf( stream, "# Our news feed, not batched to speed our posts\n");
      fprintf( stream, "%s:all/!local::\n\n", E_newsserv );
                           /* Uncompressed feed for speedy posts     */
   }

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
  char       sysFileName[FILENAME_MAX];
  char       line[BUFSIZ];
  char       *t;
  char       buf[BUFSIZ * 8];
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
  fseek(sysFileStream, 0, SEEK_SET);

  memset(buf, 0, sizeof buf);

   while (fgets(line, sizeof line, sysFileStream) != NULL)
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

         strcat(buf, t);
         wantMore = (KWBoolean) ((*t == '\\') ? KWTrue : KWFalse);
                                    /* End of entry if not explicitly
                                       continued                     */

      }  /* else if (*t != '#') */

   } /* while (fgets(line, sizeof line, sysFileStream) != NULL) */

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
      cache = malloc( cacheLength );
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

  while (isspace(*distrib))
    distrib++;

  printmsg(5, "distributions: checking %s against %s", list, distrib);

/*--------------------------------------------------------------------*/
/*       Outer loop steps through SYS file list, inner loop steps     */
/*       through list from the article to be processed                */
/*--------------------------------------------------------------------*/

  while ((listPtr = strtok(list, ", ")) != NULL)
  {

    char  tempDistrib[BUFSIZ];
    char  *distribPtr = tempDistrib;
    char  *nextDistrib = tempDistrib;

    const KWBoolean bNot = (KWBoolean) ((*listPtr == '!') ?
                                          KWTrue : KWFalse);

    strcpy( tempDistrib, distrib );

    list = strtok( NULL, "" );  /* Save rest of list                */

    if ( bNot )
      listPtr++;                 /* Step to beginning of word        */
    else
      bDef  = KWFalse;            /* We had a inclusive distribution  */

    bAll = bAll || (!bNot && equali(listPtr, "all"));
    bAll = bAll || (!bNot && equali(listPtr, "world"));

    strncpy( tempDistrib , distrib, sizeof tempDistrib );
    tempDistrib[ sizeof tempDistrib - 1] = '\0';

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
    else
      if (equal(t1, t3))
        *iSize += 10;
      else
        bMatch = KWFalse;

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
              group, pattern, bMatch ? "True" : "False", *iSize);

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
  char    *t1, *t2, *t3, *t4;
  KWBoolean bMatch, bNoMatch, bNot, success;
  int     iMatch, iNoMatch, iSize;

  printmsg(5, "newsgroups: checking %s against %s", list, groups);

  iMatch   = 0;
  iNoMatch = 0;
  bMatch = KWFalse;
  bNoMatch = KWFalse;

  t1 = groups;
  while (t1 != NULL)
  {
    t2 = strchr(t1, ',');
    if (t2 != NULL)
      *t2 = 0;

    while (isspace(*t1))
      t1++;

    t3 = list;
    while (t3 != NULL)
    {
      t4 = strchr(t3, ',');
      if (t4 != NULL)
        *t4 = 0;

      bNot = *t3 == '!';
      if (bNot)
        t3++;

      if (match(t1, t3, &iSize))
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
      t3 = t4;
      if (t3 != NULL)
        *t3++ = ',';
    }
    t1 = t2;
    if (t1 != NULL)
      *t1++ = ',';
  }

  if (bMatch)
    printmsg(7, "newsgroups: match found, size is %d", iMatch);

  if (bNoMatch)
    printmsg(7, "newsgroups: mismatch found, size is %d", iNoMatch);

  if ( bMatch && (!bNoMatch || (iMatch > iNoMatch)) )
     success = KWTrue;
  else
     success = KWFalse;

  printmsg(5, "newsgroups: results in %s", success ? "True" : "False");

  return success;

} /* newsgroups */

/*--------------------------------------------------------------------*/
/*       c h e c k _ s y s                                            */
/*                                                                    */
/*       Process posting criteria for a given article                 */
/*--------------------------------------------------------------------*/

KWBoolean check_sys(struct sys *entry, char *groups, char *distrib, char *path)
{

  KWBoolean bRet;

  printmsg(5, "check_sys: node: %s", entry->sysname);
  printmsg(5, "check_sys: groups: %s", groups);
  printmsg(5, "check_sys: distrib: %s", distrib);
  printmsg(5, "check_sys: path: %s", path);

/*--------------------------------------------------------------------*/
/*       Through out this processing, we keep the copies of the       */
/*       strings used for this system clean by copying them before    */
/*       passing them to the routines which want to tokenize them.    */
/*--------------------------------------------------------------------*/

  if (excluded(strcpy( cache, entry->sysname ), path))
    return KWFalse;

  if (bRet && (entry->exclude ))
  {
    printmsg(3, "check_sys: checking exclusions");
    if (excluded(strcpy( cache, entry->exclude ), path))
       return KWFalse;
  }

  if (bRet && (entry->distribution))
  {
    printmsg(3, "check_sys: checking distributions");
    bRet = distributions(strcpy( cache, entry->distribution ), distrib);
  }

  if (bRet && (entry->groups))
  {
    printmsg(3, "check_sys: checking groups");
    bRet = newsgroups(strcpy( cache, entry->groups ), groups);
  }

  return bRet;

} /* check_sys */

/*--------------------------------------------------------------------*/
/*       g e t _ s y s                                                */
/*                                                                    */
/*       Given a system name, return the SYS structure for it.        */
/*       Returns NULL on system not found or error.                   */
/*--------------------------------------------------------------------*/

struct sys *
get_sys( const char *name )
{
   struct sys *current = sys_list;

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

  struct sys *sys_entry;

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
