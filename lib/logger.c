/*--------------------------------------------------------------------*/
/*    l o g g e r . c                                                 */
/*                                                                    */
/*    Logging functions for UUPC/extended                             */
/*                                                                    */
/*    Copyright (c) 1992 by Kendra Electronic Wonderworks; all        */
/*    rights reserved except those explicitly granted by the          */
/*    UUPC/extended license.                                          */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: logger.c 1.8 1993/06/06 15:04:05 ahd Exp $
 *
 *    Revision history:
 *    $Log: logger.c $
 *     Revision 1.8  1993/06/06  15:04:05  ahd
 *     Trap unable to open log file
 *
 *     Revision 1.7  1993/04/11  00:32:05  ahd
 *     Global edits for year, TEXT, etc.
 *
 *     Revision 1.6  1993/03/06  22:48:23  ahd
 *     Drop dashes between log entries
 *
 *     Revision 1.5  1993/01/23  19:08:09  ahd
 *     Correct sleep.h include
 *
 * Revision 1.4  1992/11/23  03:56:06  ahd
 * Do not use expand_path to build log file name
 * Use strpool for names
 *
 * Revision 1.3  1992/11/22  20:58:55  ahd
 * Move retry of opens to FOPEN()
 *
 * Revision 1.2  1992/11/19  02:58:22  ahd
 * drop rcsid
 *
 * Revision 1.1  1992/11/16  05:00:26  ahd
 * Initial revision
 *
 */

/*--------------------------------------------------------------------*/
/*                   Standard library include files                   */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <share.h>
#include <io.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "dater.h"
#include "expath.h"
#include "logger.h"
#include "hlib.h"
#include "timestmp.h"

/*--------------------------------------------------------------------*/
/*                      Define current file name                      */
/*--------------------------------------------------------------------*/

currentfile();

/*--------------------------------------------------------------------*/
/*                          Local variables                           */
/*--------------------------------------------------------------------*/

static char *logname  = NULL;
static char *tempname = NULL;

static void copylog( void );

/*--------------------------------------------------------------------*/
/*    o p e n l o g                                                   */
/*                                                                    */
/*    Begin logging to a standard file name                           */
/*--------------------------------------------------------------------*/

void openlog( const char *log )
{
   char fname[FILENAME_MAX];
   FILE *stream = NULL;

/*--------------------------------------------------------------------*/
/*                Create the final log name for later                 */
/*--------------------------------------------------------------------*/

   logname =  (char*) ((log == NULL) ? compilen : log);
   tempname = strchr( logname, '.');
   mkfilename( fname, E_spooldir, logname );

   if ( tempname == NULL )
      strcat( fname, ".LOG" );
   logname = newstr( fname );

/*--------------------------------------------------------------------*/
/*                   Create temporary log file name                   */
/*--------------------------------------------------------------------*/

   if ( bflag[F_MULTITASK] )
   {
      char *savedir = E_tempdir;    /* Save real tempory directory   */
      short retries = 20;

      E_tempdir = E_spooldir;       /* Create log file in spool dir
                                       to allow for larger files
                                       and/or system crashes         */
      while (( stream == NULL ) && retries-- )
      {
         mktempname(fname, "LOG");  // Get a temp log file name

         stream = _fsopen(fname, "at", SH_DENYWR);

         if ( stream == NULL )
            printerr( tempname );

      } /* while */

      E_tempdir = savedir;          // Restore true temp dir
      tempname = newstr( fname );   // Save name we log to for posterity

   } /* if */
   else {
      tempname = logname;           /* Log directly to true log file */
      stream  = FOPEN( tempname , "a",TEXT_MODE );
                              /* We append in case we are not in
                                 multitask mode and we do not want
                                 to clobber the real log!            */
   } /* else */

   if ( stream == NULL )
   {
      printmsg(0,"Cannot open any log file!");
      panic();
   }

   full_log_file_name = tempname;   /* Tell printmsg() what our log
                                       file name is                  */
   logfile  = stream;               // And of the the stream itself


/*--------------------------------------------------------------------*/
/*               Request the copy function be run later               */
/*--------------------------------------------------------------------*/

   atexit( copylog );

/*--------------------------------------------------------------------*/
/*    Tag the new log file with the current time and program date.    */
/*    We don't use printmsg() because that will not display the       */
/*    time if debugging is turned up.                                 */
/*--------------------------------------------------------------------*/

   fprintf(logfile,
               "%s %s: %s %s (%s %s)\n",
               dater( time( NULL ), NULL),
               compilen, compilep, compilev, compiled, compilet);

   if ( ferror( logfile ))
   {
      printerr( tempname );
      panic();
   }

} /* openlog */

/*--------------------------------------------------------------------*/
/*    c o p y l o g                                                   */
/*                                                                    */
/*    Close and copy a log opened by openlog                          */
/*--------------------------------------------------------------------*/

static void copylog( void )
{

   FILE *input;
   FILE *output;
   char buf[BUFSIZ];
   int chars_read, chars_written;

/*--------------------------------------------------------------------*/
/*   If not multitasking, just close the file and exit gracefully     */
/*--------------------------------------------------------------------*/

   if ( !bflag[ F_MULTITASK ] )
   {
      fclose( logfile );
      logfile = stdout;
      return;
   }

/*--------------------------------------------------------------------*/
/*            We're multitasking; copy the file gracefully            */
/*--------------------------------------------------------------------*/

   output = FOPEN( logname ,"a",TEXT_MODE);

   if ( output == NULL )
   {
      printmsg(0,"Cannot merge log %s to %s", tempname, logname );
      printerr( logname );
      fclose( logfile );
      logfile = stderr;
      return;
   }

   fclose( logfile );
   logfile = output;                /* Log directly into real file   */
   full_log_file_name = logname;    /* Tell printerr we switched     */

   input = FOPEN( tempname, "r",TEXT_MODE );

   if ( input == NULL )
   {
      printerr( tempname );
      fclose( input );
      fclose( output );
      logfile = stdout;
   }

/*--------------------------------------------------------------------*/
/*           File is open, copy temporary log to end of it            */
/*--------------------------------------------------------------------*/

   while ((chars_read = fread(buf,sizeof(char), BUFSIZ, input)) != 0)
   {
      chars_written = fwrite(buf, sizeof(char), chars_read, output );

      if (chars_written != chars_read)
      {
         printerr( logname );
         clearerr( output );
         fclose( input );
         fclose( output );
         logfile  = stdout;
         return;
      }
   } /* while */

/*--------------------------------------------------------------------*/
/*                     Check for errors on input                      */
/*--------------------------------------------------------------------*/

   if ( ferror( input ))
   {
      printerr( tempname );
      clearerr( input );
   }

/*--------------------------------------------------------------------*/
/*             Close up shop and discard temporary input              */
/*--------------------------------------------------------------------*/

   fclose( input );
   fclose( output );
   logfile  = stdout;

   unlink( tempname );

} /* copylog */
