/*--------------------------------------------------------------------*/
/*       m a i l n e w m . C                                          */
/*                                                                    */
/*       Include new mail into the user mailbox                       */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1998 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by the   */
/*       UUPC/extended license agreement.                             */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *       $Id: mailnewm.c 1.1 1998/03/03 03:51:53 ahd v1-12v $
 *
 *       Revision History:
 *       $Log: mailnewm.c $
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

RCSID("$Id: mailnewm.c 1.1 1998/03/03 03:51:53 ahd v1-12v $");

currentfile();

static const char sep[] = MESSAGESEP;
static const char uidl[] = UIDL_HEADER;

/*--------------------------------------------------------------------*/
/*       m a k e U I D L                                              */
/*                                                                    */
/*       Generate Unique sequence number for this message             */
/*--------------------------------------------------------------------*/

static void
makeUIDL(FILE *stream_out)
{

#ifdef UDEBUG
   static const char mName[] = "makeUIDL";
#endif

   static long runNumber = 0;
   static long subsequence = 0;
   static time_t now;

   if (runNumber == 0)
   {
      runNumber = getSeq();
      time(&now);
   }

   fprintf(stream_out, "%s <%x.%x.%d@%.40s>\n",
            uidl,
            runNumber,
            now,
            ++subsequence,
            E_domain);

#ifdef UDEBUG
    printmsg(6, "%s: Wrote UIDL for message %d",
                  mName,
                  subsequence);
#endif

} /* makeUIDL */

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
   KWBoolean haveHeader = KWFalse;
   KWBoolean haveUIDL = KWFalse;

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

      if (haveHeader && equal(buf, "\n"))
      {
         if (! haveUIDL)
            makeUIDL(stream_out);
         haveHeader = KWFalse;
      }
      else if (equaln(buf, sep, strlen(sep)))
      {
         haveHeader = KWTrue;
         haveUIDL   = KWFalse;
      }
      else if (haveHeader &&
                equaln(buf, uidl, sizeof uidl - 1))
         haveUIDL = KWTrue;

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
