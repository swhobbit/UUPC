!include $(UUPCDEFS)

.c.obj:
  $(CC) $(CCX) -I$:{ $<}

.asm.obj:
        $(TASM) $(TASMOPT) $<,$(OBJ)\$&;

.path.c = $(RNEWS)

RNEWSOBJ = $(OBJ)\rnews.obj $(OBJ)\history.obj


EXPIREOBJ = $(OBJ)\expire.obj

rnews.com: $(UUPCCFG)     $(RNEWSOBJ) $(LIBRARIES)
        - erase rnews.exe
        $(LINKER) $(LINKOPTT) @&&|
$(STARTUPT)+
$(RNEWSOBJ)
$<
$(MAP)
$(LIBRARY)
|

rnews.exe: $(UUPCCFG)     $(RNEWSOBJ) $(LIBRARIES)
        - erase rnews.com
        $(LINKER) $(LINKOPT) @&&|
$(STARTUP)+
$(RNEWSOBJ)
$<
$(MAP)
$(LIBRARY)
|
        tdstrip -s $<

expire.com: $(UUPCCFG)     $(EXPIREOBJ) $(LIBRARIES)
        - erase expire.exe
        $(LINKER) $(LINKOPTT) @&&|
$(STARTUPT)+
$(EXPIREOBJ)
$<
$(MAP)
$(LIBRARY)
|

expire.exe: $(UUPCCFG)     $(EXPIREOBJ) $(LIBRARIES)
        - erase expire.com
        $(LINKER) $(LINKOPT) @&&|
$(STARTUP)+
$(EXPIREOBJ)
$<
$(MAP)
$(LIBRARY)
|
        tdstrip -s $<
