/* inews.c
 *
 * Post news articles locally and spool them for remote delivery.
 *
 * Author:  Kai Uwe Rommel <rommel@ars.muc.de>
 * Created: Sun Oct 03 1993
 */

/* todo: moderated groups, shadow systems delivery */

static char *rcsid =
"$Id: INEWS.C 1.1 1993/10/30 11:40:00 rommel Exp $";
static char *rcsrev = "$Revision: 1.1 $";

/* $Log: INEWS.C $
 * Revision 1.1  1993/10/30  11:40:00  rommel
 * Initial revision
 * */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <limits.h>
#include <process.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "hlib.h"
#include "getopt.h"
#include "getseq.h"
#include "import.h"
#include "logger.h"
#include "timestmp.h"
#include "execute.h"
#include "arpadate.h"

currentfile();

/*--------------------------------------------------------------------*/
/*                        Internal prototypes                         */
/*--------------------------------------------------------------------*/

static void usage( void );
static int complete_header(FILE *input, FILE *output);
static int remote_news(FILE *article);
static int spool_news(char *sysname, FILE *article, char *command);

/*--------------------------------------------------------------------*/
/*    m a i n                                                         */
/*                                                                    */
/*    Main program                                                    */
/*--------------------------------------------------------------------*/

void main( int argc, char **argv)
{
  extern char *optarg;
  extern int   optind;
  int c;
  char tempname[FILENAME_MAX];  /* temporary input file     */
  FILE *article;

/*--------------------------------------------------------------------*/
/*     Report our version number and date/time compiled               */
/*--------------------------------------------------------------------*/

#if defined(__CORE__)
  copywrong = strdup(copyright);
  checkref(copywrong);
#endif

  banner( argv );

  if (!configure( B_NEWS ))
    exit(1);                    /* system configuration failed */

  openlog( NULL );

/*--------------------------------------------------------------------*/
/*        Process our arguments                                       */
/*--------------------------------------------------------------------*/

  while ((c = getopt(argc, argv, "x:h")) !=  EOF)
    switch(c) {

    case 'x':
      debuglevel = atoi( optarg );
      break;

    case 'h':
      break;

    case '?':
      usage();
      exit(1);
      break;

    default:
      printmsg(0, "inews - invalid option -%c", c);
      usage();
      exit(2);
      break;
    }

/*--------------------------------------------------------------------*/
/*                             Initialize                             */
/*--------------------------------------------------------------------*/

  if (optind == argc - 1)
    if (freopen(argv[optind], "r", stdin) == NULL) {
      printmsg(0, "inews: cannot open article file %s", argv[optind]);
      panic();
    }

/*--------------------------------------------------------------------*/
/*      read input into temporary article file, complete headers      */
/*--------------------------------------------------------------------*/

  mktempname(tempname, "tmp");

  if ((article = FOPEN(tempname, "w", BINARY_MODE)) == NULL) {
    printmsg(0,"inews: cannot create temporary file \"%s\"", tempname);
    panic();
  }

  if (complete_header(stdin, article) == -1)
    panic();

  fclose(article);

/*--------------------------------------------------------------------*/
/*                     spool for remote delivery                      */
/*--------------------------------------------------------------------*/

  printmsg(1, "Spooling news from %s via %s", E_mailbox, E_mailserv);

  article = FOPEN(tempname, "r", TEXT_MODE);
  remote_news(article);
  fclose(article);

/*--------------------------------------------------------------------*/
/*                         deliver locally                            */
/*--------------------------------------------------------------------*/

  execute(RNEWS, NULL, tempname, NULL, TRUE, FALSE);

/*--------------------------------------------------------------------*/
/*                             cleanup                                */
/*--------------------------------------------------------------------*/

  unlink(tempname);

  exit(0);

} /* main */

/*--------------------------------------------------------------------*/
/*    u s a g e                                                       */
/*                                                                    */
/*    Print usage of program                                          */
/*--------------------------------------------------------------------*/

static void usage( void )
{
  printf( "Usage:   inews [-h]\n");
  exit(1);
} /* usage */

/*--------------------------------------------------------------------*/
/*    c o m p l e t e _ h e a d e r                                   */
/*                                                                    */
/*    Add all required headers.                                       */
/*--------------------------------------------------------------------*/

static int get_header(FILE *input, char *buffer, int size, char *name)
{
  rewind(input);

  while (fgets(buffer, size, input) != NULL)
    if (strncmp(buffer, name, strlen(name)) == 0)
      return 0;

  return -1;
} /* get_header */

