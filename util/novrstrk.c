/*--------------------------------------------------------------------*/
/*    Program:    noverstrk.c     14 August 1990                      */
/*    Author:     Andrew H. Derbyshire                                */
/*                108 Decatur St, Apt 9                               */
/*                Arlington, MA 02174                                 */
/*    Function:   Drop overstruck characters from a file              */
/*    Arguments:  input file, output file.                            */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-2002 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS information                           */
/*--------------------------------------------------------------------*/

/*
   $Log: novrstrk.c $
   Revision 1.12  2001/03/12 13:54:49  ahd
   Annual copyright update

   Revision 1.11  2000/05/12 12:32:00  ahd
   Annual copyright update

   Revision 1.10  1999/01/08 02:20:52  ahd
   Convert currentfile() to RCSID()

   Revision 1.9  1999/01/04 03:53:30  ahd
   Annual copyright change

   Revision 1.8  1998/03/01 01:36:36  ahd
   Annual Copyright Update

   Revision 1.7  1997/04/24 01:31:15  ahd
   Annual Copyright Update

   Revision 1.6  1996/01/01 21:17:59  ahd
   Annual Copyright Update

   Revision 1.5  1995/01/30 04:08:36  ahd
   Additional compiler warning fixes

   Revision 1.4  1994/02/19 05:03:07  ahd
   Use standard first header

 * Revision 1.3  1993/10/24  20:58:55  rhg
 * Clean up for MS C 7.0
 *
 * Revision 1.2  1993/04/11  00:33:54  ahd
 * Global edits for year, TEXT, etc.
 *
 * Revision 1.1  1992/11/15  04:29:22  ahd
 * Initial revision
 *
 * Revision 1.1  1992/04/27  00:27:34  ahd
 * Initial revision
 *
 */

#include "uupcmoah.h"

static char rcsid[] =
         "$Id: novrstrk.c 1.12 2001/03/12 13:54:49 ahd v1-13k $";

/*--------------------------------------------------------------------*/
/*    Revised 10 March 1991 to handle overstriking via carriage       */
/*    returns, which is how Word for Windows does it                  */
/*--------------------------------------------------------------------*/

#include "timestmp.h"

RCSID("$Id: novrstrk.c 1.12 2001/03/12 13:54:49 ahd v1-13k $");

/*--------------------------------------------------------------------*/
/*    m a i n                                                         */
/*                                                                    */
/*    Main program                                                    */
/*--------------------------------------------------------------------*/

 main( int argc, char *argv[] )
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
      int c = fgetc( input );
      switch (c)
      {
         case EOF:            /* Error or EOF */
            done = 1;
            break;

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
            buf[linesize++] = (char)c;/* Yes --> Add the end of line */
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
               buf[ column++ ] = (char) c; /* Add the character to the buf */
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

   return 0;

 } /* main */
