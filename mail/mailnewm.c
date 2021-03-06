/*--------------------------------------------------------------------*/
/*       m a i l n e w m . C                                          */
/*                                                                    */
/*       Include new mail into the user mailbox                       */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-2002 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by the   */
/*       UUPC/extended license agreement.                             */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *       $Id: mailnewm.c 1.8 2001/03/12 13:56:08 ahd v1-13k $
 *
 *       Revision History:
 *       $Log: mailnewm.c $
 *       Revision 1.8  2001/03/12 13:56:08  ahd
 *       Annual Copyright update
 *
 *       Revision 1.7  2000/05/12 12:36:30  ahd
 *       Annual copyright update
 *
 *       Revision 1.6  1999/01/08 02:21:01  ahd
 *       Convert currentfile() to RCSID()
 *
 *       Revision 1.5  1999/01/04 03:54:27  ahd
 *       Annual copyright change
 *
 *       Revision 1.4  1998/11/04 01:59:55  ahd
 *       Prevent buffer overflows when processing UIDL lines
 *       in POP3 mail.
 *       Add internal sanity checks for various client structures
 *       Convert various files to CR/LF from LF terminated lines
 *
 * Revision 1.3  1998/03/16  07:47:40  ahd
 * Delete generation UIDL, that's the POP3 server's problem
 *
 *       Revision 1.2  1998/03/09 01:18:19  ahd
 *       Change debug level of file not found message
 *
 *       Revision 1.1  1998/03/03 03:51:53  ahd
 *       Initial revision
 *
 */

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"
#include "getseq.h"
#include "mail.h"
#include "stater.h"

/*--------------------------------------------------------------------*/
/*                          Local constants                           */
/*--------------------------------------------------------------------*/

RCSID("$Id: mailnewm.c 1.8 2001/03/12 13:56:08 ahd v1-13k $");

static const char sep[] = MESSAGESEP;

/*--------------------------------------------------------------------*/
/*       I n c l u d e N e w                                          */
/*                                                                    */
/*       Includes mail from the system box into the user's local      */
/*       mailbox                                                      */
/*--------------------------------------------------------------------*/

KWBoolean
IncludeNew(const char *target, const char *user)
{
#ifdef UDEBUG
   static const char mName[] = "IncludeNew";
#endif
   time_t age;
   long size;
   FILE *stream_in;
   FILE *stream_out;

   char sysbox[FILENAME_MAX];
   char buf[BUFSIZ];

   mkmailbox(sysbox, user);

/*--------------------------------------------------------------------*/
/*      Return semi-quietly if we can't open the system mailbox       */
/*--------------------------------------------------------------------*/

   stream_in   = FOPEN(sysbox, "r", TEXT_MODE);

   if (stream_in == NULL)
   {
      if (debuglevel > 2)
         printerr(sysbox);
      return KWTrue;
   }

/*--------------------------------------------------------------------*/
/*      Determine if we have new mail, returning quietly if not       */
/*--------------------------------------------------------------------*/

   age = stater(sysbox, &size);

   if (age == (time_t) -1L)
      panic();

   printmsg(1, "Including mail from %s through %.24s",
            sysbox,
            ctime(&age));

/*--------------------------------------------------------------------*/
/*                    Now open up the output file                     */
/*--------------------------------------------------------------------*/

   stream_out  = FOPEN(target, "a+", TEXT_MODE);

   if (stream_out == NULL)
   {
      printerr(target);
      panic();
   }

/*--------------------------------------------------------------------*/
/*                       Loop to read the data                        */
/*--------------------------------------------------------------------*/

   while ((fgets(buf, sizeof buf, stream_in)) != NULL)
   {
      size_t bytes = strlen(buf);

      /* Now write it out, with error checking */
      if (fwrite(buf, sizeof(char), bytes, stream_out) != bytes)
      {
         printmsg(0, "Error including new mail into %s", target);
         printerr(target);
         fclose(stream_in);
         fclose(stream_out);
         return KWFalse;
      }

   } /* while */

/*--------------------------------------------------------------------*/
/*                   Clean up and return to caller                    */
/*--------------------------------------------------------------------*/

   if (ferror(stream_in))
   {
      printerr(sysbox);
      return KWFalse;
   }

   fclose(stream_in );
   fclose(stream_out);

   filebkup(sysbox);
   REMOVE(sysbox);

   return KWTrue;

} /* IncludeNew */
