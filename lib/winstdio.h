#ifndef _WINSTDIO_H
#define _WINSTDIO_H

/*--------------------------------------------------------------------*/
/*       w i n s t d i . o h                                          */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1999 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *       $Id: winstdio.h 1.1 1998/04/20 02:48:54 ahd v1-13f ahd $
 *
 *       Revision history:
 *       $Log: winstdio.h $
 *       Revision 1.1  1998/04/20 02:48:54  ahd
 *       Initial revision
 *
 */

/*
 * Redefine some basic stdio routines that will be supplied by
 * winstdio.c to result in a windows, rather than console type app
 */

void win_exit(int x);

real_main(int argc, char *argv[]);

int win_printf(char *fmt, ...);

int win_fprintf(FILE *stream, char *fmt, ...);

int win_isatty(int fileno);

#ifdef ferror
#undef ferror
#endif
int win_ferror( FILE *stream );

int win_fputc( int c, FILE *stream );

int win_vfprintf( FILE *stream, const char *format, va_list argptr );

void win_setTitle( const char *text );

int real_main(int argc, char **argv);

#ifndef NO_UUIO_REDEFINES
#define vfprintf win_vfprintf
#define exit win_exit
#define main real_main
#define printf win_printf
#define fprintf win_fprintf
#define isatty win_isatty
#define ferror win_ferror
#define fputc win_fputc
#endif

#endif /* _WINSTDIO_H */
