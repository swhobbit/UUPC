/* genhist.c
 *
 * (Re-)Create history database from scratch.
 * Based on parts of the old expire program.
 *
 * Author:  Kai Uwe Rommel <rommel@ars.muc.de>
 * Created: Sun Aug 15 1993
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

#include "uupcmoah.h"

static const char rcsid[] =
         "$Id: genhist.c 1.9 1994/12/22 00:24:30 ahd Exp $";

/* $Log: genhist.c $
/* Revision 1.9  1994/12/22 00:24:30  ahd
/* Annual Copyright Update
/*
/* Revision 1.8  1994/06/14 01:19:24  ahd
/* Clean yp RCS information
/* patches from Kai Uwe Rommel
/*
 * Revision 1.7  1994/03/20  23:35:57  rommel
 * Also update active file with discovered information
 *
 * Revision 1.6  1994/02/19  04:21:26  ahd
 * Use standard first header
 *
 * Revision 1.5  1994/01/01  19:14:13  ahd
 * Annual Copyright Update
 *
 * Revision 1.4  1993/12/09  13:24:25  ahd
 * Enforce options=history being set to use program
 *
 * Revision 1.3  1993/11/06  13:04:13  ahd
 * Update usage message
 *
 * Revision 1.2  1993/10/30  22:27:57  rommel
 * Lower debug level to 1
 *
 * Revision 1.1  1993/09/05  10:56:49  rommel
 * Initial revision
 * */

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

currentfile();

/*--------------------------------------------------------------------*/
/*                        Internal prototypes                         */
/*--------------------------------------------------------------------*/

static void IndexAll( void );
static void IndexOneGroup( struct grp *cur_grp );
static void IndexDirectory( struct grp *cur_grp, const char *directory );

static boolean numeric( char *start);

static void usage( void );

long total_articles = 0;
long total_files = 0;
long total_bytes = 0;

void *history;

/*--------------------------------------------------------------------*/
/*    m a i n                                                         */
/*                                                                    */
/*    Main program                                                    */
/*--------------------------------------------------------------------*/

void main( int argc, char **argv)
{
   extern char *optarg;
   extern int   optind;
   char *group = NULL;
   char file_old[FILENAME_MAX], file_new[FILENAME_MAX];
   int c;

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

   while ((c = getopt(argc, argv, "x:")) !=  EOF)
      switch(c) {

      case 'x':
         debuglevel = atoi( optarg );
         break;

      case '?':
         usage();
         exit(1);
         break;

      default:
         printmsg(0, "genhist - invalid option -%c", c);
         usage();
         exit(2);
         break;
   }

/*--------------------------------------------------------------------*/
/*                             Initialize                             */
/*--------------------------------------------------------------------*/

   tzset();                      /* Set up time zone information  */

   if (!configure( B_NEWS ))
      exit(1);   /* system configuration failed */

   openlog( NULL );

   PushDir( E_newsdir );
   atexit( PopDir );

   get_active();

   mkfilename(file_old, E_newsdir, "oldhist.dir");
   mkfilename(file_new, E_newsdir, "history.dir");
   unlink(file_old);
   rename(file_new, file_old);
   mkfilename(file_old, E_newsdir, "oldhist.pag");
   mkfilename(file_new, E_newsdir, "history.pag");
   unlink(file_old);
   rename(file_new, file_old);

   history = open_history("history");

   IndexAll();

   close_history(history);

   put_active();
   printmsg(1,"%s: Processed %ld total articles in %ld files (%ld bytes).",
                  argv[0], total_articles, total_files, total_bytes );
   exit(0);

} /* main */

/*--------------------------------------------------------------------*/
/*    I n d e x A l l                                                 */
/*--------------------------------------------------------------------*/

static void IndexAll( void )
{
   struct grp *cur_grp = group_list;

   while ( cur_grp != NULL )
   {
      IndexOneGroup( cur_grp);
      cur_grp = cur_grp->grp_next;
   }
} /* IndexAll */

/*--------------------------------------------------------------------*/
/*    I n d e x O n e G r o u p                                       */
/*--------------------------------------------------------------------*/

