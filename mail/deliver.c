/*--------------------------------------------------------------------*/
/*       d e l i v e r  . c                                           */
/*                                                                    */
/*       UUPC/extended mail delivery subroutines                      */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-2000 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: deliver.c 1.67 1999/01/11 05:43:36 ahd Exp $
 *
 *    $Log: deliver.c $
 *    Revision 1.67  1999/01/11 05:43:36  ahd
 *    Remove special case SMTP retry code
 *
 *    Revision 1.66  1999/01/08 02:21:01  ahd
 *    Convert currentfile() to RCSID()
 *
 *    Revision 1.65  1999/01/04 03:54:27  ahd
 *    Annual copyright change
 *
 *    Revision 1.64  1998/09/08 23:21:22  ahd
 *    Add setting of title for major operations
 *
 *    Revision 1.63  1998/08/29 14:17:11  ahd
 *    Add setTitle() commands to report delivery progress
 *
 * Revision 1.62  1998/03/16  07:47:40  ahd
 * Invert (correct) test for existence of mailbox before stater()
 *
 *    Revision 1.61  1998/03/16 06:41:41  ahd
 *    Support for time stamp controlling trumpt processing
 *
 *    Revision 1.60  1998/03/08 23:10:20  ahd
 *    Better support for local vs. remote source of messages
 *
 *    Revision 1.59  1998/03/01 01:28:47  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.58  1997/12/14 00:43:16  ahd
 *    Further cleanup of new sender protocol
 *
 *    Revision 1.57  1997/12/13 18:11:01  ahd
 *    Change parsing and passing of sender address information
 *
 *    Revision 1.56  1997/11/30 04:21:39  ahd
 *    Delete older RCS log comments, force full address for SMTP delivery,
 *    recongize difference between local and remote delivery
 *
 *    Revision 1.55  1997/11/25 05:05:06  ahd
 *    More robust SMTP daemon
 *
 *    Revision 1.54  1997/11/24 02:52:26  ahd
 *    First working SMTP daemon which delivers mail
 *
 *    Revision 1.53  1997/05/11 18:15:50  ahd
 *    Allow faster SMTP delivery via fastsmtp flag
 *    Move TCP/IP dependent code from rmail.c to deliver.c
 *    Allow building rmail without SMTP or TCP/IP support
 *
 *    Revision 1.52  1997/05/11 04:27:40  ahd
 *    SMTP client support for RMAIL/UUXQT
 *
 *    Revision 1.51  1997/04/24 01:08:14  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.50  1996/11/18 04:46:49  ahd
 *    Normalize arguments to bugout
 *    Reset title after exec of sub-modules
 *    Normalize host status names to use HS_ prefix
 *
 *    Revision 1.49  1996/01/20 12:58:08  ahd
 *    Specify text/binary when opening in-memory files
 *
 *    Revision 1.48  1996/01/01 21:03:40  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.47  1995/11/30 03:06:56  ahd
 *    Trap truly invalid addresses in tokenizer
 *
 *    Revision 1.46  1995/09/27 11:15:53  ahd
 *    Correct "No known delivery path for host" to report the failing
 *    host name, not the current host name.
 *
 *    Revision 1.45  1995/09/24 19:07:05  ahd
 *    Add comment to document use of monocase names for internal queue
 *
 *    Revision 1.44  1995/09/11 00:20:45  ahd
 *    Add debugging information to X.* files
 *    Use "--" option to insure user names with leading dash don't
 *    cause problems.
 *
 *    Revision 1.43  1995/07/21 13:23:19  ahd
 *    Correct wildcard routing for local host to reject mail not actually
 *    destined to us.
 *
 *    Revision 1.42  1995/03/23 01:30:34  ahd
 *    Handle empty forward files more gracefully
 *
 *    Revision 1.41  1995/03/11 22:27:34  ahd
 *    Use macro for file delete to allow special OS/2 processing
 *
 *    Revision 1.40  1995/03/11 02:04:36  ahd
 *    Correct gateway processing for local system
 *    If copydate always returns KWTrue/KWFalse, it ought to be KWBoolean
 *    Correct various compiler warnings
 *
 *    Revision 1.39  1995/03/08 02:58:08  ahd
 *    Use specific sequence numbers for VMS queue support
 *
 *    Revision 1.38  1995/02/14 04:40:42  ahd
 *    Make compare for postmaster case insensitive
 *
 *    Revision 1.37  1995/02/12 23:37:04  ahd
 *    compiler cleanup, NNS C/news support, optimize dir processing
 *
 *    Revision 1.36  1995/01/09 01:39:22  ahd
 *    Optimize UUCP processing for remote mail, break out logical
 *    queuing from actually writing the files, and don't write call
 *    file (which UUCICO could see by mistake) until we're writing
 *    it for the final time.
 *
 *    Revision 1.35  1995/01/08 19:52:44  ahd
 *    Add in memory files to RMAIL, including additional support and
 *    bug fixes.
 *
 *    Revision 1.34  1995/01/07 16:18:37  ahd
 *    Change KWBoolean to KWBoolean to avoid VC++ 2.0 conflict
 */

