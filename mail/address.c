/*--------------------------------------------------------------------*/
/*    a d d r e s s . c                                               */
/*                                                                    */
/*    Address parsing routines for UUPC/extended                      */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-2001 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: address.c 1.33 2000/05/12 12:35:45 ahd v1-13g $
 *
 *    Revision history:
 *    $Log: address.c $
 *    Revision 1.33  2000/05/12 12:35:45  ahd
 *    Annual copyright update
 *
 *    Revision 1.32  1999/01/08 02:21:01  ahd
 *    Convert currentfile() to RCSID()
 *
 *    Revision 1.31  1999/01/04 03:54:27  ahd
 *    Annual copyright change
 *
 *    Revision 1.30  1998/03/01 01:27:27  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.29  1997/12/14 21:17:03  ahd
 *    Don't free buffer we are saving input address in between calls
 *
 *    Revision 1.28  1997/12/13 18:05:06  ahd
 *    Cleanup parsing of user/host without path
 *
 *    Revision 1.27  1997/05/14 05:06:51  ahd
 *    Correct compiler warning
 *
 *    Revision 1.26  1997/05/14 05:02:55  ahd
 *    Improve error trapping in RFC-822 source route addresses
 *
 *    Revision 1.25  1997/05/11 04:27:40  ahd
 *    SMTP client support for RMAIL/UUXQT
 *
 *    Revision 1.24  1997/04/24 01:07:56  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.23  1996/11/18 04:46:49  ahd
 *    Normalize arguments to bugout
 *    Reset title after exec of sub-modules
 *    Normalize host status names to use HS_ prefix
 *
 *    Revision 1.22  1996/03/19 03:36:02  ahd
 *    Normalize case of selected variables
 *    Modify trap for doubled delimiters (%% @@ !!) to handle mixed
 *    doubled delimiters as well (!@ %@ %!).
 *
 *    Revision 1.21  1996/01/01 21:02:07  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.20  1995/11/30 03:06:56  ahd
 *    Trap truly invalid addresses in tokenizer
 *
 *    Revision 1.19  1995/09/24 19:09:26  ahd
 *    Change 'address is local' message to more correct report detected
 *    condition.
 *
 *    Revision 1.18  1995/03/12 16:42:24  ahd
 *    Don't report a wild card as an alias of a remote system
 *
 *    Revision 1.17  1995/01/07 16:18:19  ahd
 *    Change boolean to KWBoolean to avoid VC++ 2.0 conflict
 *
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
 * Correct errors in mail routing via HostPath
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

RCSID("$Id: address.c 1.33 2000/05/12 12:35:45 ahd v1-13g $");

/*--------------------------------------------------------------------*/
/*                     Local function prototypes                      */
/*--------------------------------------------------------------------*/

static char *rfc_route( char *tPtr, char **nPtr, char **pPtr );

/*--------------------------------------------------------------------*/
/*    t o k e n i z e A d d r e s s                                   */
/*                                                                    */
/*    break a UUCP path or RFC-822 address into the basic user and    */
/*    node components                                                 */
/*                                                                    */
/*    Note:    This routine assumes an address of the form            */
/*             path!node1!user@node2 is for a user@node1 routed via   */
/*             node2 and then path.                                   */
/*--------------------------------------------------------------------*/

