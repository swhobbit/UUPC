# *--------------------------------------------------------------------*
# *     u u c i c o . m a k                                            *
# *                                                                    *
# *     Changes Copyright (c) 1989-1993 by Kendra Electronic           *
# *     Wonderworks.                                                   *
# *                                                                    *
# *     All rights reserved except those explicitly granted by the     *
# *     UUPC/extended license agreement.                               *
# *--------------------------------------------------------------------*

#     $Id: e:\src\uupc\lib\RCS\lib.mak 1.7 1993/07/22 23:19:01 ahd Exp $
#
#     Revision history:
#     $Log: lib.mak $

!include $(UUPCDEFS)

.c.obj:
  $(CC) $(CCX) -I$:{ $<}

.asm.obj:
        $(TASM) $(TASMOPT) $<,$(OBJ)\$&;

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

UUCICOOBJ = $(UUCICOCOM) $(OBJ)\comm.obj $(OBJ)\fossil.obj \
            $(OBJ)\ulib.obj $(OBJ)\ulib14.obj $(OBJ)\ulibfs.obj

UUCICOWOBJ = $(UUCICOCOM) $(OBJ)\ulibwin.obj

# *--------------------------------------------------------------------*
# *                Begin rules for building modules.                   *
# *                                                                    *
# *        The first definition is the one built by default.           *
# *--------------------------------------------------------------------*

uucico.exe: $(COMMON) $(UUCICOOBJ) $(LIBRARIES)
        $(LINKER) $(LINKOPT) @&&|
$(STARTUP)+
$(UUCICOOBJ)
$<
$(MAP)
$(LIBRARY)
|
        tdstrip -s $<

uucicow.exe: $(COMMON) $(UUCICOWOBJ) $(LIBRARIES)
        $(LINKER) $(LINKOPT) @&&|
$(STARTUP)+
$(UUCICOWOBJ)
$<
$(MAP)
$(LIBRARY)
|
        tdstrip -s $<

$(OBJ)\comm.obj: $(UUCICO)\comm.asm
