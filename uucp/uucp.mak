!include $(UUPCDEFS)

.c.obj:
  $(CC) $(CCX) -I$: -n$(OBJ) $<

.asm.obj:
        $(TASM) $(TASMOPT) $<,$(OBJ)\$&;

.path.c = $(UUCP)

UUCPOBJ = $(OBJ)\uucp.obj

UUSTATOBJ = $(OBJ)\uustat.obj

UUSUBOBJ = $(OBJ)\uusub.obj

UUXQTOBJ   = $(OBJ)\uuxqt.obj

UUXOBJ = $(OBJ)\uux.obj

UUNAMEOBJ = $(OBJ)\uuname.obj

uucp.com: $(UUPCCFG)  $(UUCPOBJ) $(LIBRARIES)
        - erase uucp.exe
        $(LINKER) $(LINKOPTT) @&&|
$(STARTUPT)+
$(UUCPOBJ)
$<
$(MAP)
$(LIBRARY)
|

uucp.exe: $(UUPCCFG)     $(UUCPOBJ) $(LIBRARIES)
        - erase uucp.com
        $(LINKER) $(LINKOPT) @&&|
$(STARTUP)+
$(UUCPOBJ)
$<
$(MAP)
$(LIBRARY)
|
        tdstrip -s $<

uusub.com: $(UUPCCFG)  $(UUSUBOBJ) $(LIBRARIES)
        - erase uusub.exe
        $(LINKER) $(LINKOPTT) @&&|
$(STARTUPT)+
$(UUSUBOBJ)
$<
$(MAP)
$(LIBRARY)
|

uusub.exe: $(UUPCCFG)   $(UUSUBOBJ) $(LIBRARIES)
        - erase uusub.com
        $(LINKER) $(LINKOPT) @&&|
$(STARTUP)+
$(UUSUBOBJ)
$<
$(MAP)
$(LIBRARY)
|
        tdstrip -s $<

uuxqt.exe: $(UUPCCFG)     $(UUXQTOBJ) $(LIBRARIES)
        $(LINKER) $(LINKOPT) @&&|
$(STARTUP)+
$(UUXQTOBJ)
$<
$(MAP)
$(LIBRARY)
|
        tdstrip -s $<

uustat.com: $(UUPCCFG)  $(UUSTATOBJ) $(LIBRARIES)
        - erase uustat.exe
        $(LINKER) $(LINKOPTT) @&&|
$(STARTUPT)+
$(UUSTATOBJ)
$<
$(MAP)
$(LIBRARY)
|

uustat.exe: $(UUPCCFG)     $(UUSTATOBJ) $(LIBRARIES)
        - erase uustat.com
        $(LINKER) $(LINKOPT) @&&|
$(STARTUP)+
$(UUSTATOBJ)
$<
$(MAP)
$(LIBRARY)
|
        tdstrip -s $<

uux.com: $(UUPCCFG)  $(UUXOBJ) $(LIBRARIES)
        - erase uux.exe
        $(LINKER) $(LINKOPTT) @&&|
$(STARTUPT)+
$(UUXOBJ)
$<
$(MAP)
$(LIBRARY)
|

uux.exe: $(UUPCCFG)     $(UUXOBJ) $(LIBRARIES)
        - erase uux.com
        $(LINKER) $(LINKOPT) @&&|
$(STARTUP)+
$(UUXOBJ)
$<
$(MAP)
$(LIBRARY)
|
        tdstrip -s $<

uuname.com: $(UUPCCFG)  $(UUNAMEOBJ) $(LIBRARIES)
        - erase uuname.exe
        $(LINKER) $(LINKOPTT) @&&|
$(STARTUPT)+
$(UUNAMEOBJ)
$<
$(MAP)
$(LIBRARY)
|

uuname.exe: $(UUPCCFG)     $(UUNAMEOBJ) $(LIBRARIES)
        - erase uuname.com
        $(LINKER) $(LINKOPT) @&&|
$(STARTUP)+
$(UUNAMEOBJ)
$<
$(MAP)
$(LIBRARY)
|
        tdstrip -s $<
