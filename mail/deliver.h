/*--------------------------------------------------------------------*/
/*    d e l i v e r . h                                               */
/*                                                                    */
/*   Externally known functions in deliver.c, delivery subroutines    */
/*                in rmail component of UUPC/extended                 */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989 by Andrew H. Derbyshire.             */
/*                                                                    */
/*    Changes Copyright (c) 1990-1993 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: DELIVER.H 1.2 1992/12/04 01:03:49 ahd Exp $
 *
 *    $Log: DELIVER.H $
 * Revision 1.2  1992/12/04  01:03:49  ahd
 * Add system alias support
 *
 */

#define POSTMASTER "postmaster"

size_t Deliver( const char *input, /* Input file name                */
             char *address,           /* Target address                 */
             const boolean sysalias,
                   boolean validate); /* Validate/forward local mail    */

extern INTEGER hops;

extern boolean remoteMail;

extern char *now;

extern char fromuser[];
extern char fromnode[];

extern  char *ruser;
extern  char *rnode;
extern  char *uuser;
