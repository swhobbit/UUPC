# *--------------------------------------------------------------------*
# *      u u t r a f  . m a k                                          *
# *                                                                    *
# *      Changes Copyright (c) 1989-1999 by Kendra Electronic          *
# *      Wonderworks.                                                  *
# *                                                                    *
# *      All rights reserved except those explicitly granted by        *
# *      the UUPC/extended license agreement.                          *
# *--------------------------------------------------------------------*

#     $Id: uutraf.mak 1.6 1998/03/01 01:46:48 ahd v1-13f ahd $
#
#     Revision history:
#     $Log: uutraf.mak $
#     Revision 1.6  1998/03/01 01:46:48  ahd
#     Annual Copyright Update
#
#     Revision 1.5  1997/04/24 01:42:30  ahd
#     Annual Copyright Update
#
#     Revision 1.4  1996/01/01 21:35:57  ahd
#     Annual Copyright Update
#
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
