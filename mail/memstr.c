/*--------------------------------------------------------------------*/
/*       m e m s t r . c                                              */
/*                                                                    */
/*       Search a buffer with possible NULL's for a substring         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1998 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *       $Id: lib.h 1.43 1998/04/20 02:48:54 ahd Exp $
 *
 *       Revision history:
 *       $Log: lib.h $
 */

#include "uupcmoah.h"

#include <string.h>

/*--------------------------------------------------------------------*/
/*    m e m s t r                                                     */
/*                                                                    */
/*    String a buffer with possible nulls for a substring             */
/*--------------------------------------------------------------------*/

char *
memstr(const char *haystack,
       const char *needle,
       size_t len)
{
   size_t needleLength = strlen(needle);
   char *start = (char *) haystack;

   while(len > 0)
   {
      char *here = memchr(start, needle[0], len - needleLength + 1);

#ifdef UDEBUG2
      if (debuglevel > 4)
         printmsg(4,"memstr: Searched (%p,%x,%d) and found %p",
                 start,
                 (int) needle[0],
                 len - needleLength + 1,
                 here);
#endif

      if (here == NULL)
         break;

      /* If we found the string, return it */
      if (!strncmp(here, needle, needleLength))
         return here;

      /* Set up to search rest of buffer next iteration */
      len = len - (here - start) - 1;
      start = here + 1;
   }

   return NULL;

} /* memstr */
