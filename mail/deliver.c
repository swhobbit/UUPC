/*--------------------------------------------------------------------*/
/*    d e l i v e r  . c                                              */
/*                                                                    */
/*    UUPC/extended mail delivery subroutines                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989 by Andrew H. Derbyshire.             */
/*                                                                    */
/*    Changes Copyright (c) 1990-1992 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: DELIVER.C 1.3 1992/12/05 23:38:43 ahd Exp $
 *
 *    $Log: DELIVER.C $
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

/*--------------------------------------------------------------------*/
/*    Use a complex beep upon mail delivery if way to control the     */
/*    speaker is available; if using MS C 6.0 under DOS, we can't     */
/*    so don't try                                                    */
/*--------------------------------------------------------------------*/

#ifdef __TURBOC__
#define SMARTBEEP
#endif

#ifdef FAMILYAPI
#define SMARTBEEP
#endif

#define INCLUDE ":include:"

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <ctype.h>
#include <sys/types.h>
#include <string.h>
#include <process.h>
#include <limits.h>

#ifdef __TURBOC__
#include <dos.h>
#endif

#ifdef FAMILYAPI
#ifdef WIN32
#include <windows.h>
#else
#include <os2.h>
#endif
#endif

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "address.h"
#include "deliver.h"
#include "expath.h"
#include "getseq.h"
#include "kanjicnv.h"
#include "hlib.h"
#include "hostable.h"
#include "import.h"
#include "pushpop.h"
#include "security.h"
#include "stater.h"
#include "usertabl.h"
#include "sysalias.h"

#ifdef SMARTBEEP
#include "ssleep.h"
#endif

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
                        const boolean sysalias,  /* Already sys alias     */
                        const boolean validate );

static void trumpet( const char *tune);

static size_t DeliverRemote( const char *input, /* Input file name    */
                             const char *address,  /* Target address  */
                             const char *path);

static size_t DeliverGateway(   const char *input,
                                const char *user,
                                const char *node,
                                const struct HostTable *hostp);

static int CopyData(   const boolean remotedelivery,
                       const char *input,
                       FILE *mbox);

static char *stats( const char *fname );

/*--------------------------------------------------------------------*/
/*   Global (set by rmail.c) for number of hops this mail has seen    */
/*--------------------------------------------------------------------*/

 INTEGER hops = 0;

 boolean remoteMail = FALSE;

 char *ruser = NULL;
 char *rnode = NULL;
 char *uuser = NULL;

/*--------------------------------------------------------------------*/
/*    D e l i v e r                                                   */
/*                                                                    */
/*    Deliver mail to one user                                        */
/*--------------------------------------------------------------------*/

size_t Deliver(       const char *input,    /* Input file name       */
                            char *address,  /* Target address           */
                      const boolean sysalias,  /* Already sys alias     */
                          boolean validate)  /* Validate/forward
                                                local mail            */
{
   char node[MAXADDR];
   char path[MAXADDR];
   char user[MAXADDR];
   char *token;
   struct HostTable *hostp;

   if ( strlen( address ) >= MAXADDR )
   {
      printmsg( 0,
            "Delivering to postmaster: Excessive address length %d for %s",
               strlen(address) , address );
      return Deliver( input, E_postmaster, FALSE, TRUE);
   }

   user_at_node(address, path, node, user);

/*--------------------------------------------------------------------*/
/*                       Handle local delivery                        */
/*--------------------------------------------------------------------*/

   if (equal(path, E_nodename)) /* Local node?                       */
   {
      struct HostTable *hostx = checkname( node );
      if (hostx->hstatus == localhost)  /* Really the local node?    */
         return DeliverLocal( input, user, sysalias, validate );
                                 /* Yes!                             */
      else {
         printmsg(0,"Mail for \"%s\" via \"%s\" has no "
                    "delivery route, delivering to postmaster %s",
               address, path , E_postmaster);
         return Deliver( input, E_postmaster, FALSE, TRUE);
      } /* else */
   }  /* if */

/*--------------------------------------------------------------------*/
/*                    Do we need loop protection?                     */
/*--------------------------------------------------------------------*/

   if (hops > E_maxhops)
   {
         printmsg(0,
         "Mail for \"%s\" via \"%s\" has exceeded hop "
         "limit of %d, delivering to postmaster %s",
               address, path , E_maxhops, E_postmaster);
         return Deliver( input, E_postmaster, FALSE, TRUE);
   }

/*--------------------------------------------------------------------*/
/*                   Deliver to a gateway if needed                   */
/*--------------------------------------------------------------------*/

   hostp = checkname( path );
   if ( (hostp != BADHOST) && (hostp->hstatus == gatewayed))
      return DeliverGateway( input, user, node, hostp );

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

      userp = checkuser(user);   /* Locate user id in host table     */

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
                                      validate );

            if ( announce && ( userp != BADUSER ))
               trumpet( userp->beep);  /* Yes --> Inform the user    */
            return delivered;

         } /* if */
      } /* if */

