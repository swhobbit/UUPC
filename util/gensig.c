/*--------------------------------------------------------------------*/
/*       Program:    gensig.c             24 July 1991                */
/*       Author:     Andrew H. Derbyshire                             */
/*       Address:    ahd@kew.com                                      */
/*       Function:   Append a random quote to an electronic           */
/*                   mail signature file                              */
/*       Language:   Borland C++ 2.0 (in ANSI C mode)                 */
/*       Arguments:  Name of file with fixed text input               */
/*                   Name of file with variable quotes                */
/*                   Name of file to be written                       */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: E:\src\uupc\UTIL\RCS\GENSIG.C 1.2 1993/03/06 23:04:54 ahd Exp $
 *
 *    Revision history:
 *    $Log: GENSIG.C $
 * Revision 1.2  1993/03/06  23:04:54  ahd
 * Alter message to include output files
 *
 * Revision 1.1  1992/11/15  04:29:22  ahd
 * Initial revision
 *
 */

static char rcsid[] = "$Id: GENSIG.C 1.2 1993/03/06 23:04:54 ahd Exp $";

/*--------------------------------------------------------------------*/
/*                       Standard include files                       */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <direct.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "timestmp.h"

/*--------------------------------------------------------------------*/
/*                            Local macros                            */
/*--------------------------------------------------------------------*/

#define bitsper(s) (8 * (sizeof s))

#define bitflag(x, s) (1 << ((bitsper(s) - 1) - x))

#define biton( s, offset ) \
               (( s[ (size_t) (offset / bitsper(*s)) ] ) & \
                bitflag(offset % bitsper( *s ), *s))

#define bitoff( s, offset) (!biton(s, offset))

/*--------------------------------------------------------------------*/
/*                    Internal function prototypes                    */
/*--------------------------------------------------------------------*/

static void usage( void );

static long chooseit( struct stat *current_status,
       const char *lookaside,
                 const char *quoteused,
                 const char *fname ,
                 const char *target);

static long getquote( const char *data, const char *target);

static void CopyQuote( const char *fname, long where, FILE *stream);

static void CopyFixed( const char *fname, FILE *stream );

static long chooseavailable( const char *quoteused, long quotes );

currentfile();

/*--------------------------------------------------------------------*/
/*    m a i n                                                         */
/*                                                                    */
/*    main program                                                    */
/*--------------------------------------------------------------------*/

void main( int argc, char **argv)
{

   long where;
   FILE *stream;

   banner( argv );

/*--------------------------------------------------------------------*/
/*                  Validate the number of arguments                  */
/*--------------------------------------------------------------------*/

      if ( argc !=  4 )
         usage();

/*--------------------------------------------------------------------*/
/*    Determine the number of the quotes available, and then          */
/*    select one                                                      */
/*--------------------------------------------------------------------*/

      where = getquote( argv[2], argv[3] );

/*--------------------------------------------------------------------*/
/*                      Open up our output file                       */
/*--------------------------------------------------------------------*/

      stream = fopen( argv[3] , "w");
      if ( stream == NULL )
      {
         perror( argv[3] );
         exit(1);
      }

/*--------------------------------------------------------------------*/
/*           Copy the fixed and variable parts of the file            */
/*--------------------------------------------------------------------*/

      CopyFixed( argv[1], stream );
      CopyQuote( argv[2], where, stream );

/*--------------------------------------------------------------------*/
/*                   Close up and return to caller                    */
/*--------------------------------------------------------------------*/

      fclose( stream );
      exit( 0 );

} /* main */

/*--------------------------------------------------------------------*/
/*    u s a g e                                                       */
/*                                                                    */
/*    Report program usage and then exit                              */
/*--------------------------------------------------------------------*/

static void usage( void )
{
   printf("Usage:\tgensig\taddr-file quote-file output-file\n");
   printf("\taddr-file\tFixed portion of signature file\n");
   printf("\tquote-file\tFile of quotes, separated by delimiter lines\n");
   printf("\toutput-file\tOutput file with fixed portion and single quote\n");
   exit( 2 );
} /* usage */

