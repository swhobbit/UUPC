/*--------------------------------------------------------------------*/
/*       m e m s t r . c                                              */
/*                                                                    */
/*       Search a buffer with possible NULL's for a substring         */
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
 *       $Id: memstr.c 1.2 1999/01/04 03:54:27 ahd Exp $
 *
 *       Revision history:
 *       $Log: memstr.c $
 *       Revision 1.2  1999/01/04 03:54:27  ahd
 *       Annual copyright change
 *
 *       Revision 1.1  1998/04/24 03:30:13  ahd
 *       Initial revision
 *
 */

#include "uupcmoah.h"

#include <string.h>

RCSID("$Id$");

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
