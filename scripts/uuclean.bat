echo off
if "%1" == "cleanit" goto cleanit:
REM *------------------------------------------------------------------*
REM *       u u c l e a n . b a t                                      *
REM *                                                                  *
REM *       Batch script to clean UUPC/extended log files              *
REM *------------------------------------------------------------------*

REM *------------------------------------------------------------------*
REM *    Written by Michael Morrett (ICBJ100@INDYVAX.IUPUI.EDU).       *
REM *------------------------------------------------------------------*

REM *------------------------------------------------------------------*
REM *    Changes Copyright (c) 1989-1999 by Kendra Electronic          *
REM *    Wonderworks.                                                  *
REM *                                                                  *
REM *    All rights reserved except those explicitly granted by        *
REM *    the UUPC/extended license agreement.                          *
REM *------------------------------------------------------------------*

rem     $Id: uuclean.bat 1.7 1999/01/04 03:57:12 ahd Exp $
rem
rem     $Log: uuclean.bat $
rem     Revision 1.7  1999/01/04 03:57:12  ahd
rem     Annual copyright change
rem
rem     Revision 1.6  1998/05/11 01:27:30  ahd
rem     Add distrinct log directory
rem
rem     Revision 1.5  1998/03/06 06:53:18  ahd
rem     Add new daemons to cleanup list
rem
rem     Revision 1.4  1995/09/24 19:12:04  ahd
rem     *** empty log message ***
rem
rem     Revision 1.3  1994/04/27 00:07:33  ahd
rem     Prevent endless recursion if TEMP and TMP are not set

REM *------------------------------------------------------------------*
REM *    Define your log, spool and temp directories on following      *
REM *    lines                                                         *
REM *------------------------------------------------------------------*

if "%1" == "" %0 \uupc\log \uupc\spool %temp% %tmp% \uupc\tmp
if "%2" == "" %0 %1 \uupc\spool %temp% %tmp% \uupc\tmp
if "%3" == "" %0 %1 %2 %temp% %tmp% \uupc\tmp
echo Cleaning UUPC/extended directories %1, %2, and %3
REM *------------------------------------------------------------------*
REM *     Temporary files created by UUCICO in spool directory         *
REM *------------------------------------------------------------------*
if exist %2\*.$$$ erase %2\*.$$$
REM *------------------------------------------------------------------*
REM * Temporary files created by MAIL and RMAIL.  (and their           *
REM * backups created by many editors)                                 *
REM *------------------------------------------------------------------*
if exist %3\UUPC*.tmp erase %3\UUPC*.tmp
if exist %3\UUPC*.txt erase %3\UUPC*.txt
if exist %3\UUPC*.bak erase %3\UUPC*.bak
REM *------------------------------------------------------------------*
REM *                 Clean the various log files                      *
REM *------------------------------------------------------------------*
call %0 cleanit %1\genhist
call %0 cleanit %1\linedata
call %0 cleanit %1\newsrun
call %0 cleanit %1\rmail
call %0 cleanit %1\rnews
call %0 cleanit %1\sendbats
call %0 cleanit %1\uucico
call %0 cleanit %1\uupopd
call %0 cleanit %1\uusmtpd
call %0 cleanit %1\uuxqt
if exist %1\syslog     rename %1\syslog syslog.log
call %0 cleanit %1\syslog
REM *------------------------------------------------------------------*
REM *                      Clean up old news                           *
REM *------------------------------------------------------------------*
if exist %1\..\news\history.pag expire
goto exit
REM *------------------------------------------------------------------*
REM *                  Clean up a single log file                      *
REM *------------------------------------------------------------------*
:cleanit
if exist %2 rename %2 %2.log
if not exist %2.log goto :exit
if exist %2.004    del %2.004
if exist %2.003 rename %2.003 *.004
if exist %2.002 rename %2.002 *.003
if exist %2.001 rename %2.001 *.002
if exist %2.log rename %2.log *.001
goto exit
REM *------------------------------------------------------------------*
REM *                        All done, exit                            *
REM *------------------------------------------------------------------*
:exit