#define INCLUDE ":include:"

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"

#include <io.h>
#include <ctype.h>
#include <process.h>
#include <limits.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "address.h"
#include "deliver.h"
#include "execute.h"
#include "expath.h"
#include "getseq.h"
#include "hostable.h"
#include "imfile.h"
#include "pushpop.h"
#include "security.h"
#include "sysalias.h"
#include "timestmp.h"
#include "trumpet.h"
#include "usertabl.h"
#include "stater.h"
#include "title.h"
#include "deliverm.h"               /* Misc support functions        */

#ifdef TCPIP
#include "delivers.h"
#endif

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

 char *myProgramName = "rmail";  /* Name for recursive invocation    */
 char grade = 'C';            /* Default grade for mail sent         */

 KEWSHORT hops = 0;              /* Number hops this mail has seen   */

/*--------------------------------------------------------------------*/
/*        Define current file name for panic() and printerr()         */
/*--------------------------------------------------------------------*/

RCSID("$Id: deliver.c 1.67 1999/01/11 05:43:36 ahd Exp $");

/*--------------------------------------------------------------------*/
/*                        Internal prototypes                         */
/*--------------------------------------------------------------------*/

static size_t DeliverLocal( IMFILE *imf,        /* Input file name    */
                          const MAIL_ADDR *sender,
                          char *targetUser,     /* Target address     */
                          KWBoolean validate);  /* Validate/forward
                                                   local mail         */

static size_t ExpandAliasFile( IMFILE *imf,
                        const MAIL_ADDR *sender,
                        const char *mboxname,
                        const long start,
                        const long end,
                        KWBoolean *announce,
                        struct UserTable *userp,
                        const KWBoolean validate,
                        const char *user );

static size_t DeliverVMS( IMFILE *imf,          /* Input file name    */
                          const MAIL_ADDR *sender,
                          char *user,           /* Target address     */
                          KWBoolean validate); /* Validate/forward
                                                local mail            */

static size_t DeliverGateway(   IMFILE *imf,
                                const MAIL_ADDR *sender,
                                const char *user,
                                const char *node,
                                const struct HostTable *hostp,
                                const KWBoolean validate );

/*--------------------------------------------------------------------*/
/*    D e l i v e r                                                   */
/*                                                                    */
/*    Deliver mail to one user                                        */
/*--------------------------------------------------------------------*/

