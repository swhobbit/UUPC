/*--------------------------------------------------------------------*/
/*    Program:    fmt.c          14 August 1990                       */
/*    Author:     Andrew H. Derbyshire                                */
/*    Function:   Format lines into user specified width rows         */
/*    Arguments:  width, input file, output file.                     */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-2000 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: fmt.c 1.14 1999/01/08 02:20:52 ahd Exp $
 *
 *    Revision history:
 *    $Log: fmt.c $
 *    Revision 1.14  1999/01/08 02:20:52  ahd
 *    Convert currentfile() to RCSID()
 *
 *    Revision 1.13  1999/01/04 03:53:30  ahd
 *    Annual copyright change
 *
 *    Revision 1.12  1998/11/07 03:42:26  ahd
 *    Remove bad line terminations
 *
 *    Revision 1.11  1998/05/17 21:12:13  ahd
 *    Support -c, -s options
 *    Correct paragraph break processing
 *    Correct file names used in error messages
 *
 *    Revision 1.10  1998/03/01 01:36:26  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.9  1997/04/24 01:30:57  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.8  1996/01/01 21:17:40  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.7  1995/02/20 00:40:12  ahd
 *    Correct C compiler warnings
 *
 *    Revision 1.6  1995/01/30 04:08:36  ahd
 *    Additional compiler warning fixes
 *
 *    Revision 1.5  1994/12/09 03:42:09  ahd
 *    Include configuration to allow suppressing beep
 *
 * Revision 1.4  1994/02/19  05:01:25  ahd
 * Use standard first header
 *
 * Revision 1.3  1993/10/24  20:58:55  rhg
 * Clean up for MS C 7.0
 *
 * Revision 1.2  1993/04/11  00:33:54  ahd
 * Global edits for year, TEXT, etc.
 *
 * Revision 1.1  1992/11/15  04:29:22  ahd
 * Initial revision
 *
 * Revision 1.2  1992/04/27  00:41:11  ahd
 * Add RCS Information
 *
 */

#include "uupcmoah.h"

RCSID("$Id: fmt.c 1.14 1999/01/08 02:20:52 ahd Exp $");

#include <ctype.h>

#include "getopt.h"
#include "timestmp.h"

void static
usage( void )
{
   fprintf(stderr, "Usage:\tfmt\t[-#] infile outfile\n");
   exit(1);
}

/*--------------------------------------------------------------------*/
/*    m a i n                                                         */
/*                                                                    */
/*    main program                                                    */
/*--------------------------------------------------------------------*/

 main( int argc, char *argv[] )
 {
   int option;
   int width = 0;          /* Width of current line                  */
   int maxwidth = 72;      /* Max width of line allowed              */
   char buf[BUFSIZ];       /* Our I/O buffer                         */
   int argx = 1;           /* Current argument being processed       */
   KWBoolean punct = KWFalse;
                           /* Last character in last word was punct  */
   KWBoolean splitOnly = KWFalse;
                           /* Don't join lines together     */
   char *continueString = NULL;
   FILE *input;
   FILE *output;
   char *inputName = NULL;
   char *outputName = NULL;

/*--------------------------------------------------------------------*/
/*                        Announce our version                        */
/*--------------------------------------------------------------------*/

   banner( argv );

   while ((option = getopt(argc,
                           argv,
                           "sc:i:o:0:1:2:3:4:5:6:7:8:9:")) != EOF)
   {
      switch (option)
      {
         case '0':
         case '1':
         case '2':
         case '3':
         case '4':
         case '5':
         case '6':
         case '7':
         case '8':
         case '9':
            sprintf( buf, "%c%s", (char) option, optarg);
            maxwidth = atoi(buf);
            break;

         case 'o':
            outputName = optarg;
            break;

         case 'i':
            inputName = optarg;
            break;

         case 'c':
            continueString = strdup(optarg);
            break;

         case 's':
            splitOnly = KWTrue;
            break;

         default:
            usage();
            break;

      } /* switch (option) */

   } /* while() */

/*--------------------------------------------------------------------*/
/*               Get the file names to process, if any                */
/*--------------------------------------------------------------------*/

    if (optind != argc)
      inputName = argv[optind++];

    if (optind != argc)
      outputName = argv[optind++];

/*--------------------------------------------------------------------*/
/*                        Open the input file                         */
/*--------------------------------------------------------------------*/

    if (inputName == NULL)
    {
      input = stdin;
      inputName = "stdin";
    }
    else {

      input = fopen(inputName,"r");

      if (input == NULL)
      {
         perror(inputName);
         exit (100);
      }
    }

/*--------------------------------------------------------------------*/
/*                        Open the output file                        */
/*--------------------------------------------------------------------*/

    if (outputName == NULL)
    {
      output = stdout;
      outputName = "stdout";
    }
    else {
      output = fopen(outputName,"w");

      if (output == NULL)
      {
         perror(outputName);
         exit( 200 );
      }
    }

/*--------------------------------------------------------------------*/
/*                          Process the file                          */
/*--------------------------------------------------------------------*/

    while( fgets(buf, BUFSIZ, input) != NULL )
    {
      char *token = strtok(buf, " \t\n");

      if (token == NULL)
      {
         /* Terminate Previous line if we had one */
         if (width > 0)
            fputc('\n',output);

         /* Copy this (empty) line out as well */
         fputc('\n',output);
         width = punct = 0;
      }
      else {

         while(token != NULL)
         {

            register int toklen = (int) strlen(token);
            width = toklen + width + 1 + punct;

            if (width > max(maxwidth, toklen + 1))
            {
               if (continueString != NULL)
                  fputs(continueString, output);
               fputc('\n',output);
               width = toklen;
               punct = 0;
            }
            else {

               if (width > (toklen + 1))
               {
                  fputc(' ',output);
                  if (punct)
                     fputc(' ',output);

               }
               else
                  width = toklen;

               punct = ispunct( token[toklen - 1] ) ? 1 : 0;

            } /* else */

            fputs(token, output);
            token = strtok(NULL, " \t\n");

         } /* while */

         if (splitOnly)
         {
            fputc('\n', output);
            width = punct = 0;
         }

      } /* else */

   } /* while */

/*--------------------------------------------------------------------*/
/*                    Clean up and exit processing                    */
/*--------------------------------------------------------------------*/

   if (ferror(input))
   {
      perror(inputName);
      clearerr(input);
   }

   if (ferror(output))
   {
      perror(outputName);
      clearerr(output);
   }

   fclose(input);
   fclose(output);

   return 0;

 } /* main */
