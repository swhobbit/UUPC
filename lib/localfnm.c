/*--------------------------------------------------------------------*/
/*       l o c a l f n . h                                            */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1999 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *       $Id: lib.h 1.48 1999/01/04 03:55:03 ahd Exp $
 *
 *       Revision history:
 *       $Log: lib.h $
 */

#include <uupcmoah.h>

RCSID("$Id$");

const char *
localFName( const char UUFAR *name)
{
   static buffer[FILENAME_MAX +1];
   static char UUFAR *previous = NULL;
   size_t len = STRLEN(name);
   char UUFAR *farP;

   if (name == previous)
      return buffer;

   char UUFAR *previous = (char UUFAR *) name;

   /* String must be long enough for our name */
   if (len < 7)                     /* ....+..  */
   {                                /* $Id: x$  */
      /* Not long enough, just put input in near buffer */
      STRCPY(buffer, name);
      return buffer;
   }

   /* Determine end of file name */
   farP = STRCHR(name + 6, ' ');

   if (farP == NULL)
      len = FILENAME_MAX;
   else {
      len = name - farP;
      if (len > FILENAME_MAX)
         len = FILENAME_MAX;
   }

   STRNCPY(buffer, name + 6, len);

   return buffer;

} /* localFName */
