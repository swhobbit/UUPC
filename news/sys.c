/*--------------------------------------------------------------------*/
/*    s y s . c                                                       */
/*                                                                    */
/*    News sys file maintenance for UUPC/extended.                    */
/*                                                                    */
/*    Written by Mike McLagan <mmclagan@invlogic.com>                 */
/*                                                                    */
/*    This requires USESYSFILE option set in the UUPC.RC file.        */
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
/*    node[/exclusions]:  A node in the systems file.  Also 'me',     */
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
 *    $Id: sys.c 1.2 1995/01/02 05:03:27 ahd Exp $
 *
 *    Revision history:
 *    $Log: sys.c $
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

currentfile();

struct sys *sys_list = NULL;

static FILE *sysFileStream = NULL;

#define ME "ME"

static void bootStrap( const char *fileName );

/*--------------------------------------------------------------------*/
/*       p r o c e s s _ s y s                                        */
/*                                                                    */
/*       Process a single SYS file entry which is already buffered    */
/*       into memory                                                  */
/*--------------------------------------------------------------------*/

void process_sys(const long start, char *buf)
{
  struct sys *node = malloc(sizeof(struct sys));
  static struct sys *previous = NULL;
  char       *f1, *f2, *f3, *f4, *s1, *s2, *t;
  long       start2;

  printmsg(3, "process_sys: start %ld, entry: %s", start, buf);


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

  start2 = start + strlen(f1) + 1;

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
   * case, and refers to this machine, and is replaced by E_domain.
   */

  if ( equal(f1, ME ) || equal( f1, E_nodename ))
     node->sysname = E_domain;
  else
     node->sysname = newstr( f1 );

  if (s1 != NULL)
  {
    node->bExclude = TRUE;
    node->excl_from = start + strlen(f1) + 1;
    node->excl_to = node->excl_from + strlen(s1);
  }

  if (s2 != NULL)
  {
    node->bDistrib = TRUE;
    node->dist_from = start2 + strlen(f2) + 1;
    node->dist_to = node->dist_from + strlen(s2);
  }

  if (f2 != NULL)
  {
    node->bGroups = TRUE;
    node->grp_from = start2;
    node->grp_to = node->grp_from + strlen(f2);
  }

  printmsg(4, "Interpreted sys entry as follows:");

  if ( f1 )
     printmsg(4, "Node = %s, length = %i", f1, strlen(f1));

  if  ( s1 )
     printmsg(4, "Exclusions = %s, length = %i", s1, strlen(s1));

  if ( f2 )
     printmsg(4, "Groups = %s, length = %i", f2, strlen(f2));

  if ( s2 )
     printmsg(4, "Distributions = %s, length = %i", s2, strlen(s2));

  if ( f3 )
     printmsg(4, "Flags = %s, length = %i", f3, strlen(f3));

  if ( f4)
     printmsg(4, "Command = %s, length = %i", f4, strlen(f4));

  if (f3 != NULL)
  {

/*--------------------------------------------------------------------*/
/*                    UUPC/extended specific options.                 */
/*--------------------------------------------------------------------*/

    t = strchr(f3, 'c');            /* Do _not_ compress batches     */

    node->flag.c = (t == NULL) ? FALSE : TRUE;

    if (t != NULL)
      *t = ' ';

    t = strchr(f3, 'B');            /* Do not send undersized batches   */

    node->flag.B = (t == NULL) ? FALSE : TRUE;

    if (t != NULL)
      *t = ' ';

/*--------------------------------------------------------------------*/
/*                     Normal UNIX (C news) options                   */
/*--------------------------------------------------------------------*/

    t = strchr(f3, 'f');

    node->flag.f = (t == NULL) ? FALSE : TRUE;

    if (t != NULL)
      *t = ' ';

    t = strchr(f3, 'F');
    node->flag.F = (t == NULL) ? FALSE : TRUE;

    if (t != NULL)
      *t = ' ';

    t = strchr(f3, 'I');
    node->flag.I = (t == NULL) ? FALSE : TRUE;
    if (t != NULL)
      *t = ' ';

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

    t = strchr(f3, 'm');
    node->flag.m = (t == NULL) ? FALSE : TRUE;
    if (t != NULL)
      *t = ' ';

    t = strchr(f3, 'n');
    node->flag.n = (t == NULL) ? FALSE : TRUE;
    if (t != NULL)
      *t = ' ';

    t = strchr(f3, 'u');
    node->flag.u = (t == NULL) ? FALSE : TRUE;
    if (t != NULL)
      *t = ' ';

    t = f3;
    while (isspace(*t))
      t++;

    if (*t != 0)
    {
      printmsg(0, "process_sys: Invalid flags field %s for system %s",
                  f3,
                  f1);
      panic();
    }

    if ((node->flag.f && (node->flag.F || node->flag.I || node->flag.n)) ||
        (node->flag.F && (node->flag.I || node->flag.n)) ||
        (node->flag.I && (node->flag.n)))
    {
      printmsg(0, "process_sys: Can't specify more than one of 'fFIn' "
                  "flags in system %s", f1);
      panic();
    }

/*--------------------------------------------------------------------*/
/*       If we are batching, then the default "command" is            */
/*       actually the file name which lists the articles to be        */
/*       processed.                                                   */
/*--------------------------------------------------------------------*/

    if ((node->flag.f || node->flag.F || node->flag.I || node->flag.n) &&
        (node->command == NULL))
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
/*       If we are batching or using the default UUX command, then    */
/*       verify the system name is valid.                             */
/*--------------------------------------------------------------------*/

    if ((node->flag.f ||
         node->flag.F ||
         node->flag.I ||
         node->flag.n ||
         (node->command == NULL )) &&
        ! equal( node->sysname, E_domain ) &&
        (checkreal( node->sysname ) == BADHOST))
    {
       printmsg(0,"Invalid host %s listed for news batching in SYS file",
                  node->sysname );
       panic();
    }


/*--------------------------------------------------------------------*/
/*       If the command field was not filled in by the user or        */
/*       defaulted (via batching mode), provide our default           */
/*       command to send news onto the next system.                   */
/*--------------------------------------------------------------------*/

   if ( node->command == NULL )
   {
      char command[ FILENAME_MAX * 4 ];

      sprintf(command, "uux -p -g%c -n -x %d -C %%s!rnews",
              E_newsGrade,
              debuglevel );

      node->command = newstr( command );

   }

} /* process_sys */

void init_sys()
{

  char       sysFileName[FILENAME_MAX];
  long       where;
  long       start;
  char       line[BUFSIZ];
  char       *t;
  char       buf[BUFSIZ * 8];
  boolean    wantMore = TRUE;

  mkfilename(sysFileName, E_confdir, "SYS");

/*--------------------------------------------------------------------*/
/*               Generate a new SYS file if we need to                */
/*--------------------------------------------------------------------*/

   if ( stater( sysFileName, &where ) == -1 )
      bootStrap( sysFileName );

  sysFileStream = fopen(sysFileName, "rb");

  if ( sysFileStream == NULL )
  {
     printerr(sysFileName);
     panic();
  }

  printmsg(3, "init_sys: reading system file %s", sysFileName);
  fseek(sysFileStream, 0, SEEK_SET);

  where = 0;
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
       process_sys(start, buf );    /* Yes --> end of entry, process */
       *buf = '\0';                 /* Also, reset buffer to empty   */
    }

/*--------------------------------------------------------------------*/
/*           Buffer the new data if this is not a comment line        */
/*--------------------------------------------------------------------*/

    if (*t != '#')                  /* Comment line?                 */
    {                               /* No --> Add it to our buffer   */

       if (strlen(buf) == 0)        /* First line of entry?             */
         start = where + (t - line );/* Yes, remember where it is        */

       strcat(buf, line);

       /*
        * does line end with continuation character? lenient search, allows
        * spaces after '\' before end of line.
        */

       wantMore = (*t == '\\') ? TRUE : FALSE;

    }  /* else if (*t != '#') */

    where = ftell(sysFileStream);

  } /* while (fgets(line, sizeof line, sysFileStream) != NULL) */

