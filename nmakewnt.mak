#       $Id: NMAKEWNT.MAK 1.1 1994/03/05 21:09:21 ahd Exp $
#
#       Copyright (c) 1989-1994 by Kendra Electronic Wonderworks;
#       all rights reserved except those explicitly granted by
#       the UUPC/extended license.
#
#       $Log: NMAKEWNT.MAK $
#     Revision 1.1  1994/03/05  21:09:21  ahd
#     Initial revision
#
#

CCOPT   = $(cflags) $(cvarsmt) $(cdebug) /MT -I$(UULIB) /Fo$@
PROD    = \uupc\ntbin
ZIPID   = n
ERASE   = del
MODEL   = N                          # Really NT version in this case

LIBOSLIST=  $(OBJ)\ndirnt.obj $(OBJ)\scrsiznt.obj $(OBJ)\setstdin.obj\
            $(OBJ)\pnterr.obj

UUCICOOBJ3 = $(OBJ)\catcheru.obj $(OBJ)\dcpepkt.obj $(OBJ)\dcptpkt.obj\
              $(OBJ)\prtynt.obj $(OBJ)\pwserr.obj $(OBJ)\suspendn.obj \
              $(OBJ)\ulibip.obj $(OBJ)\ulibnt.obj
#       You need to add MYUULIBS=OLDNAMES.LIB to your environment
#       or NMAKE.MAK include file to build under NT's Visual C++

LDOPT    = -MT  -D_MT -Fe$@
OTHERLIBS=ADVAPI32.LIB WSOCK32.LIB $(MYUULIBS)
