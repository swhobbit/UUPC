/*--------------------------------------------------------------------*/
/*       d e l i v e r  . c                                           */
/*                                                                    */
/*       UUPC/extended mail delivery subroutines                      */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1997 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: deliver.c 1.55 1997/11/25 05:05:06 ahd v1-12t $
 *
 *    $Log: deliver.c $
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

/*--------------------------------------------------------------------*/
/*    Embedded Japanese support provided by Kenji Rikitake            */
/*    28-AUG-1991                                                     */
/*                                                                    */
/*    On Japanese support:                                            */
/*                                                                    */
/*    Japanese MS-DOS uses a 2byte Kanji (Japanese ideogram) code     */
/*    called "Shift-JIS".  This cannot be delivered via SMTP since    */
/*    Shift-JIS maps its first byte from 0x80-0x9f and 0xe0-0xfc.     */
/*    JUNET requests all hosts to send Kanji in a 7bit subset of      */
/*    ISO2022.  This is commonly called "JIS 7bit".                   */
/*                                                                    */
/*    To provide Japanese functionality, you need to convert all      */
/*    remote delivery messages to JIS 7bit, and all local delivery    */
/*    messages to Shift-JIS.                                          */
/*--------------------------------------------------------------------*/

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

#include "imfile.h"
#include "address.h"
#include "deliver.h"
#include "expath.h"
#include "execute.h"
#include "getseq.h"
#include "kanjicnv.h"
#include "hostable.h"
#include "import.h"
#include "pushpop.h"
#include "security.h"
#include "stater.h"
#include "usertabl.h"
#include "sysalias.h"
#include "timestmp.h"
#include "trumpet.h"
#include "arpadate.h"

#ifdef TCPIP
#include "delivers.h"
#endif

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

 char fromUser[MAXADDR] = ""; /* User id of originator               */
 char fromNode[MAXADDR] = ""; /* Node id of originator               */
 char *myProgramName = "rmail";  /* Name for recursive invocation    */
 char grade = 'C';            /* Default grade for mail sent         */

 KEWSHORT hops = 0;              /* Number hops this mail has seen   */

 KWBoolean remoteMail = KWFalse;

 char *ruser = NULL;
 char *rnode = NULL;
 char *uuser = "uucp";        /* Default actual user                 */

/*--------------------------------------------------------------------*/
/*        Define current file name for panic() and printerr()         */
/*--------------------------------------------------------------------*/

currentfile();

/*--------------------------------------------------------------------*/
/*                        Internal prototypes                         */
/*--------------------------------------------------------------------*/

static size_t DeliverLocal( IMFILE *imf,        /* Input file name    */
                          char *user,     /* Target address           */
                          KWBoolean validate); /* Validate/forward
                                                local mail            */

static size_t DeliverFile( IMFILE *imf,
                        const char *mboxname,
                        const long start,
                        const long end,
                        KWBoolean *announce,
                        struct UserTable *userp,
                        const KWBoolean validate,
                        const char *user );

static size_t queueRemote( IMFILE *imf,   /* Input file name          */
                    const char *address,  /* Target address           */
                    const char *path);

static size_t DeliverVMS( IMFILE *imf,          /* Input file name    */
                          char *user,     /* Target address           */
                          KWBoolean validate); /* Validate/forward
                                                local mail            */

static size_t DeliverGateway(   IMFILE *imf,
                                const char *user,
                                const char *node,
                                const struct HostTable *hostp,
                                const KWBoolean validate );

static KWBoolean CopyData(   const KWBoolean remotedelivery,
                             IMFILE *imf,
                             FILE *mbox);

static char *stats( IMFILE *imf );

/*--------------------------------------------------------------------*/
/*    D e l i v e r                                                   */
/*                                                                    */
/*    Deliver mail to one user                                        */
/*--------------------------------------------------------------------*/

