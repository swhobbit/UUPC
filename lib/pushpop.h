/*--------------------------------------------------------------------*/
/*    p u s h p o p . h                                               */
/*                                                                    */
/*    Functions defined in pushpop.c for UUPC/extended                */
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
 *    $Id: pushpop.h 1.8 2001/03/12 13:50:09 ahd v1-13k $
 *
 *    Revision history:
 *    $Log: pushpop.h $
 *    Revision 1.8  2001/03/12 13:50:09  ahd
 *    Annual copyright update
 *
 *    Revision 1.7  2000/05/12 12:40:50  ahd
 *    Annual copyright update
 *
 *    Revision 1.6  1999/01/04 03:55:03  ahd
 *    Annual copyright change
 *
 *    Revision 1.5  1998/03/01 01:27:10  ahd
 *    Annual Copyright Update
 *
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
