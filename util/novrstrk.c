/*--------------------------------------------------------------------*/
/*    Program:    noverstrk.c     14 August 1990                      */
/*    Author:     Andrew H. Derbyshire                                */
/*                108 Decatur St, Apt 9                               */
/*                Arlington, MA 02174                                 */
/*    Function:   Drop overstruck characters from a file              */
/*    Arguments:  input file, output file.                            */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS information                           */
/*--------------------------------------------------------------------*/

/*
   $Log: NOVRSTRK.C $
 * Revision 1.1  1992/11/15  04:29:22  ahd
 * Initial revision
 *
 * Revision 1.1  1992/04/27  00:27:34  ahd
 * Initial revision
 *
 */

static char rcsid[] =
         "$Id: NOVRSTRK.C 1.1 1992/11/15 04:29:22 ahd Exp $";

/*--------------------------------------------------------------------*/
/*    Revised 10 March 1991 to handle overstriking via carriage       */
/*    returns, which is how Word for Windows does it                  */
/*--------------------------------------------------------------------*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "lib.h"
#include "timestmp.h"

/*--------------------------------------------------------------------*/
/*    m a i n                                                         */
/*                                                                    */
/*    Main program                                                    */
/*--------------------------------------------------------------------*/

 void main( int argc, char *argv[] )
 {
   char buf[BUFSIZ];       /* Our I/O buffer                         */
   size_t column = 0;
   size_t linesize = 0;
   int argx = 1;           /* Current argument being processed       */
   FILE *input;
   FILE *output;
   int done = 0;
   int ignore = 0;

/*--------------------------------------------------------------------*/
/*                        Announce our version                        */
/*--------------------------------------------------------------------*/

   banner( argv );

/*--------------------------------------------------------------------*/
/*                            Handle help                             */
/*--------------------------------------------------------------------*/

   if (( argc > 1 ) && equal(argv[1],"-?"))
   {
      printf("Usage:\tnovrstrk\t infile outfile\n");
      exit(1);
   }
/*--------------------------------------------------------------------*/
/*                  Get the input file name, if any                   */
/*--------------------------------------------------------------------*/

    if (argx == argc)
      input = stdin;
    else {
      input = fopen(argv[argx++],"rb");
      if (input == NULL)
      {
         perror(argv[--argx]);
         exit (100);
      }
    }

/*--------------------------------------------------------------------*/
/*                    Get output file name, if any                    */
/*--------------------------------------------------------------------*/

    if (argx == argc )
      output = stdout;
    else {
      output = fopen(argv[argx++],"w");
      if (output == NULL)
      {
         perror(argv[--argx]);
         exit( 200 );
      } /* if */
    } /* else */

/*--------------------------------------------------------------------*/
/*       Main loop to drop our overstrikes                            */
/*--------------------------------------------------------------------*/

    while (!done)
    {
      char c = fgetc( input );
      if ( feof(input) || ferror(input) )
         done = 1;
      else switch (c)
      {
         case '\b':           /* Simple overstrike?                  */
            ignore = 1;       /* Yes --> Ignore it                   */
            break;

         case '\r':           /* Carriage return?                    */
            if ( column > linesize )
               linesize = column;
            column = 0;       /* Yes --> Start line over again       */
            break;

         case '\f':           /* Form feed?                          */
         case '\n':           /* New line?                           */
            if ( column > linesize )
               linesize = column;
            buf[linesize++] = c;  /* Yes --> Add the end of line     */
            buf[linesize] = '\0'; /*   ... terminate the buffer      */
            fputs(buf, output);   /*   ... write it out              */
            done = ferror(output);/*   ... check the result          */
            linesize = column = 0;/*   ... and begin a new buffer    */
            break;

         default:
            if ( ignore )
               ignore = 0;
            else if (( linesize > column ) && (c == ' '))
               column += 1;
            else
               buf[ column++ ] = c; /* Add the character to the buf */
       break;
       } /* switch */
    } /* while */

/*--------------------------------------------------------------------*/
/*                   Check for errors on the files                    */
/*--------------------------------------------------------------------*/

   if (ferror(input))
   {
      perror(argv[argc > 1 ? 1 : 0]);
      clearerr(input);
   }

   if ((column > 0) && !ferror(output))
   {
      fputs(buf, output );
      buf[linesize] = '\0';
   }

   if (ferror(output))
   {
      perror(argv[argc > 2 ? 2 : 0]);
      clearerr(output);
   }

/*--------------------------------------------------------------------*/
/*                       Close up shop and exit                       */
/*--------------------------------------------------------------------*/

   fclose(input);
   fclose(output);

   exit (0);
 } /* main */
