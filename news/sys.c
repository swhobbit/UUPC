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
/*         f,F - Data field contains a directory name to store        */
/*               articles for batching.  If this field is empty       */
/*               RNEWS will use [newsspooldir]/node/B to batch the    */
/*               files in.  F does not write size to the file.        */
/*                                                                    */
/*           I - Write message IDs instead of filenames.              */
/*                                                                    */
/*           n - Write message IDs after the filenames.               */
/*                                                                    */
/*        These do not work with SNEWS flag set, but will with either */
/*        DISTRIBUTESNEWS or NODISTRIBUTESNEWS.                       */
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
 *    $Id: lib.h 1.25 1994/12/27 20:50:28 ahd Exp $
 *
 *    Revision history:
 *    $Log: lib.h $
 */

#include "uupcmoah.h"

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include <fcntl.h>
#include <io.h>
#include <ctype.h>
#include <sys/stat.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "timestmp.h"
#include "active.h"
#include "sys.h"

currentfile();

struct sys *sys_list = NULL;

static FILE *sysfile;

void process_sys(long start,char *buf)
{
  struct sys *node;
  char       *f1,*f2,*f3,*f4,*s1,*s2,*t;
  long       start2;
  boolean    flag_f,
             flag_F,
             flag_I,
             flag_m,
             flag_n,
             flag_u;

  if (strlen(buf) == 0)
    return;

  s2 = strchr(buf,0);

  /* strip off final cr/lf pair and spaces*/
  if (*--s2 == '\n')
    *s2 = 0;
  if (*--s2 == '\r')
    *s2 = 0;
  while (*s2 == ' ')
    *s2-- = 0;

  printmsg(3,"process_sys: start %ld\nentry: %s",start,buf);
  node = malloc(sizeof(struct sys));
  memset(node,0,sizeof(struct sys));
  node->next = sys_list;
  sys_list = node;

  f1 = buf;                /* node field    */
  f2 = strchr(f1,':');     /* groups field  */
  if (f2 != NULL)
    *f2++ = 0;
  start2 = start + strlen(f1) + 1;
  f3 = strchr(f2,':');     /* flags field   */
  if (f3 != NULL)
    *f3++ = 0;
  f4 = strchr(f3,':');     /* command field */
  if (f4 != NULL)
    *f4++ = 0;
  s1 = strchr(f1,'/'); /* exclusions subfield    */
  if (s1 != NULL)
    *s1++ = 0;
  s2 = strchr(f2,'/'); /* distributions subfield */
  if (s2 != NULL)
    *s2++ = 0;
  if (f4 != NULL)
    strcpy(node->command,f4);
  if (f3 != NULL)
    strcpy(node->flags,f3);

  /*
   * if the node field has a subfield, it's been stripped off already,
   * leaving the node name all by itself.  Note that "me" is a special
   * case, and refers to this machine, and is replaced by E_nodename.
   */

  strcpy(node->sysname, equal(f1,"me") ? E_nodename : f1);
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

  printmsg(4,"Interpreted sys entry as follows:");
  printmsg(4,"Node = %s, length = %i",f1,strlen(f1));
  printmsg(4,"Exclusions = %s, length = %i",s1,strlen(s1));
  printmsg(4,"Groups = %s, length = %i",f2,strlen(f2));
  printmsg(4,"Distributions = %s, length = %i",s2,strlen(s2));
  printmsg(4,"Flags = %s, length = %i",f3,strlen(f3));
  printmsg(4,"Command = %s, length = %i",f4,strlen(f4));

  if (f3 != NULL)
  {
    t = strchr(f3,'f');
    flag_f = t != NULL;
    if (t != NULL)
      *t = ' ';

    t = strchr(f3,'F');
    flag_F = t != NULL;
    if (t != NULL)
      *t = ' ';

    t = strchr(f3,'I');
    flag_I = t != NULL;
    if (t != NULL)
      *t = ' ';

    t = strchr(f3,'L');
    if (t != NULL)
    {
      *t = ' ';
      while (isdigit(*++t)) /* strip off hops value) */
        *t = ' ';
    }

    t = strchr(f3,'m');
    flag_m = t != NULL;
    if (t != NULL)
      *t = ' ';

    t = strchr(f3,'n');
    flag_n = t != NULL;
    if (t != NULL)
      *t = ' ';

    t = strchr(f3,'u');
    flag_u = t != NULL;
    if (t != NULL)
      *t = ' ';

    t = f3;
    while (*t == ' ')
      t++;

    if (*t != 0)
    {
      printmsg(0,"process_sys: Invalid flags field %s for system %s",node->flags,f1);
      panic();
    }

    if ((flag_f && (flag_F || flag_I || flag_n)) ||
        (flag_F && (flag_f || flag_I || flag_n)) ||
        (flag_I && (flag_F || flag_f || flag_n)) ||
        (flag_n && (flag_F || flag_I || flag_f)))
    {
      printmsg(0,"process_sys: Can't specify more than one of 'fFIn' flags in system %s",f1);
      panic();
    }

    if ((flag_m || flag_u) && (group_list == NULL))
    {
      printmsg(0,"process_sys: Can't specify 'mu' flags without active file. System %s",f1);
      panic();
    }
  }
  memset(buf,0,BUFSIZ * 8);
}

