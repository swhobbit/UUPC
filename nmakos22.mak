#       $Id: nmakos22.mak 1.2 1994/03/06 12:26:04 ahd Exp $
#
#       Copyright (c) 1989-1994 by Kendra Electronic Wonderworks;
#       all rights reserved except those explicitly granted by
#       the UUPC/extended license.
#
#       $Log: nmakos22.mak $
#       Revision 1.2  1994/03/06  12:26:04  ahd
#       Twiddle compiler options
#
#     Revision 1.1  1994/03/05  21:09:21  ahd
#     Initial revision
#
#

ERASE    = del /f
EXTRA2   = $(DLLPROD)\$(DLLNAME) $(PROD)\mail.ico $(PROD)\uucico.ico
EXTRAS   = $(PROD)\pnews.cmd $(PROD)\mailchek.cmd $(PROD)\getuupc.cmd
LIBOSLIST= $(OBJ)\ndiros2.obj $(OBJ)\scrsize2.obj $(OBJ)\pos2err.obj
MODEL    = 2                    # Really OS/2 version in this case
OS2      = 1
!ifndef PROD
PROD     = \uupc\os2bin32
!endif
STACKSIZE= 0x50000
WIDTH    = 32
ZIPID    = 2
!ifndef MSGFILE
MSGFILE  =\ibmcpp\help\dde4.msg
!endif
!ifndef DLLDIR
DLLDIR   =\ibmcpp\dll
!endif
!ifndef DLLPROD
DLLPROD=$(PROD)
!endif
RTLDLL   = dde4sbs.dll
DLLNAME  = UPCR$(VERS:1.=).dll

!ifdef NODEBUG
#       Use this for production
DBGOPT = -O
!else
#       Use this for debugging
DBGOPT  = -Ti -DUDEBUG -Tx # -D__DEBUG_ALLOC__ -Wall -Wcnv- -Wext- -Wgen- -Wlan- -Wppc- -Wppt- -Wuni-
!endif

COMMOPT = -Sv -Q $(DBGOPT) -Gs -Gd
CCOPT   = $(COMMOPT) -Ss -c -Fi -Si -I$(UULIB) -Fo$@
LDOPT   = -b"/A:4 /BAT" $(COMMOPT) -Fe $@
UUCICOOBJ3 = $(OBJ)\dcpepkt.obj $(OBJ)\dcptpkt.obj $(OBJ)\ulibos2.obj \
             $(OBJ)\ulibnmp.obj $(OBJ)\prtyos2.obj $(OBJ)\suspend2.obj