static void IndexOneGroup( struct grp *cur_grp )
{
   char groupdir[FILENAME_MAX];
   char archdir[FILENAME_MAX];

   printmsg(3,"Processing news group %s", cur_grp->grp_name );

/*--------------------------------------------------------------------*/
/*                     Set up the directory names                     */
/*--------------------------------------------------------------------*/

   ImportNewsGroup( groupdir, cur_grp->grp_name, 0 );
   mkfilename( archdir, E_archivedir, &groupdir[ strlen( E_newsdir) + 1] );

/*--------------------------------------------------------------------*/
/*            Process the directory                                   */
/*--------------------------------------------------------------------*/

   IndexDirectory( cur_grp, groupdir );

} /* IndexOneGroup */

/*--------------------------------------------------------------------*/
/*    G e t H i s t o r y D a t a                                     */
/*--------------------------------------------------------------------*/

static void GetHistoryData(char *group, struct direct *dp,
                           char *messageID, char *histentry)
{
  FILE *article;
  char line[BUFSIZ], *ptr, *item;
  int line_len, first, b_xref = 0, b_msgid = 0;

  article = FOPEN(dp->d_name, "r", TEXT_MODE);
  if ( article == NULL )
  {
    printerr( dp->d_name );
    panic();
  }

  sprintf(histentry, "%ld %ld ", dp->d_modified, dp->d_size);

  while ( !b_xref || !b_msgid )
  {
    if ( fgets(line, sizeof(line), article) == NULL )
      break;

    if ( (line_len = strlen(line)) <= 1 )
      break;

    if (line[line_len - 1] == '\n')
      line[(line_len--) - 1] = '\0';

    if (line[line_len - 1] == '\r')
      line[(line_len--) - 1] = '\0';

    ptr = "Message-ID:";
    if (equalni(line, ptr, strlen(ptr)))
    {
      ptr = line + strlen(ptr) + 1;
      while (isspace(*ptr))
        ptr++;
      strcpy(messageID, ptr);
      b_msgid++;
      continue;
    }

    ptr = "Xref:";
    if (equalni(line, ptr, strlen(ptr)))
    {
      ptr = line + strlen(ptr) + 1;
      while (isspace(*ptr))
        ptr++;

      strtok(ptr, " "); /* strip off system name */
      first = 1;

      while ((item = strtok(NULL, " ")) != NULL)
      {
        if (!first)
          strcat(histentry, ",");
        first = 0;
        strcat(histentry, item);
      }

      b_xref++;
      continue;
    }
  }

  fclose(article);

  if ( !b_xref )
    sprintf(histentry, "%ld %ld %s:%s",
            dp->d_modified, dp->d_size, group, dp->d_name);
}

/*--------------------------------------------------------------------*/
/*    I n d e x D i r e c t o r y                                     */
/*--------------------------------------------------------------------*/

static void IndexDirectory( struct grp *cur_grp,
                            const char *directory )
{
   boolean not_built = TRUE;  /* Did not insure archive directory
                                 exists                           */

   long number;

   long articles = 0;
   long files = 0;
   long bytes = 0;

   DIR *dirp;
   struct direct *dp;

   char messageID[BUFSIZ], histentry[BUFSIZ];

/*--------------------------------------------------------------------*/
/*                Open up the directory for processing                */
/*--------------------------------------------------------------------*/

   if ((dirp = opendirx(directory,"*.*")) == nil(DIR))
   {
      printmsg(3, "IndexDirectory: couldn't opendir() %s", directory);
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
/*                      Add this file to history?                     */
/*--------------------------------------------------------------------*/

      if ( numeric( dp->d_name ))/* Article format name?             */
      {                          /* Yes --> Examine it closer        */
      number = atol(dp->d_name);

        printmsg(6,"Processing file %s from %s",
                 dp->d_name, dater( dp->d_modified, NULL));

        GetHistoryData(cur_grp->grp_name, dp, messageID, histentry);

        if ( add_histentry(history, messageID, histentry) )
          articles++;
      if (number < cur_grp->grp_low) /* correct 'active' file too */
        cur_grp->grp_low = number;
      else if (number >= cur_grp->grp_high)
        cur_grp->grp_high = number + 1;

        files++;
        bytes += dp->d_size;
      }

   } /* while */

/*--------------------------------------------------------------------*/
/*           Close up the directory and report what we did            */
/*--------------------------------------------------------------------*/

   closedir(dirp);

   if ( files )
      printmsg(2,"%s: %ld articles in %ld files (%ld bytes)",
                  cur_grp->grp_name, articles, files, bytes);

   total_articles += articles;
   total_files += files;
   total_bytes += bytes;

} /* IndexDirectory */

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
   printf( "Usage:   genhist\t[-x debuglevel]\n");
   exit(1);
} /* usage */
