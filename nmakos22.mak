#       $Id: nmakos22.mak 1.5 1994/04/26 02:47:50 ahd Exp $
#
#       Copyright (c) 1989-1994 by Kendra Electronic Wonderworks;
#       all rights reserved except those explicitly granted by
#       the UUPC/extended license.
#
#       $Log: nmakos22.mak $
#       Revision 1.5  1994/04/26  02:47:50  ahd
#       Re-enable automatic preset header generation - kendra's CPU is just
#       too slow not too.
#       TCP/IP support for OS/2, including UUCPD.CMD.
#
#       Revision 1.4  1994/04/24  20:30:40  ahd
#       Don't automatically generate pre-compiled headers
#       Add 32 bit TCP/IP support
#       Add title altering function
#
#       Revision 1.3  1994/03/07  06:04:32  ahd
#       Additional multi-platform cleanup
#
#       Revision 1.2  1994/03/06  12:26:04  ahd
#       Twiddle compiler options
#
#     Revision 1.1  1994/03/05  21:09:21  ahd
#     Initial revision
#
#

ERASE    = del /f
EXTRA2   = $(DLLPROD)\$(DLLNAME) $(PROD)\mail.ico $(PROD)\uucico.ico \
           $(PROD)\uucicon.exe
EXTRAS   = $(PROD)\pnews.cmd $(PROD)\mailchek.cmd $(PROD)\getuupc.cmd \
           $(PROD)\uucpd.cmd
LIBOSLIST= $(OBJ)\ndiros2.obj $(OBJ)\scrsize2.obj $(OBJ)\pos2err.obj \
           $(OBJ)\title2.obj
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
DBGOPT = -O -Gs
!else
#       Use this for debugging
DBGOPT  = -Ti -DUDEBUG -Tx # -D__DEBUG_ALLOC__ -Wall -Wcnv- -Wext- -Wgen- -Wlan- -Wppc- -Wppt- -Wuni-
!endif

COMMOPT = -Sv -Q $(DBGOPT) -Gd
CCOPT   = $(COMMOPT) -Ss -c -Si -I$(UULIB) -Fo$@ -Fi
LDOPT   = -b"/A:4 /BAT" $(COMMOPT) -Fe $@
UUCICOOBJX = $(OBJ)\dcpepkt.obj $(OBJ)\dcptpkt.obj $(OBJ)\ulibos2.obj \
             $(OBJ)\ulibnmp.obj $(OBJ)\prtyos2.obj $(OBJ)\suspend2.obj \
             $(OBJ)\psos2err.obj
UUCICOOBJ3 = $(UUCICOOBJX) $(OBJ)\ulibip.obj

OTHERLIBS=   so32dll.lib tcp32dll.lib
