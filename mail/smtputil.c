/*--------------------------------------------------------------------*/
/*       s m t p u t i l . c                                          */
/*                                                                    */
/*       SMTP commands utility functions                              */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1998 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *       $Id: smtputil.c 1.2 1997/11/25 05:05:06 ahd v1-12u $
 *
 *       Revision History:
 *       $Log: smtputil.c $
 *       Revision 1.2  1997/11/25 05:05:06  ahd
 *       More robust SMTP daemon
 *
 *       Revision 1.1  1997/11/24 02:52:26  ahd
 *       First working SMTP daemon which delivers mail
 *
 *
 */

/*--------------------------------------------------------------------*/
/*                           Include files                            */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"
#include "sysalias.h"
#include "usertabl.h"
#include "hostable.h"
#include "address.h"
#include "smtputil.h"

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

RCSID("$Id: smtputil.c 1.2 1997/11/25 05:05:06 ahd v1-12u $");

currentfile();

/*--------------------------------------------------------------------*/
/*          s t r i p A d d r e s s                                   */
/*                                                                    */
/*          Strip delimiters off address and check it's length        */
/*--------------------------------------------------------------------*/

KWBoolean
stripAddress( char *address, char response[MAXADDR] )
{
   int brackets = 0;
   int len = strlen( address );

   *response = '\0';

/*--------------------------------------------------------------------*/
/*                      Handle our very space case                    */
/*--------------------------------------------------------------------*/

   if ( equal( address, "<>" ))     /* Postmaster?                   */
      return KWTrue;                /* Yes --> Leave intact          */

/*--------------------------------------------------------------------*/
/*                     Handle various syntax checks                   */
/*--------------------------------------------------------------------*/

   if (strchr(address, '@') == NULL)   /* Any host delimiters?       */
   {
      strcpy( response, "Address does not contain at sign (@)");
      return KWFalse;
   }

   if (strpbrk(address, WHITESPACE))
   {
      strcpy( response, "Address contains white space not support by "
                        "UUCP mailers");
      return KWFalse;
   }

   if (strpbrk(address, "()"))
   {
      strcpy( response, "Address contains parantheses, not supported");
      return KWFalse;
   }

/*--------------------------------------------------------------------*/
/*              Strip angle brackets, checking for balance            */
/*--------------------------------------------------------------------*/

   if ( address[0] == '<' )
   {
      brackets++;
      MEMMOVE( address, address + 1, --len );
   }

   if ( address[len - 1] == '>' )
   {
      brackets++;
      address[--len] = '\0';
   }

   if ( brackets == 1 )
   {
      strcpy( response, "Unbalanced angle brackets (<>)");
      return KWFalse;
   }

/*--------------------------------------------------------------------*/
/*                  Look for extra bogus angle brackets               */
/*--------------------------------------------------------------------*/

   if (strpbrk(address, "<>"))
   {
      strcpy( response, "Address contains extra angle brackets, "
                        "not supported");
      return KWFalse;
   }

/*--------------------------------------------------------------------*/
/*                           Verify the length                        */
/*--------------------------------------------------------------------*/

   if ( len >= MAXADDR )
   {
      strcpy( response, "Address is too long for processing");
      return KWFalse;

   }

/*--------------------------------------------------------------------*/
/*       The address is safe for other (less paranoid) returns to     */
/*       process it                                                   */
/*--------------------------------------------------------------------*/

   return KWTrue;

} /* stripAddress */

/*--------------------------------------------------------------------*/
/*       i s V a l i d A d d r e s s                                   */
/*                                                                    */
/*       Report if an address is valid for processing                 */
/*--------------------------------------------------------------------*/

KWBoolean
isValidAddress( const char *address,
                char response[MAXADDR],
                KWBoolean *ourProblem)
{
   static const char mName[] = "isValidAddress";
   char node[MAXADDR];
   char user[MAXADDR];
   struct HostTable *hostp;

/*--------------------------------------------------------------------*/
/*                    Perform basic syntax checks                     */
/*--------------------------------------------------------------------*/

   if ( equal( address, "<>" ))
   {
      strcpy( response, "Local postmaster");
      return KWTrue;
   }

/*--------------------------------------------------------------------*/
/*                       Parse the address down                       */
/*--------------------------------------------------------------------*/

   if ( ! tokenizeAddress(address, response, node, user) )
      return KWFalse;               /* Message already in buffer     */

   printmsg(4, "%s: Address %s is user %s at %s via %s",
               mName,
               address,
               user,
               node,
               response );

   hostp = checkname( response );

/*--------------------------------------------------------------------*/
/*                      Handle gateway delivery                       */
/*--------------------------------------------------------------------*/

   if ( (hostp != BADHOST) && (hostp->status.hstatus == HS_GATEWAYED))
   {
      strcpy( response, "Supported via gateway");
      return KWTrue;
   }

/*--------------------------------------------------------------------*/
/*                 Handle SMTP delivery, if supported                 */
/*--------------------------------------------------------------------*/

   if ( (hostp != BADHOST) && (hostp->status.hstatus == HS_SMTP))
   {
      strcpy( response, "Address is remote (SMTP)");
      return KWTrue;
   }

/*--------------------------------------------------------------------*/
/*                       Handle local delivery                        */
/*--------------------------------------------------------------------*/

   if (equal(response, E_nodename)) /* Route via Local node?          */
   {
      if (equal( HostAlias( node ), E_nodename ))  /* To local node?  */
      {
         if ( isValidLocalAddress( user ))
         {
            strcpy( response, "Local address");
            return KWTrue;
         }
         else {
            strcpy( response, "Address is not known on local system");
            return KWFalse;
         }
      }
      else {
         strcpy( response, "No known delivery path for host" );
         return KWFalse;
      }

   }  /* if */

/*--------------------------------------------------------------------*/
/*         Deliver mail to a system directly connected to us          */
/*--------------------------------------------------------------------*/

   if (equal(response,node)) /* Directly connected system?      */
   {
      strcpy( response, "Neighborhood remote address");
      return KWTrue;
   }

/*--------------------------------------------------------------------*/
/*       Default remote delivery                                      */
/*--------------------------------------------------------------------*/

   strcpy( response, "Generic remote address");
   return KWTrue;

} /* isValidAddress */

/*--------------------------------------------------------------------*/
/*       i s V a l i d L o c a l A d d r e s s                        */
/*                                                                    */
/*       Verify the local part of the address for our own host        */
/*--------------------------------------------------------------------*/

KWBoolean
isValidLocalAddress( const char *local )
{

/*--------------------------------------------------------------------*/
/*    RFC-821 requires we support postmaster, which in fact we do     */
/*--------------------------------------------------------------------*/

   if ( equali( local, "postmaster"))
      return KWTrue;

/*--------------------------------------------------------------------*/
/*                      Try our local user table                      */
/*--------------------------------------------------------------------*/

   if ( checkuser(local) != BADUSER )  /* Good user id?              */
      return KWTrue;                   /* Yes --> It passes          */

/*--------------------------------------------------------------------*/
/*         Try our system alias table last, since its slowest         */
/*--------------------------------------------------------------------*/

   if ( checkalias( local ) == NULL )  /* System alias?              */
      return KWFalse;                  /* Yes --> It passes          */
   else
      return KWTrue;                   /* No, complete failure       */

} /* isValidLocalAddress */
