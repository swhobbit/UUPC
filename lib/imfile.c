/*--------------------------------------------------------------------*/
/*       i m f i l e . c                                              */
/*                                                                    */
/*       In-memory file operations, with backing disk store, for      */
/*       temporary files                                              */
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
 *    $Id: imfile.c 1.7 1995/01/09 12:35:15 ahd Exp $
 *
 *    Revision history:
 *    $Log: imfile.c $
 *    Revision 1.7  1995/01/09 12:35:15  ahd
 *    Correct VC++ compiler warnings
 *
 *    Revision 1.6  1995/01/09 01:39:22  ahd
 *    Correct error processing vsprintf() in imprintf()
 *
 *    Revision 1.5  1995/01/08 21:02:02  ahd
 *    Correct BC++ 3.1 compiler warnings
 *
 *    Revision 1.4  1995/01/08 19:52:44  ahd
 *    Add in memory files to RMAIL, including additional support and
 *    bug fixes.
 *
 *    Revision 1.3  1995/01/07 23:52:37  ahd
 *    Convert rnews to use in-memory files, debug associated functions
 *
 *    Revision 1.2  1995/01/07 20:48:21  ahd
 *    Correct 16 compile warnings
 *
 *    Revision 1.1  1995/01/07 15:43:07  ahd
 *    Initial revision
 *
 */

/*--------------------------------------------------------------------*/
/*                       UUPC/extended headers                        */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"

#ifndef BIT32ENV
#include <malloc.h>        /* _fmalloc, etc.                         */
#endif

#include <stdarg.h>
#include <errno.h>
#include <io.h>

#include "imfile.h"
#include "execute.h"

/*--------------------------------------------------------------------*/
/*                         Manifest constants                         */
/*--------------------------------------------------------------------*/

#ifdef BIT32ENV
#define IM_MAX_LENGTH 1000000
#else
#define IM_MAX_LENGTH 65000         /* Under 64K, avoid silly ptr
                                       wrap around                   */
#endif

currentfile();

#ifdef UDEBUG

/*--------------------------------------------------------------------*/
/*       i m S t a t u s                                              */
/*                                                                    */
/*       Report status of current in-memory file                      */
/*--------------------------------------------------------------------*/

static void imStatus( IMFILE *imf )
{

   if ( imf->buffer == NULL )
      printmsg(6,"imStatus: File resides on disk as %s", imf->filename );
   else
      printmsg(5,"imStatus: "
#ifdef BIT32ENV
               "%p"
#else
               "%Fp"
#endif
                  " buffer address, %ld bytes used, %ld bytes capacity, "
                  "current position %ld%s%s",
                  imf->buffer,
                  imf->inUse,
                  imf->length,
                  imf->position,
                  imeof(imf)     ? ", EOF"   : "",
                  imerror( imf ) ? ", ERROR" : "" );

} /* imStatus */
#else

#define imStatus(x)

#endif

/*--------------------------------------------------------------------*/
/*       i m R e s e r v e                                            */
/*                                                                    */
/*       Reserve memory for IM file operation, automatically          */
/*       switching to a real file if needed because of a memory       */
/*       shortage                                                     */
/*--------------------------------------------------------------------*/

static int imReserve( IMFILE *imf, const long length )
{

   long newLength = length + imf->position ;

/*--------------------------------------------------------------------*/
/*            If we have the memory allocated, return quietly         */
/*--------------------------------------------------------------------*/

   if ((imf->buffer == NULL ) || ( newLength <= imf->length ))
      return 0;

/*--------------------------------------------------------------------*/
/*             Attempt to allocate the longer buffer needed           */
/*--------------------------------------------------------------------*/

   if ( newLength <= IM_MAX_LENGTH )
   {
      char UUFAR *newBuffer;

      newLength += newLength / 2;   /* Add a fifty percent  pad      */

      if ( newLength > IM_MAX_LENGTH )    /* Is the pad allowed?     */
         newLength = IM_MAX_LENGTH; /* No --> Just use max           */

#ifdef BIT32ENV
      newBuffer = realloc( imf->buffer, newLength );
#else
      newBuffer = _frealloc( imf->buffer, (size_t) newLength );
#endif

      if ( newBuffer == NULL )
         printerr( "realloc" );
      else {
         imf->length = newLength;
         imf->buffer = newBuffer;
         return 0;
      }

   } /* if ( newLength <= IM_MAX_LENGTH ) */

/*--------------------------------------------------------------------*/
/*       For wharever reason, we need to convert processing to a      */
/*       file; do so.                                                 */
/*--------------------------------------------------------------------*/

   imf->filename = mktempname( NULL, "TMP" );

   printmsg(2,"imReserve: Switching to disk file %s after %ld/%ld bytes",
               imf->filename,
               imf->inUse,
               newLength );

   imf->stream = FOPEN( imf->filename,
                        "w+",
                        IMAGE_MODE );

   if ( imf->stream == NULL )
   {
      printerr( imf->filename );
      imf->flag |= IM_FLAG_ERROR;
      return -1;
   }

   return 0;

} /* imReserve */

