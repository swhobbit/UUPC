/*--------------------------------------------------------------------*/
/*       d e l i v e r n . c                                          */
/*                                                                    */
/*       RMAIL deliver module without TCP/IP support for              */
/*       UUPC/extended                                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1997 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: commlibn.c 1.4 1997/04/24 01:33:05 ahd Exp $
 *
 *    Revision history:
 *    $Log: commlibn.c $
 */

#define NOTCPIP            /* Don't include the TCP/IP suite      */

#include "deliver.c"