KWBoolean
tokenizeAddress(const char *raddress,
             char *hisPath,
             char *hisNode,
             char *hisUser)
{

   static char *saveaddr = NULL;
   static char *savePath;
   static char *saveNode;
   static char *saveUser;

   char *uPtr;                      /* Pointer to his user id         */
   char *nPtr;                      /* Pointer to his node id         */
   char *pPtr;                      /* Pointer to next node in path to him */
   char *tPtr;                      /* Temporary token pointer        */
   char *wPtr;                      /* Work pointer (not used between
                                       steps                          */
   char *address;
   char *errorString = hisPath ? hisPath : hisNode;
   KWBoolean foundDelimiter = KWFalse;  /*   used for double
                                             delimiter scan           */

   struct HostTable *hPtr = NULL;   /* Pointer to host name table     */

   if ( *raddress == '\0' )
   {
      strcpy( errorString ,"The address is empty." );
      return KWFalse;
   }

   if ( strlen( raddress ) >= MAXADDR )
   {
      printmsg(0, "tokenizeAddress: Unable to process %d length address: %s",
            strlen(raddress),
            raddress );
      strcpy( errorString ,"The address is too long to parse." );
      return KWFalse;
   }

#ifdef UDEBUG
   checkref( hisNode );
   checkref( hisUser );
#endif

/*--------------------------------------------------------------------*/
/*                     Determine if local address                     */
/*--------------------------------------------------------------------*/

   if (!strpbrk(raddress, "%!@"))   /* Any host delimiters?           */
   {                                /* No --> report local data       */
      strcpy(hisUser, raddress);

      if ( hisPath == NULL )
         strcpy(hisNode, E_domain);
      else {
         strcpy(hisNode, E_nodename);
         strcpy(hisPath, E_nodename);
      }

      printmsg(5, "tokenizeAddress: Address %s has userid only, no host name",
               raddress);

      return KWTrue;
   }

/*--------------------------------------------------------------------*/
/*    If the current address is the same as the last processed remote */
/*    address, then return the same information as what we determined */
/*    last time.                                                      */
/*--------------------------------------------------------------------*/

   if ((hisPath != NULL ) &&
       (saveaddr != NULL) &&
       equal(raddress, saveaddr))
   {
      strcpy(hisPath, savePath);
      strcpy(hisNode, saveNode);
      strcpy(hisUser, saveUser);
      return KWTrue;
   }

/*--------------------------------------------------------------------*/
/*       Look for unparsable addresses, with node/userid separator    */
/*       characters next to each other or beginning or ending the     */
/*       string.  Note that at sign (@) can start the string,         */
/*       because @domain:user@domain2 is a valid construct.           */
/*--------------------------------------------------------------------*/

   tPtr = (char *) (raddress + strlen(raddress) - 1);

   if (( *raddress == '!' ) || ( *tPtr == '!' ) ||
       ( *raddress == '%' ) || ( *tPtr == '%' ) ||
       ( *tPtr    == '@' ) || ( *tPtr == ':' ))
   {
      strcpy( errorString, "The address is hopelessly invalid -- "
                       "it begins or ends with a delimiter character" );
      return KWFalse;
   }

   while( tPtr > raddress )
   {
      switch( *tPtr-- )
      {
         case '@':
         case '!':
         case '%':
            if ( foundDelimiter )
            {
               strcpy( errorString, "The address is hopelessly invalid -- "
                                "it has delimiter characters back-to-back" );
               return KWFalse;
            }
            else
               foundDelimiter = KWTrue;
            break;

         default:
            foundDelimiter = KWFalse;
            break;

      } /* switch( *tPtr-- ) */

   } /* while( tPtr > raddress ) */

/*--------------------------------------------------------------------*/
/*   The address is different; save the new address and then proceed  */
/*   to parse it.                                                     */
/*--------------------------------------------------------------------*/

   address = strdup(raddress);   /* Copy address for parsing          */
   checkref(address);            /* Verify allocation worked          */

/*--------------------------------------------------------------------*/
/*    If the address has no at sign (@), but does have a percent      */
/*    sign (%), replace the last percent sign with an at sign.        */
/*--------------------------------------------------------------------*/

   if ( strchr(address, '@') == NULL )  /* Any at signs?              */
   {                                /* No --> Look further for %      */
      wPtr = strrchr(address, '%'); /* Locate any percent signs       */

      if ( wPtr != NULL )           /* Got one?                       */
         *wPtr = '@';               /* Yup --> Make it an at sign at  */
   }

/*--------------------------------------------------------------------*/
/*                   Initialize routing information                   */
/*--------------------------------------------------------------------*/

   nPtr = nil(char);             /* No known node for user            */
   pPtr = E_mailserv;            /* Default routing via mail server   */
   tPtr = address;               /* Remember start of address         */

/*--------------------------------------------------------------------*/
/*  The address may be RFC-822 syntax; attempt to parse that format   */
/*--------------------------------------------------------------------*/

   uPtr = tPtr = rfc_route( tPtr, &nPtr, &pPtr );

   if ( uPtr == NULL )
   {
      strcpy( errorString, "The address is hopelessly invalid -- "
                       "Leading RFC-822 route syntax is invalid.");
      free( address );
      return KWFalse;
   }

/*--------------------------------------------------------------------*/
/*   If the user had an RFC-822 path, then the pointer to the path is */
/*   now initialized, and the remainder of the path has been dropped  */
/*   from *tPtr; otherwise, the entire address is found via *tPtr     */
/*--------------------------------------------------------------------*/

   wPtr  = strrchr(tPtr, '@'); /* Get last at sign, since it's right
                                  to left scan (more or less)         */

/*--------------------------------------------------------------------*/
/*    Translation of following:  If the at-sign (@) is not the        */
/*    first character and the character preceding the at-sign is      */
/*    not a bang (!), then break the address down into user and       */
/*    node.                                                           */
/*--------------------------------------------------------------------*/

   if (( wPtr > tPtr ) && ( strchr("!:", *(wPtr-1)) == NULL))
   {

      uPtr  = tPtr;               /* Get user part of userid @node    */
      *wPtr++ = '\0';             /* Terminate user portion           */

      if ( *wPtr == '\0' )        /* Host MISSING?                   */
      {                           /* Yes --> Throw error, treat addr
                                             as local                */
         printmsg(0, "tokenizeAddress: Invalid RFC-822 address, missing host name: %s",
                    saveaddr );
         strcpy( errorString, "The address is hopelessly invalid -- "
                          "domain name is missing after at sign (@)" );
         free( address );
         return KWFalse;
      }
      else
         tPtr  = wPtr;           /* Get node part of userid @node    */

#ifdef UDEBUG
      printmsg(4, "tokenizeAddress: parsed user as \"%s\", node as \"%s\"",
                 uPtr,
                 tPtr );
#endif
   }

   if (tPtr != NULL)           /* Did we get a node?                  */
   {                           /* Yes --> Save it                     */
      nPtr = tPtr;
      pPtr = HostPath( nPtr, pPtr);
   } /* if */

/*--------------------------------------------------------------------*/
/*       Now, we will try stripping off any uucp path that the        */
/*       address may have acquired; we'll assume the last node is     */
/*       the addressee's node.                                        */
/*--------------------------------------------------------------------*/

   uPtr = strtok(uPtr, "!");
   tPtr = strtok(NULL, "");

   while ( tPtr != NULL )
   {
      nPtr = uPtr;                  /* First token is node            */

      if (*tPtr == '@')             /* Explicit RFC-822 route?        */
      {                             /* Yes --> Examine in detail      */

         uPtr = rfc_route( tPtr, &nPtr, &pPtr );
         if ( uPtr == NULL )
         {
            strcpy( errorString, "The address is hopelessly invalid -- "
                             "Embedded RFC-822 route syntax is invalid.");
            free( address );
            return KWFalse;
         }
         uPtr = strtok(uPtr, "!");  /* Second token, or what's
                                       left of it, is user id         */

         tPtr = strtok(NULL, "");   /* Save rest of string            */
      } /* if (*tPtr == '@') */
      else {
         uPtr = strtok(tPtr, "!");  /* Second token is user id        */
         tPtr = strtok(NULL, "");   /* Save rest of string            */
         pPtr = HostPath( nPtr, pPtr);
      } /* else */

   } /* while */

/*--------------------------------------------------------------------*/
/*   Finally, we parse off any internet mail that used the infamous % */
/*   hack (user%node1@gatewayb)                                       */
/*--------------------------------------------------------------------*/

   while ((tPtr = strrchr(uPtr, '%')) != NULL)  /* Get last percent   */

   {
      *tPtr = '@';               /* Make it an RFC-822 address        */
      uPtr  = strtok(uPtr, "@"); /* Get user part of userid @node     */
      nPtr  = strtok(NULL, "@"); /* Get node part of userid @node     */
      pPtr  = HostPath(nPtr, pPtr); /* Old node is new path           */

   } /* while */

/*--------------------------------------------------------------------*/
/*       If caller doesn't need path, return system name before we    */
/*       alias it                                                     */
/*--------------------------------------------------------------------*/

   if ( hisPath == NULL )
   {
      printmsg(9,
            "tokenizeAddress: Address \"%s\" is \"%s\" at \"%s\"",
               raddress, uPtr, nPtr );
      strcpy(hisNode, nPtr);
      strcpy(hisUser, uPtr);

      free(address);

      return KWTrue;
   }

/*--------------------------------------------------------------------*/
/*            Determine canonical  name of the target system          */
/*--------------------------------------------------------------------*/

   tPtr = HostAlias( nPtr );     /* Get possible alias name          */

   if ( *tPtr != '*' )           /* Unless a wildcard name ...       */
      nPtr = tPtr;               /* ... replace name with canonical  */

/*--------------------------------------------------------------------*/
/*   If the last known hop in the path is via our own system, but the */
/*   target node is not our own system, route the message via our     */
/*   default mail server.                                             */
/*--------------------------------------------------------------------*/

   if (equali(pPtr, E_nodename))
                              /* Is mail routed via our local system? */
   {                          /* Yes --> Determine if destined for us */

      hPtr = checkname(tPtr);          /* Locate the system, using
                                          alias from above           */

      if (hPtr == BADHOST)             /* System known?              */
      {                                /* No --> Route via default   */

         printmsg(5,
            "tokenizeAddress: Routing mail for \"%s\" via default mail server",
                  nPtr);

         pPtr = E_mailserv;

      } /* if */
   }  /* if */

/*--------------------------------------------------------------------*/
/*                         Print our results                          */
/*--------------------------------------------------------------------*/

   printmsg(9,
         "tokenizeAddress: Address \"%s\" is \"%s\" at \"%s\" via \"%s\"",
            raddress, uPtr, nPtr, pPtr);

/*--------------------------------------------------------------------*/
/*  We have parsed the address.  Fill in the information for caller   */
/*--------------------------------------------------------------------*/

#ifdef UDEBUG
   checkref( pPtr );
   checkref( nPtr );
   checkref( uPtr );
#endif

   strcpy(hisPath, pPtr);
   strcpy(hisNode, nPtr);
   strcpy(hisUser, uPtr);

/*--------------------------------------------------------------------*/
/*   Save the parsed information along with the original address we   */
/*   were passed in.  This could save breaking it down again.         */
/*--------------------------------------------------------------------*/

   if (saveaddr != NULL)            /* Was data previously allocated? */
      free(saveaddr);               /* Yes --> Free it                */

   strcpy( address, raddress );     /* Get original user input        */
   saveaddr = address;              /* Remember address for next pass */

   savePath = newstr(hisPath);
   saveNode = newstr(hisNode);
   saveUser = newstr(hisUser);

   return KWTrue;

}  /* tokenizeAddress */