/*--------------------------------------------------------------------*/
/*       i m o p e n                                                  */
/*                                                                    */
/*       Open an in memory file                                       */
/*--------------------------------------------------------------------*/

IMFILE *imopen( const long length )    /* Longest in memory
                                          buffer desired          */
{
   IMFILE *imf = malloc( sizeof (IMFILE) );

/*--------------------------------------------------------------------*/
/*          Allocate our control structure and initialize it          */
/*--------------------------------------------------------------------*/

   checkref( imf );
   memset( imf, 0, sizeof *imf );

/*--------------------------------------------------------------------*/
/*       Free our resources and return an error if we had problem     */
/*       processing the open                                          */
/*--------------------------------------------------------------------*/

   if ( imf->flag & IM_FLAG_ERROR)
   {
      imclose( imf );
      return NULL;
   }

/*--------------------------------------------------------------------*/
/*       Grab an imf buffer unless IM files are disabled, the file    */
/*       is too large, or we cannot allocate the buffer.              */
/*--------------------------------------------------------------------*/

   if (bflag[F_IMFILE] && ( length <= IM_MAX_LENGTH ))
   {

      if ( length <= 0 )
         imf->length = IM_MAX_LENGTH / 10;
      else
         imf->length = length;

#ifdef BIT32ENV
         imf->buffer = malloc( length );
#else
         imf->buffer = _fmalloc( (size_t) length );
#endif

      if ( imf->buffer == NULL )
         printerr( "malloc" );

   }  /* if ( length <= IM_MAX_LENGTH ) */
   else
      printmsg(2,"imopen: Using disk for %ld byte file (max i-m is %ld)",
                  length,
                  (long) IM_MAX_LENGTH );

/*--------------------------------------------------------------------*/
/*   Open a real file if we don't have a buffer for whatever reason   */
/*--------------------------------------------------------------------*/

   if ( imf->buffer == NULL )
   {
      imf->filename = mktempname( NULL, "TMP" );
      imf->stream = FOPEN( imf->filename, "w+", IMAGE_MODE );

      if ( imf->stream == NULL )
      {
         imclose( imf );
         return NULL;
      } /* if ( imf->stream == NULL ) */

   } /* ( length > IM_MAX_LENGTH ) */

/*--------------------------------------------------------------------*/
/*                     Return success to the caller                   */
/*--------------------------------------------------------------------*/

   imStatus( imf );

   return imf;

}  /* imopen */

/*--------------------------------------------------------------------*/
/*       i m c l o s e                                                */
/*                                                                    */
/*       Terminate processing of an IMF file                          */
/*--------------------------------------------------------------------*/

int imclose( IMFILE *imf)
{
   int result = 0;

   imStatus( imf );

   if ( imf->buffer != NULL )
   {
#ifdef BIT32ENV
      free( imf->buffer );
#else
      _ffree( imf->buffer );
#endif
   }

   if ( imf->stream != NULL )
   {
      result = fclose( imf->stream );
      unlink( imf->filename );
      free( imf->filename );
   }

   memset( imf, 0, sizeof imf );

   return result;

} /* imclose */

/*--------------------------------------------------------------------*/
/*    i m p r i n t f                                                 */
/*                                                                    */
/*    Perform formatted output to an in-memory file.                  */
/*--------------------------------------------------------------------*/

int imprintf( IMFILE *imf, const char *fmt , ...  )
{
   va_list arg_ptr;

   va_start(arg_ptr,fmt);

   if ( imf->buffer == NULL )
      return vfprintf(imf->stream , fmt, arg_ptr);
   else {
      char buffer[4096];
      int result = vsprintf(buffer, fmt, arg_ptr);

      if ( result == EOF )
         return EOF;

      if ( result > (sizeof buffer) )
      {
         printmsg(0, "imprintf: Memory overflow processing im memory file" );
         panic();                /* We corrupted the stack!          */
      }

      if ( imputs( buffer, imf ) == result )
         return result;
      else
         return EOF;

   }  /* else */

} /* imprintf */

