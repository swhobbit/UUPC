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
/*       Copyright 1989-1994 By Kendra Electronic Wonderworks;        */
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
 *       $Id: waiting.cmd 1.7 1994/02/13 04:56:14 ahd Exp $
 *
 *       $Log: waiting.cmd $
 *       Revision 1.7  1994/02/13  04:56:14  ahd
 *       Sort mailboxes by name
 *
 *Revision 1.6  1993/04/04  05:01:49  ahd
 *Use common getuupc.cmd for variable retrieval
 *
*     Revision 1.5  1993/01/23  19:15:47  ahd
*     Load required subroutine packages before using them
*
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
   return;
end;
/*--------------------------------------------------------------------*/
/*                 Do a simple exchange sort by name                  */
/*--------------------------------------------------------------------*/
do i = 1 to data.0 - 1;
   parse value space(data.i) with . . . . fname;
   do j = i + 1 to data.0;
      parse value space(data.j) with . . . . newname;
      if newname < fname then
      do;
         save = data.j
         fname = newname;
         data.j = data.i
         data.i = save;
      end;
   end j
end i;

do i = 1 to data.0
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
/*    n o v a l u e                                                   */
/*                                                                    */
/*    Trap for uninitialized variables                                */
/*--------------------------------------------------------------------*/

novalue:
trace n
signal off novalue;           /* Avoid nasty recursion         */
say 'Uninitialized variable in line' sigl || ':';
say sourceline( sigl );