size_t Deliver( IMFILE *imf,        /* Input file                    */
                const MAIL_ADDR *sender,
                const char *address,/* Target address                */
                KWBoolean validate) /* Validate/forward local mail   */
{
#ifdef UDEBUG32
   static const char mName[] = "Deliver";
#endif
   char node[MAXADDR];
   char path[MAXADDR];
   char user[MAXADDR];
   char *token;
   struct HostTable *hostp;

   if ( ! tokenizeAddress(address, path, node, user) )
   {
      return Bounce( imf,
                     sender,
                     path,
                     address,
                     address,
                     validate );
   }

   setTitle("Delivering from %s to %s",
            sender->address,
            address);

/*--------------------------------------------------------------------*/
/*                   Deliver to a gateway if needed                   */
/*--------------------------------------------------------------------*/

   hostp = checkname( path );

/*--------------------------------------------------------------------*/
/*                    Do we need loop protection?                     */
/*--------------------------------------------------------------------*/

   if (!equal(path, E_nodename) && (hops > E_maxhops))
      return Bounce(imf,
                    sender,
             "Excessive number of hops",
             address,
             address,
             validate );

/*--------------------------------------------------------------------*/
/*                      Handle gateway delivery                       */
/*--------------------------------------------------------------------*/

   if ( (hostp != BADHOST) && (hostp->status.hstatus == HS_GATEWAYED))
      return DeliverGateway( imf,
                             sender,
                             user,
                             node,
                             hostp,
                             validate );

/*--------------------------------------------------------------------*/
/*                 Handle SMTP delivery, if supported                 */
/*--------------------------------------------------------------------*/

   if ( (hostp != BADHOST) && (hostp->status.hstatus == HS_SMTP))
   {
#ifdef TCPIP
#ifdef UDEBUG32
      printmsg(5,"%s: Validate = %s, "
               "Remote Sender = %s",
               "Fast SMTP = %s, "
               "Address = %s" ,
               mName,
               validate ? "true" : "false",
               sender->remote ? "true" : "false",
               bflag[F_FASTSMTP] ? "true" : "false",
               sender->address );
#endif
      if ( validate && (sender->remote || bflag[F_FASTSMTP] ))
         return DeliverSMTP( imf,
                             sender,
                             address,
                             hostp->via );
#else
      printmsg(1,"SMTP not enabled, queuing mail for "
                 "%s@%s for deferred processing",
                 user, node );
#endif

      return DeliverRemote( imf,
                            sender,
                            address,
                            E_nodename );
   }

/*--------------------------------------------------------------------*/
/*                       Handle local delivery                        */
/*--------------------------------------------------------------------*/

   if (equal(path, E_nodename))     /* Route via Local node?          */
   {
      if (equal( HostAlias( node ), E_nodename )) /* To local node?   */
         return DeliverLocal( imf,
                              sender,
                              user,
                              validate );
                                 /* Yes --> Deliver                   */
      else
         return Bounce( imf,
                        sender,
                        "No known delivery path for host",
                         node,
                         address,
                         validate );
   }  /* if */

/*--------------------------------------------------------------------*/
/*         Deliver mail to a system directly connected to us          */
/*--------------------------------------------------------------------*/

   if (equal(path,node))   /* Directly connected system?        */
      return DeliverRemote( imf,
                            sender,
                            user,
                            path);  /* Yes                           */

/*--------------------------------------------------------------------*/
/*       Default remote delivery; strip this node and the directly    */
/*       connected system from the address, then deliver to the       */
/*       next hop on the route                                        */
/*--------------------------------------------------------------------*/

   strcpy(user,address);      /* Buffer used for deliver addr  */

   strcpy(node,address);      /* Work buffer                   */
   token = strtok(node,"!");  /* Get first host in path        */

   if (equal( HostAlias(token), E_nodename)) /* Local system?  */
   {
      token =  strtok(NULL,"");  /* Yes --> Get rest of addr   */
      strcpy(user, token);       /* Use it for address         */
      token = strtok(token,"!"); /* Get next host in path      */
   } /* if */

   if (equal( HostAlias(token), path ))  /* Next system?       */
   {
      token =  strtok(NULL,"");  /* Yes --> Get rest of addr   */
      strcpy(user, token);       /* Use it for address         */
   } /* if */

   if (!strpbrk(user,"!@"))      /* Any host delimiters?       */
   {                             /* No --> Check for % routing */
      token = strrchr(user,'%');    /* Get last percent sign   */
      if (token != NULL)
         *token = '@';           /* Make it an RFC-822 address */
      else
         printmsg(0,"Deliver: Cannot find node in \"%s\"",
               user);            /* That's odd, it should not  */
                                 /* be a local address!        */
   } /* if */

   return DeliverRemote( imf,
                         sender,
                         user,
                         path );

} /* Deliver */

/*--------------------------------------------------------------------*/
/*    D e l i v e r L o c a l                                         */
/*                                                                    */
/*    Handle local delivery, including optional forwarding            */
/*--------------------------------------------------------------------*/