/*--------------------------------------------------------------------*/
/*    r f c _ r o u t e                                               */
/*                                                                    */
/*    Strip off explicit RFC-822 routing from an address              */
/*--------------------------------------------------------------------*/

static char *rfc_route( char *tPtr, char **nPtr, char **pPtr )
{
   char *token = tPtr;

   if ( *tPtr != '@' )              /* RFC-822 path?                 */
      return tPtr;                  /* No --> No special processing  */

   tPtr = strrchr( tPtr, ':' );     /* Find end of routing           */

   if ( tPtr == NULL )              /* No end of route (error?)      */
   {
#ifdef UDEBUG
      printmsg(0,"rfc_route: No ':' in %s", token );
#endif
      return NULL;                  /* Yes --> Report the error      */
   }

   *tPtr++ = '\0';                  /* Terminate route, point to
                                       main part of address          */

   if (strchr( tPtr,'@') == NULL)   /* user@node in main part?       */
   {
#ifdef UDEBUG
      printmsg(0,"rfc_route: No ':' in %s", tPtr );
#endif
      return NULL;                  /* No --> I'm sooo confused ...  */
   }

/*--------------------------------------------------------------------*/
/*          Loop as long as we have an explicit RFC-822 path          */
/*--------------------------------------------------------------------*/

   while ((token != NULL ) && *token++ == '@')
                                    /* Explicit RFC 822 path?        */
   {
      if (( *token == ',' ) || (*token == ':') || ( *token == '\0' ))
         return NULL;               /* Flag error to caller          */

      *nPtr = strtok(token, ",:");  /* First token is path/node      */
      token = strtok(NULL, "");     /* Second has rest               */

      *pPtr = HostPath( *nPtr,  *pPtr );
                                    /* Determine actual path         */
      printmsg(9, "rfc_route: RFC-822 explicit path: "
                  "\"%s\" routed via \"%s\" is via \"%s\"",
                  tPtr,
                  *nPtr,
                  *pPtr);
   } /* while */

/*--------------------------------------------------------------------*/
/*    At this point, *nPtr is last node in list, *pPtr is path to     */
/*    *nPtr, and *tPtr is the rest of the string (userid@node)        */
/*--------------------------------------------------------------------*/

   return tPtr;

} /* rfc_route */

