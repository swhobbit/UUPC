#ifndef _SECURITY
#define _SECURITY

/*--------------------------------------------------------------------*/
/*       s e c u r i t y . h                                          */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1999 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: security.h 1.9 1998/03/01 01:27:36 ahd v1-13f ahd $
 *
 *    Revision history:
 *    $Log: security.h $
 *    Revision 1.9  1998/03/01 01:27:36  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.8  1997/03/31 07:06:38  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.7  1996/01/01 20:56:38  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.6  1995/01/07 16:16:30  ahd
 *    Change boolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.5  1994/12/22 00:13:44  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.4  1994/01/01 19:09:40  ahd
 *    Annual Copyright Update
 *
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
   KWBoolean grant;            /* KWTrue = excplitly Allow access,
                                 KWFalse = explicit deny access       */
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
      KWBoolean request;          /* KWTrue = Remote system can request
                                    files FROM local system            */
      KWBoolean sendfiles;        /* KWTrue = Send locally queued requests
                                    when other system has called us;
                                    default is KWFalse, only send files
                                    when we are the caller.  (Stupid
                                    default if you fill in other para-
                                    meters properly, but that's UUCP   */
      KWBoolean callback;         /* KWTrue = Do not process work for
                                    system when it calls in, but rather
                                    call it back    */
      KWBoolean local;            /* KWTrue = Local system, grant all
                                    requests                           */
      } ; /* HostSecurity */

/*--------------------------------------------------------------------*/
/*                             Prototypes                             */
/*--------------------------------------------------------------------*/

KWBoolean ValidateFile( const char *input,  /* Full path name         */
                      const REMOTE_ACCESS needed );

KWBoolean ValidateHost( const char *host );

KWBoolean ValidateCommand( const char *command);

KWBoolean LoadSecurity( void );

struct HostSecurity *GetSecurity( struct HostTable *hostp);

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

extern struct HostSecurity *securep;

#endif /* ifdef _SECURITY */
