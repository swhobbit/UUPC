#ifndef _GETOPT_H
#define _GETOPT_H
/*--------------------------------------------------------------------*/
/*          g e t o p t . h                                           */
/*                                                                    */
/*          Command line options parser header                        */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-2001 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: getopt.h 1.6 2000/05/12 12:40:50 ahd v1-13g $
 *
 *    Revision history:
 *    $Log: getopt.h $
 *    Revision 1.6  2000/05/12 12:40:50  ahd
 *    Annual copyright update
 *
 *    Revision 1.5  1999/01/04 03:55:03  ahd
 *    Annual copyright change
 *
 *    Revision 1.4  1998/03/01 01:26:31  ahd
 *    Annual Copyright Update
 *
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
