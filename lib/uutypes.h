#ifndef UUPC_TYPES
#define UUPC_TYPES

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989 by Andrew H. Derbyshire.             */
/*                                                                    */
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
 *    $Id: uutypes.h 1.32 1995/03/11 02:06:02 ahd Exp $
 *
 *    Revision history:
 *    $Log: uutypes.h $
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
 *    Change KWBoolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.25  1995/01/07 15:49:37  ahd
 *    Add imfile flag fpr im-memory file support
 *
 *    Revision 1.24  1995/01/02 05:04:57  ahd
 *    Pass 2 of integrating SYS file support from Mike McLagan
 *
 *    Revision 1.23  1994/12/31 03:51:25  ahd
 *    First pass of integrating Mike McLagan's news SYS file suuport
 *
 *    Revision 1.22  1994/12/22 00:14:38  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.21  1994/11/27 17:09:27  ahd
 *    Add F_SUPPRESSBEEP, to trap OS/2 dos box crash (not UUPC's fault)
 *
 *     Revision 1.20  1994/05/07  21:47:55  ahd
 *     Support suppressing prompts for empty passwords
 *
 *     Revision 1.19  1994/02/28  04:10:56  ahd
 *     Correct spelling of longname flag
 *
 *     Revision 1.18  1994/02/13  22:32:16  ahd
 *     Add mixed memory mode MEMCHR macro
 *
 *     Revision 1.17  1994/01/24  03:10:52  ahd
 *     Annual Copyright Update
 *
 *     Revision 1.16  1994/01/01  19:10:20  ahd
 *     Annual Copyright Update
 *
 *     Revision 1.15  1993/12/24  05:22:49  ahd
 *     Allow for more generic use of UUFAR type modifier
 *
 *     Revision 1.14  1993/11/14  20:55:00  ahd
 *     Add showspool option
 *
 *     Revision 1.13  1993/11/13  17:38:09  ahd
 *     Add new options for windowing under OS/2, UUCP from options
 *
 *     Revision 1.12  1993/10/30  22:29:23  rommel
 *     inews support
 *
 *     Revision 1.11  1993/10/12  01:21:25  ahd
 *     Normalize comments to PL/I style
 *
 *     Revision 1.10  1993/10/02  19:09:59  ahd
 *     Treat UUFAR as true FAR under Windows 3.x
 *
 *     Revision 1.9  1993/09/20  04:51:31  ahd
 *     TCP Support from Dave Watt
 *     't' protocol support
 *     OS/2 2.x support (BC++ 1.0 for OS/2 support)
 *
 *     Revision 1.8  1993/07/31  16:28:59  ahd
 *     Changes in support of Robert Denny's Windows Support
 *
 *     Revision 1.7  1993/07/22  23:26:19  ahd
 *     First pass of changes for Robert Denny's Windows 3.1 support
 *
 *     Revision 1.6  1993/06/13  14:12:29  ahd
 *     Changes per Mike McLagan for outbound batched news support
 *
 *     Revision 1.5  1993/05/30  00:11:03  ahd
 *     Add UUFAR (far modifier)
 *
 *     Revision 1.4  1993/05/09  03:50:05  ahd
 *     Support longname, honordebug, senddebug options
 *
 * Revision 1.3  1993/04/15  03:21:30  ahd
 * Add bounce to KWBoolean option list
 *
 * Revision 1.2  1993/04/11  00:36:59  dmwatt
 * Global edits for year, TEXT, etc.
 *
 * Revision 1.1  1993/03/06  23:09:50  ahd
 * Initial revision
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
#else
#define MEMSET(p,c,l)  _fmemset(p,c,l)
#define MEMCPY(t,s,l)  _fmemcpy(t,s,l)
#define MEMCHR(t,s,l)  _fmemchr(t,s,l)
#define MEMMOVE(t,s,l) _fmemmove(t,s,l)
#define MALLOC(l)      _fmalloc(l)
#define REALLOC(p,l)   _frealloc(p,l)
#define FREE(p)        _ffree(p)
#endif

/*--------------------------------------------------------------------*/
/*    Enumerated list used by the options= configuration line         */
/*--------------------------------------------------------------------*/

typedef enum {

/*--------------------------------------------------------------------*/
/*                          Per user options                          */
/*--------------------------------------------------------------------*/

               F_ASKCC,       /* True = ask for Carbon copies         */
               F_DOSKEY,      /* True = Use DOSKEY under when available*/
               F_BACKUP,      /* True = Backup mailbox before rewriting */
               F_DOT,         /* True = period ends a message         */
               F_AUTOPRINT,   /* True = print next msg automatically  */
               F_AUTOEDIT,    /* True = no line prompt, always edit   */
               F_AUTOINCLUDE, /* True = Perform automatic INCLUDE cmd  */
               F_AUTOSIGN,    /* True = append the signature file     */
               F_EXPERT,      /* True = Let user shoot self in foot
                                        w/o a message.                 */
               F_FROMSEP,     /* True = Allow From to split messages  */
               F_IMFILE,      /* True = Use in memory files           */
               F_PAGER,       /* True = Invert meaning of P/p T/t cmds */
               F_PURGE,       /* True = Delete mailbox, if empty      */
               F_SAVE,        /* True = Save read messages in =mbox
                                        when reading new mail.         */
               F_SAVERESENT,  /* True = Save forwarded mail second time*/
               F_SUPPRESSCOPYRIGHT,
                              /* Skip copyright message                */
               F_SUPPRESSBEEP,/* Never beep at the user                */
               F_SPEEDOVERMEMORY,
                              /* True = Be lazy in strpool()          */
               F_VERBOSE,     /* True = Verbose RMAIL output          */
               F_WINDOWS,     /* True = Run RMAIL/RNEWS as Windows pgms*/

/*--------------------------------------------------------------------*/
/*                    Per system (GLOBAL) options                     */
/*--------------------------------------------------------------------*/

               F_BANG,        /* True = write addrs in bang (!) form */
               F_BOUNCE,      /* True = Bounce bad mail to sender    */
               F_COLLECTSTATS,/* True = Report additional
                                        information in various logs  */
               F_DIRECT,      /* True = Deliver to subdirectories,
                                        not files                    */
               F_HONORDEBUG,  /* True = Use -x flag from remote system */
               F_HONORCTRL,   /* True = Honor USENET control messahes  */
               F_ESCAPE,      /* True = ESCAPE acts as Ctrl-Break     */
               F_KANJI,       /* True = enable Kanji (Japanese) support*/
               F_LONGNAME,    /* True = Exploit OS/2 and NT long names */
               F_MULTI,       /* True = Deliver to multiple addresses
                                        on one remote host at once   */
               F_MULTITASK,   /* True = System is multitasking,
                                        watch for race conditions     */
               F_NEWSRUN,     /* True = Run NEWSRUN from RNEWS        */
               F_NNS,         /* True = Use NNS  news delivery        */
               F_NEWSPANIC,   /* True = Panic UUXQT if RNEWS fails    */
               F_ONECASE,     /* True = Remote host is case insensitive*/
               F_SENDDEBUG,   /* True = Send -x flag to remote systems */
               F_SHORTFROM,   /* True = Use short From remote header   */
               F_SHOWSPOOL,   /* True = Print files xferred from spool */
               F_SNEWS,       /* True = Use Simple news delivery       */
               F_SUPPRESSFROM,/* Suppress writing From lines in Mail   */
               F_SUPPRESSLOGININFO,
                              /* Don't report UUCP/extended version,
                                 domain name, etc. at login          */
               F_SYMMETRICGRADES,
                              /* True = Use send grade as receive grade*/
               F_SYSLOG,      /* True = Write syslog with name and
                                        time of each file transferred  */
               F_UNDELETE,    /* True = Do not override OS/2 undelete
                                 support                               */
               F_SUPPRESSEMPTYPASSWORD,
                              /* Don't prompt for null passwords       */
               F_LAST }       /* Dummy - Must be last - sets array size*/
               B_FLAG;        /* Boolean flag array - ahd              */

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
typedef unsigned short KEWSHORT; /* Integers in the config file       */
typedef unsigned long  KEWLONG;    /* Integers in the config file     */
typedef unsigned long  BPS;

#endif
