#ifndef UUPC_TYPES
#define UUPC_TYPES

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989 by Andrew H. Derbyshire.             */
/*                                                                    */
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
 *    $Id: uutypes.h 1.41 1998/04/08 11:36:31 ahd Exp $
 *
 *    Revision history:
 *    $Log: uutypes.h $
 *    Revision 1.41  1998/04/08 11:36:31  ahd
 *    Add option to control commenting out of UUCP From lines
 *    by POP3 server
 *
 *    Revision 1.40  1998/03/01 01:28:21  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.39  1997/12/13 18:11:33  ahd
 *    Add new delivery options
 *
 *    Revision 1.38  1997/05/11 18:17:27  ahd
 *    Add flag for fastsmtp delivery
 *    Move TCP/IP dependent delivery code from rmail to deliver
 *
 *    Revision 1.37  1997/03/31 07:07:52  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.36  1996/01/01 21:00:14  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.35  1995/11/30 12:51:07  ahd
 *    Add flag for automatic auto-receipt
 *
 *    Revision 1.34  1995/09/04 02:14:34  ahd
 *    Add strcat, strcmp, strcpy functions in FAR form
 *
 *    Revision 1.33  1995/04/02 00:04:32  ahd
 *    Add option to supppress display of selected information by UUCICO
 *    at remote login.
 *
 *    Revision 1.32  1995/03/11 02:06:02  ahd
 *    Delete obsolete F_COMPRESSBATCH
 *
 *    Revision 1.31  1995/02/26 02:53:34  ahd
 *    Add FREE() macro to allow single path 16 bit/32 bit code
 *
 *    Revision 1.30  1995/02/20 17:30:32  ahd
 *    16 bit compiler warning cleanup
 *
 *    Revision 1.29  1995/02/12 23:39:45  ahd
 *    compiler cleanup, NNS C/news support, optimize dir processing
 *
 *    Revision 1.28  1995/01/15 19:50:02  ahd
 *    Delete global fullbatch option
 *    Add "local", "batch" options to SYS structure
 *    Allow active file to be optional
 *
 *    Revision 1.27  1995/01/07 20:48:48  ahd
 *    Correct 16 bit compiler warnings
 *
 *    Revision 1.26  1995/01/07 16:10:01  ahd
 *    Change Boolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 */

/*--------------------------------------------------------------------*/
/*         Define unique FAR keyword for selected memory hogs         */
/*--------------------------------------------------------------------*/

#if defined(BIT32ENV)
#define UUFAR
#elif defined(__TURBOC__)
#define UUFAR far
#else
#define UUFAR _far
#endif

/*--------------------------------------------------------------------*/
/*                 Handle 16 bit vs. 32 bit compilers                 */
/*--------------------------------------------------------------------*/

#if defined(BIT32ENV)
#define MEMSET(p,c,l)  memset(p,c,l)
#define MEMCHR(p,c,l)  memchr(p,c,l)
#define MEMCPY(t,s,l)  memcpy(t,s,l)
#define MEMMOVE(t,s,l) memmove(t,s,l)
#define MALLOC(l)      malloc(l)
#define REALLOC(p,l)   realloc(p,l)
#define FREE(p)        free(p)
#define STRCPY(s1,s2)  strcpy(s1,s2)
#define STRCAT(s1,s2)  strcat(s1,s2)
#define STRCMP(s1,s2)  strcmp(s1,s2)
#else
#define MEMSET(p,c,l)  _fmemset(p,c,l)
#define MEMCPY(t,s,l)  _fmemcpy(t,s,l)
#define MEMCHR(t,s,l)  _fmemchr(t,s,l)
#define MEMMOVE(t,s,l) _fmemmove(t,s,l)
#define MALLOC(l)      _fmalloc(l)
#define REALLOC(p,l)   _frealloc(p,l)
#define FREE(p)        _ffree(p)
#define STRCPY(s1,s2)  _fstrcpy(s1,s2)
#define STRCAT(s1,s2)  _fstrcat(s1,s2)
#define STRCMP(s1,s2)  _fstrcmp(s1,s2)
#endif

/*--------------------------------------------------------------------*/
/*    Enumerated list used by the options= configuration line         */
/*--------------------------------------------------------------------*/

