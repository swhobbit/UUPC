/*--------------------------------------------------------------------*/
/*       h l i b . h                                                  */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1994 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: lib.h 1.20 1994/02/20 19:16:21 ahd Exp $
 *
 *    Revision history:
 *    $Log: lib.h $
 */

#define SAME 0

#define SFILENAME "SEQF"
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
                  char *extension);                            /* ahd  */

char *mkmailbox(char *buf, const char *userid);

int filebkup( const char *input );

#ifdef __IBMC__
#define _fsopen(n, m, s) fopen(n, m)
#endif
