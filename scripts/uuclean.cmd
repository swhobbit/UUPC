/*--------------------------------------------------------------------*/
/*       Program: UUCLEAN.CMD          28 Nov 1992                    */
/*        Author: Andrew H. Derbyshire                                */
/*                P.O. Box 132                                        */
/*                Arlington, MA 01274                                 */
/*      Internet: help@kew.com                                        */
/*      Function: Perform automatic log file aging and purging        */
/*                for UUPC/extended.                                  */
/*      Language: REXX for OS/2 2.0                                   */
/*     Arguments: None                                                */
/*   Environment: OS/2 environment variable must be set to the        */
/*                location of the UUPC/extended system                */
/*                configuation file                                   */
/*                (SET UUPCSYSRC=C:\UUPC\UUPC.RC)                     */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Copyright (c) 1992 by Kendra Electronic Wonderworks, all        */
/*    rights reserved except those granted by the UUPC/extended       */
/*    license.                                                        */
/*--------------------------------------------------------------------*/

/*
 *       $Id$
 *
 *       $Log$
 */

/*--------------------------------------------------------------------*/
/*                    Trap uninitialized variables                    */
/*--------------------------------------------------------------------*/

signal on novalue
'@echo off'                   /* Do not echo command input           */

/*--------------------------------------------------------------------*/
/*    To do anything, we need the name of the UUPC/extended spool     */
/*    directory, configuration directory, and temporary directory     */
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

tempdir = getuupc("TEMPDIR" )
if tempdir == '' then
   tempdir = value('TEMP',,'OS2ENVIRONMENT')
if tempdir == '' then
   tempdir = value('TMP',,'OS2ENVIRONMENT')
if tempdir == '' then
do
   say 'No TEMP directory defined, cannot continue'
   exit 98
end

/*--------------------------------------------------------------------*/
/*                    Disable UNDELETE processing                     */
/*--------------------------------------------------------------------*/

deldir = value('DELDIR','','OS2ENVIRONMENT')

/*--------------------------------------------------------------------*/
/*    Process odd logfiles which may have been left around by         */
/*    aborted programs.                                               */
/*--------------------------------------------------------------------*/

call process spooldir,'UUPC*.LOG', 'GENERIC'

/*--------------------------------------------------------------------*/
/*             SYSLOG has funny name, so process explictly            */
/*--------------------------------------------------------------------*/

call process spooldir,'SYSLOG', 'SYSLOG'

/*--------------------------------------------------------------------*/
/*                     Process all other log files                    */
/*--------------------------------------------------------------------*/

xrc = SysFileTree(spooldir || '\*.log', 'data.','F')
if xrc == 0 then
do count = 1 to data.0
   parse upper var data.count mmddyy hhmmss bytes attr fname
   basename = filespec( 'N' , data.count )
   parse var basename stem'.'
   if left( basename, 4 ) <> 'UUPC' then   /* Don't do UUPC*.LOG again */
      call process spooldir, basename, stem
end

/*--------------------------------------------------------------------*/
/*           Clean up temporary files in the spool directory          */
/*--------------------------------------------------------------------*/

call purge spooldir, '*.TMP'     /* Created by UUCICO                */
call purge spooldir, '*.BAK'     /* Maybe created by UUCICO          */

/*--------------------------------------------------------------------*/
/*         Clean up temporary files in the Temporary directory        */
/*--------------------------------------------------------------------*/

call purge tempdir,'*.BAK'
call purge tempdir,'UUPC*.TMP'
call purge tempdir,'UUPC*.TXT'

/*--------------------------------------------------------------------*/
/*             If the news ACTIVE file exists, run expire             */
/*--------------------------------------------------------------------*/

if exist( confdir || '\active' ) then
   'expire'


/*--------------------------------------------------------------------*/
/*     Re-enable UNDELETE processing so we can clean up the cache     */
/*--------------------------------------------------------------------*/

call value 'DELDIR','deldir','OS2ENVIRONMENT'

/*--------------------------------------------------------------------*/
/*    Purge the undelete cache to improve performance and free        */
/*    space                                                           */
/*--------------------------------------------------------------------*/

if deldir <> '' then
do;
   'UNDELETE /F /S /A' spooldir;
   'UNDELETE /F /S /A' tempdir;
end;

/*--------------------------------------------------------------------*/
/*                           All done, exit                           */
/*--------------------------------------------------------------------*/

