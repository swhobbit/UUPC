/*--------------------------------------------------------------------*/
/*    g e t s e q . c                                                 */
/*                                                                    */
/*    Job sequence number routines for UUPC/extended                  */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1997 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: getseq.c 1.19 1996/01/01 20:52:08 ahd v1-12r $
 *
 *    Revision history:
 *    $Log: getseq.c $
 *    Revision 1.19  1996/01/01 20:52:08  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.18  1995/09/26 00:37:40  ahd
 *    Use unsigned sequence number for jobs to prevent mapping errors
 *
 *    Revision 1.17  1995/09/24 19:07:05  ahd
 *    Better debugging for funky 32 bit error
 *
 *    Revision 1.16  1995/03/08 03:00:20  ahd
 *    Always force new sequence number to be reasonable (0 < newseq < 1000000)
 *
 *    Revision 1.15  1995/02/20 00:40:12  ahd
 *    Correct C compiler warnings
 *
 *    Revision 1.14  1995/02/15 01:56:18  ahd
 *    Warn if sequence number is reset
 *
 *    Revision 1.13  1995/02/12 23:37:04  ahd
 *    compiler cleanup, NNS C/news support, optimize dir processing
 *
 *    Revision 1.12  1995/01/09 01:39:22  ahd
 *    Optimize retrieval of UUCP sequence number
 *
 *    Revision 1.11  1994/12/22 00:08:40  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.10  1994/02/19 04:42:01  ahd
 *    Use standard first header
 *
 *     Revision 1.9  1994/02/19  04:06:42  ahd
 *     Use standard first header
 *
 *     Revision 1.8  1994/02/19  03:50:56  ahd
 *     Use standard first header
 *
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

#include "uupcmoah.h"

#include <process.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "getseq.h"

currentfile();

/*--------------------------------------------------------------------*/
/*    g e t S e q                                                     */
/*                                                                    */
/*    Return next available sequence number for UUPC processing       */
/*--------------------------------------------------------------------*/

unsigned long getSeq()
{
   char seqfile[FILENAME_MAX];
   FILE *stream;
   unsigned long seq = 0;

   mkfilename(seqfile, E_spooldir, SFILENAME);

   if ((stream = FOPEN(seqfile, "r+", IMAGE_MODE)) == nil(FILE))
   {
      printerr( seqfile );

      if ((stream = FOPEN(seqfile, "w", IMAGE_MODE)) == nil(FILE))
      {
         printerr( seqfile );
         panic();
      }
   }
   else {
      if ( fread( &seq, sizeof seq, 1, stream ) != 1 )
         printerr( seqfile );
   }

/*--------------------------------------------------------------------*/
/*     Generate a new seed for our sequence if we can't read one      */
/*--------------------------------------------------------------------*/

   if ( ! seq++ )
   {
      seq = ((unsigned long) getpid()) % 10000000;
                                    /* Start number small, semi-unique*/

      printmsg(0,"Resetting sequence number to %lu (0x%08lx)",
                  seq,
                  seq );
   }

/*--------------------------------------------------------------------*/
/*                       Update sequence number                       */
/*--------------------------------------------------------------------*/

   rewind( stream );

   if ((fwrite( &seq, sizeof seq, 1, stream ) != 1) || fclose(stream))
   {
      printerr( seqfile );
      panic();
   }

#ifdef UDEBUG
   printmsg(8, "getseq: seq#=%ld", seq);
#endif

   return seq;

} /* getseq */

/*--------------------------------------------------------------------*/
/*       j o b N u m b e r                                            */
/*                                                                    */
/*       Given a job sequence number, returns a character string      */
/*       for use in file names                                        */
/*--------------------------------------------------------------------*/

char *jobNumber( const unsigned long sequenceIn,
                 const size_t lengthIn,
                 const KWBoolean monocase )
{
   static char buf[10];
   static const char set[] =
      "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

   unsigned long sequence = sequenceIn;
   size_t length = lengthIn;
   const int base = (sizeof set - 1) - (monocase ? 26 : 0);
   char *p = buf + sizeof buf - 2;

   if ( length >= sizeof buf )      /* User want too many digits?    */
      length = sizeof buf - 1;      /* Yes --> Limit the string      */

   while( length-- > 0 )
   {
      *p-- = set[ (size_t) (sequence % base) ];
      sequence /= base ;
   } /* while */

   p++;                             /* Step back to first character  */

   printmsg(5, "jobNumber: seq#=%ld, length = %u, job id = \"%s\"",
                sequenceIn,
                lengthIn,
                p);

   return p;

} /* jobNumber */