/*--------------------------------------------------------------------*/
/*    g e t q u o t e                                                 */
/*                                                                    */
/*    Select a quote to process                                       */
/*--------------------------------------------------------------------*/

static long getquote( const char *data, const char *target)
{
   struct stat current_status;
   long where;

   char lookaside[FILENAME_MAX];
   char quoteused[FILENAME_MAX];
   char drive[FILENAME_MAX],
        dir[FILENAME_MAX],
        file[FILENAME_MAX],
        ext[FILENAME_MAX];

/*--------------------------------------------------------------------*/
/*       Get size and data information on the quotes data file        */
/*--------------------------------------------------------------------*/

   if(stat((char *) data, &current_status ) <0)
   {
      perror( data );         /* If no data file, panic gracefully   */
      exit( 3 );
   } /* if */

/*--------------------------------------------------------------------*/
/*       Build the lookaside file name from the data file name        */
/*--------------------------------------------------------------------*/

#ifdef __TURBOC__
   fnsplit( data, drive, dir, file, ext);
   fnmerge( lookaside, drive, dir, file, ".las");
   fnmerge( quoteused, drive, dir, file, ".qus");
#else
   _splitpath( data, drive, dir, file, ext);
   _makepath( lookaside, drive, dir, file, ".las");
   _makepath( quoteused, drive, dir, file, ".qus");
#endif /* __TURBOC__ */

/*--------------------------------------------------------------------*/
/*    Now get the location of the quote; if it fails the first        */
/*    time, then the lookaside buffer is updated and we can try       */
/*    again                                                           */
/*--------------------------------------------------------------------*/

   where = chooseit( &current_status, lookaside, quoteused, data , target);
   if  (where == -1 )
   {
      where = chooseit( &current_status, lookaside, quoteused, data, target );
      if ( where == - 1)
      {
         printf("Unable to create lookaside file \"%s\"!\n",
                  lookaside );
         exit( 4 );
      } /* if ( where == - 1) */
   } /* if ( where == - 1) */

/*--------------------------------------------------------------------*/
/*                          Return to caller                          */
/*--------------------------------------------------------------------*/

   return where;

} /* getquote */

/*--------------------------------------------------------------------*/
/*    c h o o s e i t                                                 */
/*                                                                    */
/*    Returns number of quotes in file                                */
/*--------------------------------------------------------------------*/