/*--------------------------------------------------------------------*/
/*                Process the final system entry, if any              */
/*--------------------------------------------------------------------*/

   if ( *buf )
      process_sys(start, buf);

} /* init_sys */

/*
 * this could also be done by copying path into a temp buffer, and
 * tokenizing it on '!' and '\n'.  Then compare the two tokens, if
 * they match, you're outta there!  While slower, this is less memory
 * intensive.
 */

boolean excluded(char *list, char *path)
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
          return TRUE;

        /* must be directly followed by a '!' */

        t3 = strchr(t2, '!');

        if (t3 != NULL)
        {
          *t3 = 0;
          temp = strcmp(t1, t2);
          *t3 = '!';
          if (temp == 0)
            return TRUE;
        }
      }

      /* search for another occurence */
      t2 = strstr(t2 + 1, t1);

    }

    t1 = strtok(NULL, ", ");

  } /* while (t1 != NULL) */

  printmsg(5, "exclude: results in FALSE");
  return FALSE;

} /* excluded */

/*--------------------------------------------------------------------*/
/*       d i s t r i b u t i o n s                                    */
/*                                                                    */
/*       These are not as simple as above, there are matching/non     */
/*       matching issues.  !distrib is not a match, and "all"         */
/*       matches all unless an negation exists!  :(                   */
/*--------------------------------------------------------------------*/

