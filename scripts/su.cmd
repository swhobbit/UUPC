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
/*       Copyright 1990-1992 By Kendra Electronic Wonderworks;        */
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
 *       $Id: SU.CMD 1.4 1994/01/01 19:16:19 ahd Exp $
 *
 *       $Log: SU.CMD $
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
   say 'SU.CMD Copyright 1989-1994 Kendra Electronic Wonderworks';
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
   what;

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
