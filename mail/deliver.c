/*--------------------------------------------------------------------*/
/*    d e l i v e r  . c                                              */
/*                                                                    */
/*    UUPC/extended mail delivery subroutines                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1994 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: deliver.c 1.27 1994/02/19 04:19:16 ahd Exp $
 *
 *    $Log: deliver.c $
 * Revision 1.27  1994/02/19  04:19:16  ahd
 * Use standard first header
 *
 * Revision 1.27  1994/02/19  04:19:16  ahd
 * Use standard first header
 *
 * Revision 1.26  1994/02/14  01:03:56  ahd
 * Trim trailing spaces off forward file lines
 *
 * Revision 1.25  1994/01/01  19:12:17  ahd
 * Annual Copyright Update
 *
 * Revision 1.24  1993/12/29  02:46:47  ahd
 * Add Vmail queuing support
 *
 * Revision 1.23  1993/12/23  03:11:17  rommel
 * OS/2 32 bit support for additional compilers
 *
 * Revision 1.22  1993/12/09  13:24:25  ahd
 * Correct timestamp in UUCP From line
 *
 * Revision 1.21  1993/12/09  04:51:21  ahd
 * Delete bogus case from CopyData()
 *
 * Revision 1.20  1993/12/07  04:57:53  ahd
 * rename from fromUser and fromNode
 *
 * Revision 1.19  1993/12/02  02:25:12  ahd
 * Add max generated UUXQT command line length
 *
 * Revision 1.18  1993/11/13  17:43:26  ahd
 * Add call grading support
 * Add suppressfrom, shortfrom options
 *
 * Revision 1.17  1993/10/12  01:30:23  ahd
 * Normalize comments to PL/I style
 *
 * Revision 1.16  1993/09/23  03:26:51  ahd
 * Alter bounce message for "no path to host" error
 *
 * Revision 1.15  1993/09/20  04:41:54  ahd
 * OS/2 2.x support
 *
 * Revision 1.14  1993/08/02  03:24:59  ahd
 * Further changes in support of Robert Denny's Windows 3.x support
 *
 * Revision 1.13  1993/07/31  16:26:01  ahd
 * Changes in support of Robert Denny's Windows support
 *
 * Revision 1.12  1993/06/21  02:17:31  ahd
 * Correct errors in mail routing via HOSTPATH
 *
 * Revision 1.11  1993/06/13  14:06:00  ahd
 * Save invoked program name and use it for recursive calls
 * Loosen up bounced mail copy loop to avoid NT crashes
 *
 * Revision 1.10  1993/05/30  00:01:47  ahd
 * Expand path of system alias files to allow userid references
 *
 * Revision 1.9  1993/05/06  03:41:48  ahd
 * Don't rebounce mail to the postmaster
 * Change directories as needed to provide reasonable default drives
 * Do not use possibly invalid home directory to push directory on
 * system aliases
 *
 * Revision 1.8  1993/05/03  02:41:57  ahd
 * Make deliver not rebounce mail to the postmonstor
 *
 * Revision 1.7  1993/04/16  12:55:36  dmwatt
 * Windows/NT sound support
 *
 * Revision 1.6  1993/04/15  03:17:21  ahd
 * Basic bounce support
 *
 * Revision 1.5  1993/04/11  00:33:05  ahd
 * Global edits for year, TEXT, etc.
 *
 * Revision 1.4  1992/12/18  13:05:18  ahd
 * Use one token on request line for UUCP
 *
 * Revision 1.3  1992/12/05  23:38:43  ahd
 * Skip blanks as well as unprintable characters
 *
 * Revision 1.2  1992/12/04  01:00:27  ahd
 * Add system alias support
 *
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

/*--------------------------------------------------------------------*/
/*        Define current file name for panic() and printerr()         */
/*--------------------------------------------------------------------*/

currentfile();

/*--------------------------------------------------------------------*/
/*                        Internal prototypes                         */
/*--------------------------------------------------------------------*/

static size_t DeliverLocal( const char *input,  /* Input file name    */
                          char *user,     /* Target address           */
                          const boolean sysalias,
                                             /* Already sys alias     */
                          boolean validate); /* Validate/forward
                                                local mail            */

