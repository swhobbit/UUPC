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
 *    $Id: imfile.c 1.1 1995/01/07 15:43:07 ahd Exp $
 *
 *    Revision history:
 *    $Log: imfile.c $
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

/*--------------------------------------------------------------------*/
/*                         Manifest constants                         */
/*--------------------------------------------------------------------*/

#ifdef BIT32ENV
#define IM_MAX_LENGTH (1024*1024)
#else
#define IM_MAX_LENGTH (64*1024)
#endif

currentfile();

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

   imf->stream = FOPEN( imf->filename,
                        "w+",
                        IMAGE_MODE );

   if ( imf->stream == NULL )
   {
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
   IMFILE *imf = malloc( sizeof imf );

/*--------------------------------------------------------------------*/
/*          Allocate our control structure and initialize it          */
/*--------------------------------------------------------------------*/

   checkref( imf );
   memset( imf, 0, sizeof imf );

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

      if ( imf->length <= 0 )
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

   if ( imf->buffer != NULL )
   {
      _ffree( imf->buffer );
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

      if ( strlen( buffer ) > 4096 )
      {
         printmsg(0,"Memory overflow processing im memory file" );
         panic();                /* We corrupted the stack!          */
      }

      return imputs( buffer, imf );

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
      return imf->inUse > imf->position;

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

   if ( imf->buffer == NULL )
      return fgets( userBuffer, userLength, imf->stream );

   if ( imerror( imf ) || imeof( imf ))
      return NULL;

/*--------------------------------------------------------------------*/
/*               Select the string from our own buffer                */
/*--------------------------------------------------------------------*/

   stringLength = imf->inUse - imf->position;

   if ( stringLength > (userLength -1 ))
      stringLength = userLength;

   p = imf->buffer + imf->position;

   while ( (long) subscript < stringLength )
   {
      if ( p[subscript] == '\n' )
         break;
      else
         subscript++;
   }

   MEMCPY( userBuffer, imf->buffer + imf->position, subscript );
   userBuffer[ subscript ] = '\0';
   imf->position += subscript;

   return userBuffer;

} /* imgets */

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

   if ( imf->buffer == NULL )
      return fread( userBuffer, objectSize, objectCount, imf->stream );

   if ( imeof( imf ) )
      return 0;

   if ( imerror( imf ) )
      return -1;

   if ( (long) bytes <= (imf->inUse - imf->position ))
   {
      MEMCPY( userBuffer,
              imf->buffer + imf->position,
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

   if ( bytes < 0 )
   {
      errno = EINVAL;
      return -1;
   }

/*--------------------------------------------------------------------*/
/*       Verify we have the memory to write in, automatically         */
/*       switching to disk based data if we run low.                  */
/*--------------------------------------------------------------------*/

   if ( imReserve( imf, bytes ) )
      return -1;

/*--------------------------------------------------------------------*/
/*                        Actually write the data                     */
/*--------------------------------------------------------------------*/

   if ( imf->buffer == NULL )
      return fwrite( userBuffer, objectSize, objectCount, imf->stream );
   else {

      MEMCPY( imf->buffer + imf->position, userBuffer, (size_t) bytes );
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
         printmsg(4,"Shortening IMF %p from %ld to %ld bytes",
                     imf,
                     imf->length,
                     imf->inUse );
#endif

         imf->buffer = _frealloc( imf->buffer, (size_t) imf->inUse );
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
   if ( imf->buffer == NULL )
   {
      fflush( imf->stream );
      return filelength( fileno( imf->stream ) );
   }
   else
      return imf->inUse;

}   /* imlength */
