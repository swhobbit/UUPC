#ifndef UUNDIR_H
#define UUNDIR_H

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
 *    $Id: lib.h 1.4 1993/03/06 23:09:50 ahd Exp $
 *
 *    Revision history:
 *    $Log: lib.h $
 */

#define MSDOS_MAXNAMLEN 12

#define MAXNAMLEN FILENAME_MAX   // For OS/2, Win/NT compatability

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
