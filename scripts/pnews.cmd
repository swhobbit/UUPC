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
/*       Changes Copyright (c) 1989-1994 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: pnews.cmd 1.1 1994/01/18 13:31:15 ahd Exp ahd $
 *
 *    $Log$
 */

signal on novalue
'@echo off'                            /* Disable OS/2 command echo  */

/*--------------------------------------------------------------------*/
/*                    Get environment information                     */
/*--------------------------------------------------------------------*/

uupcusrrc    = value( 'UUPCUSRRC',,'OS2ENVIRONMENT');

domain       = getuupc("fromDomain", getuupc("domain"));
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

call lineout dfile,'Path:' domain || '!' || mailbox;
call lineout dfile,'From:' mailbox || '@' || domain '(' || name || ')';

if organization <> "NOT!" then
   call lineout dfile,'Organization:' organization;

call lineout dfile,'Newsgroups:' prompt( 'Newsgroups' );
call lineout dfile,'Subject:' prompt('Subject');

call lineout dfile,'Distribution: world'
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
   post = prompt('Really post')
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
parse arg prompt
result = ''
do until words( result) > 0
   call charout ,prompt || '? '
   parse pull result;
   if ( result = '.') then
      exit;
end;

result = strip(result);
return result;