size_t Deliver( IMFILE *imf,        /* Input file                    */
                const char *address,/* Target address                */
                KWBoolean validate) /* Validate/forward local mail   */
{
   char node[MAXADDR];
   char path[MAXADDR];
   char user[MAXADDR];
   char *token;
   struct HostTable *hostp;

   if ( ! tokenizeAddress(address, path, node, user) )
   {
      return Bounce( imf,
                     path,
                     address,
                     address,
                     validate );
   }

/*--------------------------------------------------------------------*/
/*                   Deliver to a gateway if needed                   */
/*--------------------------------------------------------------------*/

   hostp = checkname( path );

/*--------------------------------------------------------------------*/
/*                    Do we need loop protection?                     */
/*--------------------------------------------------------------------*/

   if (!equal(path, E_nodename) && (hops > E_maxhops))
      return Bounce(imf,
             "Excessive number of hops",
             address,
             address,
             validate );

/*--------------------------------------------------------------------*/
/*                      Handle gateway delivery                       */
/*--------------------------------------------------------------------*/

   if ( (hostp != BADHOST) && (hostp->status.hstatus == HS_GATEWAYED))
      return DeliverGateway( imf, user, node, hostp, validate );

/*--------------------------------------------------------------------*/
/*                 Handle SMTP delivery, if supported                 */
/*--------------------------------------------------------------------*/

   if ( (hostp != BADHOST) && (hostp->status.hstatus == HS_SMTP))
   {
#ifdef TCPIP
      if ( validate && (remoteMail || bflag[F_FASTSMTP] ))
         return DeliverSMTP( imf, address, hostp->via );
#else
      printmsg(1,"SMTP not available, queuing mail for %s@%s locally",
                 user, node );
#endif

      return DeliverRemote( imf, address, E_nodename );
   }

/*--------------------------------------------------------------------*/
/*                       Handle local delivery                        */
/*--------------------------------------------------------------------*/

   if (equal(path, E_nodename))     /* Route via Local node?          */
   {
      if (equal( HostAlias( node ), E_nodename )) /* To local node?   */
         return DeliverLocal( imf, user, validate );
                                 /* Yes --> Deliver                   */
      else
         return Bounce( imf,
                 "No known delivery path for host",
                  node,
                  address,
                  validate );
   }  /* if */

/*--------------------------------------------------------------------*/
/*         Deliver mail to a system directly connected to us          */
/*--------------------------------------------------------------------*/

   if (equal(path,node))   /* Directly connected system?        */
      return DeliverRemote( imf, user, path); /* Yes            */

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

   return DeliverRemote( imf, user, path );

} /* Deliver */

/*--------------------------------------------------------------------*/
/*    D e l i v e r L o c a l                                         */
/*                                                                    */
/*    Handle local delivery, including optional forwarding            */
/*--------------------------------------------------------------------*/

