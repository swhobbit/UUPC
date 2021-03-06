/*--------------------------------------------------------------------*/
/*       s m t p u t i l . c                                          */
/*                                                                    */
/*       SMTP commands utility functions                              */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-2002 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *       $Id: smtputil.c 1.12 2001/03/12 13:55:29 ahd v1-13k $
 *
 *       Revision History:
 *       $Log: smtputil.c $
 *       Revision 1.12  2001/03/12 13:55:29  ahd
 *       Annual copyright update
 *
 *       Revision 1.11  2000/10/14 12:40:44  ahd
 *       Use #define for postmaster address
 *
 *       Revision 1.10  2000/09/15 00:18:38  ahd
 *       All relaying to all addresses in "local domain"
 *
 *       Revision 1.9  2000/05/12 12:35:45  ahd
 *       Annual copyright update
 *
 *       Revision 1.8  1999/01/08 02:21:01  ahd
 *       Convert currentfile() to RCSID()
 *
 *       Revision 1.7  1999/01/04 03:54:27  ahd
 *       Annual copyright change
 *
 *       Revision 1.6  1998/11/01 20:36:52  ahd
 *       *** empty log message ***
 *
 * Revision 1.5  1998/05/11  13:55:28  ahd
 * Correct setting of local address flag
 *
 *       Revision 1.4  1998/05/11 01:20:48  ahd
 *       Correct resolution of host name to better determine
 *       local/neighorhood/remote nature of addresses
 *
 *       Revision 1.3  1998/03/01 01:32:04  ahd
 *       Annual Copyright Update
 *
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
#include "deliver.h"

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

RCSID("$Id: smtputil.c 1.12 2001/03/12 13:55:29 ahd v1-13k $");

/*--------------------------------------------------------------------*/
/*          s t r i p A d d r e s s                                   */
/*                                                                    */
/*          Strip delimiters off address and check it's length        */
/*--------------------------------------------------------------------*/

KWBoolean
stripAddress(char *address, char response[MAXADDR])
{
   int brackets = 0;
   int len = strlen(address);

   *response = '\0';

/*--------------------------------------------------------------------*/
/*                      Handle our very special case                  */
/*--------------------------------------------------------------------*/

   if (equal(address, SMTP_BOUNCE_POSTMASTER))        /* Postmaster? */
      return KWTrue;                /* Yes --> Leave intact          */

/*--------------------------------------------------------------------*/
/*                     Handle various syntax checks                   */
/*--------------------------------------------------------------------*/

   if (strchr(address, '@') == NULL)   /* Any host delimiters?       */
   {
      strcpy(response, "Address does not contain at sign (@)");
      return KWFalse;
   }

   if (strpbrk(address, WHITESPACE))
   {
      strcpy(response, "Address contains white space not supported by "
                        "UUCP mailers");
      return KWFalse;
   }

   if (strpbrk(address, "()"))
   {
      strcpy(response, "Address contains parantheses, not supported");
      return KWFalse;
   }

/*--------------------------------------------------------------------*/
/*              Strip angle brackets, checking for balance            */
/*--------------------------------------------------------------------*/

   if (address[0] == '<')
   {
      brackets++;
      MEMMOVE(address, address + 1, --len);
   }

   if (address[len - 1] == '>')
   {
      brackets++;
      address[--len] = '\0';
   }

   if (brackets == 1)
   {
      strcpy(response, "Unbalanced angle brackets (<>)");
      return KWFalse;
   }

/*--------------------------------------------------------------------*/
/*                  Look for extra bogus angle brackets               */
/*--------------------------------------------------------------------*/

   if (strpbrk(address, "<>"))
   {
      strcpy(response, "Address contains extra angle brackets, "
                        "not supported");
      return KWFalse;
   }

/*--------------------------------------------------------------------*/
/*                           Verify the length                        */
/*--------------------------------------------------------------------*/

   if (len >= MAXADDR)
   {
      strcpy(response, "Address is too long for processing");
      return KWFalse;

   }

/*--------------------------------------------------------------------*/
/*       The address is safe for other (less paranoid) routines       */
/*       to process it                                                */
/*--------------------------------------------------------------------*/

   return KWTrue;

} /* stripAddress */

/*--------------------------------------------------------------------*/
/*       i s V a l i d A d d r e s s                                  */
/*                                                                    */
/*       Report if an address is valid for processing                 */
/*--------------------------------------------------------------------*/

