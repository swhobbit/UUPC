#       $Id$
#
#       Copyright (c) 1989-1994 by Kendra Electronic Wonderworks;
#       all rights reserved except those explicitly granted by
#       the UUPC/extended license.
#
#       $Log$
#

ERASE    = del /f
EXTRA2   = $(PROD)\mail.ico $(PROD)\uucico.ico
EXTRA4   = $(PROD)\pnews.cmd
EXTRAS   = $(PROD)\pnews.cmd $(PROD)\mailchek.cmd $(PROD)\getuupc.cmd
LIBLIST  = $(MODEL)libcep+$(LIBCOMM)+os2
LIBOS    = $(LIBDIR)\$(MODEL)uupcmsc1.lib
LIBOSLIST= $(OBJ)\ndiros2.obj $(OBJ)\scrsize2.obj $(OBJ)\pos2err.obj
LINKOPT  = /exepack /far /noignorecase /nod /batch
MODEL    = M                    # Really OS/2 version in this case
OBJ      = $(SRCSLASH)objmsc2$(MODEL)
PROD     = \uupc\os2bin
STACKSIZE= 0x03000
WIDTH    = 16
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
