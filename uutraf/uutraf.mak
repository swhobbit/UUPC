# *--------------------------------------------------------------------*
# *      u u t r a f  . m a k                                          *
# *                                                                    *
# *      Changes Copyright (c) 1989-1996 by Kendra Electronic          *
# *      Wonderworks.                                                  *
# *                                                                    *
# *      All rights reserved except those explicitly granted by        *
# *      the UUPC/extended license agreement.                          *
# *--------------------------------------------------------------------*

#     $Id: uutraf.mak 1.3 1994/12/22 00:46:39 ahd v1-12q $
#
#     Revision history:
#     $Log: uutraf.mak $
#     Revision 1.3  1994/12/22 00:46:39  ahd
#     Annual Copyright Update
#
#     Revision 1.2  1994/01/01 19:28:49  ahd
#     Annual Copyright Update
#
# Revision 1.1  1993/11/21  04:46:03  ahd
# Initial revision
#

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
