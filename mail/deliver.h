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
 *    $Id: deliver.h 1.6 1993/10/12 01:35:12 ahd Exp $
 *
 *    $Log: deliver.h $
 * Revision 1.6  1993/10/12  01:35:12  ahd
 * Normalize comments to PL/I style
 *
 * Revision 1.5  1993/09/20  04:51:31  ahd
 * TCP Support from Dave Watt
 * 't' protocol support
 * OS/2 2.x support (BC++ 1.0 for OS/2 support)
 *
 * Revision 1.4  1993/06/13  14:12:29  ahd
 * Save invoked program name and use it for recursive calls
 *
 * Revision 1.3  1993/04/11  00:36:13  ahd
 * Global edits for year, TEXT, etc.
 *
 * Revision 1.2  1992/12/04  01:03:49  ahd
 * Add system alias support
 *
 */

#define POSTMASTER "postmaster"

size_t Deliver( const char *input, /* Input file name                */
             char *address,           /* Target address               */
             const boolean sysalias,
                   boolean validate); /* Validate/forward local mail  */

extern KEWSHORT hops;

extern boolean remoteMail;

extern char *now;

extern char fromuser[];
extern char fromnode[];

extern  char *ruser;
extern  char *rnode;
extern  char *uuser;
extern char grade;

extern  char *myProgramName;
