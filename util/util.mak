!include $(UUPCDEFS)

.path.c = $(UTIL)

.c.obj:
  $(CC) $(CCX) -I$:{ $<}

.asm.obj:
        $(TASM) $(TASMOPT) $<,$(OBJ)\$&;

.c.com:
        $(CC) -c- -mt -lt -v- $(CCX) -n$(SRC) $< $(UUPCLIB)

.c.exe:
        $(CC) -c- -m$(MODEL) $(CCX) -n$(SRC) $< $(UUPCLIB)

