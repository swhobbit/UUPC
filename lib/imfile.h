#ifndef _IMFILE_H
#define _IMFILE_H

/*--------------------------------------------------------------------*/
/*       i m f i l e . h                                              */
/*                                                                    */
/*       In-memory file operations, with backing disk store, for      */
/*       temporary files                                              */
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
 *    $Id: imfile.h 1.13 1999/01/04 03:55:03 ahd Exp $
 *
 *    Revision history:
 *    $Log: imfile.h $
 *    Revision 1.13  1999/01/04 03:55:03  ahd
 *    Annual copyright change
 *
 *    Revision 1.12  1998/04/19 15:33:53  ahd
 *    Add flag to denote when file size message was issued
 *
 *    Revision 1.11  1998/03/09 01:20:41  ahd
 *    Add flag for imtell() being called
 *
 *    Revision 1.10  1998/03/01 01:26:46  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.9  1997/11/24 02:59:06  ahd
 *    *** empty log message ***
 *
 *    Revision 1.8  1997/03/31 07:00:01  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.7  1996/01/20 13:11:22  ahd
 *    Allow specifying text/binary mode when processing an in-memory file
 *
 *    Revision 1.6  1996/01/01 20:58:15  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.5  1995/02/20 17:30:32  ahd
 *    16 bit compiler warning cleanup
 *
 *    Revision 1.4  1995/01/29 16:43:42  ahd
 *    IBM C/Set compiler warnings
 *
 *    Revision 1.3  1995/01/28 22:08:30  ahd
 *    Add chsize function
 *
 *    Revision 1.2  1995/01/08 19:54:01  ahd
 *    Add in-memory files to RMAIL
 *
 *    Revision 1.1  1995/01/07 15:48:31  ahd
 *    Initial revision
 *
 */

#define IM_FLAG_READ   0x01
#define IM_FLAG_WRITE  0x02
#define IM_FLAG_RW     ( IM_FLAG_READ | IM_FLAG_WRITE )
#define IM_FLAG_ERROR  0x04
#define IM_FLAG_TEXT   0x08
#define IM_FLAG_TELL   0x10         /* imtell() called for file */
#define IM_FLAG_DISKR  0x20         /* We reported the file is on disk */

#define IM_FLAG_ALL    0xff

typedef struct _IMFILE
{
   char UUFAR *buffer;
   FILE *stream;
   char *filename;
   unsigned long length;
   unsigned long inUse;
   unsigned long position;
   unsigned int  flag;

} IMFILE;

/*--------------------------------------------------------------------*/
/*                        Function prototypes                         */
/*--------------------------------------------------------------------*/

   IMFILE *imopen( const long length, const char *mode);

   int     imprintf( IMFILE *, const char *, ... );

   int     imclose( IMFILE * );

   int     imeof( IMFILE * );

   int     imerror( IMFILE * );

   int     imchsize( IMFILE *imf, long length );

   char   *imgets( char *, int, IMFILE * );

   int     imputs( const char *, IMFILE * );

   size_t  imread( void *, size_t, size_t, IMFILE * );

   int     imseek( IMFILE *, long int, int );

   long    imtell( IMFILE * );

   size_t  imwrite( const void *, size_t, size_t, IMFILE * );

   void    imrewind( IMFILE * );

   long    imlength( IMFILE *imf );

   int     imputc( int, IMFILE * );

   int     imunload( FILE *output, IMFILE *input );

   int     executeIMFCommand( const char *command,
                              IMFILE *imf,
                              const char *output,
                              const KWBoolean synchronous,
                              const KWBoolean foreground );

#define imInMemory( imf ) ((imf)->buffer != NULL )

#endif /* _IMIMFILE_H */
