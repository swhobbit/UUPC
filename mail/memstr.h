#ifndef _MEMSTR_H
#define _MEMSTR_H
/*--------------------------------------------------------------------*/
/*       m e m s t r . h                                              */
/*                                                                    */
/*       Search a buffer with possible NULL's for a substring         */
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
 *       $Id: lib.h 1.43 1998/04/20 02:48:54 ahd Exp $
 *
 *       Revision history:
 *       $Log: lib.h $
 */


#include <sys/types.h>

/*--------------------------------------------------------------------*/
/*    m e m s t r                                                     */
/*                                                                    */
/*    String a buffer with possible nulls for a substring             */
/*--------------------------------------------------------------------*/

char *
memstr(const char *haystack,
       const char *needle,
       size_t len);

#endif /* _MEMSTR_H */