static size_t DeliverLocal( IMFILE *imf,        /* Input file name    */
                          char *user,           /* Target address     */
                          KWBoolean validate)   /* KWTrue = validate,
                                                forward user's mail   */
{
   char mboxname[FILENAME_MAX];
   struct UserTable *userp = NULL;
   ALIASTABLE *aliasp = NULL;
   size_t delivered = 0;
   KWBoolean announce = KWFalse;
   FILE *mbox;

/*--------------------------------------------------------------------*/
/*    If the parameter is the postmaster, use the configuration       */
/*    defined value for the postmaster                                */
/*--------------------------------------------------------------------*/

   if (equali(user, POSTMASTER))
      user = E_postmaster;

/*--------------------------------------------------------------------*/
/*             Validate user id and check for forwarding              */
/*--------------------------------------------------------------------*/

   if (validate)
   {
      if ( equali( E_postmaster , user) )
         validate = KWFalse;     /* Don't loop delivering to postmast */

      userp = checkuser(user);   /* Locate user id in host table      */

/*--------------------------------------------------------------------*/
/*                     Process any system aliases                     */
/*--------------------------------------------------------------------*/

      aliasp = checkalias( user );  /* System alias?             */

      if ( (aliasp != NULL) && ! aliasp->recurse )
      {
         aliasp->recurse = KWTrue;
         delivered += DeliverFile( imf,
                                   SysAliases,
                                   aliasp->start,
                                   aliasp->end,
                                   &announce ,
                                   userp,
                                   validate,
                                   user );
         aliasp->recurse = KWFalse;

         if ( announce && ( userp != BADUSER ) && remoteMail )
            trumpet( userp->beep);  /* Yes --> Inform the user     */

         return delivered;

      } /* if */

/*--------------------------------------------------------------------*/
/*             No system alias, verify the user is valid              */
/*--------------------------------------------------------------------*/

      if ( userp == BADUSER )    /* Invalid user id?                  */
      {                          /* Yes --> Dump in trash bin         */
         return Bounce( imf,
                        "Invalid local address (not defined in PASSWD or ALIASES)",
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
         delivered += DeliverFile( imf,
                                   mboxname,
                                   0,
                                   LONG_MAX,
                                   &announce,
                                   userp,
                                   validate,
                                   user );

         if (announce && remoteMail)   /* Did we deliver mail locally? */
            trumpet( userp->beep);     /* Yes --> Inform the user      */
         return delivered;

      } /* if */

   } /* if (validate) */

/*--------------------------------------------------------------------*/
/*       The user is valid (or not validated) and not forwarded       */
/*--------------------------------------------------------------------*/

   if ((*user == '/') || (isalpha( *user ) && user[1] == ':'))
                              /* Absolute path from recursive call?   */
      strcpy(mboxname, user); /* Yes --> Use it as-is                 */
   else
      mkmailbox(mboxname, user);
                              /* No --> Build normal name             */

   printmsg(1,"Delivering mail %sfrom %s%s%s to %s",
                        stats( imf ),
                        ruser,
                        remoteMail ? "@" : "",
                        remoteMail ? rnode : "",
                         user );

   if ( announce && remoteMail )
      trumpet( userp->beep);  /* Local delivery, inform the user      */

   mbox = FOPEN( mboxname , "a",TEXT_MODE );
   if (mbox == NULL )
   {
      printerr(mboxname);
      printmsg(0,"Cannot open mailbox \"%s\" for output",
                  mboxname);
      panic();
   }

   if (!isatty(fileno(mbox)))
      fputs(MESSAGESEP,mbox); /* Write separator line                 */

   return CopyData( KWFalse, imf , mbox );

} /* DeliverLocal */

/*--------------------------------------------------------------------*/
/*       D e l i v e r F i l e                                        */
/*                                                                    */
/*       Process a local or system aliases file                       */
/*--------------------------------------------------------------------*/

static size_t DeliverFile( IMFILE *imf,
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
            printmsg(1,"Piping mail%s from %s@%s for %s into %s",
                        stats( imf ),
                        ruser,
                        rnode,
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
            delivered += Deliver( imf, &s[1], KWFalse );
            *announce = KWTrue;
            break;

         case ':':
         {
            char fname[FILENAME_MAX];
            strcpy( fname, nextfile);
            expand_path(nextfile, NULL, cwd, E_mailext);
            delivered += DeliverFile( imf,
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
               delivered += DeliverVMS( imf, s + 1, validate );
               break;

         case '/':               /* Save in absolute path name */
         case '~':
            if (expand_path(s, E_confdir, cwd, E_mailext) == NULL )
            {
               return Bounce(imf,
                             "Invalid path in forwarding file name",
                             s,
                             user,
                             validate );

            }
            else
               delivered += DeliverLocal( imf, s, KWFalse );
            *announce = KWTrue;
            break;

         default:                /* Deliver normally           */
              delivered += Deliver( imf, s, validate );

      } /* switch */

   } /* while */

   fclose( fwrd );

/*--------------------------------------------------------------------*/
/*            Report if we could not deliver to any users             */
/*--------------------------------------------------------------------*/

   if ( ! delivered )
      return Bounce(imf,
                    "No addresses to forward to",
                    fwrdname,
                    user,
                    validate );
   else
      return delivered;             /* Report success to caller      */

} /* DeliverFile */

/*--------------------------------------------------------------------*/
/*    D e l i v e r G a t e w a y                                     */
/*                                                                    */
/*    Deliver mail via a gateway program                              */
/*--------------------------------------------------------------------*/

static size_t DeliverGateway(   IMFILE *imf,
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
                     hostp->via,          /* Program to perform forward */
                     hostp->hostname,     /* Nominal host routing via  */
                     node ,               /* Final destination system  */
                     user,                /* user on "node" for delivery*/
                     rnode,               /* Originating node           */
                     ruser );             /* Originating user           */

   printmsg(3,"DeliverGateway: %s", command);

   printmsg(1,
      "Gatewaying mail %sfrom %s@%s to %s@%s via %s using \"%s\"",
       stats( imf ),
       ruser, rnode, user, node, hostp->hostname, hostp->via);

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
                     "Gateway command returned non-zero exit status",
                     command,
                     who,
                     validate );
   } /* else */

} /* DeliveryGateway */

