/*--------------------------------------------------------------------*/
/*    a d d r e s s . c                                               */
/*                                                                    */
/*    Address parsing routines for UUPC/extended                      */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1995 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: address.c 1.16 1994/12/22 00:18:21 ahd Exp $
 *
 *    Revision history:
 *    $Log: address.c $
 *    Revision 1.16  1994/12/22 00:18:21  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.15  1994/05/08 21:43:33  ahd
 *    Trap hostname missing normal user@node RFC-822 address
 *
 * Revision 1.14  1994/03/07  06:09:51  ahd
 * Add additional debugging messages controlled by UDEBUG
 *
 * Revision 1.13  1994/02/28  01:02:06  ahd
 * Cosmetic formatting clean ups
 *
 * Revision 1.12  1994/02/21  16:38:58  ahd
 * Don't terminate address parsing on a quoted comment
 *
 * Revision 1.11  1994/02/20  19:11:18  ahd
 * IBM C/Set 2 Conversion, memory leak cleanup
 *
 * Revision 1.10  1994/01/24  03:17:02  ahd
 * Annual Copyright Update
 *
 * Revision 1.9  1994/01/01  19:11:59  ahd
 * Annual Copyright Update
 *
 * Revision 1.8  1993/10/12  01:30:23  ahd
 * Normalize comments to PL/I style
 *
 * Revision 1.7  1993/06/22  00:55:45  ahd
 * Trap routing entries when aliasing systems
 *
 * Revision 1.6  1993/06/21  04:04:04  ahd
 * Don't fail routing loops from aliased systems with no route
 *
 * Revision 1.5  1993/06/21  02:17:31  ahd
 * Correct errors in mail routing via HOSTPATH
 *
 */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"

#include <ctype.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "address.h"
#include "hostable.h"
#include "security.h"

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

currentfile();

/*--------------------------------------------------------------------*/
/*                     Local function prototypes                      */
/*--------------------------------------------------------------------*/

static char *rfc_route( char *tptr, char **nptr, char **pptr );

/*--------------------------------------------------------------------*/
/*    u s e r _ a t _ n o d e                                         */
/*                                                                    */
/*    break a UUCP path or RFC-822 address into the basic user and    */
/*    node components                                                 */
/*                                                                    */
/*    Note:    This routine assume an address of the form             */
/*             path!node1!user@node2 is for a user@node1 routed via   */
/*             node2 and then path.                                   */
/*--------------------------------------------------------------------*/

