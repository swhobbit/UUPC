@echo off
echo off
REM *------------------------------------------------------------------*
REM *   Program:        install.bat             9 May 1993             *
REM *   Author:         Drew Derbyshire                                *
REM *   Address:        P.O. Box 132                                   *
REM *                   Arlington, MA 02174                            *
REM *   Function:       Install UUPC/extended files into               *
REM *                   reasonable default directories.                *
REM *------------------------------------------------------------------*
REM *   Note.  You may need to edit the PKZIP commands below           *
REM *   if using another program to unpack the archives.               *
REM *------------------------------------------------------------------*

if "%2" == "" goto usage
echo %0 will install UUPC/extended %1 for DOS into directory %2
echo Press ENTER to continue, Ctrl-C to abort.
pause
echo Off we go ...

echo Building directories under %2
if not exist %2\systems mkdir %2
if not exist %2\bin\uucico.exe mkdir %2\bin
mkdir %2\public
mkdir %2\samples
mkdir %2\docs
echo Unpacking executable files ...
pkunzip uupc%1r %2\bin *.*
pkunzip uupc%1o %2\bin *.*
pkunzip uupc%1n %2\bin *.*
pkunzip uupc%1d %2\docs\*.prn
echo unpacking documents
if exist %1\permissn goto done
echo unpacking Sample files
pkunzip uupc%1d %2\samples\systems
pkunzip uupc%1d %2\samples\uupc.rc
pkunzip uupc%1d %2\samples\personal.rc
pkunzip uupc%1d %2\samples\personal.sig
pkunzip uupc%1d %2\samples\hostpath
pkunzip uupc%1d %2\samples\permissn
pkunzip uupc%1d %2\samples\*.mdm
echo All done.  Now, copy the files from the %2\samples directory to
echo %2 customize as needed.
if "%UUPCSYSRC%" == "" echo  You also need update AUTOEXEC.BAT.