static int complete_header(FILE *input, FILE *output)
{
  char buf[256], *ptr;
  time_t now;
  int OK;
  unsigned lines = 0;
  char *X_fdomain = E_fdomain ? E_fdomain : E_domain;

  time(&now);

  if (get_header(input, buf, sizeof(buf), "Path:") == -1)
    fprintf(output,"Path: %s!%s\n", E_domain, E_mailbox);
  else
    fprintf(output,"Path: %s!%s", E_domain, buf + 6);

  if (get_header(input, buf, sizeof(buf), "From:") == -1)
    fprintf(output,"From: %s@%s (%s)\n", E_mailbox, X_fdomain, E_name);
  else
    fputs(buf, output);

  if (get_header(input, buf, sizeof(buf), "Newsgroups:") == -1) {
    printmsg(0, "inews: no Newsgroup: line, nothing sent.");
    return -1;
  }
  fputs(buf, output);

  if (get_header(input, buf, sizeof(buf), "Subject:") == -1)
    fputs("Subject: <none>\n", output);
  else
    fputs(buf, output);

  if (strncmp(buf, "Subject: cmsg ", 14) == 0) {
    char cmsg[256]; /* for old-style control messages such as from TRN */
    strcpy(cmsg, buf + 14);
    if (get_header(input, buf, sizeof(buf), "Control:") == -1)
      fprintf(output, "Control: %s", cmsg);
    else
      fputs(buf, output);
  }

  if (get_header(input, buf, sizeof(buf), "Distribution:") == -1)
    strcpy(buf, "Distribution: world\n");
  ptr = buf + strlen("Distribution:");
  while (*ptr && isspace(*ptr))
    ptr++;
  if (*ptr == 0 || *ptr == '\n')
    strcpy(buf, "Distribution: world\n");
  fputs(buf, output);

  if (get_header(input, buf, sizeof(buf), "Message-ID:") == -1)
    fprintf(output, "Message-ID: <%lx.%s@%s>\n", now, E_nodename, E_domain);
  else
    fputs(buf, output);

  /* fprintf(output, "Sender: %s@%s\n", E_postmaster, X_fdomain); */

  fprintf(output, "Date: %s\n", arpadate());

  OK = FALSE;
  rewind(input);

  while (fgets(buf, sizeof(buf), input) != NULL) {
    if (buf[0] == ' ' || buf[0] == '\n' || buf[0] == '\r') {
      OK = TRUE;
      break;
    }
    if (strncmp(buf,"Path:", 5) == 0 ||
        strncmp(buf,"From:", 5) == 0 ||
        strncmp(buf,"Newsgroups:", 11) == 0 ||
        strncmp(buf,"Subject:", 8) == 0 ||
        strncmp(buf,"Distribution:", 13) == 0 ||
        strncmp(buf,"Message-ID:", 11) == 0 ||
        strncmp(buf,"Sender:", 7) == 0 ||
        strncmp(buf,"Lines:", 6) == 0 ||
        strncmp(buf,"X-Posting-Software:", 19) == 0 ||
        strncmp(buf,"Date:", 5) == 0)
      continue;
    if ((ptr = strchr(buf, ':')) == NULL)
      continue;
    for (ptr++; *ptr && isspace(*ptr); ptr++);
    if (strlen(ptr) > 1)
      fputs(buf, output);
  }

  if (!OK) {
    printmsg(0, "inews: cannot find message body, nothing sent.\n");
    return -1;
  }

  while (fgets(buf, sizeof(buf), input) != NULL)
    lines++;

  fprintf(output, "X-Posting-Software: %s %s inews (%2.2s%3.3s%2.2s %5.5s)\n",
          compilep, compilev,
          &compiled[4], &compiled[0], &compiled[9], compilet);
  fprintf(output, "Lines: %d\n\n", lines);
  rewind(input);

  while (fgets(buf, sizeof(buf), input) != NULL)
    if (buf[0] == ' ' || buf[0] == '\n' || buf[0] == '\r')
      break;
  while (fgets(buf, sizeof(buf), input) != NULL)
    fputs(buf, output);

  return 0;
} /* complete_header */


/*--------------------------------------------------------------------*/
/*    r e m o t e _ n e w s                                           */
/*                                                                    */
/*    Transmit news to other systems                                  */
/*--------------------------------------------------------------------*/

