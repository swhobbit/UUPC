/* genhist.c
 *
 * (Re-)Create history database from scratch.
 * Based on parts of the old expire program.
 *
 * Author:  Kai Uwe Rommel <rommel@ars.muc.de>
 * Created: Sun Aug 15 1993
 */

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1996 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/* $Log: genhist.c $
 * Revision 1.19  1996/01/01 21:08:50  ahd
 * Annual Copyright Update
 *
 * Revision 1.18  1995/12/12 13:48:54  ahd
 * Use binary tree for news group active file
 * Use large buffers in news programs to avoid overflow of hist db recs
 * Use true recursive function to walk entire active file
 *
 * Revision 1.17  1995/12/03 13:51:44  ahd
 * Additional debugging cleanup
 *
 * Revision 1.16  1995/08/27 23:33:15  ahd
 * Load and use ACTIVE file as tree structure
 *
 * Revision 1.15  1995/03/11 22:29:24  ahd
 * Use macro for file delete to allow special OS/2 processing
 *
 * Revision 1.14  1995/01/30 04:08:36  ahd
 * Additional compiler warning fixes
 *
 * Revision 1.13  1995/01/29 14:03:29  ahd
 * Clean up IBM C/Set compiler warnings
 *
/* Revision 1.12  1995/01/15 19:48:35  ahd
/* Allow active file to be optional
/* Delete fullbatch global option
/* Add "local" and "batch" flags to SYS structure for news
/*
/* Revision 1.11  1995/01/07 16:21:14  ahd
/* Change KWBoolean to KWBoolean to avoid VC++ 2.0 conflict
/*
/* Revision 1.10  1995/01/02 05:03:27  ahd
/* Pass 2 of integrating SYS file support from Mike McLagan
/*
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

#include "uupcmoah.h"
#include <direct.h>

RCSID("$Id: genhist.c 1.19 1996/01/01 21:08:50 ahd v1-12r $");

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
#include "hdbm.h"

currentfile();

/*--------------------------------------------------------------------*/
/*                        Internal prototypes                         */
/*--------------------------------------------------------------------*/

static void IndexAll( void );
static void IndexDirectory( const char *group, const char *directory );

static KWBoolean numeric( char *start);

static void usage( void );

long total_articles = 0;
long total_files = 0;
long total_bytes = 0;

void *history;

void
backupHistory( void )
{

   char file_old[FILENAME_MAX];
   char file_new[FILENAME_MAX];
   mkfilename(file_old, E_newsdir, "oldhist.dir");
   mkfilename(file_new, E_newsdir, "history.dir");
   REMOVE(file_old);
   rename(file_new, file_old);

   mkfilename(file_old, E_newsdir, "oldhist.pag");
   mkfilename(file_new, E_newsdir, "history.pag");
   REMOVE(file_old);
   rename(file_new, file_old);
}

/*--------------------------------------------------------------------*/
/*    m a i n                                                         */
/*                                                                    */
/*    Main program                                                    */
/*--------------------------------------------------------------------*/

