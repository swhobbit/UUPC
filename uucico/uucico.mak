# *--------------------------------------------------------------------*
# *     u u c i c o . m a k                                            *
# *                                                                    *
# *     Changes Copyright (c) 1989-1993 by Kendra Electronic           *
# *     Wonderworks.                                                   *
# *                                                                    *
# *     All rights reserved except those explicitly granted by the     *
# *     UUPC/extended license agreement.                               *
# *--------------------------------------------------------------------*

#     $Id: uucico.mak 1.12 1993/09/29 13:17:37 ahd Exp $
#
#     Revision history:
#     $Log: uucico.mak $
# Revision 1.12  1993/09/29  13:17:37  ahd
# Correct build of uuport.com under DOS
#
# Revision 1.11  1993/09/27  00:44:27  ahd
# Add uuport build for OS/2.
#
# Revision 1.10  1993/09/25  03:07:09  ahd
# Add OS/2 priority function
#
# Revision 1.9  1993/09/24  03:42:24  ahd
# Add OS/2 named pipes support
#
# Revision 1.8  1993/09/20  04:36:42  ahd
# TCP/IP support from Dave Watt
# 't' protocol support
# BC++ 1.0 for OS/2 support
#
# Revision 1.7  1993/08/02  03:22:25  ahd
# Chaanges in support of Robert Denny's Windows 3.x support
#
# Revision 1.6  1993/07/31  16:21:21  ahd
# Windows 3.x support
#

.nosilent

!include $(UUPCDEFS)

!if $(NDEBUG)
LINKOPT=$(LINKOPTN)
!else
LINKOPT=$(LINKOPTD)
!endif

.c.obj:
  $(CC) $(CCX) -I$:{ $<}

.asm.obj:
        $(TASM) $(TASMOPT) $<,$(OBJ)\$&;

.c.com:
        $(CC) -I$: -c- -mt -lt -v- $(CCX) -n$(SRC) $< $(UUPCLIB)
        del $(@:.com=.obj)

# *--------------------------------------------------------------------*
# *             *Implicit Rules*                                       *
# *--------------------------------------------------------------------*

.path.asm = $(UUCICO)
.path.c   = $(UUCICO)

UUCICOCOM = $(OBJ)\checktim.obj $(OBJ)\commlib.obj $(OBJ)\dcp.obj \
            $(OBJ)\dcpfpkt.obj \
            $(OBJ)\dcpgpkt.obj $(OBJ)\dcplib.obj $(OBJ)\dcpsys.obj\
            $(OBJ)\dcpxfer.obj $(OBJ)\dcpstats.obj $(OBJ)\modem.obj\
            $(OBJ)\nbstime.obj $(OBJ)\script.obj $(OBJ)\uucico.obj

!if $d(__OS2__)
UUCICOOBJ = $(UUCICOCOM) $(OBJ)\ulibos2.obj $(OBJ)\ulibnmp.obj\
            $(OBJ)\dcptpkt.obj $(OBJ)\prtyos2.obj $(OBJ)\suspend.obj
!elif $d(WINDOWS)

# *--------------------------------------------------------------------*
# *      Delete ulibip.obj and pswerr.obj if you don't have WINSOCK.H. *
# *      You also need to define NOTCPIP to commlib.c so it doesn't    *
# *      try to include the suite's definition.                        *
# *--------------------------------------------------------------------*

UUCICOOBJ = $(UUCICOCOM) $(OBJ)\dcptpkt.obj
            $(OBJ)\ulibwin.obj $(OBJ)\suspend.obj \
            $(OBJ)\ulibip.obj $(OBJ)\pwserr.obj
!else
UUCICOOBJ = $(UUCICOCOM) $(OBJ)\comm.obj $(OBJ)\fossil.obj \
            $(OBJ)\ulib.obj $(OBJ)\ulib14.obj $(OBJ)\ulibfs.obj \
            $(OBJ)\suspend.obj
!endif

# *--------------------------------------------------------------------*
# *                Begin rules for building modules.                   *
# *                                                                    *
# *        The first definition is the one built by default.           *
# *--------------------------------------------------------------------*

uucico$(PSUFFIX).exe: $(COMMON) $(UUCICOOBJ) $(LIBRARIES)
        $(LINKER) $(LINKOPT) @&&|
$(STARTUP)+
$(UUCICOOBJ)
$<
$(MAP)
$(LIBRARY)
$(DEFFILE)
|
!if !$d(__OS2__)
        tdstrip -s $<
!endif

uuport$(PSUFFIX).exe: $(COMMON) $(OBJ)\uuport.obj $(LIBRARIES)
        $(LINKER) $(LINKOPT) @&&|
$(STARTUP)+
$(OBJ)\uuport.obj
$<
$(MAP)
$(LIBRARY)
$(DEFFILE)
|
!if !$d(__OS2__)
        tdstrip -s $<
!endif

$(OBJ)\comm.obj: $(UUCICO)\comm.asm
