/*--------------------------------------------------------------------*/
/*    a r b m a t h . c                                               */
/*                                                                    */
/*    Arbitary length math routines for UUPC/extended                 */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1999 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: arbmath.c 1.12 1999/01/04 03:52:28 ahd Exp $
 *
 *    Revision history:
 *    $Log: arbmath.c $
 *    Revision 1.12  1999/01/04 03:52:28  ahd
 *    Annual copyright change
 *
 *    Revision 1.11  1998/03/01 01:22:59  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.10  1997/03/31 06:58:00  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.9  1996/01/01 20:55:12  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.8  1995/01/29 14:07:59  ahd
 *    Clean up most IBM C/Set Compiler Warnings
 *
 *    Revision 1.7  1995/01/07 16:11:47  ahd
 *    Change KWBoolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.6  1994/12/22 00:07:10  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.5  1994/02/20 19:11:18  ahd
 *    IBM C/Set 2 Conversion, memory leak cleanup
 *
 *     Revision 1.3  1994/01/01  19:00:01  ahd
 *     Annual Copyright Update
 *
 *     Revision 1.2  1993/10/09  15:46:15  rhg
 *     ANSIify the source
 *
 */

/*--------------------------------------------------------------------*/
/*                     Standard library includes                      */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"

#include "arbmath.h"

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

RCSID("$Id$");

/*--------------------------------------------------------------------*/
/*    a d i v                                                         */
/*                                                                    */
/*    Perform arbitary length division                                */
/*                                                                    */
/*    Returns true if input number was non-zero                       */
/*--------------------------------------------------------------------*/

KWBoolean adiv( unsigned char *number,
             const unsigned divisor,
             unsigned *remain,
             const unsigned digits)
{
   unsigned subscript;
   KWBoolean nonzero = KWFalse;
   *remain = 0;

   for ( subscript = 0; subscript < digits; subscript++)
   {
      unsigned digit =  *remain * 0x100 + number[subscript];
      if ( number[subscript] )
         nonzero = KWTrue;
      *remain = digit % divisor;
      number[subscript] =  (unsigned char) (digit / divisor);
   } /* for */
   return nonzero;
} /* div */

/*--------------------------------------------------------------------*/
/*    m u l t                                                         */
/*                                                                    */
/*    Perform arbitary length multiplication                          */
/*--------------------------------------------------------------------*/

void mult(unsigned char *number,
      const unsigned range,
      const unsigned digits)
{
   unsigned subscript = digits;
   unsigned carry = 0;

   while( subscript-- > 0)
   {
      unsigned digit = number[subscript] * range + carry;
      number[subscript] = (unsigned char) (digit % 0x100);
      carry = digit / 0x100;
   } /* while */

   if ( carry != 0 )     /* Big trouble if overflow occurs   */
      panic();
} /* mult */

/*--------------------------------------------------------------------*/
/*    a d d                                                           */
/*                                                                    */
/*    Perform arbitiary length addition                               */
/*--------------------------------------------------------------------*/

void add(unsigned char *number,
      const unsigned range,
      const unsigned digits)
{
   unsigned subscript = digits;
   unsigned carry = range;

   while(( carry > 0) && ( subscript-- > 0))
   {
      unsigned digit = number[subscript] + carry;
      number[subscript] = (unsigned char) (digit % 0x100);
      carry = digit / 0x100;
   } /* while */

   if ( carry != 0 )     /* Big trouble if overflow occurs   */
      panic();

} /* add */
