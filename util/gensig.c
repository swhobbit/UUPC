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
 *    $Id: gensig.c 1.20 1999/01/08 02:20:52 ahd Exp $
 *
 *    Revision history:
 *    $Log: gensig.c $
 *    Revision 1.20  1999/01/08 02:20:52  ahd
 *    Convert currentfile() to RCSID()
 *
 *    Revision 1.19  1999/01/04 03:53:30  ahd
 *    Annual copyright change
 *
 *    Revision 1.18  1998/05/11 01:20:48  ahd
 *    Correct compiler warning
 *
 *    Revision 1.17  1998/03/01 01:36:31  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.16  1997/04/24 01:31:09  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.15  1996/11/18 04:46:49  ahd
 *    Normalize arguments to bugout
 *    Reset title after exec of sub-modules
 *    Normalize host status names to use HS_ prefix
 *
 *    Revision 1.14  1996/01/01 21:17:50  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.13  1995/03/24 04:17:22  ahd
 *    Compiler warning message cleanup, optimize for low memory processing
 *
 *    Revision 1.12  1995/03/11 22:28:11  ahd
 *    Use macro for file delete to allow special OS/2 processing
 *
 *    Revision 1.11  1995/02/21 02:47:44  ahd
 *    The compiler warnings war never ends!
 *
 *    Revision 1.10  1995/02/20 00:40:12  ahd
 *    Correct C compiler warnings
 *
 *    Revision 1.9  1995/01/29 16:43:03  ahd
 *    IBM C/Set compiler warnings
 */

#include "uupcmoah.h"

static const char rcsid[] =
      "$Id: gensig.c 1.20 1999/01/08 02:20:52 ahd Exp $";

/*--------------------------------------------------------------------*/
/*                       Standard include files                       */
/*--------------------------------------------------------------------*/

#include <io.h>
#include <direct.h>
#include <sys/stat.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "timestmp.h"

/*--------------------------------------------------------------------*/
/*                            Local macros                            */
/*--------------------------------------------------------------------*/

#define bitsPer(s) (8 * (sizeof s))

#define bitFlag(x, s) (1u << (unsigned) (bitsPer(s) - (1 + (x))))

#define bitOn( s, offset ) \
               (( s[ (size_t) (offset / bitsPer(*s)) ] ) & \
                bitFlag(offset % bitsPer( *s ), *s))

#define bitOff( s, offset) (!bitOn(s, offset))

#define LINESIZE 128

/*--------------------------------------------------------------------*/
/*                    Internal function prototypes                    */
/*--------------------------------------------------------------------*/

static void usage( void );

static long chooseit( struct stat *current_status,
       const char *lookaside,
                 const char *quotesUsed,
                 const char *fname ,
                 const char *target);

static long getquote( const char *data, const char *target);

static void CopyQuote( const char *fname, long where, FILE *stream);

static void CopyFixed( const char *fname, FILE *stream );

static long chooseavailable( const char *quotesUsed, long quotes );

RCSID("$Id: gensig.c 1.20 1999/01/08 02:20:52 ahd Exp $");

/*--------------------------------------------------------------------*/
/*    m a i n                                                         */
/*                                                                    */
/*    main program                                                    */
/*--------------------------------------------------------------------*/

main( int argc, char **argv)
{

   long where;
   FILE *stream;

   banner( argv );

   if ( ! configure( B_GENERIC ))
      panic();

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
         panic();
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
      exit(0);
      return 0;

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

   panic();

} /* usage */

/*--------------------------------------------------------------------*/
/*       n a m e s                                                    */
/*                                                                    */
/*       Determine names of files                                     */
/*--------------------------------------------------------------------*/

void static
names( const char *data, char *lookaside , char *quotesUsed )
{

   char drive[FILENAME_MAX],
        dir[FILENAME_MAX],
        file[FILENAME_MAX],
        ext[FILENAME_MAX];

/*--------------------------------------------------------------------*/
/*       Build the lookaside file name from the data file name        */
/*--------------------------------------------------------------------*/

#ifdef __TURBOC__

   fnsplit( data, drive, dir, file, ext);
   fnmerge( lookaside, drive, dir, file, ".las");
   fnmerge( quotesUsed, drive, dir, file, ".qus");

#else

   _splitpath( (char *) data, drive, dir, file, ext);
   _makepath( lookaside, drive, dir, file, ".las");
   _makepath( quotesUsed, drive, dir, file, ".qus");

#endif /* __TURBOC__ */

} /* names */

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
   char quotesUsed[FILENAME_MAX];

