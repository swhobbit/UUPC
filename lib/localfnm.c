/*--------------------------------------------------------------------*/
/*       l o c a l f n . c                                            */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-2001 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *       $Id: localfnm.c 1.4 2000/05/12 12:29:45 ahd v1-13g $
 *
 *       Revision history:
 *       $Log: localfnm.c $
 *       Revision 1.4  2000/05/12 12:29:45  ahd
 *       Annual copyright update
 *
 *       Revision 1.3  1999/01/17 19:17:28  ahd
 *       Correct parsing of file name
 *
 *       Revision 1.2  1999/01/08 02:20:43  ahd
 *       Convert currentfile() to RCSID()
 *
 *       Revision 1.1  1999/01/07 00:58:55  ahd
 *       Initial revision
 *
 */

#include "uupcmoah.h"

RCSID("$Id: localfnm.c 1.4 2000/05/12 12:29:45 ahd v1-13g $");

/*--------------------------------------------------------------------*/
/*       l o c a l F N a m e                                          */
/*                                                                    */
/*       Convert UUFAR rcsid to near string                           */
/*--------------------------------------------------------------------*/

const char *
localFName( const char UUFAR *name)
{
   static char buffer[FILENAME_MAX +1];
   static char UUFAR *previous = NULL;
   size_t len = STRLEN(name);
   char UUFAR *farP;

   if (name == previous)
      return buffer;

   previous = (char UUFAR *) name;

   /* String must be long enough for our name */
   if (len < 7)                     /* ....+..  */
   {                                /* $Id: localfnm.c 1.4 2000/05/12 12:29:45 ahd v1-13g $  */
      /* Not long enough, just put input in near buffer */
      STRCPY(buffer, name);
      return buffer;
   }

   /* Determine end of file name */
   farP = STRCHR(name + 5, ' ');

   if (farP == NULL)
      len = FILENAME_MAX;
   else {
      len = farP - (name + 5);
      if (len > FILENAME_MAX)
         len = FILENAME_MAX;
   }

   MEMCPY(buffer, name + 5, len);
   buffer[len] = '\0';

   return buffer;

} /* localFName */
