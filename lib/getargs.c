/*--------------------------------------------------------------------*/
/*    g e t a r g s . c                                               */
/*                                                                    */
/*    Support routines for UUPC/extended                              */
/*                                                                    */
/*    Changes Copyright 1990, 1991 (c) Andrew H. Derbyshire           */
/*                                                                    */
/*    History:                                                        */
/*       21Nov1991 Break out of lib.c                          ahd    */
/*--------------------------------------------------------------------*/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"

/*--------------------------------------------------------------------*/
/*    g e t a r g s                                                   */
/*                                                                    */
/*    Return a list of pointers to tokens in the given line           */
/*--------------------------------------------------------------------*/

int getargs(char *line, char **flds)
{
   int i = 0;
   char quoted = '\0';

   while ((*line != '\0') && (*line != '\n')) {
      if (isspace(*line))
         line++;
      else {
         char *out = line;
         *flds++ = line;
         i++;
         while((quoted || !isspace(*line)) && (*line != '\0'))
         {
            switch(*line)
            {
               case '"':
               case '\'':
                  if (quoted)
                  {
                     if (quoted == *line)
                     {
                        quoted = 0;
                        line++;
                     }
                     else
                        *out++ = *line++;
                  } /* if */
                  else
                     quoted = *line++;
                  break;

               case '\\':
                  switch(*++line)         /* Unless the following    */
                  {                       /* character is very       */
                     default:             /* special we pass the \   */
                        if (!isspace(*line))
                           *out++ = '\\'; /* and following char on   */
                     case '"':
                     case '\'':
                        *out++ = *line++;
                  }
                  break;

               default:
                  *out++ = *line++;

            } /*switch*/
         } /* while */
         if (isspace(*line))
            line++;
         *out = '\0';
      } /* else */
   }

   return i;

} /*getargs*/
