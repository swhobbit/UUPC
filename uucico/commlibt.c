/*--------------------------------------------------------------------*/
/*       c o m m l i b n . C                                          */
/*                                                                    */
/*       Generic communications library interface for UUPC/extended   */
/*       without TCP/IP support                                       */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-2001 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: commlibt.c 1.4 2000/05/12 12:32:55 ahd v1-13g $
 *
 *    Revision history:
 *    $Log: commlibt.c $
 *    Revision 1.4  2000/05/12 12:32:55  ahd
 *    Annual copyright update
 *
 *    Revision 1.3  1999/01/04 03:53:57  ahd
 *    Annual copyright change
 *
 *    Revision 1.2  1998/03/01 01:39:10  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.1  1997/05/11 18:15:50  ahd
 *    Initial revision
 *
 *    Revision 1.4  1997/04/24 01:33:05  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.3  1996/01/01 21:19:40  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.2  1994/12/22 00:32:55  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.1  1994/05/01 21:56:50  ahd
 *    Initial revision
 *
 */

#define TCPIP_ONLY             /* Only include the TCP/IP suite      */

#include "commlib.c"