static size_t DeliverLocal( IMFILE *imf,        /* Input file name    */
                            const MAIL_ADDR *sender,
                            char *user,         /* Target address     */
                            KWBoolean validate) /* KWTrue = validate,
                                                forward user's mail   */
{
   char mboxname[FILENAME_MAX];
   struct UserTable *userp = NULL;
   ALIASTABLE *aliasp = NULL;
   size_t delivered = 0;
   KWBoolean announce = KWFalse;
   KWBoolean isPostmaster = KWFalse;
   time_t mboxTime = 0;             /* Assume no mailbox */
   FILE *mBoxStream;

/*--------------------------------------------------------------------*/
/*    If the parameter is the postmaster, use the configuration       */
/*    defined value for the postmaster                                */
/*--------------------------------------------------------------------*/

   if (equali(user, POSTMASTER))
   {
#ifdef UDEBUG32
      printmsg(2, "DeliverLocal: Using %s as %s",
               E_postmaster,
               POSTMASTER );
#endif

      user = E_postmaster;
      isPostmaster = KWTrue;
   }

/*--------------------------------------------------------------------*/
/*             Validate user id and check for forwarding              */
/*--------------------------------------------------------------------*/

   if (validate)
   {
      userp = checkuser(user);   /* Locate user id in host table      */

/*--------------------------------------------------------------------*/
/*                     Process any system aliases                     */
/*--------------------------------------------------------------------*/

      aliasp = checkalias( user );  /* System alias?             */

      if ( (aliasp != NULL) && ! aliasp->recurse )
      {
         aliasp->recurse = KWTrue;
         delivered += ExpandAliasFile( imf,
                                       sender,
                                       SysAliases,
                                       aliasp->start,
                                       aliasp->end,
                                       &announce ,
                                       userp,
                                       validate,
                                       user );
         aliasp->recurse = KWFalse;

         /* Announce, if requested, only if remote sender and valid
            local user */
         if ( announce && ( userp != BADUSER ) && sender->remote )
            trumpet( userp->beep, sender->daemon, mboxTime );

         return delivered;

      } /* if */

      if ( isPostmaster )
         validate = KWFalse;  /* Don't loop delivering to postmast */

/*--------------------------------------------------------------------*/
/*             No system alias, verify the user is valid              */
/*--------------------------------------------------------------------*/

      if ( userp == BADUSER )    /* Invalid user id?                  */
      {                          /* Yes --> Dump in trash bin         */

         return Bounce( imf,
                        sender,
                        "Invalid local address "
                              "(not defined in PASSWD or ALIASES)",
                        user,
                        user,
                        validate );
      } /* if */

/*--------------------------------------------------------------------*/
/*               The user id validated; handle the mail               */
/*--------------------------------------------------------------------*/

      mkfilename(mboxname, userp->homedir, DOTFORWARD);

      if (access( mboxname, 0 )) /* The .forward file exists?         */
         announce = KWTrue;       /* No --> Fall through               */
      else {
         delivered += ExpandAliasFile( imf,
                                       sender,
                                       mboxname,
                                       0,
                                       LONG_MAX,
                                       &announce,
                                       userp,
                                       validate,
                                       user );

         if (announce && sender->remote)   /* Did we deliver mail locally? */
            trumpet( userp->beep, sender->daemon, mboxTime );
                                    /* Yes --> Inform the user      */
         return delivered;

      } /* if */

   } /* if (validate) */

/*--------------------------------------------------------------------*/
/*       The user is valid (or not validated) and not forwarded       */
/*--------------------------------------------------------------------*/

   if ((*user == '/') || (isalpha( *user ) && user[1] == ':'))
                              /* Absolute path from recursive call?   */
      strcpy(mboxname, user); /* Yes --> Use it as-is                 */
   else if ( bflag[F_UNIQUEMBOX] )  /* Deliver one message per file? */
      uniqueMailBoxName( user, mboxname );
                                    /* Yes --> Create file mask      */
   else
      mkmailbox(mboxname, user);    /* No --> Build normal name      */

   if ( ! access( mboxname, 0 ))
      mboxTime = stater( mboxname, NULL );

#ifdef UDEBUG
   printmsg( 4,"DeliverLocal: Sender is %s (%s at %s via %s)",
            sender->address,
            sender->user,
            sender->host,
            (sender->relay == NULL) ? "*local*" : sender->relay );
#endif

   printmsg(1,"DeliverLocal: Delivering mail %sfrom %s to %s",
              formatFileSize( imf ),
              sender->address,
              user );

   if ( announce && sender->remote )
      trumpet(userp->beep, sender->daemon, mboxTime);
                           /* Local delivery, inform the user      */

   mBoxStream = FOPEN( mboxname , "a",TEXT_MODE );
   if (mBoxStream == NULL )
   {
      printerr(mboxname);
      printmsg(0,"Cannot open mailbox \"%s\" for output",
                  mboxname);
      panic();
   }

   if (!(isatty(fileno(mBoxStream)) || bflag[F_UNIQUEMBOX] ))
      fputs(MESSAGESEP,mBoxStream); /* Write separator line          */

   putFromLine( sender, KWFalse, mBoxStream );

   return CopyData( imf, sender, KWFalse, mBoxStream );

} /* DeliverLocal */

