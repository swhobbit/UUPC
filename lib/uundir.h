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
/*    Changes Copyright (c) 1989-1993 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: uundir.h 1.3 1993/04/10 21:35:30 dmwatt Exp $
 *
 *    Revision history:
 *    $Log: uundir.h $
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

#ifndef FAMILY_API
typedef struct _FTIME           /* ftime */
        {
        unsigned short   twosecs : 5;
        unsigned short   minutes : 6;
        unsigned short   hours   : 5;
        } FTIME;

typedef struct _FDATE           /* fdate */
        {
        unsigned short   day     : 5;
        unsigned short   month   : 4;
        unsigned short   year    : 7;
        } FDATE;
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
