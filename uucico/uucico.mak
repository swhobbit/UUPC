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

UUCICOCOM = $(OBJ)\checktim.obj $(OBJ)\dcp.obj \
            $(OBJ)\dcpfpkt.obj \
            $(OBJ)\dcpgpkt.obj $(OBJ)\dcplib.obj $(OBJ)\dcpsys.obj\
            $(OBJ)\dcpxfer.obj $(OBJ)\dcpstats.obj $(OBJ)\modem.obj\
            $(OBJ)\nbstime.obj $(OBJ)\script.obj\
            $(OBJ)\ulib.obj $(OBJ)\uucico.obj

UUCICOLDOBJ = $(UUCICOCOM) $(OBJ)\comm.obj
UUCICOOBJ = $(UUCICOCOM) $(OBJ)\commfifo.obj

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

# *--------------------------------------------------------------------*
# *   Most object modules are built automatically; these depend on     *
# *   defines (UUPCV, FPROTOCOL, etc.) in the configuration file       *
# *--------------------------------------------------------------------*

$(OBJ)\ulib.obj: $(UUCICO)\ulib.c $(UUPCCFG)

$(OBJ)\dcpsys.obj: $(UUCICO)\dcpsys.c $(UUPCCFG)

$(OBJ)\commfifo.obj: $(UUCICO)\commfifo.asm