#ifdef TCPIP
/*--------------------------------------------------------------------*/
/*    D e l i v e r S M T P                                           */
/*                                                                    */
/*    Perform control processing for delivery to another UUCP node    */
/*--------------------------------------------------------------------*/

size_t DeliverSMTP( IMFILE *imf,          /* Input file name          */
                    const char *address,  /* Target address           */
                    const char *path)
{

   static char *savePath = NULL;    /* System we previously queued for*/
   static char *addrList[50];
   static int subscript = 0;
   static int addressMax = sizeof addrList / sizeof addrList[0];

/*--------------------------------------------------------------------*/
/*            Flush previously queued addresses, if needed            */
/*--------------------------------------------------------------------*/

   if (subscript)
   {
      KWBoolean queueNow = KWFalse;

      if ( path == NULL )
         queueNow = KWTrue;
      else if ( ! equal(savePath, path))
         queueNow = KWTrue;
      else if ( subscript >= addressMax )
         queueNow = KWTrue;

/*--------------------------------------------------------------------*/
/*                We need to actually perform delivery                */
/*--------------------------------------------------------------------*/

      if ( queueNow )
      {
         KWBoolean noConnect = KWFalse;
         char fromAddr[MAXADDR];

         if ( equal( fromNode , E_nodename ))   /* Local address */
            sprintf( fromAddr, "%s@%s",
                     fromUser,
                     E_fdomain);
         else if ( strchr( fromNode, '.' ) == NULL )  /* remote UUCP */
            sprintf( fromAddr, "%s!%s@%s",
                     fromNode,
                     fromUser,
                     E_fdomain);
         else
            sprintf( fromAddr, "%s@%s",      /* Remote domain address */
                     fromUser,
                     fromNode);

         if ( ! ConnectSMTP( imf,
                             savePath,
                             fromAddr,
                             addrList,
                             subscript,
                             KWTrue ) )
               noConnect = KWTrue;

         while( subscript-- > 0 )
         {
            /* Queue failed SMTP mail for local node for retry */
            if ( noConnect )
               DeliverRemote( imf, addrList[subscript], E_nodename );

            free( addrList[subscript] );
         }

         subscript = 0;

         /* Flush UUCP queue if we put entries in it */
         if ( ! noConnect )
         {
            DeliverRemote( imf, NULL, NULL );
            return 0;
         }

      } /* if ( queueNow && subscript ) */

   } /* if (savePath != NULL) */

/*--------------------------------------------------------------------*/
/*                Return if we only flushing the cache                */
/*--------------------------------------------------------------------*/

   if ( path == NULL )
      return 0;

/*--------------------------------------------------------------------*/
/*               Report and queue the current delivery                */
/*--------------------------------------------------------------------*/

   printmsg(1,"Queuing SMTP mail %sfrom %s%s%s to %s via %s",
               stats( imf ),
               ruser,
               remoteMail ? "@" : "",
               remoteMail ? rnode : "",
               address ,
               path);

   savePath = newstr( path );
   addrList[subscript] = strdup( address );
   checkref( addrList[subscript] );
   subscript++;

   return 1;

} /* DeliverSMTP */

#endif

