/*
 *       $Id$
 *
 *       $Log$
 */

/*--------------------------------------------------------------------*/
/*       Copyright 1990-1994 By Kendra Electronic Wonderworks;        */
/*       may be distributed freely if original documentation and      */
/*       source are included, and credit is given to the authors.     */
/*       For additional instructions, see README.PRN in UUPC/         */
/*       extended documentation archive.                              */
/*--------------------------------------------------------------------*/
'@echo off'
parse arg '-h' foo .
if foo == '' then
   'uustat -q'
'uucico' arg(1)
'uuxqt'