main( int argc, char **argv)
{
   extern char *optarg;
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
      switch(c)
      {

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

   if (!configure( B_NEWS ))
      exit(1);   /* system configuration failed */

   openlog( NULL );

   loadActive( KWTrue );

   backupHistory();


   PushDir( E_newsdir );            /* Save drive, directory name,
                                       creates directoryt if needed  */

   history = open_history("history");

   IndexAll();

   PopDir();

   close_history(history);

   writeActive();

   printmsg(1,"%s: Processed %ld unique articles in %ld files (%ld bytes).",
                  argv[0], total_articles, total_files, total_bytes );

   exit(0);
   return 0;

} /* main */

/*--------------------------------------------------------------------*/
/*    I n d e x O n e G r o u p                                       */
/*--------------------------------------------------------------------*/

void IndexOneGroup( const char *groupName, void *dummy )
{
   char groupdir[FILENAME_MAX];

   printmsg(4,"IndexOneGroup: %s", groupName );

/*--------------------------------------------------------------------*/
/*                     Set up the directory names                     */
/*--------------------------------------------------------------------*/

   ImportNewsGroup( groupdir, groupName, 0 );

/*--------------------------------------------------------------------*/
/*            Process the directory                                   */
/*--------------------------------------------------------------------*/

   IndexDirectory( groupName , groupdir );

} /* IndexOneGroup */

/*--------------------------------------------------------------------*/
/*    I n d e x A l l                                                 */
/*--------------------------------------------------------------------*/

static void IndexAll( void )
{

   startActiveWalk( IndexOneGroup, NULL );

} /* IndexAll */

/*--------------------------------------------------------------------*/
/*    G e t H i s t o r y D a t a                                     */
/*--------------------------------------------------------------------*/

static void GetHistoryData(const char *group,
                           struct direct *dp,
                           char *messageID,
                           char *histentry)
{

   KWBoolean bXref = KWFalse;
   KWBoolean bMsgid = KWFalse;
   FILE *article = FOPEN(dp->d_name, "r", TEXT_MODE);

  if ( article == NULL )
  {
    printerr( dp->d_name );
    panic();
  }

  sprintf(histentry, "%ld %ld ", dp->d_modified, dp->d_size);

  while ( !bXref || !bMsgid )
  {
    size_t line_len;
    static const char messageIDLiteral[] = "Message-ID:";
    static const char xrefLiteral[] = "Xref:";
    char line[(DBM_BUFSIZ / 4) * 3];

    if ( fgets(line, sizeof(line), article) == NULL )
      break;

    if ( (line_len = strlen(line)) <= 1 )
      break;

    if (line[line_len - 1] == '\n')
      line[(line_len--) - 1] = '\0';

    if (line[line_len - 1] == '\r')
      line[(line_len--) - 1] = '\0';

    if (equalni(line, messageIDLiteral, sizeof messageIDLiteral - 1))
    {
      char *ptr = line + sizeof messageIDLiteral;

      while (isspace(*ptr))
        ptr++;

      strncpy(messageID, ptr, FILENAME_MAX);
      messageID[ FILENAME_MAX - 1] = '\0';

      bMsgid = KWTrue;
      continue;
    }

    if (equalni(line, xrefLiteral, sizeof xrefLiteral - 1 ))
    {
      char *ptr = line + sizeof xrefLiteral;

      KWBoolean first = KWTrue;

      while (isspace(*ptr))
        ptr++;

      strtok(ptr, WHITESPACE); /* strip off system name */

      while ((ptr = strtok(NULL, WHITESPACE )) != NULL)
      {
        if (!first)
          strcat(histentry, ",");

        first = KWFalse;
        strcat(histentry, ptr);
      }

      bXref = KWTrue;
      continue;

    }

  } /* while ( !bXref || !bMsgid ) */

  fclose(article);

  if ( !bXref )
    sprintf(histentry, "%ld %ld %s:%s",
            dp->d_modified, dp->d_size, group, dp->d_name);

} /* GetHistoryData */

/*--------------------------------------------------------------------*/
/*    I n d e x D i r e c t o r y                                     */
/*--------------------------------------------------------------------*/

static void IndexDirectory( const char *groupName,
                            const char *directory )
{
   long number;

   long articles = 0;
   long files = 0;
   long bytes = 0;

   DIR *dirp;
   struct direct *dp;

/*--------------------------------------------------------------------*/
/*                Open up the directory for processing                */
/*--------------------------------------------------------------------*/

   if ((dirp = opendirx(directory,"*.*")) == nil(DIR))
   {
      printmsg(5, "IndexDirectory: couldn't opendir() %s", directory);
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
         char messageID[FILENAME_MAX];
         char histentry[DBM_BUFSIZ];

         number = atol(dp->d_name);

         printmsg(6,"Processing file %s from %s",
                 dp->d_name, dater( dp->d_modified, NULL));

         GetHistoryData(groupName, dp, messageID, histentry);

         if ( add_histentry(history, messageID, histentry) )
            articles++;

/*--------------------------------------------------------------------*/
/*                    Update ACTIVE file as well.                     */
/*--------------------------------------------------------------------*/

         if (number < getArticleOldest( groupName ))
            setArticleOldest( groupName, number );

         if (number > getArticleNewest( groupName ))
            setArticleNewest( groupName, number );

        files++;
        bytes += dp->d_size;

      } /* if ( numeric( dp->d_name )) */

   } /* while */

/*--------------------------------------------------------------------*/
/*           Close up the directory and report what we did            */
/*--------------------------------------------------------------------*/

   closedir(dirp);

   printmsg(files ? 2 : 3,"%s: %ld unique and %ld cross-posted articles "
                  "in %ld files (%ld bytes)",
                  groupName,
                  articles,
                  files - articles,
                  files,
                  bytes);

   if ((bflag[F_PURGE] && ( files == 0 )) )
   {
      CHDIR( E_newsdir );           /* Can't delete directory if CWD  */
      if ( rmdir( directory ))
         printerr( directory );
      else {
         printmsg(1,"Deleted directory for empty group %s (directory %s)",
                     groupName,
                     directory );
      }
   }

   total_articles += articles;
   total_files += files;
   total_bytes += bytes;

} /* IndexDirectory */

/*--------------------------------------------------------------------*/
/*    n u m e r i c                                                   */
/*                                                                    */
/*    Examines string, returns true if numeric with period            */
/*--------------------------------------------------------------------*/

static KWBoolean numeric( char *start)
{
   char *number = start;

   while (*number != '\0')
   {
      if (!isdigit(*number) && (*number != '.'))
         return KWFalse;

      number++;
   }

   return KWTrue;

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
