/*--------------------------------------------------------------------*/
/*       g e t u u p c                                                */
/*                                                                    */
/*       Get UUPC/extended configuration variable                     */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Copyright (c) 1992-1994 by Kendra Electronic Wonderworks, all   */
/*    rights reserved except those granted by the UUPC/extended       */
/*    license.                                                        */
/*--------------------------------------------------------------------*/

/*
 *       $Id: GETUUPC.CMD 1.4 1993/05/10 11:35:05 ahd Exp $
 *
 *       $Log: GETUUPC.CMD $
 *      Revision 1.4  1993/05/10  11:35:05  ahd
 *      Drop extra parse command which broken selected sewarches
 *
 *      Revision 1.3  1993/05/09  14:10:26  ahd
 *      Don't perform user level lookup unless asked
 *
 *      Revision 1.2  1993/05/04  00:25:58  ahd
 *      Support personal.rc to allow su.cmd
 *
*        Revision 1.1  1993/04/04  05:01:49  ahd
*        Initial revision
*
 */

signal on novalue
parse upper arg keyword,answer,uupcusrrc

/*--------------------------------------------------------------------*/
/*                     Get the UUPC.RC file name                      */
/*--------------------------------------------------------------------*/

uupcrc = value('UUPCSYSRC',,'OS2ENVIRONMENT')
if  uupcrc == '' then
do
   'UUPCSYSRC not set, cannot continue'
   exit
end

confdir = translate(filespec('D',uupcrc) || filespec('P',uupcrc),'\','/');

answer = search( keyword, answer, confdir, uupcrc );

if uupcusrrc <> '' then
do;
   uupcusrrc = translate( uupcusrrc , '\', '/');

   if verify( uupcusrrc, '\/', 'N') == 0 then
      uupcusrrc = confdir || '\' || uupcusrrc;

   answer = search( keyword, answer, confdir, uupcusrrc );
end;

if left(answer,1) == '\' then
   answer = filespec('D',uupcrc) || answer;
return answer;

search:procedure
parse arg keyword,answer,confdir,uupcrc
/*--------------------------------------------------------------------*/
/*     Read the file, then scope the contents a line at the time      */
/*--------------------------------------------------------------------*/

xrc = SysFileSearch( keyword || '=',uupcrc,'data.')
if xrc \= 0 then
do
   say 'SysFileSearch error' xrc 'searching' uupcrc 'for' keyword
   if ( answer == '' ) then
      exit
   else
      return answer
end

do count = 1 to data.0
   parse var data.count newkey'='string

   if translate(newkey) = keyword then
      answer = string
end

/*--------------------------------------------------------------------*/
/*    Return the value if found, return directories if we can         */
/*    generate the directory name, otherwise return default input.    */
/*--------------------------------------------------------------------*/

select;
   when ( answer \= '' ) then
      return translate(answer,'\','/')
   when ( keyword = 'CONFDIR' ) then
      return confdir;
   when ( keyword = 'SPOOLDIR' ) then
      return confdir || 'spool';
   when ( keyword = 'NEWSDIR' ) then
      return confdir || 'news';
   when ( keyword = 'ARCHIVEDIR' ) then
      return confdir || 'archive';
   when ( keyword = 'MAILDIR' ) then
      return confdir || 'mail';
   when ( keyword = 'TEMPDIR' ) then
   do;
      tempdir = value('TEMP',,'OS2ENVIRONMENT')
      if tempdir = '' then
         tempdir = value('TMP',,'OS2ENVIRONMENT')
      if tempdir = '' then
         tempdir = confdir || 'tmp';
      return tempdir;
   end;
   otherwise
      return translate(answer,'\','/')
end;
