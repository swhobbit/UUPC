#ifndef UUPC_TYPES
#define UUPC_TYPES

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989 by Andrew H. Derbyshire.             */
/*                                                                    */
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
 *    $Id: uutypes.h 1.18 1994/02/13 22:32:16 ahd Exp $
 *
 *    Revision history:
 *    $Log: uutypes.h $
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
 * Add bounce to boolean option list
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
#else
#define MEMSET(p,c,l)  _fmemset(p,c,l)
#define MEMCPY(t,s,l)  _fmemcpy(t,s,l)
#define MEMCHR(t,s,l)  _fmemchr(t,s,l)
#define MEMMOVE(t,s,l) _fmemmove(t,s,l)
#endif

/*--------------------------------------------------------------------*/
/*    Enumerated list used by the options= configuration line         */
/*--------------------------------------------------------------------*/

typedef enum {

/*--------------------------------------------------------------------*/
/*                          Per user options                          */
/*--------------------------------------------------------------------*/

               F_ASKCC,       /* TRUE = ask for Carbon copies          */
               F_DOSKEY,      /* TRUE = Use DOSKEY under DOS 5 if available*/
               F_BACKUP,      /* TRUE = Backup mailbox before rewriting */
               F_DOT,         /* TRUE = period ends a message          */
               F_AUTOPRINT,   /* TRUE = print next message automatically  */
               F_AUTOEDIT,    /* TRUE = no line prompt, always edit    */
               F_AUTOINCLUDE, /* TRUE = Perform automatic INCLUDE command */
               F_AUTOSIGN,    /* TRUE = append the signature file      */
               F_EXPERT,      /* TRUE = Let user shoot self in foot w/o
                                        a message.                     */
               F_FROMSEP,     /* TRUE = Allow From to split messages   */
               F_NEWEDITORSESSION,
                              /* TRUE = Start new session for editor   */
               F_NEWPAGERSESSION,
                              /* TRUE = Start new session for pager    */
               F_PAGER,       /* TRUE = Invert meaning of P/p T/t commands */
               F_PURGE,       /* TRUE = Delete mailbox, if empty       */
               F_SAVE,        /* TRUE = Save read messages in =mbox when
                                        reading new mail.              */
               F_SAVERESENT,  /* TRUE = Save forwarded mail a second time */
               F_SUPPRESSCOPYRIGHT,
                              /* Skip copyright message                */
               F_SPEEDOVERMEMORY,
                              /* TRUE = Be lazy in strpool()           */
               F_VERBOSE,     /* TRUE = Verbose RMAIL output           */
               F_WINDOWS,     /* TRUE = Run RMAIL/RNEWS as Windows programs*/

/*--------------------------------------------------------------------*/
/*                    Per system (GLOBAL) options                     */
/*--------------------------------------------------------------------*/

               F_BANG,        /* TRUE = re-write addresses in bang (!) form*/
               F_BOUNCE,      /* TRUE = Bounce bad mail to sender      */
               F_COLLECTSTATS,/* TRUE = Report additional information in
                                        various logs                   */
               F_DIRECT,      /* TRUE = Deliver to subdirectories, not
                                        files                          */
               F_HONORDEBUG,  /* True = Use -x flag from remote system */
               F_HONORCTRL,   /* True = Honor USENET control messahes  */
               F_ESCAPE,      /* TRUE = ESCAPE acts as Ctrl-Break      */
               F_HISTORY,     /* TRUE = Maintain history of of articles
                                        read and posted                 */
               F_KANJI,       /* TRUE = enable Kanji (Japanese) support */
               F_LONGNAME,    /* TRUE = Exploit OS/2 and NT long names  */
               F_MULTI,       /* TRUE = Deliver to multiple addresses on
                                        remote host at once             */
               F_MULTITASK,   /* TRUE = System is multitasking, watch
                                        for race conditions             */
               F_ONECASE,     /* TRUE = Remote host is case insensitive */
               F_SENDDEBUG,   /* True = Send -x flag to remote systems  */
               F_SHORTFROM,   /* TRUE = Use short From remote header    */
               F_SHOWSPOOL,   /* TRUE = Print files xferred from spool  */
               F_SNEWS,       /* TRUE = Use Simple news delivery        */
               F_SUPPRESSFROM,
                              /* Suppress writing From lines in Mail    */
               F_SYMMETRICGRADES,
                              /* TRUE = Use send grade as receive grade */
               F_SYSLOG,      /* TRUE = Write syslog with name and time
                                        of each file transferred        */
               F_UNDELETE,    /* TRUE = Do not override OS/2 undelete
                                 support                               */
               F_USESYSFILE,  /* TRUE = RNEWS forwards news per SYS file  */
                              /* GMM - Batched ouytbound news support  */
               F_UUPCNEWSSERV,
                              /* TRUE = Our news server runs UUPC too    */
               F_LAST }       /* Dummy - Must be last - defines array size */
               B_FLAG;        /* Boolean flag array - ahd              */

/*--------------------------------------------------------------------*/
/*                  Your basic Boolean logic values                   */
/*--------------------------------------------------------------------*/

#undef FALSE
#undef TRUE
typedef enum
      { FALSE = 0,
        TRUE = 1 }
        boolean;

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
