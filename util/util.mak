# *--------------------------------------------------------------------*
# *     u t i l . m a k                                                *
# *                                                                    *
# *   Changes Copyright (c) 1989-1993 by Kendra Electronic             *
# *   Wonderworks.                                                     *
# *                                                                    *
# *   All rights reserved except those explicitly granted by the       *
# *   UUPC/extended license agreement.                                 *
# *--------------------------------------------------------------------*

#     $Id: util.mak 1.4 1993/09/27 00:44:27 ahd Exp $
#
#     Revision history:
#     $Log: util.mak $
# Revision 1.4  1993/09/27  00:44:27  ahd
# Correct build of small .com files.
#
# Revision 1.3  1993/09/20  04:36:42  ahd
# TCP/IP support from Dave Watt
# 't' protocol support
# BC++ 1.0 for OS/2 support
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

.path.c = $(UTIL)

.c.obj:
  $(CC) $(CCX) -I$:{ $<}

.c.com:
        $(CC) -c- -mt -lt -v- $(CCX) -n$(SRC) $< $(UUPCLIB)
        del $(@:.com=.obj)

fmt$(PSUFFIX).exe:  $(UUPCCFG) $(OBJ)\fmt.obj $(LIBRARIES)
        $(LINKER) $(LINKOPT) @&&|
$(STARTUP)+
$(OBJ)\fmt.obj
$<
$(MAP)
$(LIBRARY)
|
!if !$d(__OS2__)
        tdstrip -s $<
!endif

gensig$(PSUFFIX).exe:  $(UUPCCFG) $(OBJ)\gensig.obj $(LIBRARIES)
        $(LINKER) $(LINKOPT) @&&|
$(STARTUP)+
$(OBJ)\gensig.obj
$<
$(MAP)
$(LIBRARY)
|
!if !$d(__OS2__)
        tdstrip -s $<
!endif

novrstr$(PSUFFIX).exe:  $(UUPCCFG) $(OBJ)\novrstrk.obj $(LIBRARIES)
        $(LINKER) $(LINKOPT) @&&|
$(STARTUP)+
$(OBJ)\novrstrk.obj
$<
$(MAP)
$(LIBRARY)
|
!if !$d(__OS2__)
        tdstrip -s $<
!endif

uupoll$(PSUFFIX).exe: $(UUPCCFG)     $(OBJ)\uupoll.obj $(LIBRARIES)
        $(LINKER) $(LINKOPT) @&&|
$(STARTUP)+
$(OBJ)\uupoll.obj
$<
$(MAP)
$(LIBRARY)
|
!if !$d(__OS2__)
        tdstrip -s $<
!endif
