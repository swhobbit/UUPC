#       $Id: nmakedos.mak 1.20 1998/11/05 03:28:45 ahd v1-13f $
#
#       Copyright (c) 1989-1999 by Kendra Electronic Wonderworks;
#       all rights reserved except those explicitly granted by
#       the UUPC/extended license.
#
#       $Log: nmakedos.mak $
#       Revision 1.20  1998/11/05 03:28:45  ahd
#       Correct directory used for ZIP archives
#
#       Revision 1.19  1998/05/17 21:11:20  ahd
#       Allow building line-length limited DOS modules in
#       different directory
#
#       Revision 1.18  1998/03/01 01:22:00  ahd
#       Annual Copyright Update
#
#       Revision 1.17  1997/04/24 01:01:16  ahd
#       Annual Copyright Update
#
#       Revision 1.16  1996/01/20 13:04:26  ahd
#       Automatically use /codeview when using link in debug mode
#
#       Revision 1.15  1996/01/07 14:12:17  ahd
#       Lower stack under DOS to 0x2000 (8K)
#
#       Revision 1.14  1996/01/01 20:47:36  ahd
#       Annual Copyright Update
#
#       Revision 1.13  1995/12/02 14:07:05  ahd
#       Make default build for DOS all .exe files
#       Go to version 1.12r
#       Allow override of STACKSIZE under DOS
#
#       Revision 1.12  1995/11/30 12:47:07  ahd
#       *** empty log message ***
#
#       Revision 1.11  1995/11/08 01:03:13  ahd
#       Allow copying between drives during install
#
#       Revision 1.10  1995/09/11 00:19:01  ahd
#       Go to version 1.12p
#       Add support for Word for Windows source archive, PIF files
#
#       Revision 1.9  1995/03/24 04:09:32  ahd
#       Allow generating debug versions of MS VC COM files.
#
#       Revision 1.8  1995/03/23 01:28:55  ahd
#       Clean up DOS build -- modules were not always regenerated
#       Clean up OS/2 build -- those messages should be tacked on the DLL,
#       not the modules.
#
#       Revision 1.7  1995/02/24 00:35:30  ahd
#       Lower stack size on COM modules
#
#       Revision 1.6  1995/02/21 13:01:20  ahd
#       Suppress map file
#
#       Revision 1.5  1995/02/20 00:07:07  ahd
#       Move various common news subroutines into own library
#       Break common library into two parts to prevent NMAKE failures
#       Kai Uwe Rommel's news cache support
#       Support MS C++ 8.0 compiler
#       General makefile clean up
#
#       Revision 1.4  1994/12/22 00:05:21  ahd
#       Annual Copyright Update
#
#       Revision 1.3  1994/03/11 01:49:00  ahd
#       Lower stack size
#
#       Revision 1.2  1994/03/07  06:00:49  ahd
#       Additional multi-platform clean up.
#
#     Revision 1.1  1994/03/05  21:09:21  ahd
#     Initial revision
#
#

!ifndef PROD
PROD    = $(PRODDRIVE)\uupc\bin
!endif
MASM    = masm.exe
!ifndef ERASE
ERASE    = del
!endif

!ifndef MODEL
MODEL   = S
!endif

!ifndef UDEBUG
!if "$(MODEL)" == "T"
NODEBUG=1
!endif
!endif

ICONS    = icons.win
#MASMOPTS = /DUUPC /DDEBUG /n /v /t /z /W2 /ZD /Mx
MASMOPTS = /DUUPC /n /v /t /z /W2 /ZI /ZD /Mx

LIBOSLIST = $(OBJ)\ndir.obj $(OBJ)\getdta.obj $(OBJ)\scrsize.obj \
            $(OBJ)\title.obj

UUCICOOBJ3= $(OBJ)\comm.obj $(OBJ)\fossil.obj $(OBJ)\suspend.obj \
            $(OBJ)\ulib.obj $(OBJ)\ulibfs.obj $(OBJ)\ulib14.obj

!ifndef STACKSIZE
STACKSIZE=2000                  # In hex, because /F below is brain dead
!endif

EXTRAT  = comm34.$(COMEXE)            # Include in test build ...
EXTRA1  = $(PROD)\rmail.pif $(PROD)\uuxqt.pif\
          $(PROD)\comm34.$(COMEXE)    # ... and ship it off to users as well
EXTRA2  = $(PROD)\uuclean.pif
LINKER  = link

ZIPID   = d

# *--------------------------------------------------------------------*
# *   Add /Zi for Codeview debugging information, and change the       *
# *   /exepack option to /codeview on the linker options (above).      *
# *                                                                    *
# *   Add /FR$(SBR)\$(@B).sbr to generate source browser files.        *
# *   (I had to delete the this because of space considerations        *
# *   on kendra.)                                                      *
# *                                                                    *
# *   Also, delete /Ocegilt (optimizer options) and /Gs (no stack      *
# *   overflow checking) if debugging and having problems.  Note       *
# *   stack overflow *is* checked in printmsg(), which is called       *
# *   by most if not all deep routines.                                *
# *                                                                    *
# *   Delete the /G2 if building for an 8086 based system.             *
# *--------------------------------------------------------------------*

!ifdef NODEBUG
#       Use this CCOPT for productiom
DBGOPT  = -Ocegilt
LDBGOPT =
!else
#       Use this CCOPT for debugging; the -FR is optional.
DBGOPT  = -Odr -Zi -DUDEBUG # -FR$(SBR)\$(@B).sbr
LDBGOPT = /CO
!endif

COMMOPT = -nologo -W4 -A$(MODEL) $(DBGOPT)
CCOPT   = $(COMMOPT) -I$(UULIB) -c -Fo$@
LDOPT   = $(COMMOPT) -F $(STACKSIZE) -Fmnul -Fe$@

LINKOPT = /batch $(LDBGOPT) /map:nul /far /noig /stack:0x$(STACKSIZE)
