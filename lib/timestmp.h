/*

   timestmp.h

   Defines functions included in timestmp.c for UUPC/extended

   Change History:

   08 Sep 90   -  Create for banner function (moved from host.c)  ahd
   07 Mar 92   -  Add current program name
 */

extern char compilev[];           /* UUPC version number           */
extern char compilep[];           /* UUPC Program name             */
extern char compilet[];           /* UUPC compile time             */
extern char compiled[];           /* UUPC compile date             */
extern char *compilen;            /* Current program name          */

void banner (char **argv);
