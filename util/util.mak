# *--------------------------------------------------------------------*
# *     u t i l . m a k                                                *
# *                                                                    *
# *   Changes Copyright (c) 1989-1995 by Kendra Electronic             *
# *   Wonderworks.                                                     *
# *                                                                    *
# *   All rights reserved except those explicitly granted by the       *
# *   UUPC/extended license agreement.                                 *
# *--------------------------------------------------------------------*

#     $Id: util.mak 1.8 1994/12/27 20:47:27 ahd Exp $
#
#     Revision history:
#     $Log: util.mak $
#     Revision 1.8  1994/12/27 20:47:27  ahd
#     Smoother call grading'
#
#     Revision 1.7  1994/12/22 00:31:47  ahd
#     Annual Copyright Update
#
#     Revision 1.6  1994/01/01 19:17:29  ahd
#     Annual Copyright Update
#
# Revision 1.5  1993/11/21  04:46:03  ahd
# Normalize OS/2 build of NOVRSTRK
#
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

.obj.com:
        $(CC) -c- -mt -lt -v- $(CCX) -n$(@D) -e$@ $< $(UUPCLIB)

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

fromwho$(PSUFFIX).exe:  $(UUPCCFG) $(OBJ)\fromwho.obj $(LIBRARIES)
        $(LINKER) $(LINKOPT) @&&|
$(STARTUP)+
$(OBJ)\fromwho.obj
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