void user_at_node(const char *raddress,
                  char *hispath,
                  char *hisnode,
                  char *hisuser)
{

   static char *saveaddr = NULL;
   static char *savepath;
   static char *savenode;
   static char *saveuser;

   char *uptr;                      /* Pointer to his user id         */
   char *nptr;                      /* Pointer to his node id         */
   char *pptr;                      /* Pointer to next node in path to him */
   char *tptr;                      /* Temporary token pointer        */
   char *wptr;                      /* Work pointer (not used between
                                       steps                          */
   char *address;

   struct HostTable *Hptr = NULL;   /* Pointer to host name table     */

   if ( strlen( raddress ) >= MAXADDR )
   {
      printmsg(0,"Unable to process %d length address: %s",
            strlen(raddress), raddress );
      panic();
   }

/*--------------------------------------------------------------------*/
/*                     Determine if local address                     */
/*--------------------------------------------------------------------*/

   if (!strpbrk(raddress,"%!@"))    /* Any host delimiters?           */
   {                                /* No --> report local data       */
      strcpy(hisuser,raddress);
      strcpy(hisnode,E_nodename);
      strcpy(hispath,E_nodename);
      strcpy(hisuser,raddress);
      printmsg(5,"user_at_node: Address '%s' is local",raddress);
      return;
   }

/*--------------------------------------------------------------------*/
/*    If the current address is the same as the last processed remote */
/*    address, then return the same information as what we determined */
/*    last time.                                                      */
/*--------------------------------------------------------------------*/

   if ((saveaddr != NULL) && equal(raddress,saveaddr))
   {
      strcpy(hispath,savepath);
      strcpy(hisnode,savenode);
      strcpy(hisuser,saveuser);
      return;
   }

/*--------------------------------------------------------------------*/
/*   The address is different; save the new address and then proceed  */
/*   to parse it.                                                     */
/*--------------------------------------------------------------------*/

   address = strdup(raddress);   /* Copy address for parsing          */
   checkref(address);            /* Verify allocation worked          */

   if (saveaddr != NULL)         /* Was the data previously allocated? */
   {                             /* Yes --> Free it                   */
      free(saveaddr);
   }

   saveaddr = strdup(address);   /* Remember address for next pass    */

/*--------------------------------------------------------------------*/
/*    If the address has no at sign (@), but does have a percent      */
/*    sign (%), replace the last percent sign with an at sign.        */
/*--------------------------------------------------------------------*/

   if ( strchr(address,'@') == NULL )  /* Any at signs?               */
   {                                /* No --> Look further for %      */
      wptr = strrchr(address,'%');  /* Locate any percent signs       */

      if ( wptr != NULL )           /* Got one?                       */
         *wptr = '@';               /* Yup --> Make it an at sign at  */
   }

/*--------------------------------------------------------------------*/
/*                   Initialize routing information                   */
/*--------------------------------------------------------------------*/

   nptr = nil(char);             /* No known node for user            */
   pptr = E_mailserv;            /* Default routing via mail server   */
   tptr = address;               /* Remember start of address         */

/*--------------------------------------------------------------------*/
/*  The address may be RFC-822 syntax; attempt to parse that format   */
/*--------------------------------------------------------------------*/

   uptr = tptr = rfc_route( tptr, &nptr, &pptr );

/*--------------------------------------------------------------------*/
/*   If the user had an RFC-822 path, then the pointer to the path is */
/*   now initialized, and the remainder of the path has been dropped  */
/*   from *tptr; otherwise, the entire address is found via *tptr     */
/*--------------------------------------------------------------------*/

   wptr  = strrchr(tptr,'@');  /* Get last at sign, since it's right
                                  to left scan (more or less)         */

/*--------------------------------------------------------------------*/
/*    Translation of following:  If the at-sign (@) is not the        */
/*    first character and the character preceding the at-sign is      */
/*    not a bang (!), then break the address down into user and       */
/*    node.                                                           */
/*--------------------------------------------------------------------*/

   if (( wptr > tptr ) && ( strchr("!:",*(wptr-1)) == NULL))
   {

      uptr  = tptr;               /* Get user part of userid @node    */
      *wptr++ = '\0';             /* Terminate user portion           */

      if ( *wptr == '\0' )        /* Host MISSING?                   */
      {                           /* Yes --> Throw error, treat addr
                                             as local                */
         printmsg(0,"Invalid RFC-822 address, missing host name: %s",
                    saveaddr );
         tptr = "???????";
      }
      else
         tptr  = wptr;           /* Get node part of userid @node    */

#ifdef UDEBUG
      printmsg(4,"user_at_node: parsed user as \"%s\", node as \"%s\"",
                 uptr,
                 tptr );
#endif
   }

   if (tptr != NULL)           /* Did we get a node?                  */
   {                           /* Yes --> Save it                     */
      nptr = tptr;
      pptr = HostPath( nptr, pptr);
   } /* if */

/*--------------------------------------------------------------------*/
/*       Now, we will try stripping off any uucp path that the        */
/*       address may have acquired; we'll assume the last node is     */
/*       the addressee's node.                                        */
/*--------------------------------------------------------------------*/

   uptr = strtok(uptr,"!");
   tptr = strtok(NULL,"");

   while ( tptr != NULL )
   {
      nptr = uptr;                  /* First token is node            */

      if (*tptr == '@')             /* Explicit RFC-822 route?        */
      {                             /* Yes --> Examine in detail      */
         uptr = strtok( rfc_route( tptr, &nptr, &pptr ), "!");
                                    /* Second token, or what's
                                       left of it, is user id         */
         tptr = strtok(NULL,"");    /* Save rest of string            */
      } /* if (*tptr == '@') */
      else {
         uptr = strtok(tptr,"!");   /* Second token is user id        */
         tptr = strtok(NULL,"");    /* Save rest of string            */
         pptr = HostPath( nptr, pptr);
      } /* else */

   } /* while */

/*--------------------------------------------------------------------*/
/*   Finally, we parse off any internet mail that used the infamous % */
/*   hack (user%node1@gatewayb)                                       */
/*--------------------------------------------------------------------*/

   while ((tptr = strrchr(uptr,'%')) != NULL)   /* Get last percent   */
   {
      *tptr = '@';               /* Make it an RFC-822 address        */
      uptr  = strtok(uptr,"@");  /* Get user part of userid @node     */
      nptr  = strtok(NULL,"@");  /* Get node part of userid @node     */
      pptr  = HostPath(nptr, pptr); /* Old node is new path           */
   } /* while */

/*--------------------------------------------------------------------*/
/*   If the last known hop in the path is via our own system, but the */
/*   target node is not our own system, route the message via our     */
/*   default mail server.                                             */
/*--------------------------------------------------------------------*/

   nptr = HostAlias( nptr );

   if (equali(pptr,E_nodename))
                              /* Is mail routed via our local system? */
   {                          /* Yes --> Determine if destined for us */
      Hptr = checkname(nptr);          /* Locate the system       */
      if (Hptr == BADHOST)             /* System known?           */
      {                                /* No --> Route default    */
         printmsg(5,
            "user_at_node: Routing mail for \"%s\" via default mail server",
                  nptr);
         pptr = E_mailserv;
      } /* if */
   }  /* if */

/*--------------------------------------------------------------------*/
/*                         Print our results                          */
/*--------------------------------------------------------------------*/

   printmsg(9,
         "user_at_node: Address \"%s\" is \"%s\" at \"%s\" via \"%s\"",
            raddress, uptr, nptr, pptr);

/*--------------------------------------------------------------------*/
/*  We have parsed the address.  Fill in the information for caller   */
/*--------------------------------------------------------------------*/

   strcpy(hispath,pptr);
   strcpy(hisnode,nptr);
   strcpy(hisuser,uptr);

/*--------------------------------------------------------------------*/
/*   Save the parsed information along with the original address we   */
/*   were passed in.  This could save breaking it down again.         */
/*--------------------------------------------------------------------*/

   savepath = newstr(hispath);
   savenode = newstr(hisnode);
   saveuser = newstr(hisuser);

   free(address);
}  /* user_at_node */

