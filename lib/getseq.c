/*--------------------------------------------------------------------*/
/*    g e t s e q . c                                                 */
/*                                                                    */
/*    Job sequence number routines for UUPC/extended                  */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1994 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: getseq.c 1.7 1994/02/18 23:09:45 ahd Exp $
 *
 *    Revision history:
 *    $Log: getseq.c $
 *     Revision 1.7  1994/02/18  23:09:45  ahd
 *     Use standard first header
 *
 *     Revision 1.6  1994/01/24  03:07:44  ahd
 *     Annual Copyright Update
 *
 *     Revision 1.5  1994/01/01  19:02:17  ahd
 *     Annual Copyright Update
 *
 *     Revision 1.4  1993/06/21  02:17:31  ahd
 *     Reset after missing file to PID
 *
 *     Revision 1.4  1993/06/21  02:17:31  ahd
 *     Reset after missing file to PID
 *
 *     Revision 1.3  1993/04/11  00:32:05  ahd
 *     Global edits for year, TEXT, etc.
 *
 * Revision 1.2  1992/11/19  02:58:00  ahd
 * drop rcsid
 *
 * Revision 1.1  1992/11/16  05:00:26  ahd
 * Initial revision
 *
 */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include <process.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"
#include "getseq.h"

currentfile();

/*--------------------------------------------------------------------*/
/*    g e t s e q                                                     */
/*                                                                    */
/*    Return next available sequence number for UUPC processing       */
/*--------------------------------------------------------------------*/

long getseq()
{
   char seqfile[FILENAME_MAX];
   FILE *seqfile_fp;
   long seq;

   mkfilename(seqfile, E_confdir, SFILENAME);
   if ((seqfile_fp = FOPEN(seqfile, "r",TEXT_MODE)) != nil(FILE))
   {
      fscanf(seqfile_fp, "%ld", &seq);
      fclose(seqfile_fp);
   }
   else {
      seq = getpid();
      printerr( seqfile );
   };

/*--------------------------------------------------------------------*/
/*                       Update sequence number                       */
/*--------------------------------------------------------------------*/

   if ((seqfile_fp = FOPEN(seqfile, "w",TEXT_MODE)) != nil(FILE))
   {
      fprintf(seqfile_fp, "%ld\n", seq+1);
      fclose(seqfile_fp);
   }
   else {
      printerr( seqfile );
      panic();
   }

   printmsg(5, "getseq: seq#=%ld", seq);
   return seq;

} /*getseq*/

/*--------------------------------------------------------------------*/
/*    J o b N u m b e r                                               */
/*                                                                    */
/*    Given a job sequence number, returns a character string for use */
/*    in file names                                                   */
/*--------------------------------------------------------------------*/

char *JobNumber( long sequence )
{
      static char buf[4];
      const long base = bflag[F_ONECASE] ? 36 : 62;
      static const char set[] =
         "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
      size_t count = sizeof buf - 1;

      buf[count] = '\0';

      sequence %= (base*base*base);

      while( count-- > 0 )
      {
         buf[count] = set[ (int) (sequence % base) ];
         sequence /= base ;
      } /* while */

      return buf;

} /* JobNumber */
