/* inews.c
 *
 * Post news articles locally and spool them for remote delivery.
 *
 * Author:  Kai Uwe Rommel <rommel@ars.muc.de>
 * Created: Sun Oct 03 1993
 */

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
 *       $Id: inews.c 1.19 1995/01/05 03:43:49 ahd Exp $
 *
 * $Log: inews.c $
 * Revision 1.19  1995/01/05 03:43:49  ahd
 * rnews SYS file support
 *
 * Revision 1.18  1995/01/03 05:32:26  ahd
 * Further SYS file support cleanup
 *
 * Revision 1.17  1995/01/02 05:03:27  ahd
 * Pass 2 of integrating SYS file support from Mike McLagan
 *
 * Revision 1.16  1994/12/27 23:35:28  ahd
 * Various contributed news fixes; make processing consistent, improve logging,
 * use consistent host names
 *
 * Revision 1.15  1994/12/27 20:51:35  ahd
 * Only searcher headers for specific fields in headers, not body
 *
 * Revision 1.14  1994/12/22 00:24:43  ahd
 * Annual Copyright Update
 *
 * Revision 1.13  1994/12/09 03:52:46  ahd
 * Generate 'U' line first in X.* files to operate with brain dead MKS
 *
 * Revision 1.12  1994/06/14  01:19:24  ahd
 * Clean yp RCS information
 * patches from Kai Uwe Rommel
 *
 * Revision 1.11  1994/04/27  00:00:29  ahd
 * back to text mode on that input file!
 *
 * Revision 1.10  1994/03/20  14:26:12  ahd
 * Normalize top of routine comments (add kew copyright)
 * Add missing ? option
 *
 * Revision 1.10  1994/03/20  14:26:12  ahd
 * Normalize top of routine comments (add kew copyright)
 * Add missing ? option
 *
 * Revision 1.9  1994/03/20  14:22:01  rommel
 * Add grade option to allow different call grades
 *
 * Revision 1.8  1994/03/15  03:02:26  ahd
 * Add error messages after RTL errors
 *
 * Revision 1.7  1994/02/26  17:19:53  ahd
 * Change BINARY_MODE to IMAGE_MODE to avoid IBM C/SET 2 conflict
 *
 * Revision 1.6  1994/02/20  19:11:18  ahd
 * IBM C/Set 2 Conversion, memory leak cleanup
 *
 * Revision 1.5  1994/02/19  04:22:52  ahd
 * Use standard first header
 *
 * Revision 1.5  1994/02/19  04:22:52  ahd
 * Use standard first header
 *
 * Revision 1.4  1994/02/13  04:46:01  ahd
 * Read input in binary mode
 *
 * Revision 1.3  1994/01/18  13:29:22  ahd
 * Various fixes from Kai Uwe Rommel
 * Modify error returns to not use comma operator
 * Use E_newsserv over E_mailserv
 *
 * Revision 1.2  1993/10/31  11:58:18  ahd
 * Delete unneeded tzset()
 *
 * Revision 1.1  1993/10/30  11:40:00  rommel
 * Initial revision
 * */

/* todo: moderated groups, shadow systems delivery */

#include "uupcmoah.h"

static const char rcsid[] =
      "$Id: inews.c 1.19 1995/01/05 03:43:49 ahd Exp $";

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include <ctype.h>
#include <limits.h>
#include <process.h>
#include <sys/types.h>
#include <sys/stat.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "getopt.h"
#include "getseq.h"
#include "import.h"
#include "logger.h"
#include "timestmp.h"
#include "execute.h"
#include "arpadate.h"
#include "hostable.h"

currentfile();

/*--------------------------------------------------------------------*/
/*           Internal prototypes and global data                      */
/*--------------------------------------------------------------------*/

static void usage( void );

static int complete_header(FILE *input, FILE *output );

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
  struct stat st;

   int result = 0;

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

  checkname( E_nodename );      /* Fill in fdomain                  */

  openlog( NULL );

