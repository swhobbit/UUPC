# *--------------------------------------------------------------------*
# *     m a i l . m a k                                                *
# *                                                                    *
# *     UUPC/extended BC++ makefile for mail programs                  *
# *                                                                    *
# *   Changes Copyright (c) 1989 by Andrew H. Derbyshire.              *
# *                                                                    *
# *   Changes Copyright (c) 1990-1992 by Kendra Electronic             *
# *   Wonderworks.                                                     *
# *                                                                    *
# *   All rights reserved except those explicitly granted by the       *
# *   UUPC/extended license agreement.                                 *
# *                                                                    *
# *--------------------------------------------------------------------*
# *                         RCS Information                            *
# *--------------------------------------------------------------------*

#
#     $Id: LIB.H 1.3 1992/12/01 04:39:34 ahd Exp $
#
#     $Log: LIB.H $
#

!include $(UUPCDEFS)

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

mail.exe: $(UUPCCFG)     $(MAILOBJ) $(LIBRARIES)
        $(LINKER) $(LINKOPT) @&&|
$(STARTUP)+
$(MAILOBJ)
$<
$(MAP)
$(LIBRARY)
|
        tdstrip -s $<

rmail.exe: $(UUPCCFG)     $(RMAILOBJ) $(LIBRARIES)
        - erase rmail.com
        $(LINKER) $(LINKOPT) @&&|
$(STARTUP)+
$(RMAILOBJ)
$<
$(MAP)
$(LIBRARY)
|
        tdstrip -s $<
