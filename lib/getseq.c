/*--------------------------------------------------------------------*/
/*    g e t s e q . c                                                 */
/*                                                                    */
/*    Job sequence number routines for UUPC/extended                  */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1990-1993 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: GETSEQ.C 1.3 1993/04/11 00:32:05 ahd Exp $
 *
 *    Revision history:
 *    $Log: GETSEQ.C $
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

#include <stdio.h>
#include <process.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "hlib.h"
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
