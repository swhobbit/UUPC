/*--------------------------------------------------------------------*/
/*    s c r s i z e . h                                               */
/*                                                                    */
/*    Report screen size under OS/2                                   */
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
 *    $Id: scrsize.h 1.2 1995/01/29 16:43:42 ahd v1-12q $
 *
 *    $Log: scrsize.h $
 *    Revision 1.2  1995/01/29 16:43:42  ahd
 *    IBM C/Set compiler warnings
 *
 *    Revision 1.1  1992/11/27 14:36:53  ahd
 *    Initial revision
 *
 */

#define PAGESIZE 25        /* Value returned in case of error        */

unsigned short scrsize( void );
