# Microsoft Developer Studio Project File - Name="uupopd" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=uupopd - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "uupopd.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "uupopd.mak" CFG="uupopd - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "uupopd - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "uupopd - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "uupopd - Win32 Release"

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
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "uupopd - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\Debug"
# PROP Intermediate_Dir "..\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /I "..\lib" /D "_WIN32" /D "_DEBUG" /D "WIN32" /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ADVAPI32.LIB WSOCK32.LIB tapi32.lib user32.lib gdi32.lib kernel32.lib OLDNAMES.LIB winmm.lib lib.lib /nologo /incremental:no /debug /machine:I386 /pdbtype:sept /libpath:"..\Debug"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "uupopd - Win32 Release"
# Name "uupopd - Win32 Debug"
# Begin Group "source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\mailnewm.c
# End Source File
# Begin Source File

SOURCE=.\memstr.c
# End Source File
# Begin Source File

SOURCE=.\pop3cmds.c
# End Source File
# Begin Source File

SOURCE=.\pop3lwc.c
# End Source File
# Begin Source File

SOURCE=.\pop3mbox.c
# End Source File
# Begin Source File

SOURCE=.\pop3user.c
# End Source File
# Begin Source File

SOURCE=.\smtpclnt.c
# End Source File
# Begin Source File

SOURCE=.\smtpcmmn.c
# End Source File
# Begin Source File

SOURCE=.\smtpd.c
# End Source File
# Begin Source File

SOURCE=.\smtpdns.c
# End Source File
# Begin Source File

SOURCE=.\smtpnett.c
# End Source File
# Begin Source File

SOURCE=.\smtpnetw.c
# End Source File
# Begin Source File

SOURCE=.\smtpserv.c
# End Source File
# Begin Source File

SOURCE=.\smtpverb.c
# End Source File
# End Group
# Begin Group "include"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\address.h
# End Source File
# Begin Source File

SOURCE=.\deliver.h
# End Source File
# Begin Source File

SOURCE=.\deliverm.h
# End Source File
# Begin Source File

SOURCE=.\delivers.h
# End Source File
# Begin Source File

SOURCE=.\mailnewm.h
# End Source File
# Begin Source File

SOURCE=.\memstr.h
# End Source File
# Begin Source File

SOURCE=.\mlib.h
# End Source File
# Begin Source File

SOURCE=.\pop3clnt.h
# End Source File
# Begin Source File

SOURCE=.\pop3lwc.h
# End Source File
# Begin Source File

SOURCE=.\pop3mbox.h
# End Source File
# Begin Source File

SOURCE=.\pop3trns.h
# End Source File
# Begin Source File

SOURCE=.\pop3user.h
# End Source File
# Begin Source File

SOURCE=.\smtpclnt.h
# End Source File
# Begin Source File

SOURCE=.\smtpcmds.h
# End Source File
# Begin Source File

SOURCE=.\smtpcmmn.h
# End Source File
# Begin Source File

SOURCE=.\smtpconn.h
# End Source File
# Begin Source File

SOURCE=.\smtpdns.h
# End Source File
# Begin Source File

SOURCE=.\smtplwc.h
# End Source File
# Begin Source File

SOURCE=.\smtpnett.h
# End Source File
# Begin Source File

SOURCE=.\smtpnetw.h
# End Source File
# Begin Source File

SOURCE=.\smtprecv.h
# End Source File
# Begin Source File

SOURCE=.\smtpserv.h
# End Source File
# Begin Source File

SOURCE=.\smtptrns.h
# End Source File
# Begin Source File

SOURCE=.\smtputil.h
# End Source File
# Begin Source File

SOURCE=.\smtpverb.h
# End Source File
# Begin Source File

SOURCE=.\sysalias.h
# End Source File
# End Group
# End Target
# End Project