/*--------------------------------------------------------------------*/
/*       E x p a n d A l i a s F i l e                                */
/*                                                                    */
/*       Process a local or system aliases file                       */
/*--------------------------------------------------------------------*/

static size_t ExpandAliasFile( IMFILE *imf,
                               const MAIL_ADDR *sender,
                               const char *fwrdname,
                               const long start,
                               const long end,
                               KWBoolean *announce,
                               struct UserTable *userp,
                               const KWBoolean validate,
                               const char *user )
{
   char buf[BUFSIZ];
   FILE *fwrd = FOPEN(fwrdname, "r",TEXT_MODE);
   char *cwd = ( ! userp) ? E_tempdir : userp->homedir;
   size_t delivered = 0;

   if ( fwrd == NULL )
   {
      printerr( fwrdname );
      return Bounce( imf,
                     sender,
                     "Cannot open forward file",
                     fwrdname,
                     user,
                     validate );
   }

   if ( start != 0 )
      fseek( fwrd, start, SEEK_SET);

   while((ftell(fwrd) < end) && (fgets( buf, BUFSIZ, fwrd) != NULL ))
   {
      char *s;
      char c;
      char *nextfile = NULL;

/*--------------------------------------------------------------------*/
/*     Kill both leading and trailing white space from this line      */
/*--------------------------------------------------------------------*/

      s = buf + strlen(buf) - 1;
      while( (s >= buf) && !isgraph(*s))
         *s-- = '\0';                  /* Trim trailing white space     */

      s = buf;
      while( *s && ! isgraph( *s ))    /* Trim leading white space     */
         s++;

/*--------------------------------------------------------------------*/
/*                     Now process the input line                     */
/*--------------------------------------------------------------------*/

      printmsg(2,"Expanding %s via %s to \"%s\"", user, fwrdname, s);

      if ( equalni( buf, INCLUDE, strlen(INCLUDE)))
      {
         nextfile = strtok( s + strlen(INCLUDE), WHITESPACE );
         if ( nextfile == NULL )
         {
            return Bounce(imf,
                          sender,
                          "Missing forwarding file for alias",
                          fwrdname,
                          user,
                          validate );
         }
         else
            c = ':';
      } /* if */
      else if ( isalpha(*s ) && (s[1] == ':'))  /* Drive name?    */
         c = '/';             /* Yes --> flag as absolute path    */
      else if ( *s == ':')    /* Avoid false triggers ...         */
         c = ' ';             /* ... by making it general case    */
      else                    /* Handle other cases in switch ... */
         c = *s;

      switch(c)
      {
         case '#':
            break;                  /* Comment, ignore            */

         case '\0':                 /* Empty line, ignore         */
            break;

         case '|':                  /* Pipe mail into a command   */
         {
            long here = ftell(fwrd);

            fclose(fwrd);
            PushDir( cwd );
            printmsg(1,"Piping mail%s from %s for %s into %s",
                        formatFileSize( imf ),
                        sender->address,
                        user,
                        s + 1 );

            imrewind( imf );
            executeIMFCommand( s + 1, imf, NULL, KWTrue, KWFalse );
            PopDir();
            delivered += 1;
            fwrd = FOPEN(fwrdname, "r", TEXT_MODE);
            fseek( fwrd, here, SEEK_SET);
            break;
         } /* case */

         case '\\':              /* Deliver without forwarding */
            delivered += Deliver( imf,
                                  sender,
                                  &s[1],
                                  KWFalse );
            *announce = KWTrue;
            break;

         case ':':
         {
            char fname[FILENAME_MAX];
            strcpy( fname, nextfile);
            expand_path(nextfile, NULL, cwd, E_mailext);
            delivered += ExpandAliasFile( imf,
                                          sender,
                                          nextfile,
                                          0,
                                          LONG_MAX,
                                          announce,
                                          userp,
                                          KWTrue,
                                          user );
            break;
         }

         case '>':               /* Deliver to V-mail address specified */
               delivered += DeliverVMS( imf,
                                        sender,
                                        s + 1,
                                        validate );
               break;

         case '/':               /* Save in absolute path name */
         case '~':
            if (expand_path(s, E_confdir, cwd, E_mailext) == NULL )
            {
               return Bounce(imf,
                             sender,
                             "Invalid path in forwarding file name",
                             s,
                             user,
                             validate );

            }
            else
               delivered += DeliverLocal( imf,
                                          sender,
                                          s,
                                          KWFalse );
            *announce = KWTrue;
            break;

         default:                /* Deliver normally           */
              delivered += Deliver( imf,
                                    sender,
                                    s,
                                    validate );

      } /* switch */

   } /* while */

   fclose( fwrd );

/*--------------------------------------------------------------------*/
/*            Report if we could not deliver to any users             */
/*--------------------------------------------------------------------*/

   if ( ! delivered )
      return Bounce(imf,
                    sender,
                    "No addresses to forward to",
                    fwrdname,
                    user,
                    validate );
   else
      return delivered;             /* Report success to caller      */

} /* ExpandAliasFile */

