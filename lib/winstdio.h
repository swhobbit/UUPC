#ifndef _WINSTDIO_H
#define _WINSTDIO_H

/*--------------------------------------------------------------------*/
/*       w i n s t d i . o h                                          */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-2002 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *       $Id: winstdio.h 1.4 2001/03/12 13:50:09 ahd v1-13k $
 *
 *       Revision history:
 *       $Log: winstdio.h $
 *       Revision 1.4  2001/03/12 13:50:09  ahd
 *       Annual copyright update
 *
 *       Revision 1.3  2000/05/12 12:40:50  ahd
 *       Annual copyright update
 *
 *       Revision 1.2  1999/01/04 03:55:03  ahd
 *       Annual copyright change
 *
 *       Revision 1.1  1998/04/20 02:48:54  ahd
 *       Initial revision
 *
 */

/*
 * Redefine some basic stdio routines that will be supplied by
 * winstdio.c to result in a windows, rather than console type app
 */

void win_exit(int x);

int real_main(int argc, char *argv[]);

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
