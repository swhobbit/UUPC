/*--------------------------------------------------------------------*/
/*       d e l i v e r n . c                                          */
/*                                                                    */
/*       RMAIL deliver module without TCP/IP support for              */
/*       UUPC/extended                                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1998 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: delivern.c 1.1 1997/05/11 18:15:50 ahd v1-12u $
 *
 *    Revision history:
 *    $Log: delivern.c $
 *    Revision 1.1  1997/05/11 18:15:50  ahd
 *    Initial revision
 *
 */

#define NOTCPIP            /* Don't include the TCP/IP suite      */

#include "deliver.c"
