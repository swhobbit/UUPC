/*--------------------------------------------------------------------*/
/*    l o g g e r . c                                                 */
/*                                                                    */
/*    Logging functions for UUPC/extended                             */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1999 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: logger.c 1.32 1998/08/02 01:01:27 ahd v1-13f ahd $
 *
 *    Revision history:
 *    $Log: logger.c $
 *    Revision 1.32  1998/08/02 01:01:27  ahd
 *    Don't print spin off message unless building for debug
 *
 *    Revision 1.31  1998/07/27 01:03:54  ahd
 *    Report exact problem with failure to remove file
 *
 *    Revision 1.30  1998/05/17  21:12:13  ahd
 *    Correct RCSID
 *
 *    Revision 1.29  1998/05/11 01:20:48  ahd
 *    Don't close temp log before copying to perm log, just rewind it
 *    Allow external routines to call copylog
 *    Remember name of previously used log between calls
 *
 *    Revision 1.28  1998/04/20 02:47:57  ahd
 *    TAPI/Windows 32 BIT GUI display support
 *
 *    Revision 1.27  1998/03/01 01:24:11  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.26  1997/03/31 07:00:21  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.25  1996/01/01 20:51:27  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.24  1995/03/12 16:42:24  ahd
 *    Use direct fopen() call, don't bother with fsopen()
 *
 *    Revision 1.23  1995/03/11 22:26:25  ahd
 *    Use macro for file delete to allow special OS/2 processing
 *
 *    Revision 1.22  1995/03/11 15:49:23  ahd
 *    Clean up compiler warnings, modify dcp/dcpsys/nbstime for better msgs
 *
 *    Revision 1.21  1995/01/07 16:13:09  ahd
 *    Change boolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 */

/*--------------------------------------------------------------------*/
/*                   Standard library include files                   */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"

#include <sys/stat.h>

#include <share.h>
#include <io.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "expath.h"
#include "logger.h"
#include "timestmp.h"

/*--------------------------------------------------------------------*/
/*                      Define current file name                      */
/*--------------------------------------------------------------------*/

currentfile();
RCSID("$Id: logger.c 1.32 1998/08/02 01:01:27 ahd v1-13f ahd $");

/*--------------------------------------------------------------------*/
/*                          Local variables                           */
/*--------------------------------------------------------------------*/

static char *permanentLogName  = NULL;
static char *currentLogName = NULL;

/*--------------------------------------------------------------------*/
/*    o p e n l o g                                                   */
/*                                                                    */
/*    Begin logging to a standard file name                           */
/*--------------------------------------------------------------------*/

void openlog(const char *log)
{
   char *newLogName;
   FILE *stream = NULL;
   int saveDebuglevel = debuglevel;

/*--------------------------------------------------------------------*/
/*                             Housekeeping                           */
/*--------------------------------------------------------------------*/

   if (E_logdir == NULL)         /* We DID call configure, didn't we? */
      panic();                   /* Ooopps --> I guess not.           */

    MKDIR(E_logdir);             /* Make sure directory exists!       */

/*--------------------------------------------------------------------*/
/*       If we already had a log file, spin it off and copy it        */
/*--------------------------------------------------------------------*/

   if (permanentLogName != NULL)
      copylog();

/*--------------------------------------------------------------------*/
/*                Create the final log name for later                 */
/*--------------------------------------------------------------------*/

   if ((log != NULL) || (permanentLogName == NULL))
   {
      char fname[FILENAME_MAX];
      char *newName = (char*) ((log == NULL) ? compilen : log);
      char *period = strchr(newName, '.');

      mkfilename(fname, E_logdir, newName);

      if (period == NULL)
         strcat(fname, ".log");

      newLogName = newstr(fname);
   }
   else
      newLogName = permanentLogName;

/*--------------------------------------------------------------------*/
/*                   Create temporary log file name                   */
/*--------------------------------------------------------------------*/

   if (bflag[F_MULTITASK])
   {
      char fname[FILENAME_MAX];
      short retries = 15;

      while ((stream == NULL) && retries--)
      {
         mkdirfilename(fname, E_logdir, "log");
                                    /* Get a temp log file name       */

         denormalize(fname);
         stream = fopen(fname, "a+");

         if (stream == NULL)
            printerr(fname);

      } /* while */

      currentLogName = newstr(fname);
                                 /* Save name we log to for posterity */

   } /* if */
   else {
      currentLogName = newLogName;  /* Log directly to true log file  */
      stream  = FOPEN(currentLogName , "a",TEXT_MODE);
                              /* We append in case we are not in
                                 multitask mode and we do not want
                                 to clobber the real log!             */
   } /* else */

   if (stream == NULL)
   {
      printmsg(0,"Cannot open any log file!");
      panic();
   }

   full_log_file_name = currentLogName;
                                    /* Tell printmsg() what our log
                                       file name is                   */
   logfile  = stream;               /* And of the the stream itself   */

/*--------------------------------------------------------------------*/
/*               Request the copy function be run later               */
/*--------------------------------------------------------------------*/

   if (permanentLogName == NULL)
      atexit(copylog);

/*--------------------------------------------------------------------*/
/*    Tag the new log file with the current time and program date.    */
/*--------------------------------------------------------------------*/

   debuglevel = 0;                  /* Insure we time stamp          */

   printmsg(0,"%s: %s %s (%s %s)",
            compilen, compilep, compilev, compiled, compilet);
   debuglevel = saveDebuglevel;

   if (ferror(logfile))
   {
      printerr(currentLogName);
      panic();
   }

/*--------------------------------------------------------------------*/
/*       Save log file name for latter reference and as a flag        */
/*       that we were called                                          */
/*--------------------------------------------------------------------*/

   permanentLogName = newLogName;

} /* openlog */

