/*--------------------------------------------------------------------*/
/*          Program:    waiting.cmd          9 Nov 1992               */
/*          Author:     Andrew H. Derbyshire                          */
/*          Address:    Kendra Electronic Wonderworks                 */
/*                      P.O. Box 132                                  */
/*                      Arlington, MA 02174                           */
/*          Internet:   help@kew.com                                  */
/*          Language:   OS/2 2.0 REXX                                 */
/*          Function:   Report mail waiting for users                 */
/*        Parameters:   None                                          */
/*       Environment:   Assumes OS/2 Environment variable             */
/*                      UUPCSYSRC has been set to name of UUPC/       */
/*                      extended system configuration file, and       */
/*                      TEMP variable been set if not defined         */
/*                      in UUPC/extended.                             */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Copyright 1990-1992 By Kendra Electronic Wonderworks;        */
/*       may be distributed freely if original documentation and      */
/*       source are included, and credit is given to the authors.     */
/*       For additional instructions, see README.PRN in UUPC/         */
/*       extended documentation archive.                              */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Based on original DOS version by various people, and         */
/*       MAILCHEK.CMD, by Evan Champion <evanc@uuisis.isis.org>       */
/*--------------------------------------------------------------------*/

/*
 *       $Id: WAITING.CMD 1.4 1993/01/01 16:47:07 ahd Exp $
 *
 *       $Log: WAITING.CMD $
*     Revision 1.4  1993/01/01  16:47:07  ahd
*     Ignore totally blank arguments
*
*     Revision 1.3  1993/01/01  16:44:04  ahd
*     Correct seven digit date bug
*
 * Revision 1.2  1992/11/28  23:08:07  ahd
 * Tweak order of procedures, add comments
 *
 */

'@echo off'
signal on novalue
Call RxFuncAdd 'SysLoadFuncs','RexxUtil','SysLoadFuncs'
Call 'SysLoadFuncs'
parse upper arg who;
if words(who) = 0 then
   who = '*';
maildir = getuupc("maildir" );
mailext = getuupc("mailext" );

return = SysFileTree(maildir || '\' || who || '.' || mailext, 'data.','F')
if return = 2 then
   say 'Not enough memory';
if data.0 = 0 then
do;
   if who = '*' then
      say 'No mail waiting in' maildir;
   else
      say 'No mail waiting for' who;
end;
else  do i = 1 to data.0
   parse value space(data.i) with mmddyy hhmmss bytes attr fname;
   if bytes > 0 then
   do
      if length(mmddyy) == 7 then
         mmddyy = '0' || mmddyy
      parse value filespec( "name", fname ) with id'.';
      if mmddyy = date('U') then
         when = hhmmss
      else
         when = substr(mmddyy,1,5) hhmmss;
      items = CountItems( space(fname) );
      say 'Mail waiting for' id 'since' when '(' || items 'items,' ,
               bytes 'bytes).'
   end
end i /* do */
exit;

/*--------------------------------------------------------------------*/
/*    C o u n t I t e m s                                             */
/*                                                                    */
/*    Determine number of items in a mailbox                          */
/*--------------------------------------------------------------------*/

CountItems:procedure
parse arg mailbox
sep = copies('01'x,19)
xrc = SysFileSearch( sep ,mailbox,'data.');
if xrc <> 0 then
do;
   say 'Internal error' xrc || ':' ,
         mailbox 'has no UUPC/extended message breaks'
   return 0;
end;
else
   return data.0;

/*--------------------------------------------------------------------*/
/*       g e t u u p c                                                */
/*                                                                    */
/*       Get UUPC/extended configuration variable                     */
/*--------------------------------------------------------------------*/

getuupc:procedure;
trace n
parse upper arg keyword,answer;

uupcrc = value('UUPCSYSRC',,'OS2ENVIRONMENT');
if  uupcrc == '' then
do;
   'UUPCSYSRC not set, cannot continue'
   exit 44;
end;

xrc = SysFileSearch( keyword || '=',uupcrc,'data.');
if xrc \= 0 then
do;
   say 'SysFileSearch error' xrc 'searching' uupcrc 'for' keyword;
   exit xrc;
end;

do count = 1 to data.0
   parse var data.count newkey'='string;

   if translate(newkey) = keyword then
      answer = string;
end;
return translate(answer,'\','/');

/*--------------------------------------------------------------------*/
/*    n o v a l u e                                                   */
/*                                                                    */
/*    Trap for uninitialized variables                                */
/*--------------------------------------------------------------------*/

novalue:
trace n
signal off novalue;           /* Avoid nasty recursion         */
say 'Uninitialized variable in line' sigl || ':';
say sourceline( sigl );
