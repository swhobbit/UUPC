/*--------------------------------------------------------------------*/
/*       c o m m l i b n . C                                          */
/*                                                                    */
/*       Generic communications library interface for UUPC/extended   */
/*       without TCP/IP support                                       */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1996 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: commlibn.c 1.2 1994/12/22 00:32:55 ahd v1-12q $
 *
 *    Revision history:
 *    $Log: commlibn.c $
 *    Revision 1.2  1994/12/22 00:32:55  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.1  1994/05/01 21:56:50  ahd
 *    Initial revision
 *
 */

#define NOTCPIP            /* Don't include the TCP/IP suite      */

#include "commlib.c"