/*--------------------------------------------------------------------*/
/*    c o p y l o g                                                   */
/*                                                                    */
/*    Close and copy a log opened by openlog                          */
/*--------------------------------------------------------------------*/

void copylog(void)
{
   static const char mName[] = "copylog";
   FILE *input;
   FILE *output;
   char buf[BUFSIZ];
   size_t chars_read;


   /* Handle aborts, which may cause this to be called extra */
   if (logfile == stderr)
      return;

/*--------------------------------------------------------------------*/
/*   If not multitasking, just close the file and return gracefully   */
/*--------------------------------------------------------------------*/

   if (!bflag[ F_MULTITASK ])
   {
      fclose(logfile);
      logfile = stderr;
      return;
   }

/*--------------------------------------------------------------------*/
/*            We're multitasking; copy the file gracefully            */
/*--------------------------------------------------------------------*/

   output = FOPEN(permanentLogName ,"a",TEXT_MODE);

   if (output == NULL)
   {
      printerr(permanentLogName);
      printmsg(0,"%s: Cannot merge log %s to %s",
                  mName,
                  currentLogName,
                  permanentLogName);
      fclose(logfile);
      logfile = stderr;
      return;
   }

   input = logfile;                 /* Save our logfile stream        */
   logfile = output;                /* Log directly into real file    */
   full_log_file_name = permanentLogName;
                                    /* Tell printerr we switched      */

   rewind(input);

/*--------------------------------------------------------------------*/
/*           File is open, copy temporary log to end of it            */
/*--------------------------------------------------------------------*/

   while ((chars_read = fread(buf,sizeof(char), BUFSIZ, input)) != 0)
   {

      if (fwrite(buf, sizeof(char), chars_read, logfile) != chars_read)
      {
         fclose(input);

         printerr(permanentLogName);
         clearerr(logfile);
         fclose(logfile);
         logfile  = stderr;

         return;
      }

   } /* while */

/*--------------------------------------------------------------------*/
/*                     Check for errors on input                      */
/*--------------------------------------------------------------------*/

   if (ferror(input))
   {
      printerr(currentLogName);
      clearerr(input);
   }

/*--------------------------------------------------------------------*/
/*             Close up shop and discard temporary input              */
/*--------------------------------------------------------------------*/

   fclose(input);

   if (REMOVE(currentLogName))
   {
      printmsg(0,"%s: Unable to delete temporary log file %s",
                  mName,
                  currentLogName );
      printerr(currentLogName);
   }

#ifdef UDEBUG
   printmsg(0,"%s: Spun off log %s to %s",
               mName,
               currentLogName,
               permanentLogName);
#endif

   fclose(logfile);
   logfile = stderr;

} /* copylog */