/*--------------------------------------------------------------------*/
/*    r f c _ r o u t e                                               */
/*                                                                    */
/*    Strip off explicit RFC-822 routing from an address              */
/*--------------------------------------------------------------------*/

static char *rfc_route( char *tptr, char **nptr, char **pptr )
{

/*--------------------------------------------------------------------*/
/*          Loop as long as we have an explicit RFC-822 path          */
/*--------------------------------------------------------------------*/

   while (*tptr == '@')        /* Explicit RFC 822 path?              */
   {
      *nptr = strtok(++tptr,",:");  /* First token is path/node       */
      tptr = strtok(NULL,""); /* Second has rest, including user id   */
      *pptr = HostPath( *nptr, *pptr );
                              /* Determine actual path                */
      printmsg(9,"rfc_route: RFC-822 explicit path: "
                  "\"%s\" routed via \"%s\" is via \"%s\"",
         tptr, *nptr, *pptr);
   } /* while */

/*--------------------------------------------------------------------*/
/*    At this point, *nptr is last node in list, *pptr is path to     */
/*    *nptr, and *tptr is the rest of the string (userid?)            */
/*--------------------------------------------------------------------*/

   return tptr;
} /* rfc_route */

/*--------------------------------------------------------------------*/
/*    H o s t A l i a s                                               */
/*                                                                    */
/*    Resolve a host alias to its real canonized name                 */
/*--------------------------------------------------------------------*/

char *HostAlias( char *input)
{
   struct HostTable *hostp;

   hostp = checkname(input);

/*--------------------------------------------------------------------*/
/*     If nothing else to look at, return original data to caller     */
/*--------------------------------------------------------------------*/

   if (hostp == BADHOST)
      return input;

/*--------------------------------------------------------------------*/
/*       If the entry has no alias and is not a real system, it's     */
/*       a routing entry and we should ignore it.                     */
/*--------------------------------------------------------------------*/

   if ((hostp->status.hstatus == phantom) && ( hostp->realname == NULL ))
      return input;

/*--------------------------------------------------------------------*/
/*      If we already chased this chain, return result to caller      */
/*--------------------------------------------------------------------*/

   if (hostp->aliased)
   {
      if ( hostp->realname  == NULL )
      {
         printmsg(0,"Alias table loop detected with host %s",
               hostp->hostname);
      }

      return hostp->realname;
   } /* if */

   hostp->aliased = KWTrue;       /* Prevent limitless recursion       */

/*--------------------------------------------------------------------*/
/*                  Determine next host in the chain                  */
/*--------------------------------------------------------------------*/

   if ( hostp->realname == NULL)  /* End of the line?        */
      hostp->realname = hostp->hostname;
   else
      hostp->realname = HostAlias(hostp->realname);

/*--------------------------------------------------------------------*/
/*                        Announce our results                        */
/*--------------------------------------------------------------------*/

   printmsg( 5, "HostAlias: \"%s\" is alias of \"%s\"",
                  input,
                  hostp->realname);

   return hostp->realname;

} /* HostAlias */

