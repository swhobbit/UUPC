/*--------------------------------------------------------------------*/
/*       l o c a l f n . c                                            */
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
 *       $Id: localfnm.c 1.1 1999/01/07 00:58:55 ahd Exp $
 *
 *       Revision history:
 *       $Log: localfnm.c $
 *       Revision 1.1  1999/01/07 00:58:55  ahd
 *       Initial revision
 *
 */

#include "uupcmoah.h"

RCSID("$Id: localfnm.c 1.1 1999/01/07 00:58:55 ahd Exp $");

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
   {                                /* $Id: localfnm.c 1.1 1999/01/07 00:58:55 ahd Exp $  */
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

   MEMCPY(buffer, name + 6, len);

   return buffer;

} /* localFName */
