#       $Id: nmakewnt.mak 1.21 1997/05/11 18:14:54 ahd v1-12s $
#
#       Copyright (c) 1989-1997 by Kendra Electronic Wonderworks;
#       all rights reserved except those explicitly granted by
#       the UUPC/extended license.
#
#       $Log: nmakewnt.mak $
#       Revision 1.21  1997/05/11 18:14:54  ahd
#       Allow co-existence of SMTP and non-SMTP rmail/rmailn modules
#
#       Revision 1.20  1997/05/11 04:25:17  ahd
#       SMTP client support for RMAIL/UUXQT
#
#       Revision 1.19  1997/04/24 01:01:37  ahd
#       Annual Copyright Update
#
#       Revision 1.18  1996/11/18 04:40:55  ahd
#       Support for building automatically under Windows 95
#
#       Revision 1.17  1996/01/01 20:47:56  ahd
#       Annual Copyright Update
#
#       Revision 1.16  1995/11/30 12:47:07  ahd
#       *** empty log message ***
#
#       Revision 1.15  1995/11/08 01:03:13  ahd
#       Allow copying between drives during install
#
#       Revision 1.14  1995/09/04 18:42:59  ahd
#       Add -DUDEBUG flag to NT build
#
#       Revision 1.13  1995/02/23 04:26:17  ahd
#       Correct extra install variable names
#
#       Revision 1.12  1995/01/22 04:15:00  ahd
#       Convert internal zip names to match external names
#
#       Revision 1.11  1995/01/13 12:23:44  ahd
#       Support for UUPCDLL.DLL for Windows NT
#
#       Revision 1.10  1995/01/09 12:34:31  ahd
#       Rename NT object files directory to 'objnt' :-)
#
#       Revision 1.9  1995/01/07 20:47:20  ahd
#       Add missing NT components
#
#       Revision 1.8  1994/12/22 00:05:33  ahd
#       Annual Copyright Update
#
#       Revision 1.7  1994/12/09 03:39:49  ahd
#       Drop regsetup.dll from install to get work done.
#
#       Revision 1.6  1994/05/30  02:19:23  ahd
#       Normalize options
#       Delete build of uupcdll.dll in prod
#
#       Revision 1.5  1994/05/23  21:40:46  dmwatt
#       Add Windows NT utilities
#       Add UUCICO without TCP/IP for OS/2 32 bit
#
#       Revision 1.4  1994/04/24  20:32:52  ahd
#       Add titlen.obj to library list
#
#       Revision 1.3  1994/03/20  00:52:26  dmwatt
#       Suppress copyirght message from compile to allow more warnings to
#       display at once.
#       Add libraries which the SDK is too stupid to include that Visual C++
#       gets right.
#
#       Revision 1.2  1994/03/07  06:02:00  dmwatt
#       Additional multi-platform cleanup
#
#     Revision 1.1  1994/03/05  21:09:21  ahd
#     Initial revision
#
#
!include <win32.mak>

!ifdef NODEBUG
UDEBUGFLAG=
!else
UDEBUGFLAG=-DUDEBUG
!endif

WIN32ENV=1
COMMOPT = -nologo  -MT $(cdebug)
CCOPT   = $(COMMOPT) $(UDEBUGFLAG) $(cflags) $(cvarsmt) -I$(UULIB) -Fo$@
!ifndef PROD
PROD    = $(PRODDRIVE)\uupc\ntbin
!endif
ZIPID   = n
ERASE   = del
MODEL   = t                          # Really NT version in this case
SMTP    = 1

LIBOSLIST=  $(OBJ)\ndirnt.obj $(OBJ)\scrsiznt.obj $(OBJ)\setstdin.obj\
            $(OBJ)\pnterr.obj $(OBJ)\titlen.obj

RMAIL_DELIVERS_SUPPORT=$(OBJ)\pwserr.obj
UUCICOOBJ3 = $(OBJ)\catcheru.obj $(OBJ)\dcpepkt.obj $(OBJ)\dcptpkt.obj\
             $(OBJ)\prtynt.obj $(OBJ)\suspendn.obj \
             $(OBJ)\ulibip.obj $(OBJ)\ulibnt.obj $(OBJ)\pwserr.obj

#       You need to add MYUULIBS=OLDNAMES.LIB to your environment
#       or NMAKE.MAK include file to build under NT's Visual C++

LDOPT    = $(COMMOPT) -Fe$@

OTHERLIBS=ADVAPI32.LIB WSOCK32.LIB kernel32.lib libcmt.lib OLDNAMES.LIB $(MYUULIBS)

EXTRAT=regsetup.exe uupcdll.dll uusmtpd.exe
EXTRA1=$(PROD)\uusmtpd.exe
EXTRA2=$(PROD)\regsetup.exe
EXTRA3=$(PROD)\uupcdll.dll
