# *--------------------------------------------------------------------*
# *     u u c p . m a k                                                *
# *                                                                    *
# *     Changes Copyright (c) 1989-1994 by Kendra Electronic           *
# *     Wonderworks.                                                   *
# *                                                                    *
# *     All rights reserved except those explicitly granted by the     *
# *     UUPC/extended license agreement.                               *
# *--------------------------------------------------------------------*

#     $Id: uucp.mak 1.4 1993/09/20 04:36:42 ahd Exp $
#
#     Revision history:
#     $Log: uucp.mak $
# Revision 1.4  1993/09/20  04:36:42  ahd
# TCP/IP support from Dave Watt
# 't' protocol support
# BC++ 1.0 for OS/2 support
#
# Revision 1.4  1993/09/20  04:36:42  ahd
# TCP/IP support from Dave Watt
# 't' protocol support
# BC++ 1.0 for OS/2 support
#
# Revision 1.3  1993/08/02  03:22:25  ahd
# Chaanges in support of Robert Denny's Windows 3.x support
#
# Revision 1.2  1993/07/31  16:21:21  ahd
# Windows 3.x support
#

!include $(UUPCDEFS)

!if $(NDEBUG)
LINKOPT=$(LINKOPTN)
!else
LINKOPT=$(LINKOPTD)
!endif

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
$(DEFFILE)
|

uucp$(PSUFFIX).exe: $(UUPCCFG)     $(UUCPOBJ) $(LIBRARIES)
        - erase uucp.com
        $(LINKER) $(LINKOPT) @&&|
$(STARTUP)+
$(UUCPOBJ)
$<
$(MAP)
$(LIBRARY)
$(DEFFILE)
|
!if !$d(__OS2__)
        tdstrip -s $<
!endif

uusub.com: $(UUPCCFG)  $(UUSUBOBJ) $(LIBRARIES)
        - erase uusub.exe
        $(LINKER) $(LINKOPTT) @&&|
$(STARTUPT)+
$(UUSUBOBJ)
$<
$(MAP)
$(LIBRARY)
|

uusub$(PSUFFIX).exe: $(UUPCCFG)   $(UUSUBOBJ) $(LIBRARIES)
        - erase uusub.com
        $(LINKER) $(LINKOPT) @&&|
$(STARTUP)+
$(UUSUBOBJ)
$<
$(MAP)
$(LIBRARY)
$(DEFFILE)
|
!if !$d(__OS2__)
        tdstrip -s $<
!endif

uuxqt$(PSUFFIX).exe: $(UUPCCFG)     $(UUXQTOBJ) $(LIBRARIES)
        $(LINKER) $(LINKOPT) @&&|
$(STARTUP)+
$(UUXQTOBJ)
$<
$(MAP)
$(LIBRARY)
$(DEFFILE)
|
!if !$d(__OS2__)
        tdstrip -s $<
!endif

uustat.com: $(UUPCCFG)  $(UUSTATOBJ) $(LIBRARIES)
        - erase uustat.exe
        $(LINKER) $(LINKOPTT) @&&|
$(STARTUPT)+
$(UUSTATOBJ)
$<
$(MAP)
$(LIBRARY)
|

uustat$(PSUFFIX).exe: $(UUPCCFG)     $(UUSTATOBJ) $(LIBRARIES)
        - erase uustat.com
        $(LINKER) $(LINKOPT) @&&|
$(STARTUP)+
$(UUSTATOBJ)
$<
$(MAP)
$(LIBRARY)
$(DEFFILE)
|
!if !$d(__OS2__)
        tdstrip -s $<
!endif

uux.com: $(UUPCCFG)  $(UUXOBJ) $(LIBRARIES)
        - erase uux.exe
        $(LINKER) $(LINKOPTT) @&&|
$(STARTUPT)+
$(UUXOBJ)
$<
$(MAP)
$(LIBRARY)
|

uux$(PSUFFIX).exe: $(UUPCCFG)     $(UUXOBJ) $(LIBRARIES)
        - erase uux.com
        $(LINKER) $(LINKOPT) @&&|
$(STARTUP)+
$(UUXOBJ)
$<
$(MAP)
$(LIBRARY)
|
!if !$d(__OS2__)
        tdstrip -s $<
!endif

uuname.com: $(UUPCCFG)  $(UUNAMEOBJ) $(LIBRARIES)
        - erase uuname.exe
        $(LINKER) $(LINKOPTT) @&&|
$(STARTUPT)+
$(UUNAMEOBJ)
$<
$(MAP)
$(LIBRARY)
|

uuname$(PSUFFIX).exe: $(UUPCCFG)     $(UUNAMEOBJ) $(LIBRARIES)
        - erase uuname.com
        $(LINKER) $(LINKOPT) @&&|
$(STARTUP)+
$(UUNAMEOBJ)
$<
$(MAP)
$(LIBRARY)
$(DEFFILE)
|
!if !$d(__OS2__)
        tdstrip -s $<
!endif
