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
            $(OBJ)\nbstime.obj $(OBJ)\script.obj $(OBJ)\ulib14.obj \
            $(OBJ)\ulibfs.obj $(OBJ)\ulib.obj $(OBJ)\uucico.obj \
            $(OBJ)\fossil.obj

UUCICOOBJ = $(UUCICOCOM) $(OBJ)\comm.obj

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

$(OBJ)\comm.obj: $(UUCICO)\comm.asm