static long chooseit( struct stat *current_status,
                 const char *lookaside,
                 const char *quoteused,
                 const char *fname,
                 const char *target)
{
   FILE *stream;
   FILE *data;
   long where;
   long quotes = 0;
   long quote;

   char buf[BUFSIZ];
   char delimiter[BUFSIZ];

/*--------------------------------------------------------------------*/
/*    Open up the lookaside file and determine if it is up to         */
/*    date.  If so, choose an entry and return it.                    */
/*--------------------------------------------------------------------*/

   stream = fopen( lookaside , "rb" );
   if ( stream != NULL )
   {
      struct stat status;
      fread( &status, sizeof status, 1, stream);
      if ((status.st_size == current_status->st_size) &&
          (status.st_mtime == current_status->st_mtime))
      {                       /* Lookaside file up to date, use it   */
         if( stat((char *) lookaside, &status ) < 0)
         {
            perror( lookaside );
            exit( 5 );
         } /* if */

         quotes = (status.st_size - sizeof status) / sizeof where;
                              /* Determine number of quote pointers
                                 in lookaside file from the file's
                                 length                              */

         quote = chooseavailable( quoteused, quotes);

         fseek( stream, quote * sizeof where , SEEK_CUR );
                              /* Step required number of quotes
                                 into the file                       */
         fread( &where, sizeof where, 1, stream);
                              /* Read offset in data file of quote   */
         printf("Chose quote %ld of %ld from %s for %s:\n\n",
                  quote + 1 , quotes, fname, target);
                              /* Announce number of quote of the day */
         fclose( stream );    /* Done with lookaside file, of course */
         return where;        /* Return position in file to caller   */
      } /* if */
      else
         fclose( stream );
   } /* if ( stream != NULL ) */
   else
      perror( lookaside );

/*--------------------------------------------------------------------*/
/*               We have to rewrite the lookaside file                */
/*--------------------------------------------------------------------*/

   unlink( quoteused );          /* Make all quotes available        */

   data   = fopen( fname, "rt");          /* Open data file to scan  */
   if ( data == NULL )                    /* Did it open?            */
   {
      perror( fname );                    /* No --> Error            */
      exit( 6 );
   }

   stream = fopen( lookaside , "wb" );    /* Open lookaside file     */
   if ( stream == NULL )                  /* Did it open?            */
   {
      perror( lookaside );                /* No --> Error            */
      exit( 7 );
   }

/*--------------------------------------------------------------------*/
/*    Start the new lookaside file with the status of the data        */
/*    file for later comparisons                                      */
/*--------------------------------------------------------------------*/

   fwrite( current_status, sizeof *current_status, 1, stream);

/*--------------------------------------------------------------------*/
/*    Get the first line of the file, which will contain the          */
/*    delimiter line                                                  */
/*--------------------------------------------------------------------*/

   fgets(delimiter, BUFSIZ, data );
   where = ftell( data );  /* Get location of possible first quote   */

/*--------------------------------------------------------------------*/
/*                  Now process the rest of the file                  */
/*--------------------------------------------------------------------*/

   while( fgets(buf, BUFSIZ, data ) != NULL )
   {
      if ( strcmp( buf, delimiter ))   /* Delimiter line?            */
      {                                /* No --> data line           */
         if ( where != -1L )  /* First line of new quote?            */
         {                    /* Yes --> Write location to lookaside */
            quotes ++ ;
            fwrite( &where, sizeof where, 1, stream);
            where = -1L;
         } /* if */
      } /* if */
      else
         where = ftell( data );        /* Delimiter line, remember
                                          location of NEXT line in
                                          file                       */
   } /* while */

/*--------------------------------------------------------------------*/
/*                 Close up shop and return to caller                 */
/*--------------------------------------------------------------------*/

   fclose( stream );
   fclose( data );

   if ( quotes == 1 )
   {
      printf("Invalid data file; first line not a delimiter line!");
      exit( 99 );
   }

   printf("Updated lookaside file %s with %d quotes.\n",
               lookaside , quotes);

   return -1;                 /* Inform caller that lookaside
                                 file was updated                    */
} /* chooseit */

/*--------------------------------------------------------------------*/
/*    c h o o s e a v a i l a b l e                                   */
/*                                                                    */
/*    Select a quote from available list                              */
/*--------------------------------------------------------------------*/

