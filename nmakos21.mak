#       $Id: nmakos21.mak 1.5 1994/12/22 00:05:39 ahd Exp $
#
#       Copyright (c) 1989-1995 by Kendra Electronic Wonderworks;
#       all rights reserved except those explicitly granted by
#       the UUPC/extended license.
#
#       $Log: nmakos21.mak $
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

ERASE    = del /f
EXTRA1   = $(PROD)\mail.ico $(PROD)\uucico.ico
EXTRAS   = $(PROD)\pnews.cmd $(PROD)\mailchek.cmd $(PROD)\getuupc.cmd   \
            $(PROD)\uuhour.cmd
LIBLIST  = $(MODEL)libcep+$(LIBCOMM)+os2
LIBOS    = $(LIBDIR)\$(MODEL)uupcmsc1.lib
LIBOSLIST= $(OBJ)\ndiros2.obj $(OBJ)\scrsize2.obj $(OBJ)\pos2err.obj    \
           $(OBJ)\title2.obj
LINKOPT  = /exepack /far /noignorecase /nod /batch
MODEL    = M                    # All OS/2 16 bit programs are medium
PROD     = \uupc\os2bin
STACKSIZE= 0x02A00
WIDTH    = 16
OS2      = 1
ZIPID    = 1

UUCICOOBJ3 = $(OBJ)\dcpepkt.obj $(OBJ)\dcptpkt.obj $(OBJ)\ulibos2.obj \
             $(OBJ)\ulibnmp.obj $(OBJ)\prtyos2.obj $(OBJ)\suspend2.obj

# *--------------------------------------------------------------------*
# *        Use this CCOPT for debugging; the /FR is optional.          *
# *--------------------------------------------------------------------*
# CCOPT   = -I$(UULIB) -A$(MODEL) -c -W4 -nologo -G2 -Od -Zi -Or -DUDEBUG -DFAMILYAPI -Fo$@ -FR$(SBR)\$(@B).sbr

# *--------------------------------------------------------------------*
# *                  Use this CCOPT for productiom                     *
# *--------------------------------------------------------------------*
CCOPT   = -I$(UULIB) -A$(MODEL) -c -W4 -nologo -G2 -Ocegilt -DFAMILYAPI -Fo$@

LDOPT   = -A$(MODEL) -Lp $(DBGOPT) -Fe$@