/*--------------------------------------------------------------------*/
/*       H o s t A l i a s                                            */
/*                                                                    */
/*       Resolve a host alias to its real canonized name              */
/*--------------------------------------------------------------------*/

char *hostAlias( const char *input, const char *fname, const size_t lineno)
{
   struct HostTable *hostP;

   hostP = checkName(input, lineno, fname );

/*--------------------------------------------------------------------*/
/*     If nothing else to look at, return original data to caller     */
/*--------------------------------------------------------------------*/

   if (hostP == BADHOST)
      return ((char *) input);

/*--------------------------------------------------------------------*/
/*       If the entry has no alias and is not a real system, it's     */
/*       a routing entry and we should ignore it.                     */
/*--------------------------------------------------------------------*/

   if ((hostP->status.hstatus == HS_PHANTOM) &&
       ( hostP->realname == NULL ))
      return ((char *) input);

/*--------------------------------------------------------------------*/
/*      If we already chased this chain, return result to caller      */
/*--------------------------------------------------------------------*/

   if (hostP->aliased)
   {
      if ( hostP->realname  == NULL )
      {
         printmsg(0, "Alias table loop detected with host %s",
               hostP->hostname);
      }

      return hostP->realname;
   } /* if */

   hostP->aliased = KWTrue;       /* Prevent limitless recursion       */

/*--------------------------------------------------------------------*/
/*                  Determine next host in the chain                  */
/*--------------------------------------------------------------------*/

   if ( hostP->realname == NULL)  /* End of the line?        */
      hostP->realname = hostP->hostname;
   else
      hostP->realname = HostAlias(hostP->realname);

/*--------------------------------------------------------------------*/
/*                        Announce our results                        */
/*--------------------------------------------------------------------*/

   printmsg( 5, "HostAlias: \"%s\" is alias of \"%s\"",
                  input,
                  hostP->realname);

   return hostP->realname;

} /* HostAlias */