/*--------------------------------------------------------------------*/
/*       i m e o f                                                    */
/*                                                                    */
/*       Report if an in-memory file is at end-of-file                */
/*--------------------------------------------------------------------*/

int imeof( IMFILE *imf )
{

   if ( imf->buffer == NULL )
      return feof( imf->stream );
   else
      return imf->inUse <= imf->position;

} /* imeof */

/*--------------------------------------------------------------------*/
/*       i m e r r o r                                                */
/*                                                                    */
/*       Report if an in-memory file has previously had an error      */
/*--------------------------------------------------------------------*/

int imerror( IMFILE *imf )
{
   if ( imf->buffer == NULL )
      return ferror( imf->stream );
   else
      return imf->flag & IM_FLAG_ERROR;

} /* imerror */

/*--------------------------------------------------------------------*/
/*       i m g e t s                                                  */
/*                                                                    */
/*       Read a string from an in-memory file                         */
/*--------------------------------------------------------------------*/

char *imgets( char *userBuffer, int userLength, IMFILE *imf )
{
   char UUFAR *p;
   long stringLength;
   size_t subscript = 0;

   imStatus( imf );

   if ( imf->buffer == NULL )
      return fgets( userBuffer, userLength, imf->stream );

   if ( imerror( imf ) || imeof( imf ))
      return NULL;

   if ( userLength < 2 )            /* Need room for \n and \0          */
   {
      errno = EINVAL;
      return NULL;
   }

/*--------------------------------------------------------------------*/
/*               Select the string from our own buffer                */
/*--------------------------------------------------------------------*/

   stringLength = imf->inUse - imf->position;

   if ( stringLength > (userLength - 1 ))
      stringLength = userLength;

#ifdef UDEBUG
   printmsg(6,"imgets: Requested up to %ld bytes, "
              "actually searching %ld bytes",
               (long) userLength,
               (long) stringLength );
#endif

   p = imf->buffer + (size_t) imf->position;

   while ( (long) subscript < stringLength )
   {
      if ( p[subscript] == '\0' )
      {
         printmsg(2,"imgets: Encountered null byte %ld bytes into search",
                     (long) subscript );
      }

      if ( p[subscript] == '\n' )
         break;
      else
         subscript++;
   }

   MEMCPY( userBuffer, p, ++subscript );
   userBuffer[ subscript ] = '\0';
   imf->position += subscript;

#ifdef UDEBUG
   printmsg(5,"imgets: Returning %d bytes = \"%s\"",
              subscript,
              userBuffer );
#endif

   return userBuffer;

} /* imgets */

/*--------------------------------------------------------------------*/
/*       i m p u t c                                                  */
/*                                                                    */
/*       Write a single character to an in-memory file.  No doubt     */
/*       this could be more efficient if it performed a subset of     */
/*       imwrite directly and didn't call imwrite to perform a one    */
/*       character memcpy, but if you want efficient, send a block    */
/*       of characters to imwrite directly!                           */
/*--------------------------------------------------------------------*/

int imputc( int in, IMFILE *imf )
{
   char c = (char) in;

   int result = imwrite( &c, sizeof c, 1, imf );

   if ( result != 1 )
      return EOF;
   else
      return c;

} /* imputc */

/*--------------------------------------------------------------------*/
/*       i m p u t s                                                  */
/*                                                                    */
/*       Write a string to a in-memory file                           */
/*--------------------------------------------------------------------*/

int imputs( const char *userString, IMFILE *imf )
{
   return imwrite( userString, 1, strlen( userString ), imf );

} /* imputs */

/*--------------------------------------------------------------------*/
/*       i m r e a d                                                  */
/*                                                                    */
/*       Read up to a user specified amount from the im-memory        */
/*       file                                                         */
/*--------------------------------------------------------------------*/

size_t  imread( void *userBuffer,
                size_t objectSize,
                size_t objectCount,
                IMFILE * imf )
{
   size_t bytes = objectSize * objectCount;

   imStatus( imf );

   if ( imf->buffer == NULL )
      return fread( userBuffer, objectSize, objectCount, imf->stream );

   if (( objectSize <= 0 ) || (objectCount <= 0))
   {
      printmsg(0, "imread: Requested read of less than zero bytes" );
      errno = EINVAL;
      return 0;
   }

   if ( imeof( imf ) )
      return 0;

   if ( imerror( imf ) )
      return 0;

   if ( (long) bytes <= (imf->inUse - imf->position ))
   {
      MEMCPY( userBuffer,
              imf->buffer + (size_t) imf->position,
              bytes );
      imf->position += bytes;
      return objectCount;
   }
   else
      return imread( userBuffer,
                     objectSize,
                     (size_t) (imf->inUse - imf->position ) / objectSize ,
                     imf );

} /* imread */

