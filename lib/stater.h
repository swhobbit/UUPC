/*--------------------------------------------------------------------*/
/*    s t a t e r                                                     */
/*                                                                    */
/*    Report date and size of a file                                  */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1997 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: stater.h 1.3 1996/01/01 20:57:15 ahd v1-12r $
 *
 *    Revision history:
 *    $Log: stater.h $
 *    Revision 1.3  1996/01/01 20:57:15  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.2  1995/01/02 05:04:57  ahd
 *    Pass 2 of integrating SYS file support from Mike McLagan
 *
 */

time_t stater(const char *file, long *size);