return

/*--------------------------------------------------------------------*/
/*    p r o c e s s                                                   */
/*                                                                    */
/*    Age a single set of log files                                   */
/*--------------------------------------------------------------------*/

process:procedure
parse upper arg spooldir, input, archive, maxsize, generation
maxgen = 5
aged   = 0                    /* Next older version was aged         */
moved  = 0                    /* This version was aged               */
if generation = '' then
   generation = 1
else
   generation = generation + 1

if maxsize = '' then
do;
   if pos('*', input) > 0 then
      maxsize = 0
   else
      maxsize = 10000;
end;
newgen = archive || '.' || right( generation, 3, '0')
target = spooldir || '\' || newgen

if substr(target,2,1) = ':' then
  target = substr( target, 3 );

/*--------------------------------------------------------------------*/
/*          Determine if file exists  if not, return quietly          */
/*--------------------------------------------------------------------*/

xrc = SysFileTree(spooldir || '\' || input , 'data.','F')
if xrc <> 0 then
   return 0
/*--------------------------------------------------------------------*/
/*             Process whatever files the search turned up            */
/*--------------------------------------------------------------------*/
do count = 1 to data.0
   data = space(data.count)
   parse upper var data mmddyy hhmmss bytes attr fname
   if bytes = 0 then             /* Kill any empty file we find      */
      call Purge fname
   else if bytes > maxsize then
   do

      if \ aged then             /* Only age olders file per run     */
         aged = process(spooldir, newgen , archive, maxsize, generation)
      say 'Aging' input 'to' target

      if generation > maxgen then      /* Really old files go away   */
         call Purge fname
      else if exist( target ) then     /* Else append if needed      */
      do
         'COPY' target || '+' || fname
         call SysFileDelete fname
      end
      else
         'MOVE' fname target     /* But move if possible, faster     */
      moved = 1
   end

end count

/*--------------------------------------------------------------------*/
/*       Flag whether or not files were moved to previous level       */
/*--------------------------------------------------------------------*/

return moved

/*--------------------------------------------------------------------*/
/*    p u r g e                                                       */
/*                                                                    */
/*    Delete files with specified mask and wildcard spec              */
/*--------------------------------------------------------------------*/

purge:procedure
parse arg directory,file
if file == '' then
   mask = directory
else
   mask = directory || '\' || file
xrc = SysFileTree(mask, 'dir.', 'SF', '*----')
if xrc <> 0 then
do
   say 'Directory read error' xrc
   exit xrc
end
if dir.0 > 1 then
   say dir.0 'files found for' mask
do count = 1 to dir.0
   parse var dir.count mmddyy hhmmss bytes attr fname
   fname = space( fname )
   xrc = sysfiledelete( fname )
   if xrc == 0 then
      say 'Deleted' fname
   else
      say 'Error deleting' fname || ', return code =' xrc
end
return

/*--------------------------------------------------------------------*/
/*    e x i s t                                                       */
/*                                                                    */
/*    Report whether or not a file exists                             */
/*--------------------------------------------------------------------*/
exist: procedure
parse arg file
xrc = SysFileTree(file, 'data.','F')
if data.0 == 0 then
   return 0
else
   return 1

/*--------------------------------------------------------------------*/
/*       g e t u u p c                                                */
/*                                                                    */
/*       Get UUPC/extended configuration variable                     */
/*--------------------------------------------------------------------*/

getuupc:procedure
parse upper arg keyword,answer

uupcrc = value('UUPCSYSRC',,'OS2ENVIRONMENT')
if  uupcrc == '' then
do
   'UUPCSYSRC not set, cannot continue'
   exit 44
end

xrc = SysFileSearch( keyword || '=',uupcrc,'data.')
if xrc \= 0 then
do
   say 'SysFileSearch error' xrc 'searching' uupcrc 'for' keyword
   exit xrc
end

do count = 1 to data.0
   parse var data.count newkey'='string

   if translate(newkey) = keyword then
      answer = string
end
return translate(answer,'\','/')

/*--------------------------------------------------------------------*/
/*    n o v a l u e                                                   */
/*                                                                    */
/*    Trap for uninitialized variables                                */
/*--------------------------------------------------------------------*/

novalue:
signal off novalue            /* Avoid nasty recursion         */
say 'Uninitialized variable in line' sigl || ':'
say sourceline( sigl )
