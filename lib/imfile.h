#ifndef _IMFILE_H
#define _IMFILE_H

/*--------------------------------------------------------------------*/
/*       i m f i l e . h                                              */
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
 *    $Id: lib.h 1.26 1994/12/31 03:51:25 ahd Exp $
 *
 *    Revision history:
 *    $Log: lib.h $
 */

#define IM_FLAG_READ   0x01
#define IM_FLAG_WRITE  0x02
#define IM_FLAG_RW     ( IM_FLAG_READ | IM_FLAG_WRITE )
#define IM_FLAG_ERROR  0x04

typedef struct _IMFILE
{
   char UUFAR *buffer;
   FILE *stream;
   char *filename;
   long length;
   long inUse;
   long position;
   char flag;

} IMFILE;

/*--------------------------------------------------------------------*/
/*                        Function prototypes                         */
/*--------------------------------------------------------------------*/

   IMFILE *imopen( const long length );
   int     imprintf( IMFILE *, const char *, ... );
   int     imclose( IMFILE * );
   int     imeof( IMFILE * );
   int     imerror( IMFILE * );
   char   *imgets( char *, int, IMFILE * );
   int     imputs( const char *, IMFILE * );
   size_t  imread( void *, size_t, size_t, IMFILE * );
   int     imseek( IMFILE *, long int, int );
   long    imtell( IMFILE * );
   size_t  imwrite( const void *, size_t, size_t, IMFILE * );
   void    imrewind( IMFILE * );
   long    imlength( IMFILE *imf );

#endif /* _IMIMFILE_H */
