#       $Id$
#
#       Copyright (c) 1989-1994 by Kendra Electronic Wonderworks;
#       all rights reserved except those explicitly granted by
#       the UUPC/extended license.
#
#       $Log$
#

CCOPT   = $(cflags) $(cvarsmt) $(cdebug) /MT -I$(UULIB) /Fo$@
OBJ     = $(SRCSLASH)objmsnt
PROD    = \uupc\ntbin
ZIPID   = n

LIBOS    = $(LIBDIR)\uupcnt.lib

LIBOSLIST=  $(OBJ)\ndirnt.obj $(OBJ)\scrsiznt.obj $(OBJ)\setstdin.obj\
            $(OBJ)\pnterr.obj

UUCICOOBJ3 = $(OBJ)\catcheru.obj$(OBJ)\dcpepkt.obj $(OBJ)\dcptpkt.obj\
              $(OBJ)\prtynt.obj $(OBJ)\pwserr.obj $(OBJ)\suspendn.obj \
              $(OBJ)\ulibip.obj $(OBJ)\ulibnt.obj
#       You need to add MYUULIBS=OLDNAMES.LIB to your environment
#       or MSCMAKE include file to build under NT's Visual C++

NTLIBS   = $(LIBCOMM) $(LIBNT) $(conlibsmt) ADVAPI32.lib WSOCK32.lib $(MYUULIBS)
LINKNOPT = $(conflags) $(ldebug) /NODEFAULTLIB
LDOPT    = -MT  -D_MT -Fe$@
