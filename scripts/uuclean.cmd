echo off
REM *------------------------------------------------------------------*
REM *    Written and contributed by Michael Morrett                    *
REM *    (ICBJ100@INDYVAX.IUPUI.EDU).  Support for by Snuffles P.      *
REM *    Bear (Snuffles@kew.com).  Comments by Drew Derbyshire         *
REM *    (help@kew.com); delete the comments to make this run faster.  *
REM *------------------------------------------------------------------*
REM *   Define your spool and temp directories on following line       *
REM *------------------------------------------------------------------*
if "%1" == "" %0 \uupc\spool %temp%
if "%2" == "" %0 %1 %temp%
REM *------------------------------------------------------------------*
REM *        Handle recursive call to process one directory            *
REM *------------------------------------------------------------------*
if "%1" == "cleanit" goto cleanit:
echo Cleaning UUPC/extended directories %1 and %2
REM *------------------------------------------------------------------*
REM *    Cruel hack for kendra's automatic sig file generation;        *
REM *                other systems will ignore this                    *
REM *------------------------------------------------------------------*
if exist \os2bin\newsigs.cmd call newsigs
REM *------------------------------------------------------------------*
REM *     Temporary files created by UUCICO in spool directory         *
REM *------------------------------------------------------------------*
if exist %1\*.$$$ erase %1\*.$$$
if exist %1\*.tmp erase %1\*.tmp
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
call %0 cleanit %1\generic
REM *------------------------------------------------------------------*
REM *            Archive and delete temporary log files                *
REM *------------------------------------------------------------------*
if not exist %1\*.log goto noneleft
copy   %1\*.log %1\generic.lgs
erase  %1\*.log
rename %1\generic.lgs generic.log
:noneleft
REM *------------------------------------------------------------------*
REM *                      Clean up old news                           *
REM *------------------------------------------------------------------*
expire
endlocal
REM *------------------------------------------------------------------*
REM *         Purge any files we killed from the undelete bucket.      *
REM *         This applies to OS/2 2.0 only.                           *
REM *------------------------------------------------------------------*
undelete %1 /a /s /f
undelete %2 /a /s /f
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
REM *------------------------------------------------------------------*
REM *                        All done, exit                            *
REM *------------------------------------------------------------------*
:exit
