/*--------------------------------------------------------------------*/
/*    p u s h p o p . h                                               */
/*                                                                    */
/*    Functions defined in pushpop.c for UUPC/extended                */
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
 *    $Id: pushpop.h 1.4 1997/03/31 07:05:47 ahd v1-12u $
 *
 *    Revision history:
 *    $Log: pushpop.h $
 *    Revision 1.4  1997/03/31 07:05:47  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.3  1996/01/01 20:58:01  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.2  1995/02/14 05:06:13  ahd
 *    Correct NT directory processing
 *
 */


void PushDir( const char *directory );

void PopDir( void );