void init_sys(FILE *sys_file)
{

  long       where;
  long       start;
  char       line[BUFSIZ];
  char       *t;
  char       *buf;

  buf = malloc(BUFSIZ * 8);
  if (buf == NULL)
  {
    printmsg(0,"Insuffient memory to allocate sys buffer");
    panic();
  }

  printmsg(3,"init_sys: reading system file");
  sysfile = sys_file;
  fseek(sysfile,0,SEEK_SET);

  where = 0;
  memset(buf,0,sizeof buf);
  while (fgets(line,sizeof line,sysfile) != NULL)
  {
    printmsg(6,"init_sys: read line length %i, \"%s\"",strlen(line),line);
    if (*line == '#')
      process_sys(start,buf);
    else
    {
      t = line;
      while (*t == ' ')
       t++;

      /* cr lf */
      if (strlen(t) == 2)
        process_sys(start,buf);
      else
      {
        if (strlen(buf) == 0)
          start = where;
        strcat(buf,line);

        /*
         * does line end with continuation character? lenient search, allows
         * spaces after '\' before end of line.
         */

        printmsg(5,"Checking end of line for continuation");
        t = strchr(line,0) - 3; /* find the end of string (subtract crlf0)*/
        while (*t == ' ')
          t--;
        printmsg(5,"Last non space character on line is %i(%c)",(int) *t,*t);

        if (*t != '\\')
          process_sys(start,buf);
      }
    }
    where = ftell(sysfile);
  }
  process_sys(start,buf);
  free(buf);
}

/*
 * this could also be done by copying path into a temp buffer, and
 * tokenizing it on '!' and '\n'.  Then compare the two tokens, if
 * they match, you're outta there!  While slower, this is less memory
 * intensive.
 */

boolean excluded(char *list,char *path)
{
  char    *t1,*t2,*t3;
  int     temp;

  printmsg(5,"exclude: checking %s against %s",list,path);
  t1 = strtok(list,",");
  while (t1 != NULL)
  {
    t2 = strstr(path,t1);
    while (t2 != NULL)
    {
      /* MUST be start of string or preceded by '!' */
      if ((t2 == path) ||
          (*(t2 - 1) == '!'))
      {
        /* is this the only entry left ? */
        if (equal(t1,t2))
          return TRUE;

        /* must be directly followed by a '!' */
        t3 = strchr(t2,'!');
        if (t3 != NULL)
        {
          *t3 = 0;
          temp = strcmp(t1,t2);
          *t3 = '!';
          if (temp == 0)
            return TRUE;
        }
      }
      /* search for another occurence */
      t2 = strstr(t2 + 1,t1);
    }
    t1 = strtok(NULL,",");
  }
  printmsg(5,"exclude: results in FALSE");
  return FALSE;
}

/*
 * these are not as simple as above, there are matching/non matching
 * issues.  !distrib is not a match, and "all" matches all! :(
 */

boolean distributions(char *list,char *distrib)
{
  char    *t1,*t3,*t4;
  boolean bRet,bAll,bNot,bExit;

  while (*distrib == ' ')
    distrib++;

  printmsg(5,"distributions: checking %s against %s",list,distrib);

  bRet  = FALSE;  /* must match something specific */
  bAll  = FALSE;
  bExit = FALSE;
  t1 = strtok(list,",");
  while ((t1 != NULL) && !bExit)
  {
    bNot = *t1 == '!';
    if (bNot)
      t1++;

    bAll = bAll || (!bNot && equali(t1,"all"));

    t3 = distrib;
    while (t3 != NULL)
    {
      t4 = strchr(t3,',');
      if (t4 != NULL)
        *t4 = 0;

      if (bNot && equali(t1,distrib))
      {
        bExit = TRUE;
        bRet = FALSE;
      }

      bRet = bRet || equali(t1,distrib);

      t3 = t4;
      if (t3 != NULL)
        *t3++ = ',';
    }
    t1 = strtok(NULL,",");
  }
  printmsg(5,"distributions: results %s",bRet || bAll ? "TRUE" : "FALSE");
  return bRet || bAll;
}

