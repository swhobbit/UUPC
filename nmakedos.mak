#       $Id: nmakedos.mak 1.4 1994/12/22 00:05:21 ahd Exp $
#
#       Copyright (c) 1989-1995 by Kendra Electronic Wonderworks;
#       all rights reserved except those explicitly granted by
#       the UUPC/extended license.
#
#       $Log: nmakedos.mak $
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

PROD    = \uupc\bin
MASM    = masm.exe
!ifndef ERASE
ERASE    = del
!endif

!ifndef MODEL
MODEL   = S
!endif

!if "$(MODEL)" == "T"
NODEBUG=1
!endif

#MASMOPTS = /DUUPC /DDEBUG /n /v /t /z /W2 /ZD /Mx
MASMOPTS = /DUUPC /n /v /t /z /W2 /ZI /ZD /Mx

LIBOSLIST = $(OBJ)\ndir.obj $(OBJ)\getdta.obj $(OBJ)\scrsize.obj \
            $(OBJ)\title.obj

UUCICOOBJ3= $(OBJ)\comm.obj $(OBJ)\fossil.obj $(OBJ)\suspend.obj \
            $(OBJ)\ulib.obj $(OBJ)\ulibfs.obj $(OBJ)\ulib14.obj

EXTRAT  = comm34.com
LINKER  = link
STACKSIZE=3000                  # In hex, because /F below is brain dead
LINKOPT = /batch /far /noignorecase /stack:0x$(STACKSIZE)
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
!else
#       Use this CCOPT for debugging; the -FR is optional.
DBGOPT  = -Odr -Zi -DUDEBUG # -FR$(SBR)\$(@B).sbr
!endif

COMMOPT = -nologo -W4 -A$(MODEL) $(DBGOPT)
CCOPT   = $(COMMOPT) -I$(UULIB) -c -Fo$@
LDOPT   = $(COMMOPT) -F $(STACKSIZE) -Fe$@
