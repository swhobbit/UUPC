/*--------------------------------------------------------------------*/
/*    r n e w s . c                                                   */
/*                                                                    */
/*    Receive incoming news into the news directory.                  */
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
 *       $Id: rnews.c 1.54 1995/02/12 23:37:04 ahd Exp $
 *
 *       $Log: rnews.c $
 *       Revision 1.54  1995/02/12 23:37:04  ahd
 *       compiler cleanup, NNS C/news support, optimize dir processing
 *
 *       Revision 1.53  1995/02/05 00:36:38  ahd
 *
 */

#include "uupcmoah.h"

static const char rcsid[] =
         "$Id: rnews.c 1.54 1995/02/12 23:37:04 ahd Exp $";

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include <ctype.h>
#include <fcntl.h>
#include <io.h>
#include <sys/stat.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "getopt.h"
#include "logger.h"
#include "timestmp.h"
#include "getseq.h"

#include "execute.h"

/*--------------------------------------------------------------------*/
/*                           Global defines                           */
/*--------------------------------------------------------------------*/

#define MAGIC_FIRST       0x1F   /* Magic numbers for compressed batches */
#define MAGIC_COMPRESS    0x9D
#define MAGIC_FREEZE      0x9F
#define MAGIC_GZIP        0x8B

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

currentfile();

/*--------------------------------------------------------------------*/
/*       q u e u e N e w s                                            */
/*                                                                    */
/*       Queue news for its next step of processing.  NNS or SNEWS    */
/*       batches just get dumped in the news directory, and/or        */
/*       data is queued for by processing by newsrun.                 */
/*--------------------------------------------------------------------*/

static void
queueNews( FILE *inputStream, const char *inputName )
{

   char commandOptions[BUFSIZ];
   int status;
   const char *command = bflag[ F_NEWSRUN ] ? "newsrun" : "uux";

/*--------------------------------------------------------------------*/
/*                 Handle snews/NNS processing first                  */
/*--------------------------------------------------------------------*/

   if ( bflag[ F_SNEWS ] || bflag[ F_NNS ] )
   {
      char workName[FILENAME_MAX];
      char buffer[BUFSIZ];
      FILE *outputStream;
      size_t length;

/*--------------------------------------------------------------------*/
/*                Create new file name and open it up                 */
/*--------------------------------------------------------------------*/

      sprintf( workName,
               "%s/UUPC%s.ART",
               E_newsdir,
               jobNumber( getSeq(), 4, KWTrue ) );

      outputStream = FOPEN( workName, "w", IMAGE_MODE );

      if ( outputStream == NULL )
      {
         printerr( workName );
         panic();
      }

/*--------------------------------------------------------------------*/
/*                      Perform the actual copy                       */
/*--------------------------------------------------------------------*/

      while ((length = fread( buffer, 1, sizeof buffer, inputStream)) != 0)
      {
         if (fwrite( buffer, 1, length, outputStream ) != length)
         {
            printerr(workName);
            panic();
         } /* if */

      } /* while */

      fclose( outputStream );

      sprintf( workName, "%s\\SYS", E_confdir );

      if ( access( workName, 0 ))
      {
         if ( debuglevel > 2 )
            printerr( workName );

         return;
      }

      rewind( inputStream );

   } /* if ( bflag[ F_SNEWS ] || bflag[ F_NNS ] ) */

/*--------------------------------------------------------------------*/
/*       Perform SYS file based processing.                           */
/*                                                                    */
/*       We play a trick with the input file -- if it's NULL when     */
/*       passed to us, we leave it that way and let the default       */
/*       standard input passed to us be used.                         */
/*--------------------------------------------------------------------*/

   if ( bflag[ F_NEWSRUN ] )
      *commandOptions = '\0';
   else
      sprintf(commandOptions, "-anews -p -g%c -n -x %d -C %s!newsrun",
              E_newsGrade,
              debuglevel,
              E_nodename );

   status = execute( command,
                     commandOptions,
                     inputName,
                     NULL,
                     KWTrue,
                     KWFalse);

   if ( status )
   {
      printmsg(0, "%s command failed with status %d",
                  command,
                  status );
      panic();
   }

} /* queueNews */

/*--------------------------------------------------------------------*/
/*    C o m p r e s s e d                                             */
/*                                                                    */
/*    Decompress news                                                 */
/*--------------------------------------------------------------------*/