/*--------------------------------------------------------------------*/
/*    H o s t P a t h                                                 */
/*                                                                    */
/*    Determine the path to a host                                    */
/*--------------------------------------------------------------------*/

char *HostPath( const char *input, const char *best)
{
   struct HostTable *hostP;

   hostP = checkname( input );

/*--------------------------------------------------------------------*/
/*     If nothing else to look at, return original data to caller     */
/*--------------------------------------------------------------------*/

   if (hostP == BADHOST)
      return ((char *) best);

   if (hostP->status.hstatus == HS_GATEWAYED)  /* Gatewayed?          */
      return hostP->hostname;      /* Yes --> Use name for path       */

   if (hostP->status.hstatus == HS_SMTP)      /* SMTP Gatewayed?      */
      return hostP->hostname;      /* Yes --> Use name for path       */

/*--------------------------------------------------------------------*/
/*      If we already chased this chain, return result to caller      */
/*--------------------------------------------------------------------*/

   if (hostP->routed)
   {
      if ( hostP->via == NULL )
      {
         if ( hostP->aliased &&
              ! equali(hostP->hostname, hostP->realname))
            hostP->via = newstr( best );
         else {
            printmsg(0, "Routing table loop discovered at host %s",
                     hostP->hostname);
            panic();
         }

      } /* if ( hostP->via == NULL ) */

      return hostP->via;

   } /* if (hostP->routed) */

   hostP->routed  = KWTrue;       /* Prevent limitless recursion       */

/*--------------------------------------------------------------------*/
/*                  Determine next host in the chain                  */
/*--------------------------------------------------------------------*/

   if ( hostP->via == NULL )
   {
      char *alias = HostAlias( hostP->hostname );

      if (equal(hostP->hostname, alias))
      {
         if (hostP->status.hstatus == HS_LOCALHOST) /* Ourself?       */
            hostP->via = E_nodename;      /* Yes --> Deliver local    */
         else if ( checkreal( hostP->hostname ) == BADHOST )
                                          /* Unknown system?          */
            hostP->via = newstr( best );  /* Yes --> Use default      */
         else
            hostP->via = hostP->hostname; /* Known --> route to it    */
      } /* if ( hostP->via == NULL ) */
      else
         hostP->via = HostPath( alias, best);

   } /* if ( hostP->via == NULL ) */

   hostP->via = HostPath( hostP->via, best );

   printmsg( 5, "HostPath: \"%s\" routed via \"%s\"", input, hostP->via);

   return hostP->via;

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
   char *namePtr = name;
   char addr[BUFSIZ];      /* User e-mail address        */
   char *addrPtr  = addr;

   char state = 'A';                /* State = skip whitespace    */
   char newstate = 'A';             /* Next state to process      */
   int bananas = 0;                 /* No () being processed now  */
   size_t len;
   KWBoolean quoted = KWFalse;

#ifdef UDEBUG
   printmsg(15, "ExtractAddress: Return address is %p", result );
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
                  addrPtr = addr;   /* Start address over      */
                  namePtr = name;   /* Start name over again   */
                  column  = nonblank - 1;
                                    /* Re-scan in new state    */
                  newstate = '>';   /* Proc all-non <> as name */
                  break;            /* Begin addr over again   */

               case ',':
                  break;            /* Terminates address      */

               case '>':
               case ')':
                  printmsg(0, "Invalid RFC-822 address: %s",nonblank);
                  panic();          /* Ooops, funky address    */
                  break;

               default:
                  newstate = state; /* stay in this state             */
                  if (!isspace(*column))
                     *(addrPtr++) = *column;

            }  /* switch(*column) */
            break;

         case '<':
            if (*column == '>')
               newstate = '>';
            else if (!isspace(*column))
               *(addrPtr++) = *column;
            break;

         case '>':
            switch( *column )
            {
               case '<':
                  newstate = '<';
                  break;

               case ')':
                  if (quoted)
                     *(namePtr++) = *column;
                  else
                     bananas--;
                  break;

               case '(':
                  if (quoted)
                     *(namePtr++) = *column;
                  else
                     bananas++;
                  break;

               case ',':
                  if ( quoted )
                     *(namePtr++) = *column; /* Take it as a literal */
                  else
                     newstate = ',';   /* Terminates address      */
                  break;

               case '"':
                  if (bananas == 0)
                  {
                     if ( quoted )
                        quoted = KWFalse;
                     else
                        quoted = KWTrue;
                     break;
                  }
                  /* else fall through */

               default:
                  *(namePtr++) = *column;
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
               *(namePtr++) = *column;
            break;

         case '"':
            if (*column == '"')
               newstate = ')';
            else
               *(namePtr++) = *column;
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
         printmsg(15, "State = %c, new state = %c, bananas %d, column = %s",
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

   *addrPtr = '\0';
   *namePtr = '\0';
   len = strlen( addr );

#ifdef UDEBUG
   printmsg(15, "name = \"%s\"", name );
   printmsg(15, "addr = \"%s\"", addr );
#endif

   if ((fullname == ADDRESSONLY) ||
       ((fullname == FULLADDRESS) && (state == 'B')))
   {
      if ( len >= MAXADDR )
      {
         printmsg(0, "ExtractAddress: Address exceeds %d characters: %s",
                     MAXADDR, addr );
         panic();
      }
      strcpy(result, addr);        /* Return the full address    */
   }
   else if (state != 'B')
   {
      while (--namePtr >= name)
      {
         if (isspace(*namePtr))
            *namePtr = '\0';
         else
            break;
      }

/*--------------------------------------------------------------------*/
/*               Strip leading blanks from the address                */
/*--------------------------------------------------------------------*/

      namePtr = name;

      while (isspace(*namePtr))
         namePtr++;

      if ( strlen( namePtr ) >= MAXADDR )
      {
         printmsg(0, "ExtractAddress: Truncating name %s", namePtr);
         namePtr[ MAXADDR - 1 ] = '\0';
      }

      if ( fullname == FULLADDRESS )
      {
         if ( len >= (MAXADDR-6) )
         {
            printmsg(0,"ExtractAddress: Address exceeds %d characters: %s",
                        MAXADDR-6, addr );
            panic();
         }

         namePtr[ MAXADDR - len - 6] = '\0';
         sprintf( result, "\"%s\" <%s>", namePtr, addr );
      }
      else
         strcpy(result, namePtr );

   } /* else */

#ifdef UDEBUG
   printmsg(4, "ExtractAddress: %s into <%s> \"%s\", state [%c]",
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
