/*--------------------------------------------------------------------*/
/*    i m p o r t . h                                                 */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1993 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: import.h 1.3 1993/10/09 15:48:20 rhg Exp $
 *
 *    Revision history:
 *    $Log: import.h $
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

boolean ValidDOSName( const char *s,
                      const boolean longname );