static int Compressed( FILE *in_stream ,
                       const char *unpacker ,
                       const char *suffix )
{

   FILE *workStream;

   char zfile[FILENAME_MAX];
   char unzfile[FILENAME_MAX];
   char buf[BUFSIZ];

   long cfile_size = 0L;
   size_t chars_read, i;
   int status = 0;
   KWBoolean needtemp = KWTrue;

/*--------------------------------------------------------------------*/
/*        Copy the compressed file to the "holding" directory         */
/*--------------------------------------------------------------------*/

   while( needtemp )
   {
      mktempname( zfile , suffix );    /* Generate "compressed" file
                                          name                        */
      strcpy( unzfile, zfile );
      unzfile[ strlen(unzfile)-2 ] = '\0';

      if ( access( unzfile, 0 ))  /* Does the host file exist?        */
         needtemp = KWFalse;       /* No, we have a good pair          */
      else
         printmsg(0, "Had compressed name %s, found %s already exists!",
                  zfile, unzfile );

   } /* while */

/*--------------------------------------------------------------------*/
/*                 Open the file, with error recovery                 */
/*--------------------------------------------------------------------*/

   if ((workStream = FOPEN(zfile, "w", IMAGE_MODE)) == nil(FILE))
   {
      printmsg(0, "Compressed: Can't open %s (%d)", zfile, errno);
      printerr(zfile);
      return 2;
   }

   printmsg(2, "Compressed: Copy to %s for later processing", zfile);

/*--------------------------------------------------------------------*/
/*                 Main loop to copy compressed file                  */
/*--------------------------------------------------------------------*/

   while ((chars_read = fread(buf, sizeof(char), BUFSIZ, in_stream)) != 0)
   {
      char *t_buf = buf;
      i = fwrite(t_buf, sizeof(char), chars_read, workStream);

      if (i != chars_read)
      {
         fclose( workStream );
         printerr( zfile );

         if ( unlink( zfile ) )  /* Kill the compressed input file   */
            printerr( zfile );

         panic();
      }

      cfile_size += (long)chars_read;

   } /* while */

   fclose(workStream);

/*--------------------------------------------------------------------*/
/*             If the file is empty, delete it gracefully             */
/*--------------------------------------------------------------------*/

   if (cfile_size == 3)
   {
      if ( unlink(zfile) )
         printerr( zfile );
      printmsg(1, "Compressed: %s empty, deleted",
                   zfile);
      return status;
   }
   else
      printmsg(2, "Compressed: Copy to %s complete, %ld characters",
               zfile, cfile_size);

/*--------------------------------------------------------------------*/
/*          Uncompress the article and feed it back to rnews          */
/*--------------------------------------------------------------------*/

   if ( E_uncompress == NULL )
      sprintf(buf, "%s -d %s", unpacker, zfile);
   else
      sprintf(buf, E_uncompress, zfile, unzfile );

   printmsg(4, "Executing command: %s", buf );
   status = executeCommand( buf, NULL, NULL, KWTrue, KWFalse);

   if ( (! access( zfile, 0 )) && unlink( zfile ) )
      printerr( zfile );

   if (status != 0)
   {
      if (status == -1)
      {
          printmsg( 0, "Compress: spawn failed completely" );
          printerr( unpacker );
      }
      else
          printmsg(0, "%s command failed (exit code %d)",
                        unpacker,
                        status);

      panic();

   } /* if status != 0 */

/*--------------------------------------------------------------------*/
/*            Now process the file as normal batched news             */
/*--------------------------------------------------------------------*/

                              /* Create uncompressed output file name */

   queueNews( workStream, unzfile );

/*--------------------------------------------------------------------*/
/*                   Clean up and return to caller                    */
/*--------------------------------------------------------------------*/

   if ( unlink( unzfile ) )
      printerr( unzfile );

   return status;

} /* Compressed */

/*--------------------------------------------------------------------*/
/*    m a i n                                                         */
/*                                                                    */
/*    Main program                                                    */
/*--------------------------------------------------------------------*/

