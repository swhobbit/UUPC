# *--------------------------------------------------------------------*
# *     r n e w s . m a k                                              *
# *                                                                    *
# *     Changes Copyright (c) 1989-1993 by Kendra Electronic           *
# *     Wonderworks.                                                   *
# *                                                                    *
# *     All rights reserved except those explicitly granted by the     *
# *     UUPC/extended license agreement.                               *
# *--------------------------------------------------------------------*

#     $Id: rnews.mak 1.3 1993/07/31 16:21:21 ahd Exp $
#
#     Revision history:
#     $Log: rnews.mak $
# Revision 1.3  1993/07/31  16:21:21  ahd
# Windows 3.x support
#

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

.path.c = $(RNEWS)

RNEWSOBJ = $(OBJ)\rnews.obj $(OBJ)\history.obj

EXPIREOBJ = $(OBJ)\expire.obj

rnews$(PSUFFIX).exe: $(UUPCCFG)     $(RNEWSOBJ) $(LIBRARIES)
        - erase rnews.com
        $(LINKER) $(LINKOPT) @&&|
$(STARTUP)+
$(RNEWSOBJ)
$<
$(MAP)
$(LIBRARY)
|
!if !$d(__OS2__)
        tdstrip -s $<
!endif

expire.com: $(UUPCCFG)     $(EXPIREOBJ) $(LIBRARIES)
        - erase expire.exe
        $(LINKER) $(LINKOPTT) @&&|
$(STARTUPT)+
$(EXPIREOBJ)
$<
$(MAP)
$(LIBRARY)
|

expire$(PSUFFIX).exe: $(UUPCCFG)     $(EXPIREOBJ) $(LIBRARIES)
        - erase expire.com
        $(LINKER) $(LINKOPT) @&&|
$(STARTUP)+
$(EXPIREOBJ)
$<
$(MAP)
$(LIBRARY)
|
!if !$d(__OS2__)
        tdstrip -s $<
!endif