boolean distributions(char *list, const char *distrib)
{

  char *listPtr;

  boolean bAll  = FALSE;      /* We saw "all" or "world"             */
  boolean bRet  = FALSE;      /* Non-global forward status           */
  boolean bFail = FALSE;      /* Punted by at least one distribution */
  boolean bDef  = TRUE;       /* We have yet to see anything but
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

    const boolean bNot = (*listPtr == '!') ? TRUE : FALSE;

    strcpy( tempDistrib, distrib );

    list = strtok( NULL, "" );  /* Save rest of list                */

    if ( bNot )
      listPtr++;                 /* Step to beginning of word        */
    else
      bDef  = FALSE;             /* We had a inclusive distribution  */

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

      if (equali(listPtr, distrib))
      {

         if ( bNot )
            bFail = TRUE;
         else
            bRet = TRUE;
      }

#ifdef UDEBUG
      printmsg(6,"Comparing %s and %s, "
                 "bAll = %s, bFail = %s, bRet = %s, bDef = %s",
                 listPtr,
                 distrib,
                 bAll  ? "TRUE" : "FALSE",
                 bFail ? "TRUE" : "FALSE",
                 bRet  ? "TRUE" : "FALSE",
                 bDef  ? "TRUE" : "FALSE" );
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

  bRet = bFail ? bRet : (bRet || bAll || bDef);

  printmsg(5, "distributions: results %s", bRet ? "TRUE" : "FALSE");

  return bRet;

} /* distributions */

/*--------------------------------------------------------------------*/
/*       m a t c h                                                    */
/*                                                                    */
/*       Perform matching on news groups                              */
/*--------------------------------------------------------------------*/

boolean match(char *group, char *pattern, int *iSize)
{

  boolean bMatch;
  char *t1, *t2, *t3, *t4;

  bMatch = TRUE;
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
        bMatch = FALSE;

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
    bMatch = (t3 == NULL);

  printmsg(5, "match: matching %s to %s resulting in %s with size %i",
              group, pattern, bMatch ? "TRUE" : "FALSE", *iSize);

  return bMatch;

} /* match */

