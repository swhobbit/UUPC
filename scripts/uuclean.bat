echo off
REM *------------------------------------------------------------------*
REM *    Written and contributed by Michael Morrett                    *
REM *    (ICBJ100@INDYVAX.IUPUI.EDU).  Support for by Snuffles P.      *
REM *    Bear (Snuffles@kew.com).  Comments by Drew Derbyshire         *
REM *    (help@kew.com); delete the comments to make this run faster.  *
REM *------------------------------------------------------------------*
rem     $Id: UUCLEAN.BAT 1.2 1992/12/01 04:39:48 ahd Exp $
rem
rem     $Log$
rem
REM *------------------------------------------------------------------*
REM *   Define your spool and temp directories on following lines      *
REM *------------------------------------------------------------------*
if "%1" == "" %0 \uupc\spool %temp% %tmp% \uupc\tmp
if "%2" == "" %0 %1 %temp% %tmp% \uupc\tmp
if "%1" == "cleanit" goto cleanit:
echo Cleaning UUPC/extended directories %1 and %2
REM *------------------------------------------------------------------*
REM *     Temporary files created by UUCICO in spool directory         *
REM *------------------------------------------------------------------*
if exist %1\*.$$$ erase %1\*.$$$
REM *------------------------------------------------------------------*
REM * Temporary files created by MAIL and RMAIL.  (and their           *
REM * backups created by many editors)                                 *
REM *------------------------------------------------------------------*
if exist %2\UUPC*.tmp erase %2\UUPC*.tmp
if exist %2\UUPC*.txt erase %2\UUPC*.txt
if exist %2\UUPC*.bak erase %2\UUPC*.bak
REM *------------------------------------------------------------------*
REM *                 Clean the various log files                      *
REM *------------------------------------------------------------------*
call %0 cleanit %1\linedata
call %0 cleanit %1\rmail
call %0 cleanit %1\rnews
call %0 cleanit %1\uuxqt
call %0 cleanit %1\uucico
call %0 cleanit %1\expire
if exist %1\syslog     rename %1\syslog syslog.log
call %0 cleanit %1\syslog
REM *------------------------------------------------------------------*
REM *                      Clean up old news                           *
REM *------------------------------------------------------------------*
if exist %1\..\active expire
goto exit
REM *------------------------------------------------------------------*
REM *                  Clean up a single log file                      *
REM *------------------------------------------------------------------*
:cleanit
if not exist %2.log goto :exit
echo Cleaning %2.log files.
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