static long chooseavailable( const char *quoteused, long quotes )
{
   FILE *stream;
   unsigned char *quotelist;

   size_t listsize = (size_t) (( quotes + bitsper(*quotelist) - 1 ) /
                              bitsper(*quotelist));
   long quote;
   long available = quotes;
   long select;

/*--------------------------------------------------------------------*/
/*                 Initialize the bit array of quotes                 */
/*--------------------------------------------------------------------*/

   quotelist = malloc( listsize );
   checkref( quotelist );

/*--------------------------------------------------------------------*/
/*             Open up the used quoted file, if possible              */
/*--------------------------------------------------------------------*/

   stream = fopen( quoteused, "rb");

/*--------------------------------------------------------------------*/
/*              Determine the number of available quotes              */
/*--------------------------------------------------------------------*/

   if ( stream != NULL )
   {

      if (fread (&available, sizeof available, 1 , stream) < 1 )
      {
         perror(  quoteused );
         available = 0;
      }
      else if ( quotes <= available )
         available = 0;
      else if (fread (quotelist, sizeof *quotelist, listsize , stream) <
                  listsize)
      {
         available = 0;
         perror( quoteused );
      }

      fclose( stream );
   } /* if */
   else
      available = 0;

   if ( available <= ( quotes / 10 ))
   {
      memset( quotelist, '\0', listsize );
      printf("Resetting available quotes list\n");
      available = quotes;
   }

/*--------------------------------------------------------------------*/
/*                       Now, choose the quote                        */
/*--------------------------------------------------------------------*/

   srand ( time( NULL ));     /* Initialize the generator      */
   select = rand() % available ;
                        /* ... Lucky seven?                    */

/*--------------------------------------------------------------------*/
/*                         end debugging info                         */
/*--------------------------------------------------------------------*/

   quote = 0;
   while( quote < quotes )
   {
      if ( bitoff( quotelist, quote ))
         select--;

      if ( select > 0 )
         quote++;
      else
         break;
   }

   if ( quote == quotes )           /* Did we overflow the array?    */
      panic();                      /* Must have a bug!              */

/*--------------------------------------------------------------------*/
/*                Now update our data and write it out                */
/*--------------------------------------------------------------------*/

   stream = fopen( quoteused, "wb");
   if ( stream == NULL )
   {
      perror( quoteused );
      panic();
   }

   quotelist[ (size_t) (quote / bitsper(*quotelist)) ] |=
               bitflag( quote % bitsper(*quotelist), *quotelist );

   available -= 1;

   fwrite(&available, sizeof available, 1 , stream);
   fwrite(quotelist,  sizeof *quotelist, listsize , stream);
   fclose( stream );

/*--------------------------------------------------------------------*/
/*                   Return the quote to the caller                   */
/*--------------------------------------------------------------------*/

   return quote;

} /* chooseavailable */

/*--------------------------------------------------------------------*/
/*    C o p y F i x e d                                               */
/*                                                                    */
/*    Copy fixed input text (user name and address) to output file    */
/*--------------------------------------------------------------------*/

static void CopyFixed( const char *fname, FILE *stream )
{
   FILE *input;
   char buf[BUFSIZ];

/*--------------------------------------------------------------------*/
/*                    Open input file for copying                     */
/*--------------------------------------------------------------------*/

   input = fopen( fname, "r");
   if ( input == NULL )
   {
      perror( fname );
      exit ( 8 );
   }

/*--------------------------------------------------------------------*/
/*             Copy the fixed input to the signature file             */
/*--------------------------------------------------------------------*/

   while( fgets( buf, BUFSIZ, input ) != NULL)
      fputs( buf, stream );

/*--------------------------------------------------------------------*/
/*                 Close up shop and return to caller                 */
/*--------------------------------------------------------------------*/

   fclose( input );

} /* CopyFixed */

/*--------------------------------------------------------------------*/
/*    C o p y Q u o t e                                               */
/*                                                                    */
/*    Write a quote to the output file                                */
/*--------------------------------------------------------------------*/

static void CopyQuote( const char *fname, long where, FILE *stream)
{
   FILE *input;
   char buf[BUFSIZ];
   char delimiter[BUFSIZ];

/*--------------------------------------------------------------------*/
/*         Open up the quotes file and get the delimiter line         */
/*--------------------------------------------------------------------*/

   input = fopen( fname, "r");
   if ( input == NULL )
   {
      perror( fname );
      exit ( 9 );
   }

   fgets(delimiter, BUFSIZ, input );

/*--------------------------------------------------------------------*/
/*   Position to the beginning of the actual quote to be processed    */
/*--------------------------------------------------------------------*/

   fseek( input, where , SEEK_SET );

/*--------------------------------------------------------------------*/
/*      Copy the quote to both the signature file and the screen      */
/*--------------------------------------------------------------------*/

   while( fgets( buf, BUFSIZ, input ) != NULL)
   {
      if (!strcmp( delimiter, buf ))   /* Delimiter line?            */
         break;                        /* Yes --> End of quote       */
      fputs( buf, stream );   /* Copy quote to signature file        */
      fputs( buf, stdout );   /* Copy quote to screen                */
   } /* while( fgets( buf, BUFSIZ, input ) != NULL) */

/*--------------------------------------------------------------------*/
/*                 Close up shop and return to caller                 */
/*--------------------------------------------------------------------*/

   fclose( input );
} /* CopyQuote */
