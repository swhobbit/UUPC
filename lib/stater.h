/*--------------------------------------------------------------------*/
/*    s t a t e r                                                     */
/*                                                                    */
/*    Report date and size of a file                                  */
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
 *    $Id: stater.h 1.6 1999/01/04 03:55:03 ahd Exp $
 *
 *    Revision history:
 *    $Log: stater.h $
 *    Revision 1.6  1999/01/04 03:55:03  ahd
 *    Annual copyright change
 *
 *    Revision 1.5  1998/03/01 01:27:44  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.4  1997/03/31 07:06:58  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.3  1996/01/01 20:57:15  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.2  1995/01/02 05:04:57  ahd
 *    Pass 2 of integrating SYS file support from Mike McLagan
 *
 */

time_t stater(const char *file, long *size);