main( int argc, char **argv)
{

   FILE *input;
   char *inputName = NULL;
   KWBoolean deleteInput = KWFalse;
   int c;
   int status;

#if defined(__CORE__)
   copywrong = strdup(copyright);
   checkref(copywrong);
#endif

   setmode(0, O_BINARY);         /* don't die on control-Z, etc.      */
   input = fdopen(0, "rb");      /* Default to stdin, but             */

   logfile = stderr;             /* Prevent redirection of error      */
                                 /* messages during configuration     */

   banner( argv );

   if (!configure( B_RNEWS ))
      exit(1);    /* system configuration failed */

   openlog( NULL );           /* Begin logging to disk            */

/*--------------------------------------------------------------------*/
/*                      Parse arguments, if any.                      */
/*--------------------------------------------------------------------*/

   if (argc > 1)
   {
      int option;

       while ((option = getopt(argc, argv, "F:f:x:")) != EOF)
       {
           switch (option)
           {
               case 'F':
                  deleteInput = KWTrue;
                  /* Fall through to regular case for file input  */

               case 'f':
                  inputName = optarg;

                  input = fopen(inputName, "rb");

                  if (input == NULL)
                  {
                     printerr( inputName );
                     panic();
                  }
                  else
                     printmsg(2, "Opened %s as input file", inputName);

                  break;

               case 'x':
                  debuglevel = atoi(optarg);
                  break;

               case '?':
                  fprintf(stderr,
                          "\nUsage:\t%s [-f newsfile] [-x debug]",
                          argv[0]);
                  exit(99);
           } /* break */

       } /* while */

/*--------------------------------------------------------------------*/
/*                Abort if any options were left over                 */
/*--------------------------------------------------------------------*/

       if (optind != argc)
       {
          puts("Extra parameter(s) at end.");
          exit(98);
       }

    } /* if (argc > 1) */

/*--------------------------------------------------------------------*/
/*    A news article/batch either has a '#' character as its first    */
/*    character or it does not.                                       */
/*                                                                    */
/*    If it does not it is a single (unbatched, uncompressed)         */
/*    article.                                                        */
/*                                                                    */
/*    If the first character _is_ a '#', the first line is #!         */
/*    cunbatch\n which means that it is a compressed batch; it        */
/*    will be uncompressed and then re-fed to rnews or the first      */
/*    line can be #! rnews nnn\n (where nnn is some number) which     */
/*    means that the next nnn characters are an article and that      */
/*    more might follow.                                              */
/*                                                                    */
/*    Due to a "feature" in the compressed batch, there may be        */
/*    _no_ articles batched (an article destined for transmittal      */
/*    is cancelled before being sent).                                */
/*                                                                    */
/*    Since this is a bit spread out, the various clauses of the      */
/*    if statement will be marked with boxed 1, 2, or 3 (and a        */
/*    brief explanation.                                              */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Compressed files need to be read in BINARY mode so that         */
/*    "magic" characters (like ^Z) don't upset it.  Batched files     */
/*    need to be read in TEXT mode so that the character count is     */
/*    correct.  The other case doesn't matter.                        */
/*--------------------------------------------------------------------*/

   c = getc(input);
   ungetc(c, input);

   if ((c != '#' && c != MAGIC_FIRST) || bflag[F_SNEWS] )
   {

      /***********************************************/
      /* 1  single (unbatched, uncompressed) article */
      /***********************************************/

      queueNews( input, inputName );

   }
   else {

      unsigned char buf[BUFSIZ];
      int bytes;
      char *unpacker = NULL, *suffix = NULL;

      bytes = fread((char *) buf, 1, 12, input);

      if (bytes == 12 && memcmp(buf, "#! ", 3) == 0
                      && memcmp(buf + 4, "unbatch", 7) == 0 )
      {
        /* ignore headers like "#! cunbatch" where the 'c' can  *
         * also be one of "fgz" for frozen or [g]zipped batches */
        bytes = fread((char *) buf, 2, 1, input);
        fseek(input, 12L, SEEK_SET);
      }
      else
        fseek(input, 0L, SEEK_SET);

      if (buf[0] == MAGIC_FIRST)
        switch (buf[1])
        {
           case MAGIC_COMPRESS:
             unpacker = "compress";
             suffix = "Z";
             break;
           case MAGIC_FREEZE:
             unpacker = "freeze";
             suffix = "F";
             break;
           case MAGIC_GZIP:
             unpacker = "gzip";
             suffix = "gz";
             break;
        }

      if (unpacker != NULL)
      {

         /***********************************************/
         /*  2  Compressed batch                        */
         /***********************************************/

         status = Compressed( input, unpacker, suffix);

      }
      else {

         /***********************************************/
         /* 3  Uncompressed batch                       */
         /***********************************************/

         queueNews( input, inputName );

      } /* else */

   } /* else */

/*--------------------------------------------------------------------*/
/*               Summarize the results of our processing              */
/*--------------------------------------------------------------------*/

   if ( deleteInput && (status == 0 ))
      remove( inputName );

   exit(0);
   return 0;

} /* main */