/*--------------------------------------------------------------------*/
/*       D e l i v e r V M S                                          */
/*                                                                    */
/*       Deliver mail into the queue of Rick Vandenburg's V-Mail      */
/*       server.  This is a nasty hack to overcome DOS <--> OS/2      */
/*       problems.                                                    */
/*--------------------------------------------------------------------*/

static size_t DeliverVMS( IMFILE *imf,          /* Input file name    */
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
                     "Cannot open V-mail data file",
                     dname,
                     user,
                     validate );
   }

   if (!CopyData( KWFalse, imf , stream ))
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

   printmsg(1,"Queueing mail %sfrom %s%s%s for V-mail alias %s",
               stats( imf ),
               ruser,
               remoteMail ? "@" : "",
               remoteMail ? rnode : "",
               user );

   return 1;

} /* DeliverVMS */

/*--------------------------------------------------------------------*/
/*    D e l i v e r R e m o t e                                       */
/*                                                                    */
/*    Perform control processing for delivery to another UUCP node    */
/*--------------------------------------------------------------------*/

size_t DeliverRemote( IMFILE *imf,        /* Input file name          */
                    const char *address,  /* Target address           */
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
         queueRemote( imf, everyone, savePath );
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

   printmsg(1,"Spooling mail %sfrom %s%s%s to %s via %s",
               stats( imf ),
               ruser,
               remoteMail ? "@" : "",
               remoteMail ? rnode : "",
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
      return queueRemote( imf, everyone, path );

   return 1;

} /* DeliverRemote */

/*--------------------------------------------------------------------*/
/*    q u e u e R e m o t e                                           */
/*                                                                    */
/*    Queue mail for delivery on another system via UUCP              */
/*--------------------------------------------------------------------*/

static size_t queueRemote( IMFILE *imf,   /* Input file               */
                    const char *command,  /* Target address           */
                    const char *path)     /* Node to queue for        */
{

   static const char spool_fmt[] = SPOOLFMT;  /* spool file name */
   static const char dataf_fmt[] = DATAFFMT;
   static const char send_cmd[]  = "S %s %s %s - %s 0666\n";

   char *seq = jobNumber( getSeq(), 3, bflag[F_ONECASE] );
   FILE *stream;              /* For writing out data                 */

   char msfile[FILENAME_MAX]; /* MS-DOS format name of files          */
   char msname[22];           /* MS-DOS format w/o path name          */

   char tmfile[15];           /* Call file, UNIX format name          */

   char ixfile[15];           /* eXecute file for remote system,
                                 UNIX format name for local system    */
   char rxfile[15];           /* Remote system UNIX name of eXecute
                                 file                                 */

   char idfile[15];           /* Data file, UNIX format name          */
   char rdfile[15];           /* Data file name on remote system,
                                 UNIX format                          */

   char *callFile = equal( E_nodename , path ) ? BIT_BUCKET : tmfile;
   char *dataFile = equal( E_nodename , path ) ? rdfile     : idfile;
   char *exqtFile = equal( E_nodename , path ) ? rxfile     : ixfile;

   sprintf(tmfile, spool_fmt, 'C', path,     grade , seq);
   sprintf(idfile, dataf_fmt, 'D', E_nodename , seq, 'd');
   sprintf(rdfile, dataf_fmt, 'D', E_nodename , seq, 'r');
   sprintf(ixfile, dataf_fmt, 'D', E_nodename , seq, 'e');
   sprintf(rxfile, dataf_fmt, 'X', E_nodename , seq, 'r');

/*--------------------------------------------------------------------*/
/*                     create remote X (xqt) file                     */
/*--------------------------------------------------------------------*/

   importpath( msname, exqtFile, path);
   mkfilename( msfile, E_spooldir, msname);

   stream = FOPEN(msfile, "w", IMAGE_MODE);

   if ( stream == NULL )
   {
      printerr(msfile);
      printmsg(0, "DeliverRemote: cannot open X file %s", msfile);
      return 0;
   } /* if */

   fprintf(stream, "U %s %s\n", uuser , E_nodename );
                                 /* Actual user running command      */
   fprintf(stream, "R %s@%s\n", ruser, rnode );
                                 /* Original requestor of command    */
   fprintf(stream, "F %s\n", rdfile );
                                 /* Required file for input          */
   fprintf(stream, "I %s\n", rdfile );
                                 /* stdin for command                */
   fprintf(stream, "C %s\n", command );
                                 /* Command to execute using file    */

/*--------------------------------------------------------------------*/
/*               Add some self-documenting information                */
/*--------------------------------------------------------------------*/

   fprintf(stream, "# Generated on %s by %s %s (built on %s %s)\n"
                   "# at %s\n",
                      E_nodename,
                      compilep,
                      compilev,
                      compiled,
                      compilet,
                      arpadate() );
   fprintf(stream, "# Call file    %s\n",    callFile );
   fprintf(stream, "# Execute file %s %s\n", idfile, rdfile );
   fprintf(stream, "# Data file    %s %s\n", ixfile, rxfile );

   fclose(stream);

/*--------------------------------------------------------------------*/
/*  Create the data file with the mail to send to the remote system   */
/*--------------------------------------------------------------------*/

   importpath(msname, dataFile, path);
   mkfilename( msfile, E_spooldir, msname);

   stream = FOPEN(msfile, "w", IMAGE_MODE);

   if (stream == NULL )
   {
      printerr(msfile);
      printmsg(0,
               "DeliverRemote: Cannot open spool file \"%s\" for output",
                msfile);
      return 0;
   }

   if (!CopyData( KWTrue, imf , stream ))
   {
      REMOVE( msfile );
      return 0;
   }

/*--------------------------------------------------------------------*/
/*                     create local C (call) file                     */
/*--------------------------------------------------------------------*/

   if ( equal( callFile, BIT_BUCKET ))
      return 1;

   importpath( msname, callFile, path);
   mkfilename( msfile, E_spooldir, msname);

   stream = FOPEN(msfile, "w",TEXT_MODE);

   if (stream == NULL)
   {
      printerr( msname );
      printmsg(0, "DeliverRemote: cannot open C file %s", msfile);
      return 0;
   }

   fprintf(stream, send_cmd, idfile, rdfile, uuser, idfile);
   fprintf(stream, send_cmd, ixfile, rxfile, uuser, ixfile);

   fclose(stream);

   return 1;

} /* queueRemote */

/*--------------------------------------------------------------------*/
/*       C o p y D a t a                                              */
/*                                                                    */
/*       Copy data into its final resting spot                        */
/*--------------------------------------------------------------------*/

static KWBoolean CopyData( const KWBoolean remotedelivery,
                     IMFILE *imf,
                     FILE *dataout)
{
   char buf[BUFSIZ];
   char trailer[BUFSIZ];
   size_t column = 0;
   size_t deliveryMode = ((size_t) remoteMail) * 2 +
                         ((size_t) remotedelivery);
   KWBoolean success = KWTrue;

   int (*put_string) (char *, FILE *) = (int (*)(char *, FILE *)) fputs;
                              /* Assume no Kanji translation needed   */
   time_t now;

   time( &now );

   sprintf(trailer, " %.24s", ctime( &now ));

   if ( !bflag[F_SHORTFROM] )
   {
      strcat( trailer, " remote from " );
      strcat( trailer, E_nodename );
   }

   imrewind( imf );

/*--------------------------------------------------------------------*/
/*    When we do the From line, we also determine if we must          */
/*    translate the data.  Note that the default is initialized to    */
/*    fputs() above.                                                  */
/*                                                                    */
/*    If Kanji is not enabled, don't translate it                     */
/*                                                                    */
/*    If local mail queued for local delivery, the data is already    */
/*    in Shift JIS, so don't translate it.                            */
/*                                                                    */
/*    If remote mail is queued for remote delivery, the data is       */
/*    already in JIS 7bit, so don't translate it.                     */
/*                                                                    */
/*    If delivering remote mail locally, translate to Shift JIS       */
/*                                                                    */
/*    If delivering local mail remotely, translate to JIS 7 bit       */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                        Generate a FROM line                        */
/*--------------------------------------------------------------------*/

   switch( deliveryMode )
   {
      case 3:                 /* Remote sender, remote delivery       */
         strcpy( buf, fromUser );
         strtok( buf, "!");   /* Get first host in list               */

         if ( bflag[ F_SUPPRESSFROM ] )
            break;            /* No operation                        */
         else if ( equal(HostAlias( buf ), fromNode ))
                              /* Host already in list?                */
         {                    /* Yes --> Don't do it twice            */
            fprintf(dataout, "From %s%s\n",
                    fromUser,
                    trailer );
         }
         else {                /* No --> Insert it                    */
            fprintf(dataout, "From %s!%s%s\n",
                    fromNode,
                    fromUser,
                    trailer );
         }
         break;

      case 2:                 /* Remote sender, local delivery        */
         if ( bflag[ F_KANJI ] )
                              /* Kanji from remote node?              */
            put_string = (int (*)(char *, FILE *)) fputs_shiftjis;
                              /* Yes --> Translate it                 */

         if ( ! bflag[ F_SUPPRESSFROM ] )
            fprintf(dataout, "From %s%s\n",
                    fromUser,
                    trailer );

         break;

      case 1:                 /* Local sender, remote delivery        */
         if ( bflag[F_KANJI]) /* Translation enabled?                 */
            put_string = (int (*)(char *, FILE *)) fputs_jis7bit;
                              /* Translate into 7 bit Kanji           */

         if ( ! bflag[ F_SUPPRESSFROM ] )
         {
            column = strlen(E_domain) - 5;
            if ((column > 0) && equali(&E_domain[column],".UUCP"))
                              /* UUCP domain?                         */
               fprintf(dataout, "From %s%s\n",
                                fromUser,
                                trailer );

                              /* Yes --> Use simple address           */
            else
               fprintf(dataout, "From %s!%s%s\n",
                       E_domain,
                       fromUser,
                       trailer );
                              /* No --> Use domain address            */
         }
         break;

      case 0:                 /* Local sender, local delivery         */
         if ( ! bflag[ F_SUPPRESSFROM ] )
            fprintf(dataout, "From %s%.25s\n", fromUser, trailer );
         break;

   } /* switch */

/*--------------------------------------------------------------------*/
/*                       Loop to copy the data                        */
/*--------------------------------------------------------------------*/

   while (imgets(buf, BUFSIZ, imf) != NULL)
   {

      if ((*put_string)(buf, dataout) == EOF)     /* I/O error? */
      {

         printerr("output");
         printmsg(0,"I/O error on \"%s\"", "output");
         fclose(dataout);
         return KWFalse;

      } /* if */

   } /* while */

/*--------------------------------------------------------------------*/
/*                      Close up shop and return                      */
/*--------------------------------------------------------------------*/

   if (imerror(imf))          /* Clean end of file on input?          */
   {
      printerr("imgets");
      panic();
   }

   fclose(dataout);
   return success;

} /* CopyData */

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
               const char *text,
               const char *data,
               const char *address ,
               const KWBoolean validate )
{
   FILE *newfile;
   char tname[FILENAME_MAX]; /* name of temporary file used */
   char buf[BUFSIZ];
   char sender[MAXADDR];

   KWBoolean bounce = bflag[F_BOUNCE];

   sprintf(sender, "%s%s%s",
               ruser,
               remoteMail ? "@" : "",
               remoteMail ? rnode : "" );

    printmsg(0,"Bounce: Mail from %s for %s failed, %s: %s",
               sender,
               address,
               text,
               (data == NULL) ? "(no data)" : data );

/*--------------------------------------------------------------------*/
/*           Never bounce mail to a select list of user ids           */
/*--------------------------------------------------------------------*/

   if ( equali( ruser, "postmaster") ||
        equali( ruser, "uucp") ||
        equali( ruser, "root") ||
        equali( ruser, "mmdf") ||
        equali( ruser, "mailer-daemon"))
      bounce = KWFalse;

   if ( ! bounce )
     return Deliver( imf, E_postmaster, validate );

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
                  ruser,
                  address, E_domain, E_nodename, text );

   if ( data != NULL )
      fprintf(newfile,
             "The problem address or file in question was:  %s\n",
             data );

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

   sprintf( buf, "-w -F %s -s \"Failed mail for %.20s\" -- %s -c postmaster",
            tname,
            address,
            sender );

    if ( execute( myProgramName, buf, NULL, NULL, KWTrue, KWFalse ))
         DeliverLocal( imf, E_postmaster, validate);

    return (1);

} /* Bounce */

