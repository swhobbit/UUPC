/*--------------------------------------------------------------------*/
/*    a r b m a t h . c                                               */
/*                                                                    */
/*    Arbitary length math routines for UUPC/extended                 */
/*                                                                    */
/*    Copyright (c) 1990, 1991 Andrew H. Derbyshire                   */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                     Standard library includes                      */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
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
   size_t subscript;
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
   int subscript = digits;
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
   int subscript = digits;
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