static int remote_news(FILE *article)
{
  char path[256], buf[256], *sysname;

  rewind(article);
  get_header(article, path, sizeof(path), "Path:");

  if ( (sysname = getenv("UUPCSHADOWS")) != NULL )
  {
    strcpy(buf, sysname);

    for (sysname = strtok(buf, WHITESPACE); sysname != NULL;
         sysname = strtok(NULL, WHITESPACE))
      if (strncmp(sysname, path + 6, strlen(sysname)) != 0)
      {                       /* do not send it to where it came from */
        rewind(article);
        spool_news(sysname, article, "rnews");
      }
  }

  return spool_news(E_mailserv, article,
                    bflag[F_UUPCNEWSSERV] ? "inews" : "rnews");
}

/*--------------------------------------------------------------------*/
/*    s p o o l _ n e w s                                             */
/*                                                                    */
/*    Spool news to other systems                                     */
/*--------------------------------------------------------------------*/

static int spool_news(char *sysname, FILE *article, char *command)
{
  static char *spool_fmt = SPOOLFMT; /* spool file name */
  static char *dataf_fmt = DATAFFMT;
  static char *send_cmd  = "S %s %s %s - %s 0666\n";
  static long seqno = 0;
  FILE *out_stream;             /* For writing out data                */
  char buf[BUFSIZ];
  unsigned len;

  char msfile[FILENAME_MAX];    /* MS-DOS format name of files         */
  char msname[22];              /* MS-DOS format w/o path name         */
  char *seq;

  char tmfile[15];              /* Call file, UNIX format name         */

  char idfile[15];              /* Data file, UNIX format name         */
  char rdfile[15];              /* Data file name on remote system,
                                   UNIX format                         */

  char ixfile[15];              /* eXecute file for remote system,
                                   UNIX format name for local system   */
  char rxfile[15];              /* Remote system UNIX name of eXecute
                                   file                                */

/*--------------------------------------------------------------------*/
/*          Create the UNIX format of the file names we need          */
/*--------------------------------------------------------------------*/

  seqno = getseq();
  seq = JobNumber(seqno);

  sprintf(tmfile, spool_fmt, 'C', sysname,  'd' , seq);
  sprintf(idfile, dataf_fmt, 'D', E_nodename , seq, 'd');
  sprintf(rdfile, dataf_fmt, 'D', E_nodename , seq, 'r');
  sprintf(ixfile, dataf_fmt, 'D', E_nodename , seq, 'e');
  sprintf(rxfile, dataf_fmt, 'X', E_nodename , seq, 'r');

/*--------------------------------------------------------------------*/
/*                     create remote X (xqt) file                     */
/*--------------------------------------------------------------------*/

  importpath(msname, ixfile, sysname);
  mkfilename(msfile, E_spooldir, msname);

  if ( (out_stream = FOPEN(msfile, "w", BINARY_MODE)) == NULL )
  {
    printmsg(0, "spool_news: cannot create X file %s", msfile);
    printerr(msfile);
    return -1;
  } /* if */

  fprintf(out_stream, "R news %s\n", E_domain);
  fprintf(out_stream, "U news %s\n", E_nodename);
  fprintf(out_stream, "F %s\n", rdfile);
  fprintf(out_stream, "I %s\n", rdfile);
  fprintf(out_stream, "C %s\n", command);

  fclose(out_stream);

/*--------------------------------------------------------------------*/
/*  Create the data file with the data to send to the remote system   */
/*--------------------------------------------------------------------*/

  importpath(msname, idfile, sysname);
  mkfilename(msfile, E_spooldir, msname);

  if ((out_stream = FOPEN(msfile, "w", BINARY_MODE)) == NULL )
  {
    printmsg(0, "spool_news: Cannot create D file %s", msfile);
    printerr(msfile);
    return -1;
  }

/*--------------------------------------------------------------------*/
/*                       Loop to copy the data                        */
/*--------------------------------------------------------------------*/

  rewind(article);

  while ((len = fread(buf, 1, sizeof(buf), article)) != 0)
  {
    if (fwrite(buf, 1, len, out_stream) != len) /* I/O error? */
    {
      printerr(msfile);
      fclose(out_stream);
      return -1;
    } /* if */
  } /* while */

  fclose(out_stream);

/*--------------------------------------------------------------------*/
/*                     create local C (call) file                     */
/*--------------------------------------------------------------------*/

  importpath(msname, tmfile, sysname);
  mkfilename(msfile, E_spooldir, msname);

  if ((out_stream = FOPEN(msfile, "w",TEXT_MODE)) == NULL)
  {
    printerr( msname );
    printmsg(0, "spool_news: cannot create C file %s", msfile);
    return -1;
  }

  fprintf(out_stream, send_cmd, idfile, rdfile, "news", idfile);
  fprintf(out_stream, send_cmd, ixfile, rxfile, "news", ixfile);

  fclose(out_stream);

  return 0;
} /* spool_news */

/* end of inews.c */