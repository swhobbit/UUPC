/*--------------------------------------------------------------------*/
/*       c o m m l i b n . C                                          */
/*                                                                    */
/*       Generic communications library interface for UUPC/extended   */
/*       without TCP/IP support                                       */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1994 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: commlib.c 1.24 1994/04/27 00:02:15 ahd Exp $
 *
 *    Revision history:
 *    $Log: commlib.c $
 */

#define NOTCPIP            /* Don't include the TCP/IP suite      */

#include "commlib.c"