/*--------------------------------------------------------------------*/
/*    D e l i v e r G a t e w a y                                     */
/*                                                                    */
/*    Deliver mail via a gateway program                              */
/*--------------------------------------------------------------------*/

static size_t DeliverGateway(   IMFILE *imf,
                                const MAIL_ADDR *sender,
                                const char *user,
                                const char *node,
                                const struct HostTable *hostp,
                                const KWBoolean validate )
{
   char command[BUFSIZ];
   int rc;

/*--------------------------------------------------------------------*/
/*    Format the command and tell the user what we're going to do     */
/*--------------------------------------------------------------------*/

   sprintf(command , "%s %s %s %s %s %s",
                     hostp->via,       /* Program to perform forward */
                     hostp->hostname,  /* Nominal host routing via   */
                     node,             /* Final destination system   */
                     user,             /* user on "node" for delivery*/
                     sender->host,     /* Originating node           */
                     sender->user );   /* Originating user           */

   printmsg(3,"DeliverGateway: %s", command);

   printmsg(1,"Gatewaying mail %sfrom %s to %s@%s via %s using \"%s\"",
               formatFileSize( imf ),
               sender->address,
               user,
               node,
               hostp->hostname,
               hostp->via);

/*--------------------------------------------------------------------*/
/*  Run the command and return caller with count of mail delivered    */
/*--------------------------------------------------------------------*/

   imrewind( imf );
   rc = executeIMFCommand( command, imf, NULL, KWTrue, KWFalse );

   if ( rc == 0 )
      return 1;
   else {
      char who[MAXADDR];

      sprintf( who, "%s@%s", user, node );
      return Bounce( imf,
                     sender,
                     "Gateway command returned non-zero exit status",
                     command,
                     who,
                     validate );
   } /* else */

} /* DeliveryGateway */

/*--------------------------------------------------------------------*/
/*       D e l i v e r V M S                                          */
/*                                                                    */
/*       Deliver mail into the queue of Rick Vandenburg's V-Mail      */
/*       server.  This is a nasty hack to overcome DOS <--> OS/2      */
/*       problems.                                                    */
/*--------------------------------------------------------------------*/