/*--------------------------------------------------------------------*/
/*    s t a t s                                                       */
/*                                                                    */
/*    Report size of file in message, if desired                      */
/*--------------------------------------------------------------------*/

static char *stats( IMFILE *imf )
{
   if (bflag[ F_COLLECTSTATS ] )
   {
      static char buf[25];  /* "(nnnnnnn bytes) " */
                            /*  ....+....+....+.. */
      sprintf(buf,   "(%ld bytes) ", imlength( imf ));
      return buf;
   } /* if */
   else
      return "";              /* Pretend we were never here       */

} /* stats */

/*--------------------------------------------------------------------*/
/*       r e t r y S M T P d e l i v e r y                            */
/*                                                                    */
/*       Perform a pure SMTP delivery queue run to the routing        */
/*       host for the first specified addressee                       */
/*--------------------------------------------------------------------*/

KWBoolean
retrySMTPdelivery( IMFILE *imf, const char **address, int addressees )
{

   char path[MAXADDR];
   char dummy[MAXADDR];
   struct HostTable *hostp;
   int subscript;

   if ( ! tokenizeAddress(address[0], path, dummy, dummy) )
   {
      Bounce( imf,
              path,
              address[0],
              address[0],
              KWTrue );
      return KWTrue;
   }

   hostp = checkname( path );

   if ((hostp != BADHOST) && (hostp->status.hstatus == HS_SMTP))
   {
#ifdef TCPIP
      char fromAddr[MAXADDR];

      if ( equal( fromNode , E_nodename ))   /* Local address */
         sprintf( fromAddr, "%s@%s",
                  fromUser,
                  E_fdomain);
      else if ( strchr( fromNode, '.' ) == NULL )  /* remote UUCP */
         sprintf( fromAddr, "%s!%s@%s",
                  fromNode,
                  fromUser,
                  E_fdomain);
      else
         sprintf( fromAddr, "%s@%s",      /* Remote domain address */
                  fromUser,
                  fromNode);

      if ( ConnectSMTP( imf,
                        hostp->via,
                        fromAddr,
                        address,
                        addressees,
                        KWTrue ))
         return KWTrue;

#else
      printmsg(0, "retrySMTPdelivery: SMTP support not available, "
                  "leaving mail queued for %s",
                  hostp->via );
#endif
      return KWFalse;               /* Report we did not deliver     */

   } /* if ((hostp != BADHOST) && (hostp->status.hstatus == HS_SMTP)) */

/*--------------------------------------------------------------------*/
/*       The routing tables have been changed, and the first          */
/*       address is no longer routed via SMTP; process all the        */
/*       mail through the standard routing/delivery routines.         */
/*--------------------------------------------------------------------*/

   printmsg(0, "retrySMTPdelivery: Routing tables changed, rerouting mail");

   for ( subscript = 0; subscript < addressees; subscript++ )
      Deliver( imf, address[subscript], KWTrue );

   return KWTrue;

} /* retrySMTPdelivery */

/*--------------------------------------------------------------------*/
/*       f l u s h Q u e u e s                                        */
/*                                                                    */
/*       Commit mail for queued addresses batched together            */
/*--------------------------------------------------------------------*/

void
flushQueues( IMFILE *imf )
{
#ifdef TCPIP
   DeliverSMTP( imf, NULL, NULL );   /* Flush any lingering remote
                                        addresses                    */
#endif

   DeliverRemote( imf, NULL, NULL );   /* Flush any lingering remote
                                          addresses                  */

} /* flushQueues */
