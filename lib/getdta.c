/*--------------------------------------------------------------------*/
/*    g e t d t a . c                                                 */
/*                                                                    */
/*    Get and set disk transfer address in MS C 6.0                   */
/*                                                                    */
/*    Changes and compilation copyright (c) 1989-1993,                */
/*    Andrew H. Derbyshire                                            */
/*                                                                    */
/*    Barry Schwartz <trashman@crud.hawkmoon.mn.org> rewrote the      */
/*    getdta function into something that works.      3/7/92          */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                  C run-time library include files                  */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <dos.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "getdta.h"

/*--------------------------------------------------------------------*/
/*    s e t d t a                                                     */
/*                                                                    */
/*    Set disk transfer address.                                      */
/*--------------------------------------------------------------------*/

void setdta( char far *dtaptr )
{
   union REGS inregs, outregs;
   struct SREGS segregs;

   /* set DTA address to our buffer */
   inregs.h.ah = 0x1a;
   segregs.ds = FP_SEG(dtaptr);
   inregs.x.dx = FP_OFF(dtaptr);
   intdosx(&inregs, &outregs, &segregs);
} /* setdta */

/*--------------------------------------------------------------------*/
/*    g e t d t a                                                     */
/*                                                                    */
/*    Get disk transfer address.                                      */
/*--------------------------------------------------------------------*/

char far *getdta( void )
{
   /* In Quick C 2.0, a function written entirely in assembly
    * language can freely alter AX, BX, CX, DX, ES, and flags.
    * The return value should be put in DS:AX. */

   _asm \
   {
   /* Execute a "Get DTA" service. */
   mov   ah,2FH
   int   21H

   /* Move the result from (ES:BX) to (DX:AX) so it will act
    * as the return value for the function. */
   mov   ax,es
   mov   dx,ax
   mov   ax,bx
   }
} /* getdta */