typedef enum
{

/*--------------------------------------------------------------------*/
/*                          Per user options                          */
/*--------------------------------------------------------------------*/

   F_ASKCC,                 /* True = ask for Carbon copies          */
   F_AUTOEDIT,              /* True = no line prompt, always edit    */
   F_AUTOINCLUDE,           /* True = Perform automatic INCLUDE cmd  */
   F_AUTOPRINT,             /* True = print next msg automatically   */
   F_AUTORECEIPT,           /* True = Request receipt                */
   F_AUTOSIGN,              /* True = append the signature file      */
   F_BACKUP,                /* True = Backup various files before
                                      rewriting                      */
   F_DISPLAYCOPYRIGHT,      /* True = Display copyright message      */
   F_DOSKEY,                /* True = Use DOSKEY under DOS when
                                      available                      */
   F_DOT,                   /* True = period ends a message          */
   F_EXPERT,                /* True = Let user shoot self in foot
                                      w/o a message.                 */
   F_FASTSMTP,              /* True = Deliver SMTP immediately       */
   F_FROMSEP,               /* True = Allow From to split messages   */
   F_IMFILE,                /* True = Use in memory files            */
   F_PAGER,                 /* True = Invert meaning of P/p T/t cmds */
   F_PURGE,                 /* True = Delete mailbox, if empty       */
   F_SAVE,                  /* True = Save read messages in ~/mbox
                                      when reading new mail.         */
   F_SAVERESENT,            /* True = Save forwarded mail 2nd time   */
   F_SUPPRESSBEEP,          /* True = Never beep at the user         */
   F_SPEEDOVERMEMORY,       /* True = Be lazy in strpool()           */
   F_VERBOSE,               /* True = Verbose RMAIL output           */
   F_WINDOWS,               /* True = Run RMAIL/RNEWS as Windows pgms*/

/*--------------------------------------------------------------------*/
/*               System options, only valid in UUPC.RC                */
/*--------------------------------------------------------------------*/

   F_BANG,                  /* True = write addrs in bang (!) form   */
   F_BOUNCE,                /* True = Bounce bad mail to sender      */

   F_COLLECTSTATS,          /* True = Report additional information
                                      in various logs                */
   F_COMMENTFROM,           /* True = Insert Comment: before UUCP
                                      "From lines" in POP3 server    */
   F_DIRECT,                /* True = Deliver to subdirectories,
                                      not files                      */
   F_DOMAINFROM,            /* True = Use FQDN From remote header    */
   F_HONORDEBUG,            /* True = Use -x flag from remote system */
   F_HONORCTRL,             /* True = Honor USENET control messahes  */
   F_ESCAPE,                /* True = ESCAPE acts as Ctrl-Break      */
   F_KANJI,                 /* True = enable Kanji (Japanese) support*/
   F_LONGNAME,              /* True = Exploit OS/2 and NT long names */
   F_MULTI,                 /* True = Deliver to multiple addresses
                                      on one remote host at once     */
   F_MULTITASK,             /* True = System is multitasking,
                                      watch for race conditions      */
   F_NEWSRUN,               /* True = Run NEWSRUN from RNEWS         */
   F_NNS,                   /* True = Use NNS news delivery          */
   F_NEWSPANIC,             /* True = Panic UUXQT if RNEWS fails     */
   F_ONECASE,               /* True = Remote host is case insensitive*/
   F_SENDDEBUG,             /* True = Send -x flag to remote systems */
   F_SHORTFROM,             /* True = Use short From remote header   */
   F_SHOWSPOOL,             /* True = Print files xferred from spool */
   F_SMTPRELAY,             /* True = Allow third-party SMTP relays  */
   F_SNEWS,                 /* True = Use Simple news delivery       */
   F_SUPPRESSEMPTYPASSWORD, /* True = No prompt for null passwords   */
   F_SUPPRESSFROM,          /* True = Suppress writing UUCP From
                                      lines in Mail                  */
   F_SUPPRESSLOGININFO,     /* True = Don't report UUCP/extended
                                      version, domain, etc. at login */
   F_SYMMETRICGRADES,       /* True = Use send grade as receive grade*/
   F_SYSLOG,                /* True = Write syslog with name and
                                      time of each file transferred  */
   F_UNDELETE,              /* True = Do not override OS/2 undelete
                                      support                        */
   F_UNIQUEMBOX,            /* True = Write single message per file  */

   F_LAST                   /* Dummy - Must be last, sets array size */
} B_FLAG;

/*--------------------------------------------------------------------*/
/*                  Your basic Boolean logic values                   */
/*--------------------------------------------------------------------*/

typedef enum
      {
         KWFalse = 0,
         KWTrue = 1
      }  KWBoolean;

/*--------------------------------------------------------------------*/
/*                      Configuration mode flags                      */
/*--------------------------------------------------------------------*/

typedef enum {
      USER_CONFIG,
      SYSTEM_CONFIG,
      MODEM_CONFIG
      } SYSMODE;

/*--------------------------------------------------------------------*/
/*               enumerated type for configuration bits               */
/*--------------------------------------------------------------------*/

typedef unsigned long CONFIGBITS;
typedef unsigned short KEWSHORT;    /* Integers in the config file   */
typedef unsigned long  KEWLONG;     /* Integers in the config file   */
typedef unsigned long  BPS;

#endif
