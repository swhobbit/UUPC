/*--------------------------------------------------------------------*/
/*    a r b m a t h . c                                               */
/*                                                                    */
/*    Arbitary length math routines for UUPC/extended                 */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1994 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: arbmath.c 1.3 1994/01/01 19:00:01 ahd Exp $
 *
 *    Revision history:
 *    $Log: arbmath.c $
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

currentfile();

/*--------------------------------------------------------------------*/
/*    a d i v                                                         */
/*                                                                    */
/*    Perform arbitary length division                                */
/*                                                                    */
/*    Returns true if input number was non-zero                       */
/*--------------------------------------------------------------------*/

boolean adiv( unsigned char *number,
             const unsigned divisor,
             unsigned *remain,
             const unsigned digits)
{
   unsigned subscript;
   boolean nonzero = FALSE;
   *remain = 0;

   for ( subscript = 0; subscript < digits; subscript++)
   {
      unsigned digit =  *remain * 0x100 + number[subscript];
      nonzero = nonzero || number[subscript];
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
