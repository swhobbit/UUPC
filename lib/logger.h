#ifndef _LOGGER_H
#define _LOGGER_H
/*--------------------------------------------------------------------*/
/*    l o g g e r . h                                                 */
/*                                                                    */
/*    Logging functions for UUPC/extended                             */
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
 *       $Id: lib.h 1.44 1998/04/27 01:57:29 ahd v1-13a $
 *
 *       Revision history:
 *       $Log: lib.h $
 */

void openlog( const char *log );

void copylog(void);

extern char *full_log_file_name;

#endif /* _LOGGER_H */
