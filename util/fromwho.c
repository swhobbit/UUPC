/*--------------------------------------------------------------------*/
/*       f r o m w h o . C                                            */
/*                                                                    */
/*       Summarize mailbox contents                                   */
/*--------------------------------------------------------------------*/

/* modified for UUPC/extended by Kai Uwe Rommel, rommel@ars.muc.de */

/*
  fromwho -- 'from' with extras

  This is a new 'from' program.  Instead of just listing who your mail is
  from, it tells you how many total messages, how many are new, and
  for each person you have mail from, tells how many messages they sent,
  how many are new, and lists the subjects of the messages.

  This code is public domain.  I might keep updating it, or I might forget
  about it ]:)  But feel free to mail comments / suggestions to me at
    jearls@data.acs.calpoly.edu

  Usage:
    fromwho [-l] [-s] [-n] [-a] [-f [<mailbox>]]

    -l  Turns listing of users/subjects off.
    -s  Turns listing of subjects off.
    -f  Sets the mailbox to read from.
    -n  List only new mail
    -a  show sender address instead of sender name

  REVISION INFORMATION

  $Revision: 1.5 $

    $Author: ahd $
      $Date: 1996/11/18 04:46:49 $

  Modification Log:

  $Log: fromwho.c $
  Revision 1.5  1996/11/18 04:46:49  ahd
  Normalize arguments to bugout
  Reset title after exec of sub-modules
  Normalize host status names to use HS_ prefix

  Revision 1.4  1995/01/30 04:08:36  ahd
  Additional compiler warning fixes

  Revision 1.3  1995/01/03 05:32:26  ahd
  Further SYS file support cleanup

  Revision 1.2  1994/12/27 20:47:55  ahd
  Smoother call grading'

  Revision 1.1  1994/12/26 14:25:23  rommel
  Initial revision

    Revision 1.3  91/12/03  11:14:33  jearls
    PATCH3: Added `-v' option to display the version number.

    Revision 1.2  91/11/23  12:49:37  jearls
    PATCH2: Fixed miscellanous bugs, added '-n' option to
    PATCH2: list only new mail.

    Revision 1.1  91/10/19  17:05:17  jearls
    PATCH1: Added support for AIX and other systems that don't
    PATCH1: use a mail spool.

    Revision 1.0  91/10/19  14:40:24  jearls
    Initial revision

*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1996 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"      /* Must be first, includes #pragmas       */

RCSID("$Id$");

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "timestmp.h"


#include "getopt.h"
currentfile();

typedef struct {
  int msgs; int new;
} mailinfo;

typedef struct subjnode {
  char *subj;
  int new;
  struct subjnode *next;
} subjnode;

typedef struct userinfo {
  char *name;
  mailinfo info;

  subjnode *subjs, *lastsub;

  struct userinfo *next;
} userinfo;

/* translate MAILFILE into a real path */

char *translate(char *mf, char *n, char *h)
{
  size_t len;
  char *s, *p, *q;

  len = strlen(mf);

  if (*mf == '~')
    len += strlen(h) - 1;

  p = mf;

  while ((p = strchr(p, '*')) != NULL)
  {
    p++;
    len += strlen(n)-1;
  }

  s = (char *)malloc(len+1);

  if (s)
  {
    p = s;
    if (*mf == '~')
    {
      while (*h)
        *(p++) = *(h++);
      mf++;
    }
    else
      p = s;

    while (*mf)
      if ((*p = *(mf++)) == '*')
      {
        q = n;
        while (*q)
          *(p++) = *(q++);
      }
      else
        p++;

    *p = '\0';
  }

  return(s);

}

