/*--------------------------------------------------------------------*/
/*    Program:    fmt.c          14 August 1990                       */
/*    Author:     Andrew H. Derbyshire                                */
/*                108 Decatur St, Apt 9                               */
/*                Arlington, MA 02174                                 */
/*    Function:   Format lines into user specified width rows         */
/*    Arguments:  width, input file, output file.                     */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: fmt.c 1.3 1993/10/24 20:58:55 rhg Exp $
 *
 *    Revision history:
 *    $Log: fmt.c $
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

static char rcsid[] = "$Id: fmt.c 1.3 1993/10/24 20:58:55 rhg Exp $";

#include <ctype.h>

#include "timestmp.h"

/*--------------------------------------------------------------------*/
/*    m a i n                                                         */
/*                                                                    */
/*    main program                                                    */
/*--------------------------------------------------------------------*/

 void main( int argc, char *argv[] )
 {
   int width = 0;          /* Width of current line                  */
   int maxwidth = 72;      /* Max width of line allowed              */
   char buf[BUFSIZ];       /* Our I/O buffer                         */
   int punct = 0;          /* Last character in last word was punct  */
   int argx = 1;           /* Current argument being processed       */
   FILE *input;
   FILE *output;

/*--------------------------------------------------------------------*/
/*                        Announce our version                        */
/*--------------------------------------------------------------------*/

   banner( argv );

/*--------------------------------------------------------------------*/
/*                            Handle help                             */
/*--------------------------------------------------------------------*/

   if (( argc > 1 ) && equal(argv[1],"-?"))
   {
      printf("Usage:\tfmt\t[-#] infile outfile\n");
      exit(1);
   }

/*--------------------------------------------------------------------*/
/*       Get the line width if the user specified it                  */
/*--------------------------------------------------------------------*/

    if ((argx < argc) && (*argv[argx] == '-'))
      maxwidth = atoi( argv[argx++] );

/*--------------------------------------------------------------------*/
/*                  Get the input file name, if any                   */
/*--------------------------------------------------------------------*/

    if (argx == argc)
      input = stdin;
    else {
      input = fopen(argv[argx++],"r");
      if (input == NULL)
      {
         perror(argv[--argx]);
         exit (100);
      }
    }

/*--------------------------------------------------------------------*/
/*                  Get the output file name, if any                  */
/*--------------------------------------------------------------------*/

    if (argx == argc )
      output = stdout;
    else {
      output = fopen(argv[argx++],"w");
      if (output == NULL)
      {
         perror(argv[--argx]);
         exit( 200 );
      }
    }

/*--------------------------------------------------------------------*/
/*                          Process the file                          */
/*--------------------------------------------------------------------*/

    while( fgets(buf, BUFSIZ, input) != NULL )
    {
      char *token = strtok(buf, " \t\n");
      if (token == NULL )
      {
         fputc('\n',output);
         width = punct = 0;
      }
      else while(token != NULL)
      {
         register int toklen = (int) strlen(token);
         width = toklen + width + 1 + punct;
         if (width > max(maxwidth, toklen + 1))
         {
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
      } /* else while */
   } /* while */

   if (ferror(input))
   {
      perror(argv[1]);
      clearerr(input);
   }

   fclose(input);
   fclose(output);

   exit (0);
 } /* main */
