#ifndef _GETOPT_H
#define _GETOPT_H
/*--------------------------------------------------------------------*/
/*          g e t o p t . h                                           */
/*                                                                    */
/*          Command line options parser header                        */
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
 *    $Id: lib.h 1.35 1996/01/07 14:18:18 ahd v1-12r $
 *
 *    Revision history:
 *    $Log: lib.h $
 */


#define getopt(x,y,z)   GETOPT(x,y,z)  /* Avoid GNU conflict   */

#ifdef __cplusplus
   extern "C" {
#endif

extern int getopt(int argc, char **argv, char *opts);

extern int optind;
extern char *optarg;

#ifdef __cplusplus
   }
#endif

#endif  /* _GETOPT_H */
