#       $Id: nmakos21.mak 1.6 1995/01/22 04:15:00 ahd Exp ahd $
#
#       Copyright (c) 1989-1995 by Kendra Electronic Wonderworks;
#       all rights reserved except those explicitly granted by
#       the UUPC/extended license.
#
#       $Log: nmakos21.mak $
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

ERASE    = del /f               # Assumes actually building on OS/2 2.x
EXTRA1   = $(PROD)\mail.ico $(PROD)\uucico.ico
EXTRAS   = $(PROD)\pnews.cmd $(PROD)\mailchek.cmd $(PROD)\getuupc.cmd   \
            $(PROD)\uuhour.cmd
LIBOSLIST= $(OBJ)\ndiros2.obj $(OBJ)\scrsize2.obj $(OBJ)\pos2err.obj    \
           $(OBJ)\title2.obj
MODEL    = M                    # All OS/2 16 bit programs are medium
PROD     = \uupc\os2bin
STACKSIZE= 0x02A00
ZIPID    = 1

UUCICOOBJ3 = $(OBJ)\dcpepkt.obj $(OBJ)\dcptpkt.obj $(OBJ)\ulibos2.obj \
             $(OBJ)\ulibnmp.obj $(OBJ)\prtyos2.obj $(OBJ)\suspend2.obj

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
