msc:
        $(MAKE) all \
        CC="cl -nologo -Zp -AM -O" O=".obj" LFLAGS="-Lp -F 8000"
ibmc:
        $(MAKE) all \
        CC="icc -q -O" O=".obj" LFLAGS="/B/ST:0x8000"

CFLAGS = -I. -DDEBUG -DSYSVR3 -DUUPC -I../lib
LIBS =  $(TMP)\muupcmsc.lib $(TMP)\muupcos2.lib mlibcep.lib os2

.SUFFIXES: .c $O

.c$O:
        $(CC) $(CFLAGS) -c $<

OBJECTS = uutraf$O version$O alist$O list$O llist$O

all: uutraf.exe

uutraf.exe: $(OBJECTS)
#       $(CC) $(CFLAGS) $(OBJECTS) -Fe$@ $(LFLAGS) $(LIBS)
        link /exepack /far /noignorecase /nod /batch $(OBJECTS),$@,NUL.,$(LIBS),..\uupcos2p.def

uutraf$O: uutraf.h defs.h version.h
version$O: version.h patchlevel.h
