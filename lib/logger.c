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
 *    $Header: E:\SRC\UUPC\LIB\RCS\logger.c 1.3 1992/11/22 20:58:55 ahd Exp ahd $
 *
 *    Revision history:
 *    $Log: logger.c $
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
#include <ssleep.h>

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

      E_tempdir = E_spooldir;       /* Create log file in spool dir
                                       to allow for larger files
                                       and/or system crashes         */
      tempname = newstr( mktempname(fname, "LOG"));
                                    /* Get the file name             */
      E_tempdir = savedir;          /* Restore true temp dir         */
   } /* if */
   else
      tempname = logname;           /* Log directly to true log file */

   full_log_file_name = tempname;   /* Tell printmsg() what our log
                                       file name is                  */

/*--------------------------------------------------------------------*/
/*                    Open the temporary log file                     */
/*--------------------------------------------------------------------*/

   logfile = FOPEN( tempname , "a", TEXT );
                              /* We append in case we are not in
                                 multitask mode and we do not want
                                 to clobber the real log!            */

   if ( logfile == NULL )
   {
      printerr( tempname );
      panic();
   }

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
               "-----------\n%s %s: %s %s (%s %s)\n",
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

   output = FOPEN( logname ,"a", TEXT);

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

   input = FOPEN( tempname, "r", TEXT );

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
