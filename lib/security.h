#ifndef _SECURITY
#define _SECURITY

/*--------------------------------------------------------------------*/
/*       s e c u r i t y . h                                          */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1994 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: security.h 1.3 1993/10/12 01:22:27 ahd Exp $
 *
 *    Revision history:
 *    $Log: security.h $
 *     Revision 1.3  1993/10/12  01:22:27  ahd
 *     Normalize comments to PL/I style
 *
 *     Revision 1.2  1993/09/20  04:51:31  ahd
 *     TCP Support from Dave Watt
 *     't' protocol support
 *     OS/2 2.x support (BC++ 1.0 for OS/2 support)
 *
 */

#define ANY_HOST     "OTHER"
#define ANY_COMMAND  "ALL"

/*--------------------------------------------------------------------*/
/*             Directory list structed used by host table             */
/*--------------------------------------------------------------------*/

typedef enum {
   ALLOW_READ,
   ALLOW_WRITE
   } REMOTE_ACCESS;

struct DIRLIST {
   char *path;                /* Directory we are authorizing        */
   REMOTE_ACCESS priv;        /* ALLOW_READ/ALLOW_WRITE              */
   boolean grant;             /* TRUE = excplitly Allow access,
                                 FALSE = explicit deny access        */
   } ;

/*--------------------------------------------------------------------*/
/*                        Security information                        */
/*--------------------------------------------------------------------*/

struct HostSecurity {
      char **validate;           /* List of machines allowed to use
                                    logname when calling in            */
      char *myname;              /* Name to use when communicating with
                                    this remote system                 */
      char *pubdir;              /* Public directory to use for this
                                    system                             */
      char **commands;           /* Commands allowed for this system   */
      struct DIRLIST *dirlist;   /* List of directories this system
                                    can access                         */
      size_t dirsize;            /* Size of the directory list         */
      boolean request;           /* TRUE = Remote system can request
                                    files FROM local system            */
      boolean sendfiles;         /* TRUE = Send locally queued requests
                                    when other system has called us;
                                    default is FALSE, only send files
                                    when we are the caller.  (Stupid
                                    default if you fill in other para-
                                    meters properly, but that's UUCP   */
      boolean callback;          /* TRUE = Do not process work for
                                    system when it calls in, but rather
                                    call it back    */
      boolean local;             /* TRUE = Local system, grant all
                                    requests                           */
      } ; /* HostSecurity */

/*--------------------------------------------------------------------*/
/*                             Prototypes                             */
/*--------------------------------------------------------------------*/

boolean ValidateFile( const char *input,  /* Full path name          */
                      const REMOTE_ACCESS needed );

boolean ValidateHost( const char *host );

boolean ValidateCommand( const char *command);

boolean LoadSecurity( void );

struct HostSecurity *GetSecurity( struct HostTable *hostp);

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

extern struct HostSecurity *securep;

#endif /* ifdef _SECURITY */