/*--------------------------------------------------------------------*/
/*             No system alias, verify the user is valid              */
/*--------------------------------------------------------------------*/

      if ( userp == BADUSER )    /* Invalid user id?                 */
      {                          /* Yes --> Dump in trash bin        */
         printmsg(0,
               "\"%s\" is an invalid user, delivering to %s",
               user, E_postmaster);
         return DeliverLocal( input, E_postmaster, FALSE, validate);
      } /* if */

/*--------------------------------------------------------------------*/
/*               The user id validated; handle the mail               */
/*--------------------------------------------------------------------*/

      mkfilename(mboxname, userp->homedir, DOTFORWARD);

      if (access( mboxname, 0 )) /* The .forward file exists?        */
         announce = TRUE;        /* No --> Fall through              */
      else {
         delivered += DeliverFile( input,
                                   mboxname,
                                   0,
                                   LONG_MAX,
                                   &announce,
                                   userp,
                                   FALSE,
                                   validate);

         if (announce)        /* Did we deliver mail locally?        */
            trumpet( userp->beep);  /* Yes --> Inform the user       */
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

   if ( announce )
      trumpet( userp->beep);  /* Local delivery, inform the user     */

   mbox = FOPEN( mboxname , "a", TEXT );
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
                        const boolean sysalias,  /* Already sys alias     */
                        const boolean validate )
{
   char buf[BUFSIZ];
   FILE *fwrd = FOPEN(fwrdname, "r", TEXT);
   int delivered = 0;

   if ( fwrd == NULL )
   {
      printerr( fwrdname );
      printmsg(0,"Cannot open forward file %s, delivering to %s",
               fwrdname,
               E_postmaster );
      return DeliverLocal( input, E_postmaster, sysalias, validate );
   }

   if ( start != 0 )
      fseek( fwrd, start, SEEK_SET);

   while((ftell(fwrd) < end) && (fgets( buf , BUFSIZ , fwrd) != NULL ))
   {
      char command[BUFSIZ];
      char *s = buf;
      char c;
      char *nextfile = NULL;

      if ( buf[ strlen(buf) - 1 ]== '\n')
         buf[ strlen(buf) - 1 ] = '\0';

      while( *s && ! isgraph( *s ))    /* Trim leading white space      */
         s++;

      printmsg(8,"Forwarding to \"%s\"", s);
      if ( equalni( buf, INCLUDE, strlen(INCLUDE)))
      {
         nextfile = strtok( s + strlen(INCLUDE), WHITESPACE );
         if ( nextfile == NULL )
         {
            printmsg(0,"%s: Missing file name after %s, "
                       "delivering to %s",
                        fwrdname, INCLUDE, E_postmaster );
            s = E_postmaster;
            c = *s;
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
            sprintf(command , "%s < %s", &s[1], input);
            printmsg(1,"Executing \"%s\" in %s",
                  command, userp->homedir);
            PushDir( userp->homedir );
            system(command);                 /* FIX THIS */
            PopDir();
            delivered += 1;
            fwrd = FOPEN(fwrdname, "r", TEXT);
            fseek( fwrd, here, SEEK_SET);
            break;
         } /* case */

         case '\\':              /* Deliver without forwarding */
            delivered += Deliver( input, &s[1], TRUE, FALSE );
            *announce = TRUE;
            break;

         case ':':
            delivered += DeliverFile( input, nextfile, 0, LONG_MAX,
                                      announce, userp,
                                      FALSE, TRUE );
            break;

         case '/':               /* Save in absolute path name */
         case '~':
            if (expand_path(s, NULL, userp->homedir,
                            E_mailext) == NULL )
            {
               printmsg(0,
                     "Invalid path in filename, delivering to %s",
                      E_postmaster);
               return DeliverLocal( input, E_postmaster,
                                    sysalias, validate );
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
/*    t r u m p e t                                                   */
/*                                                                    */
/*    Trumpet the arrival of remote mail to a local user              */
/*--------------------------------------------------------------------*/

static void trumpet( const char *tune)
{
#ifdef SMARTBEEP
   char buf[BUFSIZ];
   char *token = buf;
   size_t tone, duration;
#endif

   if ((tune == NULL) || !remoteMail) /* Should we announce?  */
      return;                 /* No --> Return quietly (literally)   */

/*--------------------------------------------------------------------*/
/*             We are to announce the arrival of the mail             */
/*--------------------------------------------------------------------*/

#ifdef SMARTBEEP
   strcpy(buf,tune);          /* Save the data                       */

   while( (token = strtok( token, ",")) != NULL)
   {
      tone = (size_t) atoi(token);
      token = strtok( NULL, ",");
      duration = (token == NULL) ? 500 : (size_t) atoi(token);

#ifdef __TURBOC__
      if (tone == 0)
         nosound();
      else
         sound( tone );
      ddelay( duration );
#else
      if (tone == 0)
         ddelay(duration);
      else {
#ifdef WIN32
         Beep( tone, duration );
#else
         DosBeep( tone, duration );
#endif
      }
#endif /* SMARTBEEP */

      token = NULL;           /* Look at next part of string   */
   } /* while */

#ifdef __TURBOC__
   nosound();
#endif
#else

/*--------------------------------------------------------------------*/
/*      We cannot play the requested tune; just beep at the user      */
/*--------------------------------------------------------------------*/

   fputc('\a', stdout);
#endif /* SMARTBEEP */

} /* trumpet */

/*--------------------------------------------------------------------*/
/*    D e l i v e r G a t e w a y                                     */
/*                                                                    */
/*    Deliver mail via a gateway program                              */
/*--------------------------------------------------------------------*/

static size_t DeliverGateway(   const char *input,
                                const char *user,
                                const char *node,
                                const struct HostTable *hostp)
{
   char command[BUFSIZ];

/*--------------------------------------------------------------------*/
/*    Format the command and tell the user what we're going to do     */
/*--------------------------------------------------------------------*/

   sprintf(command , "%s %s %s %s < %s",
                     hostp->via,          /* Program to perform forward */
                     hostp->hostname,     /* Nominal host routing via   */
                     node ,               /* Final destination system   */
                     user,                /* user on "node" for delivery*/
                     input);              /* The data to forward        */

   printmsg(1,
      "Gatewaying mail %sfrom %s@%s to %s@%s via %s using \"%s\"",
       stats( input ),
       ruser, rnode, user, node, hostp->hostname, hostp->via);
   printmsg(3,"DeliverGateway: %s",command);

/*--------------------------------------------------------------------*/
/*  Run the command and return caller with count of mail delivered    */
/*--------------------------------------------------------------------*/

   system(command);
   return 1;

} /* DeliveryGateway */

/*--------------------------------------------------------------------*/
/*    D e l i v e r R e m o t e                                       */
/*                                                                    */
/*    Queue mail for delivery on another system via UUCP              */
/*--------------------------------------------------------------------*/

static size_t DeliverRemote( const char *input, /* Input file name    */
                    const char *address,  /* Target address           */
                    const char *path)
{
   static char *spool_fmt = SPOOLFMT;              /* spool file name */
   static char *dataf_fmt = DATAFFMT;
   static char *send_cmd  = "S %s %s %s - %s 0666\n";
   static long seqno = 0;
   static char *SavePath = NULL;
   FILE *stream;              /* For writing out data                */
   static char everyone[500]; /* 512, with room for "rmail "         */

   char msfile[FILENAME_MAX]; /* MS-DOS format name of files         */
   char msname[22];           /* MS-DOS format w/o path name         */

   char tmfile[15];           /* Call file, UNIX format name         */
   static char ixfile[15];    /* eXecute file for remote system,
                                UNIX format name for local system   */
   static char idfile[15];    /* Data file, UNIX format name         */
   static char rdfile[15];    /* Data file name on remote system,
                                 UNIX format                         */
   static char rxfile[15];    /* Remote system UNIX name of eXecute
                                 file                                */

   printmsg(1,"Spooling mail %sfrom %s%s%s to %s via %s",
               stats( input ),
               ruser,
               remoteMail ? "@" : "",
               remoteMail ? rnode : "",
               address ,
               path);

/*--------------------------------------------------------------------*/
/*          Create the UNIX format of the file names we need          */
/*--------------------------------------------------------------------*/

   if ((seqno == 0) ||
       (SavePath == NULL) ||
       !equal(SavePath, path) ||
       ((int) (strlen(everyone) + strlen(address) + 2) > (int) sizeof everyone))
   {
      char *seq;
      seqno = getseq();
      seq = JobNumber( seqno );

      if  (SavePath != NULL )
      {
         free(SavePath);
         SavePath = NULL;
      } /* if */

      sprintf(tmfile, spool_fmt, 'C', path,     'C' , seq);
      sprintf(idfile, dataf_fmt, 'D', E_nodename , seq, 'd');
      sprintf(rdfile, dataf_fmt, 'D', E_nodename , seq, 'r');
      sprintf(ixfile, dataf_fmt, 'D', E_nodename , seq, 'e');
      sprintf(rxfile, dataf_fmt, 'X', E_nodename , seq, 'r');
      strcpy(everyone,address);

   } /* if */
   else {
      strcat(everyone," ");
      strcat(everyone,address);
   } /* else */

/*--------------------------------------------------------------------*/
/*                     create remote X (xqt) file                     */
/*--------------------------------------------------------------------*/

   importpath( msname, ixfile, path);
   mkfilename( msfile, E_spooldir, msname);

   stream = FOPEN(msfile, "w", BINARY);
   if ( stream == NULL )
   {
      printerr(msfile);
      printmsg(0, "DeliverRemote: cannot open X file %s", msfile);
      return 0;
   } /* if */


   fprintf(stream, "R %s@%s\nU %s %s\nF %s\nI %s\nC rmail %s\n",
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

   stream = FOPEN(msfile, "w", BINARY);
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

   stream = FOPEN(msfile, "w", TEXT);
   if (stream == NULL)
   {
      printerr( msname );
      printmsg(0, "DeliverRemote: cannot open C file %s", msfile);
      return 0;
   }

   fprintf(stream, send_cmd, idfile, rdfile, uuser, idfile);
   fprintf(stream, send_cmd, ixfile, rxfile, uuser, ixfile);
   fclose(stream);

   if (bflag[F_MULTI])        /* Deliver to multiple users at once?  */
      SavePath = strdup(path);   /* Yes --> Save routing info        */

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
   FILE *datain = FOPEN(input, "r", TEXT);
   char buf[BUFSIZ];
   int column = 0;
   boolean success = TRUE;

   int (*put_string) (char *, FILE *) = (int (*)(char *, FILE *)) fputs;
                              /* Assume no Kanji translation needed  */

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
      case 3:                 /* Remote sender, remote delivery      */
         strcpy( buf, fromuser );
         strtok( buf, "!");   /* Get first host in list              */

         if ( equal(HostAlias( buf ), fromnode ))
                              /* Host already in list?               */
         {                    /* Yes --> Don't do it twice           */
            fprintf(dataout, "From %s %s remote from %s\n",
                     fromuser, now, E_nodename);
            break;
         }
         else {                /* No --> Insert it                    */
            fprintf(dataout, "From %s!%s %s remote from %s\n",
                     fromnode, fromuser, now, E_nodename);
            break;
         }

/*--------------------------------------------------------------------*/
/*    Note:  For the Kanji translation we re-check the                */
/*    remoteDelivery flag since we do the fall through from above.    */
/*--------------------------------------------------------------------*/

      case 2:                 /* Remote sender, local delivery       */
         if ( bflag[ F_KANJI ] )
                              /* Kanji from remote node?             */
            put_string = (int (*)(char *, FILE *)) fputs_shiftjis;
                              /* Yes --> Translate it                */

         fprintf(dataout, "From %s %s remote from %s\n",
                  fromuser, now, fromnode);
         break;

      case 1:                 /* Local sender, remote delivery       */
         if ( bflag[F_KANJI]) /* Translation enabled?                */
            put_string = (int (*)(char *, FILE *)) fputs_jis7bit;
                              /* Translate into 7 bit Kanji          */

         column = strlen(E_domain) - 5;
         if ((column > 0) && equali(&E_domain[column],".UUCP"))
                              /* UUCP domain?                        */
            fprintf(dataout, "From %s %s remote from %s\n",
                             fromuser, now, E_nodename);
                              /* Yes --> Use simple address          */
         else
            fprintf(dataout, "From %s!%s %s remote from %s\n",
                  E_domain, fromuser, now, E_nodename);
                              /* No --> Use domain address           */
         break;

      case 0:                 /* Local sender, local delivery        */
         fprintf(dataout, "From %s %s\n", fromuser, now);
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

   if (ferror(datain))        /* Clean end of file on input?         */
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