/*--------------------------------------------------------------------*/
/*       i m w r i t e                                                */
/*                                                                    */
/*       Write user data into the in-memory file                      */
/*--------------------------------------------------------------------*/

size_t  imwrite(const void *userBuffer,
                size_t objectSize,
                size_t objectCount,
                IMFILE *imf )
{
   long bytes = objectSize * objectCount;

/*--------------------------------------------------------------------*/
/*            Verify we have a reasonable amount to write             */
/*--------------------------------------------------------------------*/

   if ( bytes <= 0 )
   {
      errno = EINVAL;
      return 0;
   }

/*--------------------------------------------------------------------*/
/*       Verify we have the memory to write in, automatically         */
/*       switching to disk based data if we run low.                  */
/*--------------------------------------------------------------------*/

   if ( imReserve( imf, bytes ) )
      return 0;                     /* Report failure if problems    */

   imStatus( imf );

/*--------------------------------------------------------------------*/
/*                        Actually write the data                     */
/*--------------------------------------------------------------------*/

   if ( imf->buffer == NULL )
      return fwrite( userBuffer, objectSize, objectCount, imf->stream );
   else {

      MEMCPY( imf->buffer + (size_t) imf->position,
              userBuffer,
              (size_t) bytes );

      imf->position += bytes;

      if ( imf->inUse < imf->position )
         imf->inUse = imf->position;

      return objectCount;

   } /* else */

} /* imwrite */

/*--------------------------------------------------------------------*/
/*       i m s e e k                                                  */
/*                                                                    */
/*       Update the current file position                             */
/*--------------------------------------------------------------------*/

int imseek( IMFILE *imf, long int offset, int whence)
{
   long absoluteOffset;

   imStatus( imf );

   if ( imf->buffer == NULL )
      return fseek( imf->stream, offset, whence );

/*--------------------------------------------------------------------*/
/*    Determine the absolute offset from the beginning of the file    */
/*--------------------------------------------------------------------*/

   switch( whence )
   {

      case SEEK_SET:
         absoluteOffset = offset;
         break;

      case SEEK_CUR:
         absoluteOffset = imf->position + offset;
         break;

      case SEEK_END:
         absoluteOffset = imf->inUse + offset;
         break;

      default:
         errno = EINVAL;
         return -1;

   } /* switch( whence ) */

/*--------------------------------------------------------------------*/
/*                  Verify the offset is in our file                  */
/*--------------------------------------------------------------------*/

   if ((absoluteOffset < 0) || (absoluteOffset > imf->inUse ))
   {
      printmsg( 0, "Invalid seek position %ld for IMF %p; file is %d bytes",
                   absoluteOffset,
                   imf,
                   imf->inUse );
      errno = EINVAL;
      return -1;
   }

/*--------------------------------------------------------------------*/
/*      Reset the current file pointer and return to the caller       */
/*--------------------------------------------------------------------*/

   imf->position = absoluteOffset;

   return 0;

} /* imseek */

/*--------------------------------------------------------------------*/
/*       i m t e l l                                                  */
/*                                                                    */
/*       Return current position in in-memory file                    */
/*--------------------------------------------------------------------*/

long imtell( IMFILE *imf )
{
   if ( imf->buffer ==  NULL )
      return ftell( imf->stream );
   else
      return imf->position;

} /* imtell */

/*--------------------------------------------------------------------*/
/*       i m r e w i n d                                              */
/*                                                                    */
/*       Rewind an in-memory file                                     */
/*--------------------------------------------------------------------*/

void imrewind( IMFILE *imf)
{

   imStatus( imf );

   if ( imf->buffer == NULL )
   {
      fflush( imf->stream );
      rewind( imf->stream );
   }
   else {

      imseek( imf, 0 , SEEK_SET );

      if ( imf->inUse < (imf->length / 2))   /* Really overlength ?  */
      {                             /* Yes --> Shorten it up         */

#ifdef UDEBUG
         printmsg(4,"imrewind: Shortening IMF %p from %ld to %ld bytes",
                     imf,
                     imf->length,
                     imf->inUse );
#endif

#ifdef BIT32ENV
         imf->buffer = realloc( imf->buffer, (size_t) imf->inUse );
#else
         imf->buffer = _frealloc( imf->buffer, (size_t) imf->inUse );
#endif
         checkref( imf->buffer );
         imf->length = imf->inUse;

      }  /* if ( imf->inUse < (imf->length / 2)) */

   } /* else */

} /* imrewind */

