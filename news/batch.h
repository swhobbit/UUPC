#ifndef _BATCH_H
#define _BATCH_H

/*--------------------------------------------------------------------*/
/*       b a t c h . h                                                */
/*                                                                    */
/*       Include file for batching news files                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1995 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: batch.h 1.1 1994/12/31 03:51:25 ahd Exp $
 *
 *    Revision history:
 *    $Log: batch.h $
 *    Revision 1.1  1994/12/31 03:51:25  ahd
 *    First pass of integrating Mike McLagan's news SYS file suuport
 *
 */

#include "sys.h"

void process_batch(const struct sys *node,
                   const char *system,
                   const char *artnames);

#endif /* _BATCH_H */
