/*
 *       $Id: uuio.cmd 1.6 1997/04/24 01:30:23 ahd v1-12u $
 *
 *       $Log: uuio.cmd $
 *       Revision 1.6  1997/04/24 01:30:23  ahd
 *       Annual Copyright Update
 *
 *       Revision 1.5  1996/01/01 21:16:26  ahd
 *       Annual Copyright Update
 *
 *       Revision 1.4  1994/12/22 00:29:41  ahd
 *       Annual Copyright Update
 *
 *       Revision 1.3  1994/05/23 23:20:05  ahd
 *       Correct copyright
 *
 *       Revision 1.2  1994/04/26  03:32:25  ahd
 *       Don't print status if hot handle on command line (speeds
 *       up TCP/IP starts from INETD)
 *
 */

/*--------------------------------------------------------------------*/
/*       Copyright 1989-1998 By Kendra Electronic Wonderworks;        */
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