static size_t DeliverVMS( IMFILE *imf,          /* Input file name    */
                          const MAIL_ADDR *sender,
                          char *user,     /* Target address           */
                          KWBoolean validate)  /* Validate/forward
                                                local mail            */
{
   char dname[FILENAME_MAX];
   char xname[FILENAME_MAX];
   char *seq = jobNumber( getSeq(), 4, KWTrue );
                                 /* monocase for internal DOS usage  */
   FILE *stream;

   if (( E_vmsQueueDir == NULL ) || (E_vmail == NULL ))
      return Bounce( imf,
                     sender,
                     ( E_vmsQueueDir == NULL ) ?
                              "VMSQueueDir not defined" :
                              "VMail program name not defined",
                     NULL,
                     user,
                     validate );

   sprintf( dname, "%s/uupc%s.d", E_vmsQueueDir, seq );
   sprintf( xname, "%s/uupc%s.x", E_vmsQueueDir, seq );

/*--------------------------------------------------------------------*/
/*                  Now create the actual data file                   */
/*--------------------------------------------------------------------*/

   stream = FOPEN(dname, "w", TEXT_MODE );
   if (stream == NULL )
   {
      printerr(dname);
      return Bounce( imf,
                     sender,
                     "Cannot open V-mail data file",
                     dname,
                     user,
                     validate );
   }

   if (!CopyData( imf, sender, KWFalse, stream ))
   {
      REMOVE( dname );
      return 0;
   }

/*--------------------------------------------------------------------*/
/*       Now create the command file, a simple cousin to a UUCP       */
/*       command file                                                 */
/*--------------------------------------------------------------------*/

   stream = FOPEN(xname, "w", TEXT_MODE );

   if (stream == NULL )
   {
      printerr(xname);
      return Bounce( imf,
                     sender,
                     "Cannot open V-mail command file",
                     xname,
                     user,
                     validate );
   }

   denormalize( dname );
   denormalize( E_vmail );
   fprintf( stream, "D %s\nC %s %s\n", dname, E_vmail, user );

   if ( ferror( stream ))
   {
      perror( xname );
      panic();
   }

   fclose( stream );

   printmsg(1,"Queueing mail %s from %s for V-mail alias %s",
               formatFileSize( imf ),
               sender->address,
               user );

   return 1;

} /* DeliverVMS */

/*--------------------------------------------------------------------*/
/*    D e l i v e r R e m o t e                                       */
/*                                                                    */
/*    Perform control processing for delivery to another UUCP node    */
/*--------------------------------------------------------------------*/

size_t
DeliverRemote( IMFILE *imf,               /* Input file name      */
               const MAIL_ADDR *sender,
               const char *address,       /* Target address       */
               const char *path)
{

   static char *savePath = NULL;    /* System we previously queued for*/
   static char everyone[512];       /* People we queued for via system*/

/*--------------------------------------------------------------------*/
/*            Flush previously queued addresses, if needed            */
/*--------------------------------------------------------------------*/

   if (savePath != NULL)
   {
      KWBoolean queueNow = KWFalse;

      if (( E_maxuuxqt <= 0 ) || ( E_maxuuxqt > sizeof everyone ))
         E_maxuuxqt = sizeof everyone;

      if ( path == NULL )
         queueNow = KWTrue;
      else if ( ! equal(savePath, path))
         queueNow = KWTrue;
      else if ((strlen(everyone) + strlen(address) + 2) > E_maxuuxqt)
         queueNow = KWTrue;

      if ( queueNow )
      {
         queueRemote( imf,
                      sender,
                      everyone,
                      savePath,
                      grade );
         savePath = NULL;
      }

   } /* if (savePath != NULL) */

/*--------------------------------------------------------------------*/
/*                Return if we only flushing the cache                */
/*--------------------------------------------------------------------*/

   if ( path == NULL )
      return 0;

/*--------------------------------------------------------------------*/
/*               Report and queue the current delivery                */
/*--------------------------------------------------------------------*/

   printmsg(1,"Spooling mail %sfrom %s to %s via %s",
               formatFileSize( imf ),
               sender->address,
               address ,
               path);

   if ( savePath == NULL )
      strcpy( everyone, "rmail");

   strcat(everyone, " ");
   strcat(everyone, address);

/*--------------------------------------------------------------------*/
/*       Either set up for queuing more addresses, or deliver what    */
/*       we just received                                             */
/*--------------------------------------------------------------------*/

   if (bflag[F_MULTI])        /* Deliver to multiple users at once?   */
      savePath = newstr(path);   /* Yes --> Save routing info         */
   else
      return queueRemote( imf,
                          sender,
                          everyone,
                          path,
                          grade);

   return 1;

} /* DeliverRemote */

