# Microsoft Developer Studio Project File - Name="uupclib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=uupclib - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "uupclib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "uupclib.mak" CFG="uupclib - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "uupclib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "uupclib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "uupclib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "uupclib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\Debug"
# PROP Intermediate_Dir "..\Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MDd /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WIN32" /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "uupclib - Win32 Release"
# Name "uupclib - Win32 Debug"
# Begin Group "Source"

# PROP Default_Filter "*.c"
# Begin Source File

SOURCE=.\abspath.c
# End Source File
# Begin Source File

SOURCE=.\arbmath.c
# End Source File
# Begin Source File

SOURCE=.\arpadate.c
# End Source File
# Begin Source File

SOURCE=.\bugout.c
# End Source File
# Begin Source File

SOURCE=.\catcher.c
# End Source File
# Begin Source File

SOURCE=.\chdir.c
# End Source File
# Begin Source File

SOURCE=.\checkptr.c
# End Source File
# Begin Source File

SOURCE=.\configur.c
# End Source File
# Begin Source File

SOURCE=.\creat.c
# End Source File
# Begin Source File

SOURCE=.\dater.c
# End Source File
# Begin Source File

SOURCE=.\dos2unix.c
# End Source File
# Begin Source File

SOURCE=.\execute.c
# End Source File
# Begin Source File

SOURCE=.\expath.c
# End Source File
# Begin Source File

SOURCE=.\export.c
# End Source File
# Begin Source File

SOURCE=.\filebkup.c
# End Source File
# Begin Source File

SOURCE=.\fopen.c
# End Source File
# Begin Source File

SOURCE=.\getargs.c
# End Source File
# Begin Source File

SOURCE=.\getdrive.c
# End Source File
# Begin Source File

SOURCE=.\getopt.c
# End Source File
# Begin Source File

SOURCE=.\getseq.c
# End Source File
# Begin Source File

SOURCE=.\hostable.c
# End Source File
# Begin Source File

SOURCE=.\hostatus.c
# End Source File
# Begin Source File

SOURCE=.\hostrset.c
# End Source File
# Begin Source File

SOURCE=.\imfile.c
# End Source File
# Begin Source File

SOURCE=.\import.c
# End Source File
# Begin Source File

SOURCE=.\kanjicnv.c
# End Source File
# Begin Source File

SOURCE=.\localfnm.c
# End Source File
# Begin Source File

SOURCE=.\lock.c
# End Source File
# Begin Source File

SOURCE=.\logger.c
# End Source File
# Begin Source File

SOURCE=.\makebuf.c
# End Source File
# Begin Source File

SOURCE=.\mkdir.c
# End Source File
# Begin Source File

SOURCE=.\mkfilenm.c
# End Source File
# Begin Source File

SOURCE=.\mkmbox.c
# End Source File
# Begin Source File

SOURCE=.\mktempnm.c
# End Source File
# Begin Source File

SOURCE=.\ndirnt.c
# End Source File
# Begin Source File

SOURCE=.\normaliz.c
# End Source File
# Begin Source File

SOURCE=.\pnterr.c
# End Source File
# Begin Source File

SOURCE=.\printerr.c
# End Source File
# Begin Source File

SOURCE=.\printmsg.c
# End Source File
# Begin Source File

SOURCE=.\pushpop.c
# End Source File
# Begin Source File

SOURCE=.\readnext.c
# End Source File
# Begin Source File

SOURCE=.\rename.c
# End Source File
# Begin Source File

SOURCE=.\safeio.c
# End Source File
# Begin Source File

SOURCE=.\safeout.c
# End Source File
# Begin Source File

SOURCE=.\scrsiznt.c
# End Source File
# Begin Source File

SOURCE=.\security.c
# End Source File
# Begin Source File

SOURCE=.\setstdin.c
# End Source File
# Begin Source File

SOURCE=.\ssleep.c
# End Source File
# Begin Source File

SOURCE=.\stater.c
# End Source File
# Begin Source File

SOURCE=.\strpool.c
# End Source File
# Begin Source File

SOURCE=.\timestmp.c
# End Source File
# Begin Source File

SOURCE=.\titlen.c
# End Source File
# Begin Source File

SOURCE=.\trumpet.c
# End Source File
# Begin Source File

SOURCE=.\usertabl.c
# End Source File
# Begin Source File

SOURCE=.\usrcatch.c
# End Source File
# Begin Source File

SOURCE=.\validcmd.c
# End Source File
# End Group
# Begin Group "Include"

# PROP Default_Filter "*.h"
# Begin Source File

SOURCE=.\arbmath.h
# End Source File
# Begin Source File

SOURCE=.\arpadate.h
# End Source File
# Begin Source File

SOURCE=.\catcher.h
# End Source File
# Begin Source File

SOURCE=.\confvars.h
# End Source File
# Begin Source File

SOURCE=.\dater.h
# End Source File
# Begin Source File

SOURCE=.\dos2unix.h
# End Source File
# Begin Source File

SOURCE=.\execute.h
# End Source File
# Begin Source File

SOURCE=.\expath.h
# End Source File
# Begin Source File

SOURCE=.\export.h
# End Source File
# Begin Source File

SOURCE=.\getopt.h
# End Source File
# Begin Source File

SOURCE=.\getseq.h
# End Source File
# Begin Source File

SOURCE=.\hlib.h
# End Source File
# Begin Source File

SOURCE=.\hostable.h
# End Source File
# Begin Source File

SOURCE=.\hostatus.h
# End Source File
# Begin Source File

SOURCE=.\hostrset.h
# End Source File
# Begin Source File

SOURCE=.\imfile.h
# End Source File
# Begin Source File

SOURCE=.\import.h
# End Source File
# Begin Source File

SOURCE=.\kanjicnv.h
# End Source File
# Begin Source File

SOURCE=.\lib.h
# End Source File
# Begin Source File

SOURCE=.\lock.h
# End Source File
# Begin Source File

SOURCE=.\logger.h
# End Source File
# Begin Source File

SOURCE=.\makebuf.h
# End Source File
# Begin Source File

SOURCE=.\pnterr.h
# End Source File
# Begin Source File

SOURCE=.\pos2err.h
# End Source File
# Begin Source File

SOURCE=.\pushpop.h
# End Source File
# Begin Source File

SOURCE=.\readnext.h
# End Source File
# Begin Source File

SOURCE=.\safeio.h
# End Source File
# Begin Source File

SOURCE=.\scrsize.h
# End Source File
# Begin Source File

SOURCE=.\security.h
# End Source File
# Begin Source File

SOURCE=.\setstdin.h
# End Source File
# Begin Source File

SOURCE=.\ssleep.h
# End Source File
# Begin Source File

SOURCE=.\stater.h
# End Source File
# Begin Source File

SOURCE=.\timestmp.h
# End Source File
# Begin Source File

SOURCE=.\title.h
# End Source File
# Begin Source File

SOURCE=.\trumpet.h
# End Source File
# Begin Source File

SOURCE=.\usertabl.h
# End Source File
# Begin Source File

SOURCE=.\usrcatch.h
# End Source File
# Begin Source File

SOURCE=.\uundir.h
# End Source File
# Begin Source File

SOURCE=.\uupcmoah.h
# End Source File
# Begin Source File

SOURCE=.\uutypes.h
# End Source File
# Begin Source File

SOURCE=.\win32ver.h
# End Source File
# End Group
# End Target
# End Project
