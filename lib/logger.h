#ifndef _LOGGER_H
#define _LOGGER_H
/*--------------------------------------------------------------------*/
/*    l o g g e r . h                                                 */
/*                                                                    */
/*    Logging functions for UUPC/extended                             */
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
 *       $Id: logger.h 1.4 2000/05/12 12:40:50 ahd v1-13g $
 *
 *       Revision history:
 *       $Log: logger.h $
 *       Revision 1.4  2000/05/12 12:40:50  ahd
 *       Annual copyright update
 *
 *       Revision 1.3  1999/01/04 03:55:03  ahd
 *       Annual copyright change
 *
 *       Revision 1.2  1998/05/11 01:25:40  ahd
 *       Make copylog public
 *
 */

void openlog( const char *log );

void copylog(void);

extern char *full_log_file_name;

#endif /* _LOGGER_H */