KWBoolean
isValidAddress(const char *address,
                char response[MAXADDR],
                KWBoolean *ourProblem)
{
   static const char mName[] = "isValidAddress";
   char node[MAXADDR];
   char user[MAXADDR];
   char path[MAXADDR];

   size_t nodeLength;
   size_t localDomainLength;

   struct HostTable *hostp;
   *ourProblem = KWTrue;         /* Assume it's our mail             */

/*--------------------------------------------------------------------*/
/*                    Perform basic syntax checks                     */
/*--------------------------------------------------------------------*/

   if (equal(address, SMTP_BOUNCE_POSTMASTER))
   {
      strcpy(response, "SMTP Postmaster");
      *ourProblem = KWFalse;     /* Too generic to trust             */
      return KWTrue;
   }

/*--------------------------------------------------------------------*/
/*                       Parse the address down                       */
/*--------------------------------------------------------------------*/

   if (! tokenizeAddress(address, path, node, user))
   {
      strcpy(response, path);
      return KWFalse;               /* Message already in buffer     */
   }

   printmsg(4, "%s: Address %s is user %s at %s via %s",
               mName,
               address,
               user,
               node,
               path);

   hostp = checkname(node);

/*--------------------------------------------------------------------*/
/*                      Handle gateway delivery                       */
/*--------------------------------------------------------------------*/

   if ((hostp != BADHOST) && (hostp->status.hstatus == HS_GATEWAYED))
   {
      strcpy(response, "Supported via gateway");
      return KWTrue;
   }

/*--------------------------------------------------------------------*/
/*                 Handle SMTP delivery, if supported                 */
/*--------------------------------------------------------------------*/

   if ((hostp != BADHOST) && (hostp->status.hstatus == HS_SMTP))
   {
      strcpy(response, "Explicitly routed to SMTP");
      return KWTrue;
   }

/*--------------------------------------------------------------------*/
/*                       Handle local delivery                        */
/*--------------------------------------------------------------------*/

   if (equal(path, E_nodename))     /* Route via Local node?          */
   {
      if (equal(HostAlias(node), E_nodename))  /* To local node?  */
      {
         if (isValidLocalAddress(user))
         {
            strcpy(response, "Valid local address");
            return KWTrue;
         }
         else {
            strcpy(response, "Address not known on local system");
            return KWFalse;
         }
      }
      else {
         strcpy(response, "No known delivery path for host");
         return KWFalse;
      }

   }  /* if */

/*--------------------------------------------------------------------*/
/*         Deliver mail to a system directly connected to us          */
/*--------------------------------------------------------------------*/

   if (equal(path, node))        /* Directly connected system?       */
   {
      strcpy(response, "Neighborhood UUCP remote address");
      return KWTrue;
   }

/*--------------------------------------------------------------------*/
/*         Deliver mail to a system in our local domain               */
/*--------------------------------------------------------------------*/

   nodeLength = strlen(node);
   localDomainLength = strlen(E_localdomain);

   if ((nodeLength >= localDomainLength) &&
       equali(E_localdomain, node + (nodeLength - localDomainLength)))
   {
      strcpy(response, "Neighborhood domain address");
      return KWTrue;
   }

/*--------------------------------------------------------------------*/
/*       Default remote delivery                                      */
/*--------------------------------------------------------------------*/

   strcpy(response, "Generic remote address");
   *ourProblem = KWFalse;         /* We don't know their host        */
   return KWTrue;

} /* isValidAddress */

/*--------------------------------------------------------------------*/
/*       i s V a l i d L o c a l A d d r e s s                        */
/*                                                                    */
/*       Verify the local part of the address for our own host        */
/*--------------------------------------------------------------------*/

KWBoolean
isValidLocalAddress(const char *local)
{

/*--------------------------------------------------------------------*/
/*    RFC-821 requires we support postmaster, which in fact we do     */
/*--------------------------------------------------------------------*/

   if (equali(local, "postmaster"))
      return KWTrue;

/*--------------------------------------------------------------------*/
/*                      Try our local user table                      */
/*--------------------------------------------------------------------*/

   if (checkuser(local) != BADUSER)    /* Good user id?              */
      return KWTrue;                   /* Yes --> It passes          */

/*--------------------------------------------------------------------*/
/*         Try our system alias table last, since its slowest         */
/*--------------------------------------------------------------------*/

   if (checkalias(local) == NULL)      /* System alias?              */
      return KWFalse;                  /* Yes --> It passes          */
   else
      return KWTrue;                   /* No, complete failure       */

} /* isValidLocalAddress */
