/*
   hostable.h

   routines included in hostable.c

   Copyright (c) 1989, 1990  Andrew H. Derbyshire

   Update History:

   18Mar90     Create from router.c                                  ahd
 */

#ifndef __HOSTABLE
#define __HOSTABLE

#ifndef __LIB
#error Need "LIB.H"
#endif

#define UUCPSHELL "uucp"         /* Shell for UUCP users                */
#define ANONYMOUS_HOST "*anonymous" /* Anonymous systems                */

#define BADHOST NULL

#define HOSTLEN   8           /* max length of host name without '\0'   */

/*--------------------------------------------------------------------*/
/*    Note in the following table that "fake" hosts must precede      */
/*    "nocall" and "real" hosts must follow it.                       */
/*--------------------------------------------------------------------*/

typedef enum
                { phantom = 0,      /* Entry not fully initialized      */
                  localhost,        /* This entry is for ourselves      */
                  gatewayed,        /* This entry is delivered to via   */
                                    /* an external program on local sys */
                  nocall,           /* real host, never called          */
                  autodial,         /* Dialing the phone now            */
                  invalid_device,   /* Invalid systems file             */
                  nodevice,         /* Could not open device            */
                  startup_failed,   /* Determined system, start failed  */
                  inprogress,       /* Call now active                  */
                  callback_req,     /* System must call us back         */
                  dial_script_failed,
                                    /* Modem initialize failed          */
                  dial_failed,      /* Hardcoded auto-dial failed       */
                  script_failed,    /* script in L.SYS failed           */
                  max_retry,        /* Have given up calling this sys   */
                  too_soon,         /* In retry mode, too soon to call  */
                  succeeded,        /* self-explanatory                 */
                  wrong_host,       /* Call out failed, wrong system    */
                  unknown_host,     /* Call in failed, unknown system   */
                  call_failed,      /* Connection aborted for various
                                       reasons                          */
                  wrong_time,       /* Unable to call because of time   */
                  called,           /* success this run of UUCICO       */
                  last_status }
                        hostatus;

/*--------------------------------------------------------------------*/
/*                          Status information                        */
/*--------------------------------------------------------------------*/

struct HostStats {
      time_t ltime;              /* Last time this host was called      */
      time_t lconnect;           /* Last time we actually connected     */
      unsigned long calls;       /* Total number of calls to host       */
      unsigned long connect;     /* Total length of connections to host */
      unsigned long fsent;       /* Total files sent to this host       */
      unsigned long freceived;   /* Total files received from this host */
      unsigned long bsent;       /* Total bytes sent to this host       */
      unsigned long breceived;   /* Total bytes received from this host */
      unsigned long errors;      /* Total transmission errors noted     */
      unsigned long packets;     /* Total packets exchanged             */
      hostatus save_hstatus;     /* host status, as defined by hostatus */
   };

/*--------------------------------------------------------------------*/
/*                          Master hostable                           */
/*--------------------------------------------------------------------*/

struct  HostTable {
      char  *hostname;           /* Name of the host in question        */
      char  *via;                /* Host hostname is routed via         */
      char  *realname;           /* Alias of this host name             */
      struct HostStats *hstats;  /* Point to stats for real hosts only  */
      struct HostSecurity *hsecure; /* Security Information, real hosts
                                       only                             */
      boolean anylogin;          /* TRUE = Can login with any generic
                                    user id                             */
      boolean  aliased;          /* TRUE = alias has been optimized     */
      boolean  routed;           /* TRUE = route has been optimized     */
      hostatus hstatus;          /* host status, as defined by hostatus */
   };

struct HostTable *searchname(const char *name, const size_t namel);

struct HostTable *checkname(const char *name);

struct HostTable *checkreal(const char *name);

struct HostTable *nexthost( const boolean start );

#endif
