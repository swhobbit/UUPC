/*--------------------------------------------------------------------*/
/*    Program:    mailcheck         (92-10-12)                        */
/*    Author:     Evan Champion                                       */
/*    Address:    evanc@uuisis.isis.org                               */
/*    Language:   Procedure Langauges/2 (OS/2 REXX) with              */
/*                Visual REXX                                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Requires Visual Rexx, available from IBM's OS/2 Internet        */
/*    archive or CompuServ                                            */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*   Change history                                                   */
/*   09 Nov 92 ahd   Use UUPCSYSRC to locate the mail directory and   */
/*                   mailbox extension                                */
/*--------------------------------------------------------------------*/

/* setup VREXX */
signal on novalue
call RxFuncAdd 'VInit', 'VREXX', 'VINIT'
initcode = VInit()

if initcode = 'ERROR' then call CLEANUP

signal on failure name CLEANUP
signal on halt name CLEANUP
signal on syntax name CLEANUP

/* say product info to the fullscreen */

say 'MailCheck [92-10-12]'
say 'by Evan Champion (evanc@uuisis.isis.org)'
say ' '
say 'Enter Control-C to close'

/* load functions in RexxUtil */

call RxFuncAdd 'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
call SysLoadFuncs

y = 300 /* x and y values for printing text */
x = 10  /* in the PM window */
cycle = 30;                   /* Check for mail every ...      */

maildir = getuupc("maildir" );
mailext = getuupc("mailext" );

empty  = 'WHITE' /* window background colour */
full   = 'BLACK' /* window background colour */
colour = empty;
pos.left= 70 /* window positions, in percent of desktop */
pos.bottom = 0
pos.right = 100
pos.top = 3

title = 'Mailcheck [92-11-10]' /* title w/version number */

mailcheckID = VOpenWindow(title, colour, 'pos')
info.   = '';                 /* Flag we have no mailbox info  */

do forever /* start checking */
   return = SysFileTree(maildir || '\*.' || mailext, 'found.', 'F')
   if return = 2 then
      call CLEANUP            /* if there isn't enough memory */
   if found.0 = 0 then
   do;
      if colour = full then
      do;
         call vforecolor mailcheckID,full;
         call vbackcolor mailcheckID,empty;
         colour = empty;
      end;
      call SAYINFO            /* if there are no files */
      call SysSleep Cycle;
      drop counts.
      info. = '';
   end;
   else do;
      pause = cycle % found.0
      if pause < 2 then
         pause = 2;
      if colour = empty then
      do;
         call vforecolor mailcheckID,empty;
         call vbackcolor mailcheckID,full;
         colour = full
      end;
      do i = 1 to found.0
           parse var found.i mmddyy hhmmss bytes attr fname;
           parse value filespec( "name", fname ) with id'.';
           if mmddyy = date('U') then
              when = hhmmss
           else
              when = substr(mmddyy,1,5) hhmmss;
           if info.id <> found.i then
           do;
              count.id = CountItems( strip(fname) );
                              /* Only read file when truly needed */
              info.id = found.i
                              /* Remember that we read the file   */
           end;
           call SAYINFO id, when, bytes, count.id ;
           call SysSleep pause;
      end i /* do */
   end;
end /* do */

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

SAYINFO:
parse arg xwho, xwhen, xbytes, xitems
   if xwho = '' then
      output = 'No new mail' /* if there was no mail */
   else
      output = xwho || '  ' || xwhen '(' || xitems 'items,' ,
               xbytes || ' bytes)' ;
   call VClearWindow mailcheckID /* scroll through ID's with mail */
   call VSay mailcheckID, x, y, output
   return

CLEANUP:
   call VExit /* cleanup and exit */
   exit

/*--------------------------------------------------------------------*/
/*       C o u n t I t e m s                                          */
/*                                                                    */
/*       Count number of items in a mailbox.                          */
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
signal Cleanup
