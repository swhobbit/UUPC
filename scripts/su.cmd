/*--------------------------------------------------------------------*/
/*          Program:    su.cmd            3 May 1993                  */
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
/*       Copyright 1989-1998 By Kendra Electronic Wonderworks;        */
/*       may be distributed freely if original documentation and      */
/*       source are included, and credit is given to the authors.     */
/*       For additional instructions, see README.PRN in UUPC/         */
/*       extended documentation archive.                              */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       REXX function to change effective user id and optionally     */
/*       run a command as that user.                                  */
/*--------------------------------------------------------------------*/

/*
 *       $Id: su.cmd 1.11 1997/04/24 01:30:11 ahd v1-12u $
 *
 *       $Log: su.cmd $
 *       Revision 1.11  1997/04/24 01:30:11  ahd
 *       Annual Copyright Update
 *
 *       Revision 1.10  1996/01/01 21:16:32  ahd
 *       Annual Copyright Update
 *
 *       Revision 1.9  1995/01/02 00:32:29  ahd
 *       Handle environment without UUPCUSRRC set (use LOGNAME)
 *
 *       Revision 1.8  1994/12/27 23:37:01  ahd
 *       Add contributed fix to allow running rexx scripts within SU.CMD
 *
 *       Revision 1.7  1994/12/22 00:29:29  ahd
 *       Annual Copyright Update
 *
 *       Revision 1.6  1994/05/23 23:16:48  ahd
 *       Correct copyright
 *       Normalize to only one trailing backslash in name
 *
 *       Revision 1.5  1994/05/04  02:05:03  ahd
 *       Also set LOGNAME for various OS/2 programs
 *
 *Revision 1.4  1994/01/01  19:16:19  ahd
 *Annual Copyright Update
 *
 *      Revision 1.3  1993/05/09  03:51:26  ahd
 *      Don't uppercase the input
 *
 *      Revision 1.2  1993/05/04  00:25:58  ahd
 *      COnvert to REXX
 *
 *
 */

'@echo off'
signal on novalue
parse source stuff
Call RxFuncAdd 'SysLoadFuncs','RexxUtil','SysLoadFuncs'
Call 'SysLoadFuncs'
parse arg who what;
if words(who) = 0 then
do;
   say 'SU.CMD Copyright 1989-1998 Kendra Electronic Wonderworks';
   say 'No user specified to SU to';
   exit 1;
end;
who = strip(who);

confdir = getuupc("CONFDIR")
if confdir == '' then
do
   say 'No configuration directory defined, cannot continue'
   exit 98
end
/*--------------------------------------------------------------------*/
/*                  Don't double the last backslash                   */
/*--------------------------------------------------------------------*/
if  right( confDir , 1 ) == '\' then
   confDir = left(confDir, length( confdir ) - 1);

uupcusrc = confDir || '\' || who || '.RC'
who      = getuupc("MAILBOX",who,uupcusrc)

if \ exist( uupcusrc ) then
do;
   say uupcusrc 'does not exist!  Please verify' who 'is a valid user'
   exit 97;
end;

call SetLocal;
uupcusrc = value( 'UUPCUSRRC',uupcusrc,'OS2ENVIRONMENT');
logname = value( 'LOGNAME',who,'OS2ENVIRONMENT');

if words(what) == 0 then
do;
   prompt = 'Enter EXIT to logout from' who || '$_' || ,
            value( 'PROMPT',,'OS2ENVIRONMENT');       /* Make new prompt */
   prompt = value( 'PROMPT',prompt,'OS2ENVIRONMENT'); /* Set new prompt  */
   value( 'COMSPEC',,'OS2ENVIRONMENT');
   prompt = value( 'PROMPT',prompt,'OS2ENVIRONMENT'); /* Restore prompt   */
end;
else
   value( 'COMSPEC',, 'OS2ENVIRONMENT') "/C" what

uupcusrc = value( 'UUPCUSRRC',uupcusrc,'OS2ENVIRONMENT');
                                 /* Restore original value           */
logname = value( 'LOGNAME',logname,'OS2ENVIRONMENT');
call endlocal;
exit rc;

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
/*    n o v a l u e                                                   */
/*                                                                    */
/*    Trap for uninitialized variables                                */
/*--------------------------------------------------------------------*/

novalue:
trace n
signal off novalue;           /* Avoid nasty recursion         */
say 'Uninitialized variable in line' sigl || ':';
say sourceline( sigl );