/*--------------------------------------------------------------------*/
/*       i m l e n g t h                                              */
/*                                                                    */
/*       Return the length of an in-memory file                       */
/*--------------------------------------------------------------------*/

long imlength( IMFILE *imf )
{

   imStatus( imf );

   if ( imf->buffer == NULL )
   {
      fflush( imf->stream );
      return filelength( fileno( imf->stream ) );
   }
   else
      return imf->inUse;

}   /* imlength */

/*--------------------------------------------------------------------*/
/*       i m u n l o a d                                              */
/*                                                                    */
/*       Blast an in-memory file into a real file, beginning with     */
/*       the current file position of both files.                     */
/*--------------------------------------------------------------------*/

int imunload( FILE *output, IMFILE *imf )
{
  char *ioBuf    = NULL;
  int  ioBufSize = (28 * 1024);

/*--------------------------------------------------------------------*/
/*       We invert our normal logic, because 16 bit allocated in      */
/*       FAR memory cannot be blasted directly to disk.  Thus, we     */
/*       handle the simple (and fast) 32 bit copy first and save      */
/*       the funky disk logic for last.                               */
/*--------------------------------------------------------------------*/

#ifdef BIT32ENV

   if ( imf->buffer != NULL )
   {
      while (! imeof( imf ))
      {
         size_t bytes = imf->inUse - imf->position;

         if ( bytes > fwrite( imf->buffer + imf->position,
                              sizeof (char),
                              bytes,
                              output ))
            return -1;              /* Report error to caller        */

         imf->position += bytes;

      } /* for */

      return 0;                     /* Return success to caller      */

   } /* if ( imf->buffer != NULL ) */

#endif /* BIT32ENV */

/*--------------------------------------------------------------------*/
/*       We need to buffer the input to output, process as normal     */
/*       files                                                        */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                     Allocate a nice I/O bufer                      */
/*--------------------------------------------------------------------*/

   while (( ioBuf == NULL ) && (ioBufSize >= BUFSIZ))
   {

      ioBuf = malloc( ioBufSize );

      if (ioBuf == NULL)
      {
         if ( debuglevel > 2 )
            printerr( "imunload: malloc:" );

         ioBufSize /= 2;            /* Try for half the buffer       */
      }

   } /* while (( ioBuf == NULL ) && (ioBufSize >= BUFSIZ)) */

   if ( ioBuf == NULL )
   {
      printmsg(0,"imunload: Unablet to allocate I/O buffer for copy");
      panic();
   }

/*--------------------------------------------------------------------*/
/*                         Now copy the file                          */
/*--------------------------------------------------------------------*/

   while (! imeof( imf ))
   {
      size_t bytes = imread( ioBuf, sizeof (char), ioBufSize, imf );

      if ((imerror( imf ) || ((long) bytes < 0)))
      {
         free(ioBuf);
         return -1;
      }

      if ( bytes > fwrite( ioBuf,
                           sizeof (char),
                           (size_t) bytes,
                           output ))
      {
         free(ioBuf);
         return -1;              /* Report error to caller        */
      }

   } /* for */

   return 0;                     /* Return success to caller      */

} /* imunload */

/*--------------------------------------------------------------------*/
/*       e x e c u t e I M F C o m m a n d                            */
/*                                                                    */
/*       Execute a command with IMF input by copying the in-memory    */
/*       file to disk if needed.                                      */
/*--------------------------------------------------------------------*/

int executeIMFCommand( const char *command,
                       IMFILE  *imf,
                       const char *output,
                       const KWBoolean synchronous,
                       const KWBoolean foreground )
{
   char tempName[ FILENAME_MAX ];
   FILE *stream;
   int status;

   if ( imf->buffer == NULL )
      return executeCommand( command,
                             imf->filename,
                             output,
                             synchronous,
                             foreground );


   mktempname( tempName, "TMP" );

   stream = FOPEN( tempName, "w", TEXT_MODE );

   if ( stream == NULL )
   {
      printerr( tempName );
      return -1;
   }

   if ( imunload( stream, imf ) )
   {
      printerr( tempName );
      return -1;
   }

   fclose( stream );

   status = executeCommand( command,
                            tempName,
                            output,
                            synchronous,
                            foreground );

   unlink( tempName );

   return status;

}  /* executeIMFCommand */
