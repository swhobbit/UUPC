/*--------------------------------------------------------------------*/
/*    h  o s t a b l e . h                                            */
/*                                                                    */
/*    Routines included in hostable.c                                 */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
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
  *   $Id: hostable.h 1.18 1998/03/01 01:26:40 ahd v1-13b $
  *
  *   $Log: hostable.h $
 *     Revision 1.18  1998/03/01  01:26:40  ahd
 *     Annual Copyright Update
 *
  *   Revision 1.17  1997/11/24 02:59:06  ahd
  *   Add macro to allow program to look up local host name without checkname
  *
  *   Revision 1.16  1997/03/31 06:59:43  ahd
  *   Annual Copyright Update
  *
  *   Revision 1.15  1996/11/19 00:25:20  ahd
  *   Normalize host status flags
  *
  *   Revision 1.14  1996/01/01 20:55:56  ahd
  *   Annual Copyright Update
  *
  *   Revision 1.13  1995/07/21 13:28:20  ahd
  *   Correct compiler warnings
  *
  *   Revision 1.12  1995/01/29 16:43:42  ahd
  *   IBM C/Set compiler warnings
  *
  *   Revision 1.11  1995/01/07 16:15:48  ahd
  *   Change KWBoolean to KWBoolean to avoid VC++ 2.0 conflict
  *
  *   Revision 1.10  1994/12/27 23:36:37  ahd
  *   Allow hostable.h, usertable.h to be included without others
  *
  *   Revision 1.9  1994/12/22 00:13:02  ahd
  *   Annual Copyright Update
  *
  *   Revision 1.8  1994/05/08 21:46:58  ahd
  *   Use name of caller of checkname(), checkreal() when panicing on
  *   bad parameters
  *
 *     Revision 1.7  1994/02/20  19:16:21  ahd
 *     IBM C/Set 2 Conversion, memory leak cleanup
 *
 *     Revision 1.6  1994/01/24  03:10:11  ahd
 *     Annual Copyright Update
 *
 *     Revision 1.5  1994/01/01  19:08:48  ahd
 *     Annual Copyright Update
 *
 *     Revision 1.4  1993/10/12  01:21:25  ahd
 *     Normalize comments to PL/I style
 *
 *     Revision 1.3  1993/07/06  10:57:42  ahd
 *     Under Windows/NT pack structure to insure proper alignment
 *
  *    Revision 1.2  1992/12/30  12:52:31  dmwatt
  *    Use shorts for table to insure compatiablity across 32 bit/16 bit OSes
  *
  *
  *   18Mar90     Create from router.c
  *   21Dec92     Make all "enum hostatus"'s into unsigned shorts for
  *               NT portability
  */

#ifndef __HOSTABLE
#define __HOSTABLE

#ifndef __LIB
#error Need "LIB.H"
#endif

#define ANONYMOUS_HOST "*anonymous" /* Anonymous systems               */

#define BADHOST NULL

#define HOSTLEN   8           /* max length of host name without '\0'  */

/*--------------------------------------------------------------------*/
/*    Note in the following table that "fake" hosts must precede      */
/*    "nocall" and "real" hosts must follow it.                       */
/*--------------------------------------------------------------------*/

typedef enum
{
   HS_PHANTOM = 0,                /* Not fully initialized           */
   HS_LOCALHOST,                  /* Local system                    */
   HS_GATEWAYED,                  /* Mail delivered via ext program  */
   HS_SMTP,                       /* SMTP relay host name            */
   HS_NOCALL,                     /* Real host, never (yet) called   */
   HS_AUTODIAL,                   /* Dialing the phone now           */
   HS_INVALID_DEVICE,             /* Invalid device in systems file  */
   HS_NODEVICE,                   /* Could not open device           */
   HS_STARTUP_FAILED,             /* Determined system, start failed */
   HS_INPROGRESS,                 /* Call now active                 */
   HS_CALLBACK_REQ,               /* System must call us back        */
   HS_DIAL_SCRIPT_FAILED,
                                  /* Modem initialize failed         */
   HS_DIAL_FAILED,                /* Hardcoded auto-dial failed      */
   HS_SCRIPT_FAILED,              /* script in L.SYS failed          */
   HS_MAX_RETRY,                  /* Have given up calling this sys  */
   HS_TOO_SOON,                   /* In retry mode, too soon to call */
   HS_SUCCEEDED,                  /* self-explanatory                */
   HS_WRONG_HOST,                 /* Call out failed, wrong system   */
   HS_UNKNOWN_HOST,               /* Call in failed, unknown system  */
   HS_CALL_FAILED,                /* Abort after conversation startup*/
   HS_WRONG_TIME,                 /* Unable to call because of time  */
   HS_CALLED,                     /* Call during this exec success   */
   HS_ZZ_LAST
} HOSTSTATUS;

/*--------------------------------------------------------------------*/
/*                          Status information                        */
/*--------------------------------------------------------------------*/

#if defined(WIN32) || defined(__OS2__)
#pragma pack(1)
#endif

struct HostStats {
      time_t ltime;              /* Last time this host was called     */
      time_t lconnect;           /* Last time we actually connected    */
      unsigned long  calls;      /* Total number of calls to host      */
      unsigned long  connect;    /* Total length of connections to host*/
      unsigned long  fsent;      /* Total files sent to this host      */
      unsigned long  freceived;  /* Total files received from this host*/
      unsigned long  bsent;      /* Total bytes sent to this host      */
      unsigned long  breceived;  /* Total bytes received from this host*/
      unsigned long  errors;     /* Total transmission errors noted    */
      unsigned long  packets;    /* Total packets exchanged            */
      unsigned short hstatus;    /* Status information                 */
   };

#if defined(WIN32) || defined(__OS2__)
#pragma pack()
#endif

struct HostSecurity;             /* Declare tag to avoid silly warnings */

/*--------------------------------------------------------------------*/
/*                          Master hostable                           */
/*--------------------------------------------------------------------*/

struct  HostTable {
      char  *hostname;           /* Name of the host in question       */
      char  *via;                /* Host hostname is routed via        */
      char  *realname;           /* Alias of this host name            */
      struct HostSecurity *hsecure; /* Security Information, real hosts
                                       only                            */
      KWBoolean anylogin;        /* KWTrue = Can login with any generic
                                    user id                            */
      KWBoolean  aliased;        /* KWTrue = alias has been optimized  */
      KWBoolean  routed;         /* KWTrue = route has been optimized  */
      struct HostStats status;   /* Status info for real hosts only    */
   };

struct HostTable *searchname(const char *name, const size_t namel);

#define checkname(host) checkName(host, __LINE__, cfnptr )

#define checkself(host) searchname( host, 255 )

struct HostTable *checkName(const char *name,
                            const size_t line,
                            const char *function);

#define checkreal(host) checkReal(host, __LINE__, cfnptr )

struct HostTable *checkReal(const char *name,
                            const size_t line,
                            const char *function);

struct HostTable *nexthost( const KWBoolean start );

#endif
