/*--------------------------------------------------------------------*/
/*       d e l i v e r n . c                                          */
/*                                                                    */
/*       RMAIL deliver module without TCP/IP support for              */
/*       UUPC/extended                                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-2000 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: delivern.c 1.3 1999/01/04 03:54:27 ahd Exp $
 *
 *    Revision history:
 *    $Log: delivern.c $
 *    Revision 1.3  1999/01/04 03:54:27  ahd
 *    Annual copyright change
 *
 *    Revision 1.2  1998/03/01 01:33:32  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.1  1997/05/11 18:15:50  ahd
 *    Initial revision
 *
 */

#define NOTCPIP            /* Don't include the TCP/IP suite      */

#include "deliver.c"