/*--------------------------------------------------------------------*/
/*       Get size and data information on the quotes data file        */
/*--------------------------------------------------------------------*/

   if (stat((char *) data, &current_status ) <0)
   {
      perror( data );         /* If no data file, panic gracefully   */
      panic();
   } /* if */

/*--------------------------------------------------------------------*/
/*    Now get the location of the quote; if it fails the first        */
/*    time, then the lookaside buffer is updated and we can try       */
/*    again                                                           */
/*--------------------------------------------------------------------*/

   names( data, lookaside, quotesUsed );

   where = chooseit( &current_status, lookaside, quotesUsed, data , target);

   if  (where == -1 )
   {
      where = chooseit( &current_status,
                        lookaside,
                        quotesUsed,
                        data,
                        target );

      if ( where == - 1)
      {
         printf("Unable to create lookaside file \"%s\"!\n",
                  lookaside );
         panic();
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
                 const char *quotesUsed,
                 const char *fname,
                 const char *target)
{
   FILE *stream;
   FILE *data;
   long where;
   long quotes = 0;
   long quote;

   char buf[LINESIZE];
   char delimiter[LINESIZE];

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
            panic();
         } /* if */

         quotes = (status.st_size - sizeof status) / sizeof where;
                              /* Determine number of quote pointers
                                 in lookaside file from the file's
                                 length                              */

         quote = chooseavailable( quotesUsed, quotes);
         printf("Chose quote %ld of %ld from %s for %s:\n\n",
                  quote + 1 , quotes, fname, target);
                              /* Announce number of quote of the day */

         fseek( stream, (long) quote * sizeof where , SEEK_CUR );
                              /* Step required number of quotes
                                 into the file                       */
         fread( &where, sizeof where, 1, stream);
                              /* Read offset in data file of quote   */
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

   REMOVE( quotesUsed );          /* Make all quotes available        */

   data   = fopen( fname, "r");           /* Open data file to scan  */

   if ( data == NULL )                    /* Did it open?            */
   {
      perror( fname );                    /* No --> Error            */
      panic();
   }

   stream = fopen( lookaside , "wb" );    /* Open lookaside file     */

   if ( stream == NULL )                  /* Did it open?            */
   {
      perror( lookaside );                /* No --> Error            */
      panic();
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

   fgets(delimiter, sizeof delimiter, data );
   where = ftell( data );  /* Get location of possible first quote   */

/*--------------------------------------------------------------------*/
/*                  Now process the rest of the file                  */
/*--------------------------------------------------------------------*/

   while( fgets(buf, LINESIZE, data ) != NULL )
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
      panic();
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

static long
chooseavailable( const char *quotesUsed, long quotes )
{
   FILE *stream;
   unsigned char *quoteList;
   unsigned char byte;

   size_t listsize = (size_t) (( quotes + bitsPer(*quoteList) - 1 ) /
                              bitsPer(*quoteList));

   size_t subscript;
   unsigned long quote;
   long available = quotes;
   long select;

/*--------------------------------------------------------------------*/
/*                 Initialize the bit array of quotes                 */
/*--------------------------------------------------------------------*/

   quoteList = malloc( listsize * sizeof quoteList);
   checkref( quoteList );

/*--------------------------------------------------------------------*/
/*             Open up the used quoted file, if possible              */
/*--------------------------------------------------------------------*/

   stream = fopen( quotesUsed, "rb");

/*--------------------------------------------------------------------*/
/*              Determine the number of available quotes              */
/*--------------------------------------------------------------------*/

   if ( stream != NULL )
   {

      if (fread (&available, sizeof available, 1 , stream) < 1 )
      {
         perror(  quotesUsed );
         available = 0;
      }
      else if ( quotes <= available )
         available = 0;
      else if (fread (quoteList, sizeof *quoteList, listsize , stream) <
                  listsize)
      {
         available = 0;
         perror( quotesUsed );
      }

      fclose( stream );
   } /* if */
   else
      available = 0;

   if ( available <= ( quotes / 10 ))
   {
      memset( quoteList, '\0', listsize * sizeof *quoteList );
      printf("Resetting available quotes list\n");
      available = quotes;
   }

/*--------------------------------------------------------------------*/
/*                       Now, choose the quote                        */
/*--------------------------------------------------------------------*/

   srand ( (unsigned) time( NULL )); /* Initialize the generator     */
   select = rand() % available ; /* ... Lucky seven?                 */

/*--------------------------------------------------------------------*/
/*                         end debugging info                         */
/*--------------------------------------------------------------------*/

   quote = 0;

   while( (long) quote < quotes )
   {
      if ( bitOff( quoteList, quote ))
         select--;

      if ( select > 0 )
         quote++;
      else
         break;
   }

   if ( (long) quote == quotes )    /* Did we overflow the array?    */
   {
      printf("Internal error at line %d -- internal quote list exhausted",
               __LINE__ );
      panic();                      /* Must have a bug!              */
   }

/*--------------------------------------------------------------------*/
/*                Now update our data and write it out                */
/*--------------------------------------------------------------------*/

   stream = fopen( quotesUsed, "wb");

   if ( stream == NULL )
   {
      perror( quotesUsed );
      panic();
   }

   subscript = (size_t) (quote / bitsPer(*quoteList));
   byte = (unsigned char) bitFlag( quote % bitsPer(*quoteList), *quoteList );
   quoteList[ subscript ] = (unsigned char) (quoteList[ subscript ] | byte);

   available -= 1;

   fwrite(&available, sizeof available, 1 , stream);
   fwrite(quoteList,  sizeof *quoteList, listsize , stream);
   fclose( stream );

/*--------------------------------------------------------------------*/
/*                   Return the quote to the caller                   */
/*--------------------------------------------------------------------*/

   return (long) quote;

} /* chooseavailable */

/*--------------------------------------------------------------------*/
/*    C o p y F i x e d                                               */
/*                                                                    */
/*    Copy fixed input text (user name and address) to output file    */
/*--------------------------------------------------------------------*/

static void CopyFixed( const char *fname, FILE *stream )
{
   FILE *input;
   char buf[LINESIZE];

/*--------------------------------------------------------------------*/
/*                    Open input file for copying                     */
/*--------------------------------------------------------------------*/

   input = fopen( fname, "r");
   if ( input == NULL )
   {
      perror( fname );
      panic();
   }

/*--------------------------------------------------------------------*/
/*             Copy the fixed input to the signature file             */
/*--------------------------------------------------------------------*/

   while( fgets( buf, sizeof buf, input ) != NULL)
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
   char buf[LINESIZE];
   char delimiter[LINESIZE];

/*--------------------------------------------------------------------*/
/*         Open up the quotes file and get the delimiter line         */
/*--------------------------------------------------------------------*/

   input = fopen( fname, "r");

   if ( input == NULL )
   {
      perror( fname );
      panic();
   }

   fgets(delimiter, sizeof delimiter, input );

/*--------------------------------------------------------------------*/
/*   Position to the beginning of the actual quote to be processed    */
/*--------------------------------------------------------------------*/

   fseek( input, where , SEEK_SET );

/*--------------------------------------------------------------------*/
/*      Copy the quote to both the signature file and the screen      */
/*--------------------------------------------------------------------*/

   while( fgets( buf, sizeof buf, input ) != NULL)
   {
      if (!strcmp( delimiter, buf ))   /* Delimiter line?            */
         break;                        /* Yes --> End of quote       */

      fputs( buf, stream );   /* Copy quote to signature file        */
      fputs( buf, stdout );   /* Copy quote to screen                */

   } /* while( fgets( buf, sizeof buf, input ) != NULL) */

/*--------------------------------------------------------------------*/
/*                 Close up shop and return to caller                 */
/*--------------------------------------------------------------------*/

   fclose( input );

} /* CopyQuote */
