#ifndef UUPC_TYPES
#define UUPC_TYPES

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989 by Andrew H. Derbyshire.             */
/*                                                                    */
/*    Changes Copyright (c) 1990-1993 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: uutypes.h 1.10 1993/10/02 19:09:59 ahd Exp $
 *
 *    Revision history:
 *    $Log: uutypes.h $
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
               F_HONORDEBUG,  /* True = Accept -x flag from remote systems */
               F_ESCAPE,      /* TRUE = Allow ESCAPE to act as Ctrl-Break */
               F_HISTORY,     /* TRUE = Maintain history of of articles
                                        read and posted                */
               F_KANJI,       /* TRUE = enable Kanji (Japanese) support */
               B_LONGNAME,      /* TRUE = Exploit long names under OS/2, NT */
               F_MULTI,       /* TRUE = Deliver to multiple addresses on
                                        remote host at once            */
               F_MULTITASK,   /* TRUE = System is multitasking, watch
                                        for race conditions            */
               F_ONECASE,     /* TRUE = Remote host is case insensitive */
               F_SENDDEBUG,   /* True = Send -x flag to remote systems */
               F_SNEWS,       /* TRUE = Use Simple news delivery       */
               F_SYMMETRICGRADES,
                              /* TRUE = Use send grade as receive grade */
               F_SYSLOG,      /* TRUE = Write syslog with name and time of
                                        each file transferred          */
               F_UNDELETE,    /* TRUE = Do not override OS/2 undelete
                                 support                               */
               F_USESYSFILE,  /* TRUE = RNEWS forwards news per SYS file  */
                              /* GMM - Batched ouytbound news support  */
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
