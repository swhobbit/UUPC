/*--------------------------------------------------------------------*/
/*       Program:    pnews.cmd                                        */
/*       Author:     Snuffles P. Bear                                 */
/*                   Kendra Electronic Wonderworks                    */
/*                   P.O. Box 132                                     */
/*                   Arlington, MA 02174                              */
/*                                                                    */
/*       Function:   Quick and (very) dirty OS/2 posting program      */
/*                   for UUPC/extended                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Note: This program doesn't support prefixed keywords in      */
/*             the UUPC/extended RC file, and further presumes you    */
/*             always want your posts saved for posterity and/or      */
/*             law suits.  In truth, this program was a quickly       */
/*             hacked prototype and no more or less.                  */
/*                                                                    */
/*             Did we mention it also ALWAYS tacks on your .sig       */
/*             file?  Or that it may not work on multiple disk        */
/*             systems?                                               */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-2000 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: pnews.cmd 1.11 1999/01/04 03:56:38 ahd Exp $
 *
 *    $Log: pnews.cmd $
 *    Revision 1.11  1999/01/04 03:56:38  ahd
 *    checkin
 *
 *    Revision 1.10  1998/05/18 12:25:09  ahd
 *    Correct end-of-line character from linefeed to CR/LF
 *
 *      Revision 1.9  1998/03/01  01:32:25  ahd
 *      Annual Copyright Update
 *
 *    Revision 1.8  1997/04/24 01:30:05  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.7  1996/01/01 21:16:47  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.6  1995/12/12 13:52:11  ahd
 *    Delete generation of ANY path from the pnews.cmd; inews will add it
 *
 *    Revision 1.5  1995/01/03 05:36:27  ahd
 *    Allow prompting with a default value
 *    Prompt for distribution
 *
 *    Revision 1.4  1995/01/02 00:32:29  ahd
 *    Handle environment without UUPCUSRRC set (use LOGNAME)
 *
 *    Revision 1.3  1994/12/22 00:29:24  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.2  1994/02/13 04:56:14  ahd
 *    Add RCS log field
 *
 */

signal on novalue
'@echo off'                            /* Disable OS/2 command echo  */

/*--------------------------------------------------------------------*/
/*                    Get environment information                     */
/*--------------------------------------------------------------------*/

uupcusrrc    = value( 'UUPCUSRRC',,'OS2ENVIRONMENT');

if uupcusrrc = '' then
   uupcusrrc    = value( 'LOGNAME',,'OS2ENVIRONMENT') || '.rc'

domain       = getuupc("fromDomain", getuupc("domain"));
nodename     = getuupc("nodename" );
editor       = getuupc("editor",,uupcusrrc);
home         = getuupc("home",,uupcusrrc);
mailbox      = getuupc("mailbox",,uupcusrrc);
name         = getuupc("name",,uupcusrrc);
organization = getuupc("organization","NOT!",uupcusrrc);
signature    = getuupc("signature","NOT!",uupcusrrc);
tempdir      = getuupc("TEMPDIR" )

/*--------------------------------------------------------------------*/
/*                      Create a posting header                       */
/*--------------------------------------------------------------------*/

dfile        = SysTempFileName( tempdir || '\POST????.TXT');

call lineout dfile,'From:' mailbox || '@' || domain '(' || name || ')';

if organization <> "NOT!" then
   call lineout dfile,'Organization:' organization;

call lineout dfile,'Newsgroups:' prompt( 'Newsgroups' );
call lineout dfile,'Subject:' prompt('Subject');

call lineout dfile,'Distribution:' prompt('Distribution','world')
call lineout dfile,'';           /* Terminate the header             */
call lineout dfile;              /* Close the file                   */

/*--------------------------------------------------------------------*/
/*                  Allow the user to edit the text                   */
/*--------------------------------------------------------------------*/

xrc = SysFileTree(dfile , 'old.','F')
if xrc <> 0 then
   exit rc;

parse value editor with editor '%s' rest;
editor dfile rest;

xrc = SysFileTree(dfile , 'new.','F')
if xrc <> 0 then
   exit rc;

if old.1 == new.1 then
do;
   say 'No text entered, post aborted!'
   call sysfiledelete dfile;
   exit 99;
end

/*--------------------------------------------------------------------*/
/*              Save the file whether or not we post it               */
/*--------------------------------------------------------------------*/

savefile = home || '\' || 'USENETPR.SPB'

call lineout savefile,copies('01'x,19)
call lineout savefile,'Date:' left(date('w'),3) || ',' date() time() 'LOCAL';
do while(lines(dfile))
   call lineout savefile,linein(dfile)
end

call lineout savefile

say 'Copy saved in' savefile;

/*--------------------------------------------------------------------*/
/*       Determine if the user really wants to post the article       */
/*--------------------------------------------------------------------*/

do until (post == 'Y') | (post == 'N')
   post = prompt('Really post','y')
   post = left(post,1);
   if post = 'y' then
      post = 'Y'
   if post = 'n' then
      post = 'N';
end

 if post == 'N' then
 do;
   call sysfiledelete dfile;
   exit;
 end;

/*--------------------------------------------------------------------*/
/*                  Append the user's signature file                  */
/*--------------------------------------------------------------------*/

if signature <> 'NOT!' then
do;
   if pos( '\' , signature ) == 0 then
      signature = home || '\' || signature;

   call lineout dfile,'--';         /* Terminate bost of message     */
   do while( lines(signature) )
      call lineout dfile,linein(signature)
   end;

   call lineout dfile;              /* Close the file          */
end;

/*--------------------------------------------------------------------*/
/*                          Execute the post                          */
/*--------------------------------------------------------------------*/

'inews <' dfile;

/*--------------------------------------------------------------------*/
/*                   Delete our work file and exit                    */
/*--------------------------------------------------------------------*/

call sysfiledelete dfile;
exit rc;

/*--------------------------------------------------------------------*/
/*       p r o m p t                                                  */
/*                                                                    */
/*       Prompt for a user line of input.  Loops until user           */
/*       provides input, exits if '.' entered.                        */
/*--------------------------------------------------------------------*/

prompt:procedure
parse arg prompt, default
result = ''
if default <> '' then
   prompt = prompt '(' || default || ')';

do until words( result) > 0
   call charout , prompt || '? '
   parse pull result;
   if ( result = '.') then
      exit;

   if ( result = '' ) then
      result = default;

end;

result = strip(result);
return result;