boolean match(char *group,char *pattern,int *iSize)
{

  boolean bMatch;
  char *t1,*t2,*t3,*t4;

  bMatch = TRUE;
  *iSize = 0;
  t1 = group;
  t3 = pattern;
  while (bMatch && (t1 != NULL) && (t3 != NULL))
  {
    t2 = strchr(t1,'.');
    if (t2 != NULL)
      *t2 = 0;
    t4 = strchr(t3,'.');
    if (t4 != NULL)
      *t4 = 0;

    /*
     * "all" as a word in pattern matches everything, counts less
     * than one word.  For these purposes, a word counts as 10, "all"
     * counts as 8.
     */

    if (equal(t3,"all"))
      *iSize += 8;
    else
      if (equal(t1,t3))
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

  printmsg(5,"match: matching %s to %s resulting in %s with size %i",
              group,pattern,bMatch ? "TRUE" : "FALSE", *iSize);

  return bMatch;
}

boolean newsgroups(char *list,char *groups)
{
  char    *t1,*t2,*t3,*t4;
  boolean bMatch,bNoMatch,bNot;
  int     iMatch,iNoMatch,iSize;

  printmsg(5,"newsgroups: checking %s against %s",list,groups);

  iMatch   = 0;
  iNoMatch = 0;
  bMatch = FALSE;
  bNoMatch = FALSE;

  t1 = groups;
  while (t1 != NULL)
  {
    t2 = strchr(t1,',');
    if (t2 != NULL)
      *t2 = 0;

    while (*t1 == ' ')
      t1++;

    t3 = list;
    while (t3 != NULL)
    {
      t4 = strchr(t3,',');
      if (t4 != NULL)
        *t4 = 0;

      bNot = *t3 == '!';
      if (bNot)
        t3++;

      if (match(t1,t3,&iSize))
        if (bNot)
        {
          bNoMatch = TRUE;
          if (iSize > iNoMatch)
            iNoMatch = iSize;
        }
        else
        {
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
    printmsg(7,"newsgroups: match found, size is %d",iMatch);

  if (bNoMatch)
    printmsg(7,"newsgroups: mismatch found, size is %d",iNoMatch);

  printmsg(5,"newsgroups: results in %s",
              bMatch && (!bNoMatch || (iMatch > iNoMatch)) ? "TRUE" : "FALSE");

  return (bMatch &&
          (!bNoMatch ||
           (iMatch > iNoMatch)));  /* see SYS.DOC for explanation */
}

void read_block(long from,long f_to,char *buf)
{
  int  read,size;
  char *t1,*t2;

  size = (int) (f_to - from);
  memset(buf,0,BUFSIZ * 8);
  fseek(sysfile,from,SEEK_SET);
  read = fread(buf,1,size,sysfile);
  if (read != size)
  {
    printmsg(0,"Unable to read sys file from %ld for %ld bytes.",from,size);
    panic();
  }

  /* compress out spaces, continuations and eoln's */
  t1 = strchr(buf,'\\');
  while (t1 != NULL)
  {
    memmove(t1,t1 + 1,strlen(t1));
    t1 = strchr(t1,'\\');
  }
  t1 = strchr(buf,'\n');
  while (t1 != NULL)
  {
    memmove(t1,t1 + 1,strlen(t1));
    t1 = strchr(t1,'\n');
  }
  t1 = strchr(buf,'\r');
  while (t1 != NULL)
  {
    memmove(t1,t1 + 1,strlen(t1));
    t1 = strchr(t1,'\r');
  }
  t1 = strchr(buf,' ');
  while (t1 != NULL)
  {
    t2 = t1;
    while (*++t2 == ' ');
    memmove(t1,t2,strlen(t2) + 1);
    t1 = strchr(t1,' ');
  }

}

boolean check_sys(struct sys *entry,char *groups,char *distrib,char *path)
{

  char *buf;
  boolean bRet,bDistrib;

  buf = malloc(BUFSIZ * 8);
  if (buf == NULL)
  {
    printmsg(0,"check_sys:  Unable to allocate memory to read sys entry");
    panic();
  }

  printmsg(3,"check_sys: checking!");
  printmsg(5,"check_sys: node: %s",entry->sysname);
  printmsg(5,"check_sys: groups: %s",groups);
  printmsg(5,"check_sys: distrib: %s",distrib);
  printmsg(5,"check_sys: path: %s",path);

  bRet = !excluded(entry->sysname,path);
  if (bRet && (entry->bExclude))
  {
    printmsg(3,"check_sys: checking exclusions");
    read_block(entry->excl_from,entry->excl_to,buf);
    bRet = !excluded(buf,path);
  }

  bDistrib = FALSE;

  if (bRet && (entry->bDistrib))
  {
    printmsg(3,"check_sys: checking distributions");
    read_block(entry->dist_from,entry->dist_to,buf);
    bDistrib = distributions(buf,distrib);
  }

  if (bRet && (entry->bGroups))
  {
    printmsg(3,"check_sys: checking groups");
    if (!bDistrib)
    {
      read_block(entry->grp_from,entry->grp_to,buf);
      bDistrib = newsgroups(buf,distrib);
    }
    if (bDistrib)
    {
      read_block(entry->grp_from,entry->grp_to,buf);
      bRet = newsgroups(buf,groups);
    }
  }

  bRet = bRet && bDistrib;

  printmsg(3,"check_sys: returning %s",bRet ? "TRUE" : "FALSE");

  free(buf);
  return bRet;
}

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
}
