/*--------------------------------------------------------------------*/
/*       Program: UUHOUR.CMD           14 Feb 1994                    */
/*        Author: Andrew H. Derbyshire                                */
/*                P.O. Box 132                                        */
/*                Arlington, MA 01274                                 */
/*      Internet: help@kew.com                                        */
/*      Function: Queue dummy requests for remote systems to all      */
/*                UUPOLL to run in poll systems with work mode        */
/*      Language: REXX for OS/2 2.0                                   */
/*     Arguments: None                                                */
/*   Environment: OS/2 environment variable must be set to the        */
/*                location of the UUPC/extended system                */
/*                configuation file                                   */
/*                (SET UUPCSYSRC=C:\UUPC\UUPC.RC)                     */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Copyright (c) 1989-2002 by Kendra Electronic Wonderworks, all   */
/*    rights reserved except those granted by the UUPC/extended       */
/*    license.                                                        */
/*--------------------------------------------------------------------*/

/*
 *       $Id: uuhour.cmd 1.9 2001/03/12 13:57:10 ahd v1-13k $
 *
 *       $Log: uuhour.cmd $
 *       Revision 1.9  2001/03/12 13:57:10  ahd
 *       Annual copyright update
 *
 *       Revision 1.8  2000/05/12 12:46:40  ahd
 *       Annual copyright update
 *
 *       Revision 1.7  1999/01/04 03:56:38  ahd
 *       checkin
 *
 *       Revision 1.6  1998/05/18 12:25:09  ahd
 *       Correct end-of-line character from linefeed to CR/LF
 *
 *      Revision 1.5  1998/03/01  01:32:40  ahd
 *      Annual Copyright Update
 *
 *       Revision 1.4  1997/04/24 01:30:18  ahd
 *       Annual Copyright Update
 *
 *       Revision 1.3  1996/01/01 21:16:20  ahd
 *       Annual Copyright Update
 *
 *       Revision 1.2  1994/12/22 00:29:35  ahd
 *       Annual Copyright Update
 *
 *       Revision 1.1  1994/08/08 00:33:18  ahd
 *       Initial revision
 *
 */

/*--------------------------------------------------------------------*/
/*                    Trap uninitialized variables                    */
/*--------------------------------------------------------------------*/
signal on novalue
'@echo off'                   /* Do not echo command input           */

Call RxFuncAdd 'SysLoadFuncs','RexxUtil','SysLoadFuncs'
Call 'SysLoadFuncs'

/*--------------------------------------------------------------------*/
/*    To do anything, we need the name of the UUPC/extended spool     */
/*    directory and the configuration directory.                      */
/*--------------------------------------------------------------------*/

spooldir = getuupc("SPOOLDIR" )
if spooldir == '' then
do
   say 'No spool directory defined, cannot continue'
   exit 99
end

confdir = getuupc("CONFDIR" )
if confdir == '' then
do
   say 'No configuration directory defined, cannot continue'
   exit 98
end

last = spooldir || '\lastpoll.dat';
poll = confdir  || '\poll';

/*--------------------------------------------------------------------*/
/*       Determine the last time we polled, and don't execute if      */
/*       we already polled this hour                                  */
/*--------------------------------------------------------------------*/

thisHour = time('h')
xrc = SysFileTree(last, 'data.','FT')

if xrc == 0  then
   if data.0 > 0 then
   do;
      parse upper var data.1 timestamp bytes attr fname
      parse var timestamp yy '/' mm '/' dd '/' lastHour '/'
      lastday = yy || mm || dd
      if lastday = substr( date('s'), 3) & lasthour = thisHour then
      do;
         say 'Already called this hour';
         exit 0;
      end;
   end

/*--------------------------------------------------------------------*/
/*          Update the file time stamp to the current time.           */
/*--------------------------------------------------------------------*/

call sysfiledelete last;
call charout last, '';
call charout last

/*--------------------------------------------------------------------*/
/*       Loop through the poll file list, queuing polls for the       */
/*       desired systems                                              */
/*--------------------------------------------------------------------*/

do while lines(poll)
   parse value linein(poll) with system times '#' comment;

/*--------------------------------------------------------------------*/
/*              Process the hours for a specific system.              */
/*--------------------------------------------------------------------*/

   do while times <> ''
      parse var times first','times
      if first = thisHour then
         'UUSTAT -P' system;
   end /* do while */
end;

/*--------------------------------------------------------------------*/
/*                Close poll data file and we're done.                */
/*--------------------------------------------------------------------*/

call lineout poll
