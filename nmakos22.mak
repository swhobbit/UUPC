#       $Id: nmakos22.mak 1.27 1998/11/04 01:59:13 ahd v1-13f $
#
#       Copyright (c) 1989-1999 by Kendra Electronic Wonderworks;
#       all rights reserved except those explicitly granted by
#       the UUPC/extended license.
#
#       $Log: nmakos22.mak $
#       Revision 1.27  1998/11/04 01:59:13  ahd
#       Support easily moving the zip archives to a different directory
#       from the rest of the build.
#
#       Revision 1.26  1998/10/28 03:18:17  ahd
#       Allow program other than LIB to handle library archives for OS/2
#       Correct OS/2 compile options for Visual C++ 3.0
#
#       Revision 1.25  1998/04/27 01:54:27  ahd
#       Revise icon source directory
#
#       Revision 1.24  1998/03/01 19:36:36  ahd
#       First compiling POP3 server which accepts user id/password
#
#       Revision 1.23  1998/03/01 01:22:14  ahd
#       Annual Copyright Update
#
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

ICONS    = icons.os2
ERASE    = del /f
EXTRA1   = $(DLLPROD)\$(DLLNAME) $(PROD)\mail.ico $(PROD)\uucico.ico \
           $(PROD)\uucicon.exe $(PROD)\rmailn.exe $(PROD)\uusmtpd.exe  \
           $(PROD)\uupopd.exe
EXTRAS   = $(PROD)\pnews.cmd $(PROD)\mailchek.cmd $(PROD)\getuupc.cmd \
           $(PROD)\uucpd.cmd $(PROD)\uuhour.cmd
EXTRAT   = uusmtpd.exe  uupopd.exe
LIBOSLIST= $(OBJ)\ndiros2.obj $(OBJ)\scrsize2.obj $(OBJ)\pos2err.obj \
           $(OBJ)\remove2.obj $(OBJ)\title2.obj
MODEL    = 2                    # Really OS/2 version in this case

!ifndef PROD
PROD     = $(PRODDRIVE)\uupc\os2bin32
!endif

!ifndef LIBPROG
LIBPROG=LIBV2R1
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

RTLDLL   = cppos30.dll
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

COMMOPT = -Q -Gd -w2
CCOPT   = $(COMMOPT) -Wall -Wcnv- -Wext- -Wgen- -Wlan- -Wppc- -Wppt- -Wuni- $(DBGOPT) -c -Si -I$(UULIB) -Fo$@
#LDOPT   = $(COMMOPT)  $(DBGOPT) -b"/A:4 /pmtype:vio" -Fe $@
LDOPT   = $(COMMOPT)  $(DBGOPT) -b"/A:4" -Fe $@
