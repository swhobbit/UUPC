/*--------------------------------------------------------------------*/
/*    h  o s t a b l e . h                                            */
/*                                                                    */
/*    Routines included in hostable.c                                 */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
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
  *   $Id: hostable.h 1.10 1994/12/27 23:36:37 ahd Exp $
  *
  *   $Log: hostable.h $
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
                { phantom = 0,      /* Entry not fully initialized     */
                  localhost,        /* This entry is for ourselves     */
                  gatewayed,        /* This entry is delivered to via  */
                                    /* an external program on local sys */
                  nocall,           /* real host, never called         */
                  autodial,         /* Dialing the phone now           */
                  invalid_device,   /* Invalid systems file            */
                  nodevice,         /* Could not open device           */
                  startup_failed,   /* Determined system, start failed */
                  inprogress,       /* Call now active                 */
                  callback_req,     /* System must call us back        */
                  dial_script_failed,
                                    /* Modem initialize failed         */
                  dial_failed,      /* Hardcoded auto-dial failed      */
                  script_failed,    /* script in L.SYS failed          */
                  max_retry,        /* Have given up calling this sys  */
                  too_soon,         /* In retry mode, too soon to call */
                  succeeded,        /* self-explanatory                */
                  wrong_host,       /* Call out failed, wrong system   */
                  unknown_host,     /* Call in failed, unknown system  */
                  call_failed,      /* Connection aborted for various
                                       reasons                         */
                  wrong_time,       /* Unable to call because of time  */
                  called,           /* success this run of UUCICO      */
                  last_status }
                        hostatus;

/*--------------------------------------------------------------------*/
/*                          Status information                        */
/*--------------------------------------------------------------------*/

#if defined(WIN32) || defined(__OS2__)
#pragma pack(1)
#endif

struct HostStats {
      time_t ltime;              /* Last time this host was called     */
      time_t lconnect;           /* Last time we actually connected    */
      unsigned long calls;       /* Total number of calls to host      */
      unsigned long connect;     /* Total length of connections to host */
      unsigned long fsent;       /* Total files sent to this host      */
      unsigned long freceived;   /* Total files received from this host */
      unsigned long bsent;       /* Total bytes sent to this host      */
      unsigned long breceived;   /* Total bytes received from this host */
      unsigned long errors;      /* Total transmission errors noted    */
      unsigned long packets;     /* Total packets exchanged            */
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
      KWBoolean anylogin;         /* KWTrue = Can login with any generic
                                    user id                            */
      KWBoolean  aliased;         /* KWTrue = alias has been optimized   */
      KWBoolean  routed;          /* KWTrue = route has been optimized   */
      struct HostStats status;   /* Status info for real hosts only    */
   };

struct HostTable *searchname(const char *name, const size_t namel);

#define checkname(host) checkName(host, __LINE__, cfnptr )

struct HostTable *checkName(const char *name,
                            const int line,
                            const char *function);

#define checkreal(host) checkReal(host, __LINE__, cfnptr )

struct HostTable *checkReal(const char *name,
                            const int line,
                            const char *function);

struct HostTable *nexthost( const KWBoolean start );

#endif