static int DeliverFile( const char *input,
                        const char *mboxname,
                        const long start,
                        const long end,
                        boolean *announce,
                        struct UserTable *userp,
                        const boolean sysalias,  /* Already sys alias */
                        const boolean validate,
                        const char *user );

static size_t DeliverRemote( const char *input, /* Input file name    */
                             const char *address,  /* Target address  */
                             const char *path);

static size_t DeliverVMS( const char *input,    /* Input file name    */
                          char *user,     /* Target address           */
                          boolean validate); /* Validate/forward
                                                local mail            */

static size_t DeliverGateway(   const char *input,
                                const char *user,
                                const char *node,
                                const struct HostTable *hostp,
                                const boolean validate );

static int CopyData(   const boolean remotedelivery,
                       const char *input,
                       FILE *mbox);

static char *stats( const char *fname );

size_t Bounce( const char *input,
               const char *text,
               const char *data,
               const char *address,
               const boolean validate );

/*--------------------------------------------------------------------*/
/*   Global (set by rmail.c) for number of hops this mail has seen    */
/*--------------------------------------------------------------------*/

 KEWSHORT hops = 0;

 boolean remoteMail = FALSE;

 char *ruser = NULL;
 char *rnode = NULL;
 char *uuser = NULL;

/*--------------------------------------------------------------------*/
/*    D e l i v e r                                                   */
/*                                                                    */
/*    Deliver mail to one user                                        */
/*--------------------------------------------------------------------*/

size_t Deliver(       const char *input,    /* Input file name        */
                            char *address,  /* Target address          */
                      const boolean sysalias,  /* Already sys alias    */
                          boolean validate)  /* Validate/forward
                                                local mail            */
{
   char node[MAXADDR];
   char path[MAXADDR];
   char user[MAXADDR];
   char *token;
   struct HostTable *hostp;

   if ( strlen( address ) >= MAXADDR )
      return Bounce( input,
                     "Excessive address length",
                     address,
                     address,
                     validate );

   user_at_node(address, path, node, user);

/*--------------------------------------------------------------------*/
/*                       Handle local delivery                        */
/*--------------------------------------------------------------------*/

   if (equal(path, E_nodename)) /* Local node?                        */
   {
      struct HostTable *hostx = checkname( node );
      if (hostx->status.hstatus == localhost)  /* Really the local node?     */
         return DeliverLocal( input, user, sysalias, validate );
                                 /* Yes!                              */
      else
         return Bounce( input,
                 "No known delivery path for host",
                  address,
                  address,
                  validate );
   }  /* if */

/*--------------------------------------------------------------------*/
/*                    Do we need loop protection?                     */
/*--------------------------------------------------------------------*/

   if (hops > E_maxhops)
      return Bounce(input,
             "Excessive number of hops",
             address,
             address,
             validate );

/*--------------------------------------------------------------------*/
/*                   Deliver to a gateway if needed                   */
/*--------------------------------------------------------------------*/

   hostp = checkname( path );
   if ( (hostp != BADHOST) && (hostp->status.hstatus == gatewayed))
      return DeliverGateway( input, user, node, hostp, validate );

/*--------------------------------------------------------------------*/
/*         Deliver mail to a system directory connected to us         */
/*--------------------------------------------------------------------*/

   if (equal(path,node))   /* Directly connected system?          */
      return DeliverRemote( input, user, path); /* Yes            */

/*--------------------------------------------------------------------*/
/*   Default delivery; strip any this node and the directly           */
/*   connected system from the address, then deliver to the next      */
/*   hop on the route                                                 */
/*--------------------------------------------------------------------*/

   strcpy(node,address);
   token = strtok(node,"!");  /* Get first host in path        */
   if (equal( HostAlias(token), E_nodename)) /* Local system?  */
   {
      token =  strtok(NULL,"");  /* Yes --> Get rest of addr   */
      strcpy(address, token);    /* Use it for address         */
      token = strtok(token,"!"); /* Get next host in path      */
   } /* if */

   if (equal( HostAlias(token), path ))  /* Next system?       */
   {
      token =  strtok(NULL,"");  /* Yes --> Get rest of addr   */
      strcpy(address, token);    /* Use it for address         */
   } /* if */

   if (!strpbrk(address,"!@"))   /* Any host delimiters?       */
   {                             /* No --> Check for % routing */
      token = strrchr(address,'%'); /* Get last percent sign   */
      if (token != NULL)
         *token = '@';           /* Make it an RFC-822 address */
      else
         printmsg(0,"Deliver: Cannot find node in \"%s\"",
               address);         /* That's odd, it should not  */
                                 /* be a local address!        */
   } /* if */

   return DeliverRemote( input, address, path );

} /* Deliver */

