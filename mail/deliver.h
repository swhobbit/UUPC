/*--------------------------------------------------------------------*/
/*    d e l i v e r . h                                               */
/*                                                                    */
/*   Externally known functions in deliver.c, delivery subroutines    */
/*                in rmail component of UUPC/extended                 */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989 by Andrew H. Derbyshire.             */
/*                                                                    */
/*    Changes Copyright (c) 1990-1992 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: LIB.H 1.3 1992/12/01 04:39:34 ahd Exp $
 *
 *    $Log: LIB.H $
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
