#ifndef UUNDIR_H
#define UUNDIR_H

#ifdef WIN32
#include <time.h>
#endif

/*--------------------------------------------------------------------*/
/*       u u n d i r . h                                              */
/*                                                                    */
/*       UUPC/extended directory search functions                     */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       ndir.h for MS-DOS by Samuel Lam <skl@van-bc.UUCP>, June/87   */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1998 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: uundir.h 1.12 1997/04/24 00:58:49 ahd v1-12u $
 *
 *    Revision history:
 *    $Log: uundir.h $
 *
 *    Revision 1.10  1996/01/01 20:56:03  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.9  1995/07/21 13:28:20  ahd
 *    Correct compiler warnings
 *
 *    Revision 1.8  1994/12/22 00:14:26  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.7  1994/01/01 19:10:14  ahd
 *    Annual Copyright Update
 *
 *     Revision 1.6  1993/12/23  03:19:49  rommel
 *     OS/2 32 bit support for additional compilers
 *
 *     Revision 1.5  1993/11/06  17:57:46  rhg
 *     Drive Drew nuts by submitting cosmetic changes mixed in with bug fixes
 *
 *     Revision 1.4  1993/10/12  01:22:27  ahd
 *     Normalize comments to PL/I style
 *
 *     Revision 1.3  1993/04/10  21:35:30  dmwatt
 *     Windows/NT fixes
 *
 *     Revision 1.2  1993/04/05  04:38:55  ahd
 *     Add time stamp/size information
 *
 */

#define MSDOS_MAXNAMLEN 12

#define MAXNAMLEN FILENAME_MAX   /* For OS/2, Win/NT compatability     */

struct direct {
   long d_ino;
   time_t d_modified;
   long  d_size;
   short d_reclen;
   short d_namlen;
   char d_name[MAXNAMLEN];
};

#if defined(FAMILYAPI) || defined(__OS2__)

#define INCL_BASE
#include <os2.h>

#else

#if _MSC_VER >= 800
#pragma warning(disable:4214)   /* suppress non-standard bit-field warnings */
#elif _MSC_VER >= 700
#pragma warning(disable:4001)   /* suppress non-standard bit-field warnings */
#endif

typedef struct _FTIME           /* ftime */
        {
        unsigned twosecs : 5;
        unsigned minutes : 6;
        unsigned hours   : 5;
        } FTIME;

typedef struct _FDATE           /* fdate */
        {
        unsigned day     : 5;
        unsigned month   : 4;
        unsigned year    : 7;
        } FDATE;

#if _MSC_VER >= 800
#pragma warning(default:4214)   /* restore non-standard bit-field warnings */
#elif _MSC_VER >= 700
#pragma warning(disable:4001)   /* restore non-standard bit-field warnings */
#endif

#endif

typedef struct {
   char filereserved[21];
   char fileattr;
   FTIME filetime;
   FDATE filedate;
   long filesize;
   char filename[MSDOS_MAXNAMLEN + 1];
} DTA;

typedef struct {
   char dirid[4];
   struct direct dirent;
   DTA dirdta;
   int dirfirst;
} DIR;

extern DIR *opendirx(const char *dirname, char *pattern);

#define opendir(x) opendirx(x, "*.*")

extern struct direct *readdir(DIR *dirp);
extern void closedir(DIR *dirp);

#endif
