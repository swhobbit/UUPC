# *--------------------------------------------------------------------*
# *     m a i l . m a k                                                *
# *                                                                    *
# *     UUPC/extended BC++ makefile for mail programs                  *
# *                                                                    *
# *     Changes Copyright (c) 1989-1992 by Kendra Electronic           *
# *     Wonderworks.                                                   *
# *                                                                    *
# *     All rights reserved except those explicitly granted by the     *
# *     UUPC/extended license agreement.                               *
# *                                                                    *
# *--------------------------------------------------------------------*

# *--------------------------------------------------------------------*
# *                         RCS Information                            *
# *--------------------------------------------------------------------*

#     $Id: mail.mak 1.3 1993/07/31 16:21:21 ahd Exp $
#
#     $Log: mail.mak $
# Revision 1.3  1993/07/31  16:21:21  ahd
# Windows 3.x support
#
# Revision 1.2  1992/12/04  01:00:13  ahd
# Add sysalias to rmail build

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

.path.c = $(MAIL)

MAILOBJ = $(OBJ)\alias.obj $(OBJ)\mail.obj $(OBJ)\mailblib.obj\
          $(OBJ)\maillib.obj $(OBJ)\mailsend.obj $(OBJ)\mlib.obj\
          $(OBJ)\address.obj

RMAILOBJ = $(OBJ)\deliver.obj $(OBJ)\rmail.obj $(OBJ)\address.obj \
           $(OBJ)\sysalias.obj

mail$(PSUFFIX).exe: $(UUPCCFG)     $(MAILOBJ) $(LIBRARIES)
        $(LINKER) $(LINKOPT) @&&|
$(STARTUP)+
$(MAILOBJ)
$<
$(MAP)
$(LIBRARY)
$(DEFFILE)
|
!if !$d(__OS2__)
        tdstrip -s $<
!endif

rmail$(PSUFFIX).exe: $(UUPCCFG)     $(RMAILOBJ) $(LIBRARIES)
        - erase rmail.com
        $(LINKER) $(LINKOPT) @&&|
$(STARTUP)+
$(RMAILOBJ)
$<
$(MAP)
$(LIBRARY)
$(DEFFILE)
|
!if !$d(__OS2__)
        tdstrip -s $<
!endif