/*--------------------------------------------------------------------*/
/*    H o s t P a t h                                                 */
/*                                                                    */
/*    Determine the path to a host                                    */
/*--------------------------------------------------------------------*/

char *HostPath( char *input, char *best)
{
   struct HostTable *hostp;

   hostp = checkname( input );

/*--------------------------------------------------------------------*/
/*     If nothing else to look at, return original data to caller     */
/*--------------------------------------------------------------------*/

   if (hostp == BADHOST)
      return best;

   if (hostp->status.hstatus == gatewayed)  /* Gatewayed?             */
      return hostp->hostname;      /* Yes --> Use name for path       */

/*--------------------------------------------------------------------*/
/*      If we already chased this chain, return result to caller      */
/*--------------------------------------------------------------------*/

   if (hostp->routed)
   {
      if ( hostp->via == NULL )
      {
         if ( hostp->aliased &&
              ! equali(hostp->hostname,hostp->realname))
            hostp->via = best;
         else {
            printmsg(0,"Routing table loop discovered at host %s",
                     hostp->hostname);
            panic();
         }

      } /* if ( hostp->via == NULL ) */

      return hostp->via;

   } /* if (hostp->routed) */

   hostp->routed  = KWTrue;       /* Prevent limitless recursion       */

/*--------------------------------------------------------------------*/
/*                  Determine next host in the chain                  */
/*--------------------------------------------------------------------*/

   if ( hostp->via == NULL )
   {
      char *alias = HostAlias( hostp->hostname );

      if (equal(hostp->hostname,alias))
      {
         if (hostp->status.hstatus == localhost) /* Ourself?          */
            hostp->via = E_nodename;      /* Yes --> Deliver local    */
         else if ( checkreal( hostp->hostname ) == BADHOST )
                                          /* Unknown system?          */
            hostp->via = best;            /* Yes --> Use default      */
         else
            hostp->via = hostp->hostname; /* Known --> route to it    */
      } /* if ( hostp->via == NULL ) */
      else
         hostp->via = HostPath( alias, best);

   } /* if ( hostp->via == NULL ) */

   hostp->via = HostPath( hostp->via, best );

   printmsg( 5, "HostPath: \"%s\" routed via \"%s\"", input, hostp->via);

   return hostp->via;

} /* HostPath */

/*--------------------------------------------------------------------*/
/*    E x t r a c t A d d r e s s                                     */
/*                                                                    */
/*    Returns the user name (if available and requested or            */
/*    E-mail address of the user                                      */
/*--------------------------------------------------------------------*/