boolean newsgroups(char *list, char *groups)
{
  char    *t1, *t2, *t3, *t4;
  boolean bMatch, bNoMatch, bNot;
  int     iMatch, iNoMatch, iSize;

  printmsg(5, "newsgroups: checking %s against %s", list, groups);

  iMatch   = 0;
  iNoMatch = 0;
  bMatch = FALSE;
  bNoMatch = FALSE;

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
          bNoMatch = TRUE;
          if (iSize > iNoMatch)
            iNoMatch = iSize;
        }
        else {
          bMatch = TRUE;
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

  printmsg(5, "newsgroups: results in %s",
              bMatch && (!bNoMatch || (iMatch > iNoMatch)) ? "TRUE" : "FALSE");

  return (bMatch &&
          (!bNoMatch ||
           (iMatch > iNoMatch)));  /* see SYS.DOC for explanation */
}

/*--------------------------------------------------------------------*/
/*       r e a d _ b l o c k                                          */
/*                                                                    */
/*       Read a block of data from the open SYS file                  */
/*--------------------------------------------------------------------*/

void read_block(long from, long f_to, char *buf)
{
  int  read, size;
  char *t1, *t2;

  size = (int) (f_to - from);
  memset(buf, 0, BUFSIZ * 8);
  if ( fseek(sysFileStream, from, SEEK_SET) )
  {
      printmsg(0, "Unable to seek SYS file to offset %ld.", from);
      printerr("seek" );
      panic();
  }

  read = fread(buf, 1, size, sysFileStream);

  if (read != size)
  {
    printmsg(0, "Unable to read sys file from %ld for %ld bytes.", from, size);

    if ( read < 0 )
       printerr( "read" );
    panic();

  }

  /* compress out spaces, continuations and eoln's */

  t1 = strchr(buf, '\\');
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

} /* read_block */

/*--------------------------------------------------------------------*/
/*       c h e c k _ s y s                                            */
/*                                                                    */
/*       Process posting criteria for a given article                 */
/*--------------------------------------------------------------------*/

boolean check_sys(struct sys *entry, char *groups, char *distrib, char *path)
{

  char *buf;
  boolean bRet = !excluded(entry->sysname, path);

  buf = malloc(BUFSIZ * 8);
  if (buf == NULL)
  {
    printmsg(0, "check_sys:  Unable to allocate memory to read sys entry");
    panic();
  }

  printmsg(3, "check_sys: checking!");
  printmsg(5, "check_sys: node: %s", entry->sysname);
  printmsg(5, "check_sys: groups: %s", groups);
  printmsg(5, "check_sys: distrib: %s", distrib);
  printmsg(5, "check_sys: path: %s", path);

  if (bRet && (entry->bExclude))
  {
    printmsg(3, "check_sys: checking exclusions");
    read_block(entry->excl_from, entry->excl_to, buf);
    bRet = !excluded(buf, path);
  }

  if (bRet && (entry->bDistrib))
  {
    printmsg(3, "check_sys: checking distributions");
    read_block(entry->dist_from, entry->dist_to, buf);
    bRet = distributions(buf, distrib);
  }

  if (bRet && (entry->bGroups))
  {
    printmsg(3, "check_sys: checking groups");

    read_block(entry->grp_from, entry->grp_to, buf);
    bRet = newsgroups(buf, groups);
  }

  printmsg(3, "check_sys: returning %s", bRet ? "TRUE" : "FALSE");

  free(buf);
  return bRet;

} /* check_sys */

void exit_sys(void)
{

  struct sys *sys_entry;

  while (sys_list != NULL)
  {
    sys_entry = sys_list;
    sys_list = sys_list -> next;
    free(sys_entry);
  }

  sys_list = NULL;

   if (sysFileStream != NULL)
     fclose(sysFileStream);
}

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
   fprintf( stream, "ME:all\n" );

/*--------------------------------------------------------------------*/
/*         Everyone else gets our full feed sans-local stuff.         */
/*--------------------------------------------------------------------*/

   if ( E_newsserv )
   {
      fprintf( stream, "# Our news feed, not batched to speed our posts\n");
      fprintf( stream, "%s:all/!local::\n", E_newsserv );
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
