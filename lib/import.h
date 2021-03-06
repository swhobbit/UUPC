/*--------------------------------------------------------------------*/
/*    i m p o r t . h                                                 */
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
 *    $Id: import.h 1.13 2001/03/12 13:50:09 ahd v1-13k $
 *
 *    Revision history:
 *    $Log: import.h $
 *    Revision 1.13  2001/03/12 13:50:09  ahd
 *    Annual copyright update
 *
 *    Revision 1.12  2000/05/12 12:40:50  ahd
 *    Annual copyright update
 *
 *    Revision 1.11  1999/01/04 03:55:03  ahd
 *    Annual copyright change
 *
 *    Revision 1.10  1998/03/01 01:26:49  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.9  1997/03/31 07:00:10  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.8  1996/01/01 20:58:35  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.7  1995/01/07 16:15:57  ahd
 *    Change boolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.6  1994/12/22 00:13:09  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.5  1994/01/01 19:08:58  ahd
 *    Annual Copyright Update
 *
 *     Revision 1.4  1993/10/12  01:21:25  ahd
 *     Normalize comments to PL/I style
 *
 *     Revision 1.3  1993/10/09  15:48:20  rhg
 *     ANSIfy the source
 *
 *     Revision 1.2  1993/09/02  12:08:49  ahd
 *     HPFS Support
 *
 */

/*--------------------------------------------------------------------*/
/*    Constants used for mapping between character sets; changing     */
/*    any of these parameters requires clearing all the files from    */
/*    UUPC/extended spool directories.                                */
/*--------------------------------------------------------------------*/

#define DOSCHARS  "!#$%&'()-0123456789@^_`abcdefghijklmnopqrstuvwxyz{}~"
                              /* List of allowed characters in MS-DOS
                                 file names; from Page 1.15 of Zenith
                                 MS-DOS 3 manual                      */
#define UNIX_START_C '#'      /* First character in the UNIX char set */
                              /* we map to the DOS character set      */
#define UNIX_END_C   'z'      /* last UNIX char we map                */

/*--------------------------------------------------------------------*/
/*    Routines defined in import.c                                    */
/*--------------------------------------------------------------------*/

void importpath(char *host,
                const char *canon,
                const char *remote);

KWBoolean ValidDOSName( const char *s,
                      const KWBoolean longname );
