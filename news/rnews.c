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
 *       $Id: rnews.c 1.62 1995/03/11 22:30:11 ahd Exp $
 *
 *       $Log: rnews.c $
 *       Revision 1.62  1995/03/11 22:30:11  ahd
 *       Use macro for file delete to allow special OS/2 processing
 *
 *       Revision 1.61  1995/03/11 01:59:57  ahd
 *       Delete redundant copy message
 *
 *       Revision 1.60  1995/03/07 23:36:50  ahd
 *       Add options to newsrun to avoid aborts from bogus args under OS/2
 *
 *       Revision 1.59  1995/02/21 02:47:44  ahd
 *       The compiler warnings war never ends!
 *
 *       Revision 1.58  1995/02/20 17:28:43  ahd
 *       in-memory file support, 16 bit compiler clean up
 *
 *       Revision 1.57  1995/02/15 02:03:39  ahd
 *       Insure snews/nns support generates SYS file if needed for shadow
 *       systems.
 *
 *       Revision 1.56  1995/02/15 01:56:18  ahd
 *       Allow concurrent exploiting of NNS. SNEWS, and SYS file support
 *
 *       Revision 1.55  1995/02/14 04:38:42  ahd
 *       Correct problems with directory processing under NT
 *
 *       Revision 1.54  1995/02/12 23:37:04  ahd
 *       compiler cleanup, NNS C/news support, optimize dir processing
 *
 *       Revision 1.53  1995/02/05 00:36:38  ahd
 *
 */

#include "uupcmoah.h"

static const char rcsid[] =
         "$Id: rnews.c 1.62 1995/03/11 22:30:11 ahd Exp $";

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
/*       Queue uncompressed (batched or unbatched) news for the       */
/*       newsrun program.                                             */
/*--------------------------------------------------------------------*/

