#ifndef _HLIB_H

/*--------------------------------------------------------------------*/
/*       h l i b . h                                                  */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1996 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: hlib.h 1.12 1995/03/12 16:39:40 ahd v1-12q $
 *
 *    Revision history:
 *    $Log: hlib.h $
 *    Revision 1.12  1995/03/12 16:39:40  ahd
 *    Pass in file name information for OS/2 remove error processingh
 *
 *    Revision 1.11  1995/02/12 23:39:45  ahd
 *    compiler cleanup, NNS C/news support, optimize dir processing
 *
 *    Revision 1.10  1995/01/30 04:05:39  ahd
 *    Additional compiler warning fixes, optimize path normalizing
 *
 *    Revision 1.9  1995/01/28 23:13:37  ahd
 *    Add isAbsolutePath()
 *
 *    Revision 1.8  1995/01/09 01:43:38  ahd
 *    Rename sequence file to have extension of .DAT; this keeps it
 *    from mixing in with most directory names when sorting by extension
 *
 *    Revision 1.7  1994/12/31 03:51:25  ahd
 *    First pass of integrating Mike McLagan's news SYS file suuport
 *
 *    Revision 1.6  1994/12/22 00:12:57  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.5  1994/02/28 01:11:02  ahd
 *    Change BINARY_MODE to IMAGE_MODE,  allow longer addresses in real OS's
 *
 */

#define SAME 0

#define SFILENAME "SEQF.DAT"
#define DOTFORWARD "forward"

#define SEPCHAR '/'

#define CONSOLE "CON"           /* "filename" of the console */

/*--------------------------------------------------------------------*/
/*              Define the device name of the bit bucket              */
/*--------------------------------------------------------------------*/

#ifdef WIN32
#define BIT_BUCKET "NUL:"
#else
#define BIT_BUCKET "/dev/nul"    /* NOTE: DOS uses only one L in NUL */
#endif

char *normalize( const char *path );

char getDrive( const char *currentDirectory );

KWBoolean isAbsolutePath( const char *path);

#define denormalize( path ) { char *xxp = path; \
   while ((xxp = strchr(xxp,'/')) != NULL)  \
      *xxp++ = '\\';  }

#define renormalize( path ) { char *xxp = path; \
   while ((xxp = strchr(xxp,'\\')) != NULL)  \
      *xxp++ = '/';  }

#ifdef __IBMC__
#define TEXT_MODE    ""
#else
#define TEXT_MODE    "t"
#endif

#define IMAGE_MODE   "b"

#ifdef BIT32ENV
#define MAXADDR   512            /* Max length of address strings      */
#else
#define MAXADDR   128            /* Max length of address strings      */
#endif

#define MAXLIST   99             /* Max number of entries in arg
                                    arrays                             */

void mkfilename(char *pathname,
                const char *path,
                const char *name);                             /* ahd  */

char *mktempname( char *input,
                  const char *extension);                      /* ahd  */

char *mkdirfilename(char *input,
                    const char *dir,
                    const char *extension);                     /* gmm */

char *mkmailbox(char *buf, const char *userid);

int filebkup( const char *input );

/*--------------------------------------------------------------------*/
/*          Handle special file removal processing for OS/2           */
/*--------------------------------------------------------------------*/

#if defined(__OS2__)

#define REMOVE(x) removeOS2File(x, __FILE__, __LINE__ )

int removeOS2File( const char *fname, const char *file, const size_t line );

#else
#define REMOVE(x) unlink(x)
#endif

#endif /* _HLIB_H */
