#ifndef _MEMSTR_H
#define _MEMSTR_H
/*--------------------------------------------------------------------*/
/*       m e m s t r . h                                              */
/*                                                                    */
/*       Search a buffer with possible NULL's for a substring         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-2000 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *       $Id: memstr.h 1.2 1999/01/04 03:56:09 ahd Exp $
 *
 *       Revision history:
 *       $Log: memstr.h $
 *       Revision 1.2  1999/01/04 03:56:09  ahd
 *       Annual copyright change
 *
 *       Revision 1.1  1998/04/24 03:34:25  ahd
 *       Initial revision
 *
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
