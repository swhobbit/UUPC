/*--------------------------------------------------------------------*/
/*       p w s e r r . c                                              */
/*                                                                    */
/*       Report error message from Windows sockets error list         */
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
 *    $Id: pwserr.c 1.7 1994/02/19 05:09:44 ahd Exp $
 *
 *    Revision history:
 *    $Log: pwserr.c $
 * Revision 1.7  1994/02/19  05:09:44  ahd
 * Use standard first header
 *
 * Revision 1.6  1994/01/01  19:20:39  ahd
 * Annual Copyright Update
 *
 * Revision 1.5  1993/10/30  22:07:19  dmwatt
 * Host byte ordering and far/near pointer corrections
 *
 * Revision 1.5  1993/10/30  22:07:19  dmwatt
 * Host byte ordering and far/near pointer corrections
 *
 * Revision 1.4  1993/10/12  01:32:08  ahd
 * Normalize comments to PL/I style
 *
 * Revision 1.3  1993/10/03  20:37:34  ahd
 * Correct FAR pointer references
 *
 * Revision 1.2  1993/10/02  23:45:16  ahd
 * Delete unneeded function
 *
 * Revision 1.1  1993/10/02  23:12:35  dmwatt
 * Initial revision
 *
 */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"

#include <io.h>

#include "winsock.h"       /* So we search local directories           */

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                     Local function prototypes                      */
/*--------------------------------------------------------------------*/

const char UUFAR *LookupWSError(int err);

/*--------------------------------------------------------------------*/
/*                          Local variables                           */
/*--------------------------------------------------------------------*/

typedef struct wserr {
        int errNo;
        const char UUFAR *str;
} WSERR;

static WSERR wsErrors[] = {
  { WSABASEERR,         "[0] No Error" },
  { WSAEINTR,           "[10004] Interrupted system call" },
  { WSAEBADF,           "[10009] Bad file number" },
  { WSAEACCES,          "[10013] Permission denied" },
  { WSAEFAULT,          "[10014] Bad address" },
  { WSAEINVAL,          "[10022] Invalid argument" },
  { WSAEMFILE,          "[10024] Too many open files" },
  { WSAEWOULDBLOCK,     "[10035] Operation would block" },
  { WSAEINPROGRESS,     "[10036] Operation now in progress" },
  { WSAEALREADY,        "[10037] Operation already in progress" },
  { WSAENOTSOCK,        "[10038] Socket operation on non-socket" },
  { WSAEDESTADDRREQ,    "[10039] Destination address required" },
  { WSAEMSGSIZE,        "[10040] Message too long" },
  { WSAEPROTOTYPE,      "[10041] Protocol wrong type for socket" },
  { WSAENOPROTOOPT,     "[10042] Bad protocol option" },
  { WSAEPROTONOSUPPORT, "[10043] Protocol not supported" },
  { WSAESOCKTNOSUPPORT, "[10044] Socket type not supported" },
  { WSAEOPNOTSUPP,      "[10045] Operation not supported on socket" },
  { WSAEPFNOSUPPORT,    "[10046] Protocol family not supported" },
  { WSAEAFNOSUPPORT,    "[10047] Address family not supported by protocol family" },
  { WSAEADDRINUSE,      "[10048] Address already in use" },
  { WSAEADDRNOTAVAIL,   "[10049] Can't assign requested address" },
  { WSAENETDOWN,        "[10050] Network is down" },
  { WSAENETUNREACH,     "[10051] Network is unreachable" },
  { WSAENETRESET,       "[10052] Net dropped connection or reset" },
  { WSAECONNABORTED,    "[10053] Software caused connection abort" },
  { WSAECONNRESET,      "[10054] Connection reset by peer" },
  { WSAENOBUFS,         "[10055] No buffer space available" },
  { WSAEISCONN,         "[10056] Socket is already connected" },
  { WSAENOTCONN,        "[10057] Socket is not connected" },
  { WSAESHUTDOWN,       "[10058] Can't send after socket shutdown" },
  { WSAETOOMANYREFS,    "[10059] Too many references, can't splice" },
  { WSAETIMEDOUT,       "[10060] Connection timed out" },
  { WSAECONNREFUSED,    "[10061] Connection refused" },
  { WSAELOOP,           "[10062] Too many levels of symbolic links" },
  { WSAENAMETOOLONG,    "[10063] File name too long" },
  { WSAEHOSTDOWN,       "[10064] Host is down" },
  { WSAEHOSTUNREACH,    "[10065] No Route to Host" },
  { WSAENOTEMPTY,       "[10066] Directory not empty" },
  { WSAEPROCLIM,        "[10067] Too many processes" },
  { WSAEUSERS,          "[10068] Too many users" },
  { WSAEDQUOT,          "[10069] Disc Quota Exceeded" },
  { WSAESTALE,          "[10070] Stale NFS file handle" },
  { WSAEREMOTE,         "[10071] Too many levels of remote in path" },
  { WSASYSNOTREADY,     "[10091] Network SubSystem is unavailable" },
  { WSAVERNOTSUPPORTED, "[10092] WINSOCK DLL Version out of range" },
  { WSANOTINITIALISED,  "[10093] Successful WSASTARTUP not yet performed" },
  { WSAHOST_NOT_FOUND,  "[11001] Host not found" },
  { WSATRY_AGAIN,       "[11002] Non-Authoritative Host not found" },
  { WSANO_RECOVERY,     "[11003] Non-Recoverable errors: FORMERR, REFUSED, NOTIMP" },
  { WSANO_DATA,         "[11004] Valid name, no data record of requested type" },
  { -1,                                 NULL }
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

#include "pwserr.h"

/*--------------------------------------------------------------------*/
/*    p W S e r r                                                     */
/*                                                                    */
/*    Perform a perror() with logging                                 */
/*--------------------------------------------------------------------*/

void pWSErr(const size_t lineno,
             const char *fname,
             const char *prefix,
             int rc)
{
   boolean redirect = ((logfile != stdout) && !isatty(fileno(stdout)));

   const char UUFAR *msg;

   msg = LookupWSError(rc);

   if (msg == NULL) {
      printmsg(0, "Invalid error sent to pWSErr: %d", rc);
      return;
   }

/*--------------------------------------------------------------------*/
/*           Display the message with option file location            */
/*--------------------------------------------------------------------*/

   printmsg(2,"Windows sockets error %d in %s at line %d ...",
            (int) rc, fname, lineno );

#ifdef _Windows
   printmsg(0,"%s: %Fs", prefix, msg);
#else
   printmsg(0,"%s: %s", prefix, msg);
#endif

   if ( redirect )
#ifdef _Windows
     fprintf(stdout,"%s: %Fs\n", prefix, msg);
#else
     fprintf(stdout,"%s: %s\n", prefix, msg);
#endif

} /* pWSErr */

/*--------------------------------------------------------------------*/
/*       L o o k u p W S E r r o r                                    */
/*                                                                    */
/*       Look up an error in our list of strings                      */
/*--------------------------------------------------------------------*/

const char UUFAR *LookupWSError(int err)
{
   WSERR *pwsErr = wsErrors;

   while (pwsErr->errNo != -1 && pwsErr->errNo != err)
      pwsErr++;

   return pwsErr->str;

} /* LookupWSError */