mailinfo mailstats(FILE *f, userinfo **ui, int addr)
{
  mailinfo *mp, mi;
  char buf[1024];
  int stat, lp, offs;
  userinfo *p, *b, *n;

  mi.msgs = mi.new = 0;

  stat = 0;
  while (fgets(buf, 1024, f))
  {
    if ((!stat) && (!strncmp(buf, "From:", 5)))
    {
      offs = 5;
      if (addr)
      {
        while (buf[offs] != '<')
          offs++;
        for (offs++, lp=0; buf[lp+offs] != '\n' && buf[lp+offs] != '>'; lp++)
          buf[lp] = buf[lp+offs];
      } else {
        while (isspace(buf[offs]) || buf[offs] == '"')
          offs++;
        for (lp=0; buf[lp+offs] != '\n' &&
             buf[lp+offs] != '"' && buf[lp+offs] != '<'; lp++)
          buf[lp] = buf[lp+offs];
      }
      buf[lp] = '\0';
      stat = 1;
      mi.msgs++;
      p = *ui; b = (userinfo *)NULL;
      while (p && (strcmp(buf, p->name) > 0))
        p = (b=p)->next;
      if (!p || (strcmp(buf, p->name)))
      {
        n = (userinfo *)malloc(sizeof(userinfo));
        n->name = strdup(buf);
        n->info.msgs = n->info.new = 0;
        n->subjs = n->lastsub = (subjnode *)NULL;
        n->next = p;
        if (b)
          b->next = n;
        else
          *ui = n;
        p = n;
      }
      mp = &(p->info);
      mp->msgs++;
      if (p->lastsub)
        p->lastsub = (p->lastsub->next = (subjnode *)malloc(sizeof(subjnode)));
      else
        p->subjs = p->lastsub = (subjnode *)malloc(sizeof(subjnode));
      p->lastsub->subj = (char *)NULL;
      p->lastsub->new = 0;
      p->lastsub->next = (subjnode *)NULL;
    }
    if ((stat) && (!strncmp(buf, "Subject:", 8)))
      if (!p->lastsub->subj)
      {
        buf[strlen(buf)-1] = '\0';
        p->lastsub->subj = strdup(buf+9);
      }
    if ((stat) && (!strncmp(buf, "Status:", 7)))
    {
      stat++;
    }
    if ((stat) && (*buf == '\n'))
    {
      if (stat == 1)
      {
        mi.new++;
        mp->new++;
        p->lastsub->new = 1;
      }
      stat = 0;
    }
  }

  return(mi);
}

void usage(char *name)
{
  printf("\nUsage: %s [-l] [-s] [-n] [-a] [-f [<filename>]]\n", name);
  printf("\n\
    -l  Turns listing of users/subjects off.\n\
    -s  Turns listing of subjects off.\n\
    -f  Sets the mailbox to read from.\n\
    -n  List only new mail\n\
    -a  show sender address instead of sender name\n\
  ");
  exit(-1);
}

int main(int argc, char **argv)
{

  mailinfo mi;
  userinfo *ui = (userinfo *)NULL, *p;
  subjnode *sp, *tp;
  FILE *f;
  char mbox[256], *myname, *prog;
  int c, listflag = 1, subjflag = 1, newflag = 0, addrflag = 0;

  banner( argv );

  if (!configure( B_MAIL ))
    panic();

  if ((myname = getenv("LOGNAME")) == NULL)
    myname = E_mailbox;

  sprintf(mbox, "%s/%s", E_maildir, myname);

/* parse arguments */
  prog = *argv;
  while ((c = getopt(argc, argv, "slnavf:?")) != EOF)
  {
    switch (c)
    {
      case 'f' :  strcpy(mbox, optarg);
                  break;
      case 's' :  subjflag = 0;
                  break;
      case 'l' :  listflag = 0;
                  break;
      case 'n' :  newflag = 1;
                  break;
      case 'a' :  addrflag = 1;
                  break;
      case 'v' :  puts("fromwho, by johnson earls.  $Revision: 1.5 $");
                  exit(0);
      default :   usage(prog);
    }
  }

/* open the file */
  f = (FILE *)fopen(mbox, "r");
  if (f == (FILE *)NULL)
  {
    perror(mbox);
    exit(errno);
  }

/* get the info */
  mi = mailstats(f, &ui, addrflag);

  (void)fclose(f);

/* print everything out */
  if (newflag)
    printf("%s contains %d new messages.\n", mbox, mi.new);
  else
    printf("%s contains %d messages, %d new.\n", mbox, mi.msgs, mi.new);
  while (ui)
  {
    if (listflag && (!newflag || ui->info.new))
      if (newflag)
        printf("  %-50s:  %d new messages.\n", ui->name, ui->info.new);
      else
        printf("  %-50s:  %d messages, %d new.\n", ui->name, ui->info.msgs, ui->info.new);
    for (sp = ui->subjs; sp; )
    {
      if (sp->subj)
      {
        if (listflag && subjflag)
          if (sp->new || !newflag)
            printf("    %c %s\n", (sp->new ? '>' : ' '), sp->subj);
        free(sp->subj);
      } else
        if (listflag && subjflag)
          if (sp->new || !newflag)
            printf("    %c <none>\n", (sp->new ? '>' : ' '));
      sp = (tp=sp) -> next;
      free(tp);
    }
    ui = (p=ui)->next;
    free(p->name);
    free(p);
  }

  return 0;

} /* main */
