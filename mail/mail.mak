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

#     $Id: mail.mak 1.6 1997/12/22 16:46:45 ahd v1-13f $
#
#     $Log: mail.mak $
#     Revision 1.6  1997/12/22 16:46:45  ahd
#     Delete OS/2 support
#     Add new deliverm.obj to RMAIL
#
#     Revision 1.5  1996/01/20 12:56:35  ahd
#     Correct name of nickname alias processing module to 'nickname.c'
#
#     Revision 1.4  1993/09/20 04:36:42  ahd
#     TCP/IP support from Dave Watt
#     't' protocol support
#     BC++ 1.0 for OS/2 support
#
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

MAILOBJ = $(OBJ)\nickname.obj $(OBJ)\mail.obj $(OBJ)\mailblib.obj\
          $(OBJ)\maillib.obj $(OBJ)\mailsend.obj $(OBJ)\mlib.obj\
          $(OBJ)\address.obj

RMAILOBJ = $(OBJ)\rmail.obj \
           $(OBJ)\deliver.obj \
           $(OBJ)\deliverm.obj \
           $(OBJ)\address.obj \
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
        tdstrip -s $<

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
        tdstrip -s $<
