#       $Id: nmakewnt.mak 1.9 1995/01/07 20:47:20 ahd Exp $
#
#       Copyright (c) 1989-1995 by Kendra Electronic Wonderworks;
#       all rights reserved except those explicitly granted by
#       the UUPC/extended license.
#
#       $Log: nmakewnt.mak $
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

COMMOPT = -nologo  -MT $(cdebug)
CCOPT   = $(COMMOPT) $(cflags) $(cvarsmt) -I$(UULIB) -Fo$@
PROD    = \uupc\ntbin
ZIPID   = n
ERASE   = del
MODEL   = t                          # Really NT version in this case

LIBOSLIST=  $(OBJ)\ndirnt.obj $(OBJ)\scrsiznt.obj $(OBJ)\setstdin.obj\
            $(OBJ)\pnterr.obj $(OBJ)\titlen.obj

UUCICOOBJ3 = $(OBJ)\catcheru.obj $(OBJ)\dcpepkt.obj $(OBJ)\dcptpkt.obj\
              $(OBJ)\prtynt.obj $(OBJ)\pwserr.obj $(OBJ)\suspendn.obj \
              $(OBJ)\ulibip.obj $(OBJ)\ulibnt.obj

#       You need to add MYUULIBS=OLDNAMES.LIB to your environment
#       or NMAKE.MAK include file to build under NT's Visual C++

LDOPT    = $(COMMOPT) -Fe$@
LIBEXE   = lib

OTHERLIBS=ADVAPI32.LIB WSOCK32.LIB kernel32.lib libcmt.lib OLDNAMES.LIB $(MYUULIBS)

EXTRAT=regsetup.exe uupcdll.dll
EXTRA3=$(PROD)\regsetup.exe
#EXTRA4=$(PROD)\uupcdll.dll
