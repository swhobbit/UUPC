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
 *    $Id: lib.h 1.25 1994/12/27 20:50:28 ahd Exp $
 *
 *    Revision history:
 *    $Log: lib.h $
 */

void xmit_news( char *sysname, FILE *in_stream );

void batch_news(char *directory,char *artfile);

#endif /* _BATCH_H */
