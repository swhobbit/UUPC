#ifndef _GETOPT_H
#define _GETOPT_H
/*--------------------------------------------------------------------*/
/*          g e t o p t . h                                           */
/*                                                                    */
/*          Command line options parser header                        */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1998 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: getopt.h 1.3 1997/03/31 06:59:25 ahd v1-12u $
 *
 *    Revision history:
 *    $Log: getopt.h $
 *    Revision 1.3  1997/03/31 06:59:25  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.2  1996/03/18 03:48:14  ahd
 *    Allow compilation under C++ compilers
 *
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