/*--------------------------------------------------------------------*/
/*       b o u n c e                                                  */
/*                                                                    */
/*       Report failed mail to a user.  Based on code contributed     */
/*       by Kevin Meyer <kmeyer@sauron.alt.za>                        */
/*                                                                    */
/*       This code has a major hole in that the address it replies    */
/*       to is weak, really having been previously only been used     */
/*       for internal messages.  Perhaps the full address from the    */
/*       UUCP From line should be used.                               */
/*--------------------------------------------------------------------*/

size_t Bounce( IMFILE *imf,
               const MAIL_ADDR *sender,
               const char *text,
               const char *data,
               const char *failedAddress ,
               const KWBoolean validate )
{
   FILE *newfile;
   char tname[FILENAME_MAX]; /* name of temporary file used */
   char buf[BUFSIZ];
   MAIL_ADDR daemon;
   char daemonAddress[MAXADDR];

   KWBoolean bounce = bflag[F_BOUNCE];

   memset( &daemon, 0, sizeof daemon );
   daemon.host = E_domain;
   daemon.user = "uucp";

   strcpy( daemonAddress, daemon.user );
   strcat( daemonAddress, "@" );
   strcat( daemonAddress, daemon.host );
   daemon.address = daemonAddress;

   printmsg(0,"Bounce: Mail from %s for %s failed, %s: %s",
              sender->address,
              failedAddress,
              text,
              (data == NULL) ? "(no data)" : data );

/*--------------------------------------------------------------------*/
/*           Never bounce mail to a select list of user ids           */
/*--------------------------------------------------------------------*/

   if ( equali( sender->user, "postmaster") ||
        equali( sender->user, "uucp") ||
        equali( sender->user, "root") ||
        equali( sender->user, "mmdf") ||
        equali( sender->user, "mailer-daemon"))
      bounce = KWFalse;

   if ( ! bounce )
     return Deliver( imf, &daemon, POSTMASTER, validate );

   mktempname( tname , "tmp");  /* Generate a temp file name           */

   if ((newfile = FOPEN(tname, "w", TEXT_MODE ))==NULL)
   {
       printerr( tname );
       panic();
   };

   fprintf(newfile,
     "Dear %s,\n"
     "Your message for address <%s> could not be delivered at system\n"
     "%s (uucp node %s) for the following reason:\n\t\t%s.\n",
                  sender->user,
                  failedAddress,
                  E_domain,
                  E_nodename,
                  text );

   if ( data != NULL )
   {
      fputs("The problem address or file in question was: ",newfile);
      fputs(data, newfile);
      fputc('\n', newfile);
   }

      fprintf(newfile,
              "\nA copy of the failed mail follows.\n\n"
              "Electronically Yours,\n"
              "%s %s UUCP mailer daemon\n",
              compilep, compilev );

    fputs("\n------ Failed Message Follows -----\n", newfile);

    imrewind( imf );
    imunload( newfile, imf );
    fclose(newfile);

/*--------------------------------------------------------------------*/
/*          Recursively invoke RMAIL to deliver our message           */
/*--------------------------------------------------------------------*/

   putenv("LOGNAME=uucp");

   sprintf( buf,
            "-w -F %s -s \"Failed mail for %.20s\" -- %s -c postmaster",
            tname,
            failedAddress,
            sender->address );

    if ( execute( myProgramName, buf, NULL, NULL, KWTrue, KWFalse ))
    {
         printerr("execute");
         DeliverLocal( imf, &daemon, POSTMASTER, validate);
    }

    printmsg(2,"bounce: rmail delivery complete.");

    return (1);

} /* Bounce */

/*--------------------------------------------------------------------*/
/*       f l u s h Q u e u e s                                        */
/*                                                                    */
/*       Commit mail for queued addresses batched together            */
/*--------------------------------------------------------------------*/

void
flushQueues( IMFILE *imf,
             const MAIL_ADDR *sender )
{
#ifdef TCPIP
   DeliverSMTP( imf, sender, NULL, NULL );
#endif

   DeliverRemote( imf, sender, NULL, NULL );

} /* flushQueues */

/*--------------------------------------------------------------------*/
/*       s e t D e l i v e r y G r a d e                              */
/*                                                                    */
/*       Set the delivery grade for UUCP mail                         */
/*--------------------------------------------------------------------*/

void
setDeliveryGrade( const char inGrade )
{
   grade = inGrade;
}
