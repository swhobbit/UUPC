#ifndef _BATCH_H
#define _BATCH_H

/*--------------------------------------------------------------------*/
/*       b a t c h . h                                                */
/*                                                                    */
/*       Include file for batching news files                         */
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
 *    $Id: batch.h 1.6 1999/01/04 03:55:33 ahd Exp $
 *
 *    Revision history:
 *    $Log: batch.h $
 *    Revision 1.6  1999/01/04 03:55:33  ahd
 *    Annual copyright change
 *
 *    Revision 1.5  1998/03/01 01:31:14  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.4  1997/04/24 01:40:01  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.3  1996/01/01 21:09:16  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.2  1995/01/02 05:04:57  ahd
 *    Pass 2 of integrating SYS file support from Mike McLagan
 *
 *    Revision 1.1  1994/12/31 03:51:25  ahd
 *    First pass of integrating Mike McLagan's news SYS file suuport
 *
 */

#include "sys.h"

void process_batch(const struct sys *node,
                   const char *system,
                   const char *artnames);

#endif /* _BATCH_H */