char *ExtractAddress(char *result,
                    const char *input,
                    FULLNAME fullname)
{
   char *nonblank = NULL;
   char *column  = (char *) input;
   char name[BUFSIZ];      /* User full name             */
   char *nameptr = name;
   char addr[BUFSIZ];      /* User e-mail address        */
   char *addrptr  = addr;

   char state = 'A';                /* State = skip whitespace    */
   char newstate = 'A';             /* Next state to process      */
   int bananas = 0;                 /* No () being processed now  */
   int len;
   KWBoolean quoted = KWFalse;

#ifdef UDEBUG
   printmsg(15,"ExtractAddress: Return address is %p", result );
#endif

   *result  = '\0';

/*--------------------------------------------------------------------*/
/*   Begin loop to copy the input field into the address and or the   */
/*   user name.  We will begin by copying both (ignoring whitespace   */
/*   for addresses) because we won't know if the input field is an    */
/*   address or a name until we hit either a special character of     */
/*   some sort.                                                       */
/*--------------------------------------------------------------------*/

   while ((*column != '\0') && (state != ','))
   {
      switch (state) {
         case 'A':
            if (isspace(*column))   /* Found first non-blank? */
               break;               /* No --> keep looking    */
            nonblank = column;
            state = 'B';
                                    /* ... and fall through           */
         case 'B':
         case ')':
            newstate = *column;
            switch(*column) {
               case '(':
                  bananas++;
                  break;

               case '"':
                  break;

               case '<':
                  addrptr = addr;   /* Start address over      */
                  nameptr = name;   /* Start name over again   */
                  column  = nonblank - 1;
                                    /* Re-scan in new state    */
                  newstate = '>';   /* Proc all-non <> as name */
                  break;            /* Begin addr over again   */

               case ',':
                  break;            /* Terminates address      */

               case '>':
               case ')':
                  printmsg(0,"Invalid RFC-822 address: %s",nonblank);
                  panic();          /* Ooops, funky address    */
                  break;

               default:
                  newstate = state; /* stay in this state             */
                  if (!isspace(*column))
                     *(addrptr++) = *column;

            }  /* switch(*column) */
            break;

         case '<':
            if (*column == '>')
               newstate = '>';
            else if (!isspace(*column))
               *(addrptr++) = *column;
            break;

         case '>':
            switch( *column )
            {
               case '<':
                  newstate = '<';
                  break;

               case ')':
                  if (quoted)
                     *(nameptr++) = *column;
                  else
                     bananas--;
                  break;

               case '(':
                  if (quoted)
                     *(nameptr++) = *column;
                  else
                     bananas++;
                  break;

               case ',':
                  if ( quoted )
                     *(nameptr++) = *column; /* Take it as a literal */
                  else
                     newstate = ',';   /* Terminates address      */
                  break;

               case '"':
                  if (bananas == 0)
                  {
                     quoted = !quoted;
                     break;
                  }
                  /* else fall through */

               default:
                  *(nameptr++) = *column;
            } /* switch */
            break;

         case '(':
            if (*column == '(')
               ++bananas;
            else if (*column == ')')
            {
               if (--bananas == 0)
               {
                  newstate = ')';
                  break;
               }
            }
            else
               *(nameptr++) = *column;
            break;

         case '"':
            if (*column == '"')
               newstate = ')';
            else
               *(nameptr++) = *column;
                     break;

         case ',':
            newstate = ',';   /* Exit parse              */
            break;

         default:
            panic();          /* Logic error, bad state        */
            break;

      }  /* switch (state) */

#ifdef UDEBUG
      if ( debuglevel > 14 )
         printmsg(15,"State = %c, new state = %c, bananas %d, column = %s",
                     state,
                     newstate,
                     bananas,
                     column );
#endif
      state = newstate;
      column++;

   } /* while */

/*--------------------------------------------------------------------*/
/*                   Verify we retrieved an address                   */
/*--------------------------------------------------------------------*/

   if (state == 'A')
   {
      printmsg(0, "ExtractAddress: Could not find address in \"%s\"",
               column);
      panic();
   }

/*--------------------------------------------------------------------*/
/*                 Fill in the results for the caller                 */
/*--------------------------------------------------------------------*/

   *addrptr = '\0';
   *nameptr = '\0';
   len = strlen( addr );

#ifdef UDEBUG
   printmsg(15,"name = \"%s\"", name );
   printmsg(15,"addr = \"%s\"", addr );
#endif

   if ((fullname == ADDRESSONLY) ||
       ((fullname == FULLADDRESS) && (state == 'B')))
   {
      if ( len >= MAXADDR )
      {
         printmsg(0,"ExtractAddress: Address exceeds %d characters: %s",
                     MAXADDR, addr );
         panic();
      }
      strcpy(result,addr);         /* Return the full address    */
   }
   else if (state != 'B')
   {
      while (--nameptr >= name)
      {
         if (isspace(*nameptr))
            *nameptr = '\0';
         else
            break;
      }

/*--------------------------------------------------------------------*/
/*               Strip leading blanks from the address                */
/*--------------------------------------------------------------------*/

      nameptr = name;
      while (isspace(*nameptr))
         nameptr++;

      if ( strlen( nameptr ) >= MAXADDR )
      {
         printmsg(0,"ExtractAddress: Truncating name %s", nameptr);
         nameptr[ MAXADDR - 1 ] = '\0';
      }

      if ( fullname == FULLADDRESS )
      {
         if ( len >= (MAXADDR-6) )
         {
            printmsg(0,"ExtractAddress: Address exceeds %d characters: %s",
                        MAXADDR-6, addr );
            panic();
         }

         nameptr[ MAXADDR - len - 6] = '\0';
         sprintf( result, "\"%s\" <%s>", nameptr, addr );
      }
      else
         strcpy(result, nameptr );

   } /* else */

#ifdef UDEBUG
   printmsg(4,"ExtractAddress: %s into <%s> \"%s\", state [%c]",
            nonblank,
            addr,
            (fullname) ? result : name,
            state);
#endif

/*--------------------------------------------------------------------*/
/*   Return the position of the next address, if any, to the caller   */
/*--------------------------------------------------------------------*/

   if ( *column == '\0')
      return NULL;
   else
      return column;

} /*ExtractAddress*/