/*--------------------------------------------------------------------*/
/*        Process our arguments                                       */
/*--------------------------------------------------------------------*/

  while ((c = getopt(argc, argv, "g:x:h?")) !=  EOF)
    switch(c)
    {

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

   if ( E_newsserv == NULL )
      E_newsserv = E_mailserv;

/*--------------------------------------------------------------------*/
/*                             Initialize                             */
/*--------------------------------------------------------------------*/

  if (optind == argc - 1)
    if (freopen(argv[optind], "r", stdin) == NULL)
    {
      printmsg(0, "inews: cannot open article file %s", argv[optind]);
      printerr( argv[optind] );
      panic();
    }

/*--------------------------------------------------------------------*/
/*      read input into temporary article file, complete headers      */
/*--------------------------------------------------------------------*/

  mktempname(tempname, "tmp");

  if ((article = FOPEN(tempname, "w", IMAGE_MODE)) == NULL)
  {
    printmsg(0,"inews: cannot create temporary file \"%s\"", tempname);
    printerr( tempname );
    panic();
  }

  if (complete_header(stdin, article ) == -1)
     panic();

  fclose(article);

  if (stat(tempname, &st) == -1)
    panic();

/*--------------------------------------------------------------------*/
/*                         deliver locally                            */
/*--------------------------------------------------------------------*/

  result = execute(RNEWS, NULL, tempname, NULL, KWTrue, KWFalse);

/*--------------------------------------------------------------------*/
/*                             cleanup                                */
/*--------------------------------------------------------------------*/

  unlink(tempname);

  exit( result );

} /* main */

/*--------------------------------------------------------------------*/
/*    u s a g e                                                       */
/*                                                                    */
/*    Print usage of program                                          */
/*--------------------------------------------------------------------*/

static void usage( void )
{

  printf( "Usage: inews [-h] [-x debuglevel]\n");
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
  {
    if (strncmp(buffer, name, strlen(name)) == 0)  /* Find header?   */
      return 0;                        /* Yes, return success        */

    if (buffer[0] == ' ' || buffer[0] == '\n' || buffer[0] == '\r')
                                       /* Run out of header to scan? */
       return -1;                      /* Yes -> Return failure      */
  }

/*--------------------------------------------------------------------*/
/*       We read the entire header and never found the requested      */
/*       header.  Return failure to the caller                        */
/*--------------------------------------------------------------------*/

  return -1;

} /* get_header */

static int complete_header(FILE *input, FILE *output )
{
  char buf[BUFSIZ], *ptr;
  time_t now;
  int OK, i;
  unsigned lines = 0;

  time(&now);

  if (get_header(input, buf, sizeof(buf), "Path:") == -1)
  {
    fprintf(output,"Path: %s\n", E_mailbox );
  }
  else
     fputs( buf, output );


  if (get_header(input, buf, sizeof(buf), "From:") == -1)
    fprintf(output,"From: %s@%s (%s)\n", E_mailbox, E_fdomain, E_name);
  else
    fputs(buf, output);

  if (get_header(input, buf, sizeof(buf), "Newsgroups:") == -1)
  {
    printmsg(0, "inews: no Newsgroup: line, nothing sent.");
    return -1;
  }

  fputs(buf, output);

  if (get_header(input, buf, sizeof(buf), "Subject:") == -1)
    fputs("Subject: <none>\n", output);
  else
    fputs(buf, output);

  if (strncmp(buf, "Subject: cmsg ", 14) == 0)
  {
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
  {
    for (i = 0; i < (int) strlen(E_nodename); i++)
      sprintf(buf + i * 2, "%02x", E_nodename[i] & 0x5F);
    fprintf(output, "Message-ID: <%lx.%s@%s>\n", now, buf, E_fdomain);
  }
  else
    fputs(buf, output);

  fprintf(output, "Date: %s\n", arpadate());

  OK = KWFalse;
  rewind(input);

  while (fgets(buf, sizeof(buf), input) != NULL)
  {
    if (buf[0] == ' ' || buf[0] == '\n' || buf[0] == '\r')
    {
      OK = KWTrue;
      break;
    }
    if (strncmp(buf,"Path:", 5) == 0 ||
        strncmp(buf,"From:", 5) == 0 ||
        strncmp(buf,"Newsgroups:", 11) == 0 ||
        strncmp(buf,"Subject:", 8) == 0 ||
        strncmp(buf,"Distribution:", 13) == 0 ||
        strncmp(buf,"Message-ID:", 11) == 0 ||
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

  if (!OK)
  {
    printmsg(0, "inews: cannot find message body, nothing sent.\n");
    return -1;
  }

  while (fgets(buf, sizeof(buf), input) != NULL)
    lines++;

  fprintf(output, "X-Posting-Software: %s %s inews (%2.2s%3.3s%2.2s %5.5s)\n",
          compilep,
          compilev,
          &compiled[4],
          &compiled[0],
          &compiled[9],
          compilet);

  fprintf(output, "Lines: %d\n\n", lines);
  rewind(input);

  while (fgets(buf, sizeof(buf), input) != NULL)
    if (buf[0] == ' ' || buf[0] == '\n' || buf[0] == '\r')
      break;

  while (fgets(buf, sizeof(buf), input) != NULL)
    fputs(buf, output);

  return 0;

} /* complete_header */