static void
queueNews( const char *inputName )
{

   char commandOptions[FILENAME_MAX];
   int status;
   const char *command = bflag[ F_NEWSRUN ] ? "newsrun" : "uux";

/*--------------------------------------------------------------------*/
/*       Amazing but true, the IBM C/Set++ compiler may pass an       */
/*       extra (invalid) argument when using the spawn() system       */
/*       call if no arguments are passed.  Our workaround is to       */
/*       always pass an argument (debug level) to newsrun.            */
/*--------------------------------------------------------------------*/

   if ( bflag[ F_NEWSRUN ] )
      sprintf( commandOptions, "-x %d", debuglevel );
   else
      sprintf(commandOptions, "-anews -p -g%c -n -x %d -C %s!newsrun -x %d",
              E_newsGrade,
              debuglevel,
              E_nodename,
              debuglevel );

/*--------------------------------------------------------------------*/
/*       We play a trick with the input file -- if it's NULL when     */
/*       passed to us, we leave it that way and let the default       */
/*       standard input passed to us be used.                         */
/*--------------------------------------------------------------------*/

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
/*       s a v e N e w s                                              */
/*                                                                    */
/*       Save an entire news for processing by an third-party         */
/*       program                                                      */
/*--------------------------------------------------------------------*/

static void saveNews( FILE *inputStream, const char *extension )
{
   char workName[FILENAME_MAX];
   FILE *outputStream;
   size_t length;

#if defined(FAMILYAPI) || defined(BIT32ENV)
   char buf[BUFSIZ * 10];
#else
   char buf[BUFSIZ];
#endif

/*--------------------------------------------------------------------*/
/*                Create new file name and open it up                 */
/*--------------------------------------------------------------------*/

   sprintf( workName,
            "%s/UUPC%s.%s",
            E_newsdir,
            jobNumber( getSeq(), 4, KWTrue ),
            extension );

   outputStream = FOPEN( workName, "w", IMAGE_MODE );

   if ( outputStream == NULL )
   {
      printerr( workName );
      panic();
   }

/*--------------------------------------------------------------------*/
/*                      Perform the actual copy                       */
/*--------------------------------------------------------------------*/

   while ((length = fread( buf, 1, sizeof buf, inputStream)) != 0)
   {
      if (fwrite( buf, 1, length, outputStream ) != length)
      {
         printerr(workName);
         panic();
      } /* if */

   } /* while */

   fclose( outputStream );

   rewind( inputStream );

} /* saveNews */

/*--------------------------------------------------------------------*/
/*    C o m p r e s s e d                                             */
/*                                                                    */
/*    Decompress news                                                 */
/*--------------------------------------------------------------------*/

static char *Compressed( FILE *in_stream ,
                         const char *unpacker ,
                         const char *suffix  )
{

   FILE *workStream;

   char zfile[FILENAME_MAX];
   char unzfile[FILENAME_MAX];

#if defined(FAMILYAPI) || defined(BIT32ENV)
   char buf[BUFSIZ * 10];
#else
   char buf[BUFSIZ];
#endif

   long cfile_size = 0L;
   size_t length;
   int status = 0;
   KWBoolean needtemp = KWTrue;

/*--------------------------------------------------------------------*/
/*        Copy the compressed file to the "holding" directory         */
/*--------------------------------------------------------------------*/

   while( needtemp )
   {
      if ( bflag[F_NNS] )
         sprintf( zfile,
                  "%s/UUPC%s.%s",
                  E_newsdir,
                  jobNumber( getSeq(), 4, KWTrue ),
                  suffix );
      else
         mktempname( zfile , suffix ); /* Generate "compressed" file
                                          name                        */
      strcpy( unzfile, zfile );
      *strrchr( unzfile, '.' ) = '\0'; /* No extension on uncomp file */

      if ( access( unzfile, 0 ))  /* Does the host file exist?        */
         needtemp = KWFalse;      /* No, we have a good pair          */
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
      panic();
   }

/*--------------------------------------------------------------------*/
/*                 Main loop to copy compressed file                  */
/*--------------------------------------------------------------------*/

   while ((length = fread(buf, sizeof(char), sizeof buf, in_stream)) != 0)
   {
      if (fwrite(buf, sizeof(char), length, workStream) != length)
      {
         fclose( workStream );
         printerr( zfile );

         if ( REMOVE( zfile ) )  /* Kill the compressed input file   */
            printerr( zfile );

         panic();
      }

      cfile_size += (long) length;

   } /* while */

   fclose(workStream);

/*--------------------------------------------------------------------*/
/*       Due to a "feature" in the compressed batch, there may be     */
/*       _no_ articles batched (an article destined for               */
/*       transmittal is cancelled before being sent).                 */
/*                                                                    */
/*       If the file is empty, delete it gracefully                   */
/*--------------------------------------------------------------------*/

   if (cfile_size == 3)
   {
      if ( REMOVE(zfile) )
         printerr( zfile );
      printmsg(1, "Compressed: %s empty, deleted",
                   zfile);
      return NULL;
   }
   else
      printmsg(2, "Compressed: Copy to %s complete, %ld characters",
                  zfile,
                  cfile_size);

/*--------------------------------------------------------------------*/
/*          Uncompress the article and feed it back to rnews          */
/*--------------------------------------------------------------------*/

   if ( E_uncompress == NULL )
      sprintf(buf, "%s -d %s", unpacker, zfile);
   else
      sprintf(buf, E_uncompress, zfile, unzfile );

   printmsg(4, "Executing command: %s", buf );
   status = executeCommand( buf, NULL, NULL, KWTrue, KWFalse);

   if ( (! access( zfile, 0 )) && REMOVE( zfile ) )
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
/*                  Make the name nice for NNS mode                   */
/*--------------------------------------------------------------------*/

   if ( bflag[F_NNS] )
   {
      char finalName[FILENAME_MAX];
      strcpy( finalName, unzfile );
      strcat( finalName, ".NNS" );

      if ( rename( unzfile, finalName ))
      {
         printerr( finalName );
         panic();
      }

      return newstr( finalName );
   }
   else
      return newstr( unzfile );     /* Just use original temp name   */

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
   KWBoolean useSYSFile  = KWTrue;
   int c;

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
/*       Determine if we should queue work for SYS file oriented      */
/*       processing.  This processing is unconditional if we are      */
/*       not saving the data for a third-party program, otherwise     */
/*       we queue for SYS file processing only if the file exists.    */
/*--------------------------------------------------------------------*/

   if ( bflag[F_SNEWS] || bflag[F_NNS] )
   {
      char workName[FILENAME_MAX];

      sprintf( workName, "%s\\SYS", E_confdir );

      if ( access( workName, 0 ) && (getenv( "UUPCSHADOWS" ) == NULL ))
      {
         if ( debuglevel > 2 )
            printerr( workName );

         useSYSFile = KWFalse;
      }
      else
         useSYSFile = KWTrue;

   } /* if ( bflag[F_SNEWS] || bflag[F_NNS] ) */

/*--------------------------------------------------------------------*/
/*              SNEWS gets our raw input. whatever it is              */
/*--------------------------------------------------------------------*/

    if ( bflag[F_SNEWS] )
       saveNews( input , "ART" );

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
/*    Since this is a bit spread out, the various clauses of the      */
/*    if statement will be marked with boxed 1, 2, or 3 (and a        */
/*    brief explanation.                                              */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Compressed files need to be read in BINARY mode so that         */
/*    "magic" characters (like ^Z) don't upset it.  Batched files     */
/*    need to be read in BINARY mode so that the character count is   */
/*    correct.  The other case doesn't matter.                        */
/*--------------------------------------------------------------------*/

   c = getc(input);
   ungetc(c, input);

   if ((c != '#' && c != MAGIC_FIRST)  )
   {

      /***********************************************/
      /* 1  single (unbatched, uncompressed) article */
      /***********************************************/

      fseek(input, 0L, SEEK_SET);

      if ( bflag[F_NNS] )
         saveNews( input, "NNS" );

      if ( useSYSFile )
         queueNews( inputName );

   }
   else {
                              /*           ....+....1..   */
      unsigned char buf[12];  /* Length of #! ?unbatch\n  */
      char *unpacker = NULL, *suffix = NULL;

      ungetc(c, input);

      size_t bytes = fread((char *) buf, 1, sizeof buf, input);

      if (bytes == 12 && memcmp(buf, "#! ", 3) == 0
                      && memcmp(buf + 4, "unbatch", 7) == 0 )
      {
        /* ignore headers like "#! cunbatch" where the 'c' can  *
         * also be one of "fgz" for frozen or [g]zipped batches */

        bytes = fread((char *) buf, 2, 1, input);     /* Get magic bytes */
        fseek(input, (long) sizeof buf, SEEK_SET);    /* Back to magic B */

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

         char *tempName = Compressed( input, unpacker, suffix);

         if ( useSYSFile && ( tempName != NULL ))
            queueNews( tempName );

         if ( (! bflag[F_NNS] ) && REMOVE( tempName ) )
            printerr( tempName );

      }
      else {

         /***********************************************/
         /* 3  Uncompressed batch                       */
         /***********************************************/

         if ( bflag[F_NNS] )
            saveNews( input, "NNS" );

         if ( useSYSFile )
            queueNews( inputName );

      } /* else */

   } /* else */

/*--------------------------------------------------------------------*/
/*                     Clean up and exit gracefully                   */
/*--------------------------------------------------------------------*/

   if ( deleteInput && REMOVE( inputName ))
      printerr( inputName );

   return 0;

} /* main */
