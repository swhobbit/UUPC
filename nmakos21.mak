#       $Id: nmakos21.mak 1.15 2000/05/12 12:08:51 ahd v1-13g $
#
#       Copyright (c) 1989-2001 by Kendra Electronic Wonderworks;
#       all rights reserved except those explicitly granted by
#       the UUPC/extended license.
#
#       $Log: nmakos21.mak $
#       Revision 1.15  2000/05/12 12:08:51  ahd
#       Annual copyright update
#
#       Revision 1.14  1999/01/04 03:51:49  ahd
#       Annual copyright change
#
#       Revision 1.13  1998/04/27 01:54:27  ahd
#       Revise icon source directory
#
#       Revision 1.12  1998/03/01 01:22:12  ahd
#       Annual Copyright Update
#
#       Revision 1.11  1997/04/24 01:01:43  ahd
#       Annual Copyright Update
#
#       Revision 1.10  1996/01/01 23:54:01  ahd
#       Use larger stack under OS/2 16 bit
#       Annual copyright update
#       Rename user "alias" module to nickname.obj for mail.exe
#
#       Revision 1.9  1995/11/08 01:03:13  ahd
#       Allow copying between drives during install
#
#       Revision 1.8  1995/02/20 17:24:05  ahd
#       ooops, we did need that OS=1
#
#       Revision 1.7  1995/02/20 00:07:07  ahd
#       Move various common news subroutines into own library
#       Break common library into two parts to prevent NMAKE failures
#       Kai Uwe Rommel's news cache support
#       Support MS C++ 8.0 compiler
#       General makefile clean up
#
#       Revision 1.6  1995/01/22 04:15:00  ahd
#       Convert internal zip names to match external names
#
#       Revision 1.5  1994/12/22 00:05:39  ahd
#       Annual Copyright Update
#
#       Revision 1.4  1994/06/13 00:08:33  ahd
#       Add UUHOUR command to build
#
#       Revision 1.3  1994/04/24  20:28:00  ahd
#       Add title routine to build
#
#       Revision 1.2  1994/03/07  06:00:49  ahd
#       Additional multi-platform clean up.
#
#     Revision 1.1  1994/03/05  21:09:21  ahd
#     Initial revision
#
#

ICONS    = icons.os2
ERASE    = del /f               # Assumes actually building on OS/2 2.x
EXTRA1   = $(PROD)\mail.ico $(PROD)\uucico.ico
EXTRAS   = $(PROD)\pnews.cmd $(PROD)\mailchek.cmd $(PROD)\getuupc.cmd   \
            $(PROD)\uuhour.cmd
LIBOSLIST= $(OBJ)\ndiros2.obj $(OBJ)\scrsize2.obj $(OBJ)\pos2err.obj    \
           $(OBJ)\title2.obj
MODEL    = M                    # All OS/2 16 bit programs are medium
!ifndef PROD
PROD     = $(PRODDRIVE)\uupc\os2bin
!endif
STACKSIZE= 0x03000
ZIPID    = 1
OS2=1                   # Enables generation of .DEF file

UUCICOOBJ3 = $(OBJ)\ulibos2.obj \
             $(OBJ)\prtyos2.obj $(OBJ)\suspend2.obj

!ifdef NODEBUG
#       Use this for production
DBGOPT = -Ocegilt
!else
#       Use this for debugging; the -FR is optional
DBGOPT  = -Odr -Zi -DUDEBUG                     # -FR$(SBR)\$(@B).sbr
!endif

COMMOPT = -nologo -A$(MODEL) $(DBGOPT)
CCOPT   = $(COMMOPT) -I$(UULIB) -c -W4 -nologo -G2 -DFAMILYAPI -Fo$@
LDOPT   = $(COMMOPT) -Lp -Fe$@
