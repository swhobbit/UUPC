/*--------------------------------------------------------------------*/
/*       p s o s 2 e r r . c                                          */
/*                                                                    */
/*       Report error message from OS/2 sockets error list            */
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
/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"

#include <nerrno.h>
#include <io.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                     Local function prototypes                      */
/*--------------------------------------------------------------------*/

static const char *LookupOS2SocketError(int err);

/*--------------------------------------------------------------------*/
/*                          Local variables                           */
/*--------------------------------------------------------------------*/

typedef struct wserr {
        int errNo;
        const char *str;
} WSERR;

static WSERR wsErrors[] = {
 { SOCEPERM,               "[10001] Not owner" },
 { SOCESRCH,               "[10003] No such process" },
 { SOCEINTR,               "[10004] Interrupted system call" },
 { SOCENXIO,               "[10006] No such device or address" },
 { SOCEBADF,               "[10009] Bad file number" },
 { SOCEACCES,              "[10013] Permission denied" },
 { SOCEFAULT,              "[10014] Bad address" },
 { SOCEINVAL,              "[10022] Invalid argument" },
 { SOCEMFILE,              "[10024] Too many open files" },
 { SOCEPIPE,               "[10032] Broken pipe" },
 { SOCEOS2ERR,             "[10100] OS/2 Error" },
 { SOCEWOULDBLOCK,         "[10035] Operation would block" },
 { SOCEINPROGRESS,         "[10036] Operation now in progress" },
 { SOCEALREADY,            "[10037] Operation already in progress" },
 { SOCENOTSOCK,            "[10038] Socket operation on non-socket" },
 { SOCEDESTADDRREQ,        "[10039] Destination address required" },
 { SOCEMSGSIZE,            "[10040] Message too long" },
 { SOCEPROTOTYPE,          "[10041] Protocol wrong type for socket" },
 { SOCENOPROTOOPT,         "[10042] Protocol not available" },
 { SOCEPROTONOSUPPORT,     "[10043] Protocol not supported" },
 { SOCESOCKTNOSUPPORT,     "[10044] Socket type not supported" },
 { SOCEOPNOTSUPP,          "[10045] Operation not supported on socket" },
 { SOCEPFNOSUPPORT,        "[10046] Protocol family not supported" },
 { SOCEAFNOSUPPORT,        "[10047] Address family not supported by protocol family" },
 { SOCEADDRINUSE,          "[10048] Address already in use" },
 { SOCEADDRNOTAVAIL,       "[10049] Can't assign requested address" },
 { SOCENETDOWN,            "[10050] Network is down" },
 { SOCENETUNREACH,         "[10051] Network is unreachable" },
 { SOCENETRESET,           "[10052] Network dropped connection on reset" },
 { SOCECONNABORTED,        "[10053] Software caused connection abort" },
 { SOCECONNRESET,          "[10054] Connection reset by peer" },
 { SOCENOBUFS,             "[10055] No buffer space available" },
 { SOCEISCONN,             "[10056] Socket is already connected" },
 { SOCENOTCONN,            "[10057] Socket is not connected" },
 { SOCESHUTDOWN,           "[10058] Can't send after socket shutdown" },
 { SOCETOOMANYREFS,        "[10059] Too many references: can't splice" },
 { SOCETIMEDOUT,           "[10060] Connection timed out" },
 { SOCECONNREFUSED,        "[10061] Connection refused" },
 { SOCELOOP,               "[10062] Too many levels of symbolic links" },
 { SOCENAMETOOLONG,        "[10063] File name too long" },
 { SOCEHOSTDOWN,           "[10064] Host is down" },
 { SOCEHOSTUNREACH,        "[10065] No route to host" },
 { SOCENOTEMPTY,           "[10066] Directory not empty" },
 { -1,                              NULL }
};

/* Copies string corresponding to the error code provided    */
/* into buf, maximum length len. Returns length actually     */
/* copied to buffer, or zero if error code is unknown.       */
/* String resources should be present for each error code    */
/* using the value of the code as the string ID (except for  */
/* error = 0, which is mapped to WSABASEERR to keep it with  */
/* the others). The DLL is free to use any string IDs that   */
/* are less than WSABASEERR for its own use. The LibMain     */
/* procedure of the DLL is presumed to have saved its        */
/* HINSTANCE in the global variable hInst.                   */

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "psos2err.h"

/*--------------------------------------------------------------------*/
/*    p S O S 2 E r r                                                 */
/*                                                                    */
/*    Perform a perror() with logging                                 */
/*--------------------------------------------------------------------*/

void pSOS2Err(const size_t lineno,
              const char *fname,
              const char *prefix,
              int rc)
{
   boolean redirect = ((logfile != stdout) && !isatty(fileno(stdout)));

   const char *msg;

   msg = LookupOS2SocketError(rc);

   if (msg == NULL)
   {
      printmsg(0,
               "%s: undefined OS/2 socket error in file %s at line %d: %d",
               prefix,
               fname,
               lineno,
               rc);
      return;
   }

/*--------------------------------------------------------------------*/
/*           Display the message with option file location            */
/*--------------------------------------------------------------------*/

   printmsg(2,"OS/2 socket error %d in %s at line %d ...",
            (int) rc, fname, lineno );

   printmsg(0,"%s: %s", prefix, msg);

   if ( redirect )
     fprintf(stdout,"%s: %s\n", prefix, msg);

} /* pSOS2Err */

/*--------------------------------------------------------------------*/
/*       L o o k u p O S 2 S o c k e t E r r o r                      */
/*                                                                    */
/*       Look up an error in our list of strings                      */
/*--------------------------------------------------------------------*/

static const char *LookupOS2SocketError(int err)
{
   WSERR *pwsErr = wsErrors;

   while (pwsErr->errNo != -1 && pwsErr->errNo != err)
      pwsErr++;

   return pwsErr->str;

} /* LookupOS2SocketError */
