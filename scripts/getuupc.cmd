/*--------------------------------------------------------------------*/
/*       g e t u u p c                                                */
/*                                                                    */
/*       Get UUPC/extended configuration variable                     */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Copyright (c) 1992-1993 by Kendra Electronic Wonderworks, all   */
/*    rights reserved except those granted by the UUPC/extended       */
/*    license.                                                        */
/*--------------------------------------------------------------------*/

/*
 *       $Id$
 *
 *       $Log$
 */

parse upper arg keyword,answer

/*--------------------------------------------------------------------*/
/*                     Get the UUPC.RC file name                      */
/*--------------------------------------------------------------------*/
uupcrc = value('UUPCSYSRC',,'OS2ENVIRONMENT')
if  uupcrc == '' then
do
   'UUPCSYSRC not set, cannot continue'
   exit 44
end

confdir = translate(filespec('D',uupcrc) || filespec('P',uupcrc),'\','/');

/*--------------------------------------------------------------------*/
/*     Read the file, then scope the contents a line at the time      */
/*--------------------------------------------------------------------*/

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
   otherwise
      return translate(answer,'\','/')
end;
