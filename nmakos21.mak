#       $Id: nmakos21.mak 1.2 1994/03/07 06:00:49 ahd Exp $
#
#       Copyright (c) 1989-1994 by Kendra Electronic Wonderworks;
#       all rights reserved except those explicitly granted by
#       the UUPC/extended license.
#
#       $Log: nmakos21.mak $
#       Revision 1.2  1994/03/07  06:00:49  ahd
#       Additional multi-platform clean up.
#
#     Revision 1.1  1994/03/05  21:09:21  ahd
#     Initial revision
#
#

ERASE    = del /f
EXTRA2   = $(PROD)\mail.ico $(PROD)\uucico.ico
EXTRAS   = $(PROD)\pnews.cmd $(PROD)\mailchek.cmd $(PROD)\getuupc.cmd
LIBLIST  = $(MODEL)libcep+$(LIBCOMM)+os2
LIBOS    = $(LIBDIR)\$(MODEL)uupcmsc1.lib
LIBOSLIST= $(OBJ)\ndiros2.obj $(OBJ)\scrsize2.obj $(OBJ)\pos2err.obj    \
           $(OBJ)\title2.obj
LINKOPT  = /exepack /far /noignorecase /nod /batch
MODEL    = M                    # Really OS/2 version in this case
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
