#       $Id$
#
#       Copyright (c) 1989-1994 by Kendra Electronic Wonderworks;
#       all rights reserved except those explicitly granted by
#       the UUPC/extended license.
#
#       $Log$
#

DEFFILE  = $(SRCSLASH)UUPCOS2.DEF
ERASE    = del /f
EXTRA2   = $(PROD)\mail.ico $(PROD)\uucico.ico
EXTRA4   = $(PROD)\pnews.cmd
EXTRAS   = $(PROD)\pnews.cmd $(PROD)\mailchek.cmd $(PROD)\getuupc.cmd
LIBOSLIST= $(OBJ)\ndiros2.obj $(OBJ)\scrsize2.obj $(OBJ)\pos2err.obj
MODEL    = 2                    # Really OS/2 version in this case
OBJ      = $(SRCSLASH)objicc
OS232BIT = 1
PROD     = \uupc\os2bin32
STACKSIZE= 0x80000
WIDTH    = 32
ZIPID    = 2

#       Use this for debugging
# DBGOPT  = -Ti -DUDEBUG -D__DEBUG_ALLOC__
#       Use this for production
DBGOPT = -O

CCOPT   = -q -Ss+ -Se -Gd $(DBGOPT) -I$(UULIB) -c -Fi -Si -Fo$@
LDOPT   = $(DBGOPT) -B"/batch" -Fe $@
UUCICOOBJ3 = $(OBJ)\dcpepkt.obj $(OBJ)\dcptpkt.obj $(OBJ)\ulibos2.obj \
             $(OBJ)\ulibnmp.obj $(OBJ)\prtyos2.obj $(OBJ)\suspend2.obj
