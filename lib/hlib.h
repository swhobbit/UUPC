#ifndef _HLIB_H

/*--------------------------------------------------------------------*/
/*       h l i b . h                                                  */
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
 *    $Id: hlib.h 1.8 1995/01/09 01:43:38 ahd Exp $
 *
 *    Revision history:
 *    $Log: hlib.h $
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

#define isAbsolutePath( path )                                       \
         (( *path == '/' ) ||                                        \
          ( *path == '\\' ) ||                                       \
          ( (isalpha(path[0]) && ( path[1] == ':' )) &&              \
            ((path[2] == '\\') || (path[2] == '/')) ))

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

#ifdef __IBMC__
#define _fsopen(n, m, s) fopen(n, m)
#endif

#endif /* _HLIB_H */
