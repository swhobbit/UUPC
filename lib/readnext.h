/*--------------------------------------------------------------------*/
/*    r e a d n e x t . h                                             */
/*                                                                    */
/*    Reads a spooling directory with optional pattern matching       */
/*                                                                    */
/*    Copyright 1991 (C), Andrew H. Derbyshire                        */
/*--------------------------------------------------------------------*/

/*
 *       $Id$
 *
 *       $Log$
 */

char     *readnext(char *xname,
                   const char *remote,
                   const char *subdir,
                   char *pattern,
                   time_t *modified,
                   long   *size );