/*--------------------------------------------------------------------*/
/*    D e l i v e r L o c a l                                         */
/*                                                                    */
/*    Handle local delivery, including optional forwarding            */
/*--------------------------------------------------------------------*/

static size_t DeliverLocal( const char *input,
                                          /* Input file name          */
                          char *user,     /* Target address           */
                          const boolean sysalias,
                                          /* Already sys alias     */
                          boolean validate)  /* TRUE = validate,
                                                forward user's mail   */
{
   char mboxname[FILENAME_MAX];
   struct UserTable *userp = NULL;
   ALIASTABLE *aliasp = NULL;
   int delivered = 0;
   boolean announce = FALSE;
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
      validate = strcmp( E_postmaster , user);
                                 /* Don't loop delivering to postmast*/

      userp = checkuser(user);   /* Locate user id in host table      */

/*--------------------------------------------------------------------*/
/*                     Process any system aliases                     */
/*--------------------------------------------------------------------*/

      if ( ! sysalias )
      {
         aliasp = checkalias( user );  /* System alias?             */

         if ( aliasp != NULL )
         {
            delivered += DeliverFile( input,
                                      SysAliases,
                                      aliasp->start,
                                      aliasp->end,
                                      &announce ,
                                      userp,
                                      TRUE,
                                      validate,
                                      user );

            if ( announce && ( userp != BADUSER ) && remoteMail )
               trumpet( userp->beep);  /* Yes --> Inform the user     */
            return delivered;

         } /* if */
      } /* if */

/*--------------------------------------------------------------------*/
/*             No system alias, verify the user is valid              */
/*--------------------------------------------------------------------*/

      if ( userp == BADUSER )    /* Invalid user id?                  */
      {                          /* Yes --> Dump in trash bin         */
         return Bounce( input,
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
         announce = TRUE;        /* No --> Fall through               */
      else {
         delivered += DeliverFile( input,
                                   mboxname,
                                   0,
                                   LONG_MAX,
                                   &announce,
                                   userp,
                                   FALSE,
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
                        stats( input ),
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

   return CopyData( FALSE, input , mbox );

} /* DeliverLocal */

/*--------------------------------------------------------------------*/
/*       D e l i v e r F i l e                                        */
/*                                                                    */
/*       Process a local or system aliases file                       */
/*--------------------------------------------------------------------*/

static int DeliverFile( const char *input,
                        const char *fwrdname,
                        const long start,
                        const long end,
                        boolean *announce,
                        struct UserTable *userp,
                        const boolean sysalias,  /* Already sys alias */
                        const boolean validate,
                        const char *user )
{
   char buf[BUFSIZ];
   FILE *fwrd = FOPEN(fwrdname, "r",TEXT_MODE);
   char *cwd = sysalias ? E_tempdir : userp->homedir;
   int delivered = 0;

   if ( fwrd == NULL )
   {
      printerr( fwrdname );
      return Bounce( input,
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

      printmsg(8,"Forwarding to \"%s\"", s);

      if ( equalni( buf, INCLUDE, strlen(INCLUDE)))
      {
         nextfile = strtok( s + strlen(INCLUDE), WHITESPACE );
         if ( nextfile == NULL )
         {
            return Bounce(input,
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
            break;            /* Comment, ignore            */

         case '\0':
            break;            /* Empty line, ignore         */

         case '|':               /* Pipe mail into a command   */
         {
            long here = ftell(fwrd);

            fclose(fwrd);
            PushDir( cwd );
            printmsg(1,"Piping mail%s from %s@%s for %s into %s",
                        stats( input ),
                        ruser,
                        rnode,
                        user,
                        s + 1 );

            executeCommand( s + 1, input, NULL, TRUE, FALSE );
            PopDir();
            delivered += 1;
            fwrd = FOPEN(fwrdname, "r",TEXT_MODE);
            fseek( fwrd, here, SEEK_SET);
            break;
         } /* case */

         case '\\':              /* Deliver without forwarding */
            delivered += Deliver( input, &s[1], TRUE, FALSE );
            *announce = TRUE;
            break;

         case ':':
         {
            char fname[FILENAME_MAX];
            strcpy( fname, nextfile);
            expand_path(nextfile, NULL, cwd, E_mailext);
            delivered += DeliverFile( input, nextfile, 0, LONG_MAX,
                                      announce, userp,
                                      FALSE, TRUE, user );
            break;
         }

         case '>':               /* Deliver to V-mail address specified */
               delivered += DeliverVMS( input, s + 1, validate );
               break;

         case '/':               /* Save in absolute path name */
         case '~':
            if (expand_path(s, NULL, cwd, E_mailext) == NULL )
            {
               return Bounce(input,
                             "Invalid path in forwarding file name",
                             s,
                             user,
                             validate );

            }
            else
               delivered += DeliverLocal( input, s, sysalias, FALSE );
            *announce = TRUE;
            break;

         default:                /* Deliver normally           */
              delivered += Deliver( input, s, sysalias, validate );
      } /* switch */
   } /* while */

   fclose( fwrd );

   return delivered;

} /* DeliverFile */

/*--------------------------------------------------------------------*/
/*    D e l i v e r G a t e w a y                                     */
/*                                                                    */
/*    Deliver mail via a gateway program                              */
/*--------------------------------------------------------------------*/

static size_t DeliverGateway(   const char *input,
                                const char *user,
                                const char *node,
                                const struct HostTable *hostp,
                                const boolean validate )
{
   char command[BUFSIZ];
   int rc;

/*--------------------------------------------------------------------*/
/*    Format the command and tell the user what we're going to do     */
/*--------------------------------------------------------------------*/

   sprintf(command , "%s %s %s %s",
                     hostp->via,          /* Program to perform forward */
                     hostp->hostname,     /* Nominal host routing via  */
                     node ,               /* Final destination system  */
                     user );              /* user on "node" for delivery*/

   printmsg(3,"DeliverGateway: %s", command);

   printmsg(1,
      "Gatewaying mail %sfrom %s@%s to %s@%s via %s using \"%s\"",
       stats( input ),
       ruser, rnode, user, node, hostp->hostname, hostp->via);

/*--------------------------------------------------------------------*/
/*  Run the command and return caller with count of mail delivered    */
/*--------------------------------------------------------------------*/

   rc = executeCommand( command, input, NULL, TRUE, FALSE );

   if ( rc == 0 )
      return 1;
   else {
      char who[MAXADDR];

      sprintf( who, "%s@%s", user, node );
      return Bounce( input,
                     "Gateway command returned non-zero exit status",
                     command,
                     who,
                     validate );
   } /* else */

} /* DeliveryGateway */

/*--------------------------------------------------------------------*/
/*       D e l i v e r V M S                                          */
/*                                                                    */
/*       Deliver mail into the queue of Rick Vandenburg's             */
/*       V-Mail server.  This is a nasty hack to overcome DOS <-->    */
/*       OS/2 problems.                                               */
/*--------------------------------------------------------------------*/

static size_t DeliverVMS( const char *input,    /* Input file name    */
                          char *user,     /* Target address           */
                          boolean validate)  /* Validate/forward
                                                local mail            */
{
   char *saveTemp = E_tempdir;
   char dname[FILENAME_MAX];
   char xname[FILENAME_MAX];
   FILE *stream;

   if (( E_vmsQueueDir == NULL ) || (E_vmail == NULL ))
      return Bounce( input,
                     ( E_vmsQueueDir == NULL ) ?
                              "VMSQueueDir not defined" :
                              "VMail program name not defined",
                     NULL,
                     user,
                     validate );

   E_tempdir = E_vmsQueueDir;    /* Need to generate names for VMS   */

   mktempname( dname, "D");
   mktempname( xname, "X");

   E_tempdir = saveTemp;         /* Restore true directory name      */

/*--------------------------------------------------------------------*/
/*                  Now create the actual data file                   */
/*--------------------------------------------------------------------*/

   stream = FOPEN(dname, "w", TEXT_MODE );
   if (stream == NULL )
   {
      printerr(dname);
      return Bounce( input,
                     "Cannot open V-mail data file",
                     dname,
                     user,
                     validate );
   }

   if (!CopyData( FALSE, input , stream ))
   {
      remove( dname );
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
      return Bounce( input,
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
               stats( input ),
               ruser,
               remoteMail ? "@" : "",
               remoteMail ? rnode : "",
               user );

   return 1;

} /* DeliverVMS */

/*--------------------------------------------------------------------*/
/*    D e l i v e r R e m o t e                                       */
/*                                                                    */
/*    Queue mail for delivery on another system via UUCP              */
/*--------------------------------------------------------------------*/

static size_t DeliverRemote( const char *input, /* Input file name    */
                    const char *address,  /* Target address           */
                    const char *path)
{

#define EVERY_PAD 10

   static char *spool_fmt = SPOOLFMT;              /* spool file name */
   static char *dataf_fmt = DATAFFMT;
   static char *send_cmd  = "S %s %s %s - %s 0666\n";
   static long seqno = 0;
   static char *SavePath = NULL;
   FILE *stream;              /* For writing out data                 */
   static char everyone[BUFSIZ];

   char msfile[FILENAME_MAX]; /* MS-DOS format name of files          */
   char msname[22];           /* MS-DOS format w/o path name          */

   char tmfile[15];           /* Call file, UNIX format name          */
   static char ixfile[15];    /* eXecute file for remote system,
                                UNIX format name for local system   */
   static char idfile[15];    /* Data file, UNIX format name          */
   static char rdfile[15];    /* Data file name on remote system,
                                 UNIX format                          */
   static char rxfile[15];    /* Remote system UNIX name of eXecute
                                 file                                 */

   printmsg(1,"Spooling mail %sfrom %s%s%s to %s via %s",
               stats( input ),
               ruser,
               remoteMail ? "@" : "",
               remoteMail ? rnode : "",
               address ,
               path);

   if ( ! E_maxuuxqt )
      E_maxuuxqt = sizeof everyone;

/*--------------------------------------------------------------------*/
/*          Create the UNIX format of the file names we need          */
/*--------------------------------------------------------------------*/

   if ((seqno == 0) ||
       (SavePath == NULL) ||
       !equal(SavePath, path) ||
       ((strlen(everyone) + strlen(address) + 2) > E_maxuuxqt))
   {
      char *seq;
      seqno = getseq();
      seq = JobNumber( seqno );

      if  (SavePath != NULL )
         SavePath = NULL;

      sprintf(tmfile, spool_fmt, 'C', path,     grade , seq);
      sprintf(idfile, dataf_fmt, 'D', E_nodename , seq, 'd');
      sprintf(rdfile, dataf_fmt, 'D', E_nodename , seq, 'r');
      sprintf(ixfile, dataf_fmt, 'D', E_nodename , seq, 'e');
      sprintf(rxfile, dataf_fmt, 'X', E_nodename , seq, 'r');
      strcpy( everyone,"rmail");

   } /* if */

   strcat(everyone, " ");
   strcat(everyone, address);

/*--------------------------------------------------------------------*/
/*                     create remote X (xqt) file                     */
/*--------------------------------------------------------------------*/

   importpath( msname, ixfile, path);
   mkfilename( msfile, E_spooldir, msname);

   stream = FOPEN(msfile, "w", BINARY_MODE);
   if ( stream == NULL )
   {
      printerr(msfile);
      printmsg(0, "DeliverRemote: cannot open X file %s", msfile);
      return 0;
   } /* if */

   fprintf(stream, "R %s@%s\nU %s %s\nF %s\nI %s\nC %s\n",
               ruser, rnode, uuser , E_nodename,
               rdfile, rdfile, everyone);
   fclose(stream);

   if (SavePath != NULL)
      return 1;

/*--------------------------------------------------------------------*/
/*  Create the data file with the mail to send to the remote system   */
/*--------------------------------------------------------------------*/

   importpath(msname, idfile, path);
   mkfilename( msfile, E_spooldir, msname);

   stream = FOPEN(msfile, "w", BINARY_MODE);
   if (stream == NULL )
   {
      printerr(msfile);
      printmsg(0,
               "DeliverRemote: Cannot open spool file \"%s\" for output",
                msfile);
      return 0;
   }

   if (!CopyData( TRUE, input , stream ))
   {
      remove( msfile );
      return 0;
   }

/*--------------------------------------------------------------------*/
/*                     create local C (call) file                     */
/*--------------------------------------------------------------------*/

   importpath( msname, tmfile, path);
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

   if (bflag[F_MULTI])        /* Deliver to multiple users at once?   */
      SavePath = newstr(path);   /* Yes --> Save routing info         */

   return 1;

} /* DeliverRemote */

/*--------------------------------------------------------------------*/
/* C o p y D a t a                                                    */
/*                                                                    */
/* Copy data into its final resting spot                              */
/*--------------------------------------------------------------------*/

static int CopyData( const boolean remotedelivery,
                     const char *input,
                     FILE *dataout)
{
   FILE *datain = FOPEN(input, "r",TEXT_MODE);
   char buf[BUFSIZ];
   char trailer[BUFSIZ];
   int column = 0;
   boolean success = TRUE;

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

/*--------------------------------------------------------------------*/
/*                      Verify the input opened                       */
/*--------------------------------------------------------------------*/

   if (datain == NULL)
   {
      printerr(input);
      printmsg(0,"Unable to open input file \"%s\"", input);
      fclose(dataout);
      return 0;
   } /* datain */

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

   switch( (int) remoteMail * 2 + (int) remotedelivery )
   {
      case 3:                 /* Remote sender, remote delivery       */
         strcpy( buf, fromUser );
         strtok( buf, "!");   /* Get first host in list               */

         if ( bflag[ F_SUPPRESSFROM ] )
            ;                 /* No operation                        */
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

   while (fgets(buf, BUFSIZ, datain) != NULL)
   {
      if ((*put_string)(buf, dataout) == EOF)     /* I/O error? */
      {
         printerr("output");
         printmsg(0,"I/O error on \"%s\"", "output");
         fclose(dataout);
         return 0;
      } /* if */
   } /* while */

/*--------------------------------------------------------------------*/
/*                      Close up shop and return                      */
/*--------------------------------------------------------------------*/

   if (ferror(datain))        /* Clean end of file on input?          */
   {
      printerr(input);
      clearerr(datain);
      success = FALSE;
   }

   fclose(datain);
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

size_t Bounce( const char *input,
               const char *text,
               const char *data,
               const char *address ,
               const boolean validate )
{
   FILE *newfile, *otherfile;
   char tname[FILENAME_MAX]; /* name of temporary file used */
   char buf[BUFSIZ];
   char sender[MAXADDR];

   boolean bounce = bflag[F_BOUNCE];

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
      bounce = FALSE;

   if ( ! bounce )
     return Deliver( input, E_postmaster, FALSE, validate );

   mktempname( tname , "tmp");  /* Generate a temp file name           */

   if ((otherfile = FOPEN(input,"r", TEXT_MODE ))==NULL)
   {
       printerr( input );
       panic();
   };

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

    while ( fgets(buf, sizeof buf, otherfile) != NULL)
      fputs(buf, newfile);

    fclose(newfile);
    fclose(otherfile);

/*--------------------------------------------------------------------*/
/*          Recursively invoke RMAIL to deliver our message           */
/*--------------------------------------------------------------------*/

   putenv("LOGNAME=uucp");

   sprintf( buf, "-w -F %s -s \"Failed mail for %.20s\" %s -c postmaster",
            tname,
            address,
            sender );

    if ( execute( myProgramName, buf, NULL, NULL, TRUE, FALSE ))
         DeliverLocal( input, E_postmaster, FALSE, validate);

    return (1);

} /* Bounce */

/*--------------------------------------------------------------------*/
/*    s t a t s                                                       */
/*                                                                    */
/*    Report size of file in message, if desired                      */
/*--------------------------------------------------------------------*/

static char *stats( const char *fname )
{
   if (bflag[ F_COLLECTSTATS ] )
   {
      long size;
      time_t ltime = stater(fname, &size);

      if ( ltime == -1 )
      {
         printerr( fname );
         return "(unknown size)";
      }
      else {
         static char buf[25];  /* "(nnnnnnn bytes) " */
                               /*  ....+....+....+.. */
         sprintf(buf,   "(%ld bytes) ",size );
         return buf;
      } /* else */
   } /* if */
   else
      return "";              /* Pretend we were never here       */

} /* stats */
