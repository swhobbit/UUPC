#       $Id: nmakos22.mak 1.22 1997/11/28 04:51:14 ahd v1-12u $
#
#       Copyright (c) 1989-1998 by Kendra Electronic Wonderworks;
#       all rights reserved except those explicitly granted by
#       the UUPC/extended license.
#
#       $Log: nmakos22.mak $
#       Revision 1.22  1997/11/28 04:51:14  ahd
#       SMTPD initial OS/2 support
#
#       Revision 1.21  1997/05/20 05:55:41  ahd
#       Support for TCP/IP RMAIL
#
#       Revision 1.20  1997/05/11 18:14:54  ahd
#       Allow co-existence of SMTP and non-SMTP rmail/rmailn modules
#
#       Revision 1.19  1997/05/11 04:25:17  ahd
#       SMTP client support for RMAIL/UUXQT
#
#       Revision 1.18  1997/04/24 01:01:49  ahd
#       Annual Copyright Update
#
#       Revision 1.17  1996/01/01 20:47:44  ahd
#       Annual Copyright Update
#
#       Revision 1.16  1995/11/30 12:47:07  ahd
#       Support new disk layout
#
#       Revision 1.15  1995/11/08 01:03:13  ahd
#       Allow copying between drives during install
#
#       Revision 1.14  1995/03/23 01:28:55  ahd
#       Add special OS/2 delete file function
#
#       Revision 1.13  1995/02/20 17:24:05  ahd
#       ooops, we did need that OS=1
#
#       Revision 1.12  1995/02/20 00:07:07  ahd
#       Move various common news subroutines into own library
#       Break common library into two parts to prevent NMAKE failures
#       Kai Uwe Rommel's news cache support
#       Support MS C++ 8.0 compiler
#       General makefile clean up
#
#       Revision 1.11  1995/01/29 14:04:46  ahd
#       Enable most IBM C/Set++ compiler warnings
#
#       Revision 1.10  1995/01/22 04:15:00  ahd
#       Convert internal zip names to match external names
#
#       Revision 1.9  1994/12/22 00:05:44  ahd
#       Annual Copyright Update
#
#       Revision 1.8  1994/06/13 00:08:33  ahd
#       Add UUHOUR command to build
#
#       Revision 1.7  1994/05/25  20:58:34  ahd
#       Don't enable automatic generation of precompiled headers in
#       makefile; we can do that via ICC environment variable.
#
#       Revision 1.6  1994/05/23  21:40:09  ahd
#       Add support for non-TCP/IP UUCICO under 32 bit OS/2
#
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
EXTRA1   = $(DLLPROD)\$(DLLNAME) $(PROD)\mail.ico $(PROD)\uucico.ico \
           $(PROD)\uucicon.exe $(PROD)\rmailn.exe $(PROD)\uusmtpd.exe
EXTRAS   = $(PROD)\pnews.cmd $(PROD)\mailchek.cmd $(PROD)\getuupc.cmd \
           $(PROD)\uucpd.cmd $(PROD)\uuhour.cmd
EXTRAT   = uusmtpd.exe
LIBOSLIST= $(OBJ)\ndiros2.obj $(OBJ)\scrsize2.obj $(OBJ)\pos2err.obj \
           $(OBJ)\remove2.obj $(OBJ)\title2.obj
MODEL    = 2                    # Really OS/2 version in this case

!ifndef PROD
PROD     = $(PRODDRIVE)\uupc\os2bin32
!endif

STACKSIZE= 0x50000
ZIPID    = 2

!ifndef MSGFILE
MSGFILE  =$(IBMCPP)\ibmcpp\help\dde4.msg
!endif

!ifndef DLLDIR
DLLDIR   =$(IBMCPP)\ibmcpp\dll
!endif

!ifndef DLLPROD
DLLPROD=$(PROD)
!endif

RTLDLL   = dde4sbs.dll
DLLNAME  = UPCR$(VERS:1.=).dll
OS2=1                   # Enables generation of .DEF file
SMTP    = 1

RMAIL_DELIVERS_SUPPORT=$(OBJ)\psos2err.obj
ULIBOBJS   = $(OBJ)\ulibip.obj $(OBJ)\ulibos2.obj $(OBJ)\psos2err.obj \
             $(OBJ)\ulibnmp.obj
UUCICOOBJX = $(OBJ)\dcpepkt.obj $(OBJ)\dcptpkt.obj \
             $(OBJ)\prtyos2.obj $(OBJ)\suspend2.obj \
             $(OBJ)\ulibos2.obj $(OBJ)\psos2err.obj $(OBJ)\ulibnmp.obj
UUCICOOBJ3 = $(UUCICOOBJX) $(OBJ)\ulibip.obj

OTHERLIBS=   so32dll.lib tcp32dll.lib

!ifdef NODEBUG
#       Use this for production
DBGOPT = -O -Gs -Wtrd- -Wuse- -Wpar-
!else
#       Use this for debugging
DBGOPT  =-Ti -Tx -DUDEBUG # -D__DEBUG_ALLOC__
!endif

COMMOPT = -Q -Gd
CCOPT   = $(COMMOPT) -Wall -Wcnv- -Wext- -Wgen- -Wlan- -Wppc- -Wppt- -Wuni- $(DBGOPT) -c -Si -I$(UULIB) -Fo$@
LDOPT   = $(COMMOPT)  $(DBGOPT) -b"/A:4 /BAT" -Fe $@
