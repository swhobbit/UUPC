@echo off
REM *------------------------------------------------------------------*
REM *   Program:        su.bat          03 Nov 1990                    *
REM *   Author:         Andrew H. Derbyshire                           *
REM *   Address:        help@kendra.kew.com                            *
REM *   Function:       Demonstration of multiple user support         *
REM *                   with UUPC/extended                             *
REM *------------------------------------------------------------------*
REM *  This command file provides a simple example of changing the *
REM *  active user id (the user id used when sending mail) in      *
REM *  UUPC/extended.  It assumes:                                 *
REM *                                                              *
REM *  1) The configuration directory is C:\LIB\UUPC,              *
REM *                                                              *
REM *  2) That each user has a PERSONAL.RC file under under his    *
REM *     under own name in the configuration directory.           *
REM *                                                              *
REM *  3) The MS-DOS environment has enough space free to save an  *
REM *     extra copy of the DOS prompt and the current UUPCUSRC    *
REM *     variable.                                                *
REM *                                                              *
REM *  4) A copy of COMMAND.COM can be invoked via the COMSPEC     *
REM *     variable.                                                *
REM *--------------------------------------------------------------*
REM *       If the user didn't invoke us properly, give help       *
REM *--------------------------------------------------------------*
if not "%1" == ""   goto ok
echo Syntax:   %0 userid command
echo The command is optional; if not specified, a sub-shell will be
echo invoked.  An example:
echo           %0 postmast mail
goto quit
REM *--------------------------------------------------------------*
REM *                Save the current environment                  *
REM *--------------------------------------------------------------*
:ok
SET UUPCPRMPT=%prompt%
SET UUPCUSRRX=%UUPCUSRRC%
REM *------------------------------------------------------------------*
REM *   Change the following line if your UUPC PERSONAL.RC files       *
REM *                    aren't in C:\uupc                         *
REM *------------------------------------------------------------------*
SET UUPCUSRRC=e:\uupc\%1.rc
REM *------------------------------------------------------------------*
REM *                    Verify the file exists                        *
REM *------------------------------------------------------------------*
if not exist %UUPCUSRRC% goto error
PROMPT Enter EXIT to logout from %1$_%UUPCPRMPT%
REM *------------------------------------------------------------------*
REM *   Make the text window bigger (use a neat program that comes     *
REM *   with KEDIT from Mansfield Software.)                           *
REM *------------------------------------------------------------------*
if exist e:\kedit\textwin.exe textwin maximize
REM *------------------------------------------------------------------*
REM *            Run a sub-shell with the new variables                *
REM *------------------------------------------------------------------*
if "%2" == "" goto shell
%2 %3 %4 %5 %6 %7 %8 %9
goto exit
:shell
%comspec%
goto exit
REM *------------------------------------------------------------------*
REM *             Come here to issue an error message                  *
REM *------------------------------------------------------------------*
:error
echo %UUPCUSRRC% doesn't exist!  Please verify %1 is a valid user.
REM *------------------------------------------------------------------*
REM *                   Reset variables and exit                       *
REM *------------------------------------------------------------------*
:exit
PROMPT %UUPCPRMPT%
SET UUPCUSRRC=%UUPCUSRRX%
SET UUPCPRMPT=
SET UUPCUSRRX=
:quit
