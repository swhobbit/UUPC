/*--------------------------------------------------------------------*/
/*    n d i r . c                                                     */
/*                                                                    */
/*    Berkeley-style directory reading routine on MS-DOS by Samuel    */
/*    Lam <skl@van-bc.UUCP>, June/87                                  */
/*                                                                    */
/*    Changes Copyright (c) 1990, 1991 by Andrew H. Derbyshire        */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                   Standard library include files                   */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <dos.h>
#include <stdlib.h>
#include <time.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "uundir.h"
#include "getdta.h"

#include "dos2unix.h"

#ifndef __TURBOC__
#include "getdta.h"           /* Custom versions of Disk Xfer Addr
                                 functions for MS environment only   */
#endif

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

currentfile();

static DIR *thisDirP = NULL;
static DIR *lastDirP = NULL;
static openForBusiness = FALSE;

/*--------------------------------------------------------------------*/
/*    o p e n d i r x                                                 */
/*                                                                    */
/*    Open a directory                                                */
/*--------------------------------------------------------------------*/

extern DIR *opendirx( const char *dirname, char *pattern)
{
   union REGS inregs, outregs;
   struct SREGS segregs;
   char pathname[FILENAME_MAX];
   DTA far *dtasave;
   DTA far *dtaptr;
   char far *pathptr;

/*--------------------------------------------------------------------*/
/*                    Build pathname to be scanned                    */
/*--------------------------------------------------------------------*/

   strcpy(pathname, dirname);
   if ((*pattern != '/') || (dirname[ strlen(dirname) - 1] != '/'))
      strcat(pathname,"/");
   strcat(pathname, pattern);

   /* allocate control block */
   thisDirP = malloc(sizeof(DIR));

/*--------------------------------------------------------------------*/
/*                     Set disk transfer address                      */
/*--------------------------------------------------------------------*/

   dtasave = (DTA far *)getdta();
   dtaptr = (DTA far *)&(thisDirP->dirdta);
   setdta((char far *)dtaptr);

/*--------------------------------------------------------------------*/
/*                      look for the first file                       */
/*--------------------------------------------------------------------*/

   inregs.h.ah = 0x4e;
   pathptr = (char far *)pathname;
   segregs.ds = FP_SEG(pathptr);
   inregs.x.dx = FP_OFF(pathptr);
   inregs.x.cx = 0;   /* attribute */
   intdosx(&inregs, &outregs, &segregs);

   /* bad directory name? */
   if (outregs.x.cflag && (outregs.x.ax == 2 || outregs.x.ax == 3)) {
      free(thisDirP);
      return NULL;
   }

   thisDirP->dirfirst = outregs.x.cflag ? outregs.x.ax : 0;

   setdta((char far *)dtasave);
   strcpy(thisDirP->dirid, "DIR");

   printmsg(2,"opendir: Address is %p", thisDirP );
   openForBusiness = TRUE;
   return thisDirP;

} /*opendir*/


/*--------------------------------------------------------------------*/
/*    r e a d d i r                                                   */
/*                                                                    */
/*    Get next entry in a directory                                   */
/*--------------------------------------------------------------------*/

struct direct *readdir(DIR *dirp)
{
   int errcode;

/*--------------------------------------------------------------------*/
/*    Debugging code for failures when running on Novell networks     */
/*--------------------------------------------------------------------*/

   if ( dirp == NULL )
   {
      flushall();
      printmsg(0,"readdir: INTERNAL ERROR: dirp pointer is NULL");
      printmsg(0,"readdir: Snuffles debug code: %s %p %p %p",
                  openForBusiness ? "Open" : "Closed",
                  lastDirP,
                  thisDirP );
      flushall();
      panic();
   }

   if (!equal(dirp->dirid, "DIR"))
   {
      flushall();
      printmsg(0,"readdir: INTERNAL ERROR: No search in progress");
      printmsg(0,"readdir: Snuffles debug code: %s %p %p %p %s",
                  openForBusiness ? "Open" : "Closed",
                  lastDirP,
                  thisDirP,
                  dirp,
                  dirp->dirid );
      flushall();
      panic();
   }

   if (dirp->dirfirst == -1) {
      union REGS inregs, outregs;
      struct SREGS segregs;
      DTA far *dtaptr;
      DTA far *dtasave;

     /* set DTA address to our buffer each time we're called */
      dtasave = (DTA far *)getdta();
      dtaptr = (DTA far *)&(dirp->dirdta);
      setdta((char far *)dtaptr);

      inregs.h.ah = 0x4f;
      segregs.ds = FP_SEG(dtaptr);
      inregs.x.dx = FP_OFF(dtaptr);
      intdosx(&inregs, &outregs, &segregs);
      errcode = outregs.x.cflag ? outregs.x.ax : 0;

      setdta((char far *)dtasave);  /* Restore DTA address     */

   } else {

      errcode = dirp->dirfirst;
      dirp->dirfirst = -1;

   };

   /* no more files in directory? */
   if (errcode == 18)
      return NULL;

   if ( errcode != 0)
   {
      errno = errcode;
      printerr( "readdir" );
      panic();
   }

   dirp->dirent.d_ino = -1;   /* no inode information */
   strlwr(strcpy(dirp->dirent.d_name, dirp->dirdta.filename));
   dirp->dirent.d_namlen = strlen(dirp->dirent.d_name);
   dirp->dirent.d_reclen = sizeof(struct direct) - (MAXNAMLEN + 1) +
      ((((dirp->dirent.d_namlen + 1) + 3) / 4) * 4);

   dirp->dirent.d_modified = dos2unix( dirp->dirdta.filedate,
                                        dirp->dirdta.filetime );
   dirp->dirent.d_size     = dirp->dirdta.filesize;

   return &(dirp->dirent);

} /*readdir*/

/*--------------------------------------------------------------------*/
/*    c l o s e d i r                                                 */
/*                                                                    */
/*    Close a directory                                               */
/*--------------------------------------------------------------------*/

void closedir(DIR *dirp)
{

   strcpy(dirp->dirid, "CLO");
   lastDirP = dirp;
   openForBusiness = FALSE;
   printmsg(2,"closedir: Freeing dirp at %p", dirp );
   free(dirp);

} /*closedir*/
