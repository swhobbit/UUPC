# *--------------------------------------------------------------------*
# *      u u t r a f  . m a k                                          *
# *                                                                    *
# *      Changes Copyright (c) 1989-1993 by Kendra Electronic          *
# *      Wonderworks.                                                  *
# *                                                                    *
# *      All rights reserved except those explicitly granted by        *
# *      the UUPC/extended license agreement.                          *
# *--------------------------------------------------------------------*

#     $Id: util.mak 1.4 1993/09/27 00:44:27 ahd Exp $
#
#     Revision history:
#     $Log: util.mak $

!include $(UUPCDEFS)

!if $(NDEBUG)
LINKOPT=$(LINKOPTN)
!else
LINKOPT=$(LINKOPTD)
!endif

!if $d(__OS2__)
MATHLIBS=
!else
MATHLIBS=$(BLIB)\MATH$(SUFFIX).LIB+$(BLIB)\EMU.LIB+
!endif

.path.c = $(UUTRAF)

.c.obj:
        $(CC) $(CCX) -w-pro -DDEBUG -DUUPC -DSYSVR3 -I$:{ $<}

UUTRAFOBJ = $(OBJ)\uutraf.obj $(OBJ)\version.obj $(OBJ)\alist.obj\
            $(OBJ)\list.obj $(OBJ)\llist.obj

uutraf$(PSUFFIX).exe:  $(UUPCCFG) $(UUTRAFOBJ) $(LIBRARIES)
        $(LINKER) $(LINKOPT) @&&|
$(STARTUP)+
$(UUTRAFOBJ)
$<
$(MAP)
$(MATHLIBS)$(LIBRARY)
$(DEFFILE)
|
!if !$d(__OS2__)
        tdstrip -s $<
!endif
