# *--------------------------------------------------------------------*
# *     r n e w s . m a k                                              *
# *                                                                    *
# *     Changes Copyright (c) 1989-1995 by Kendra Electronic           *
# *     Wonderworks.                                                   *
# *                                                                    *
# *     All rights reserved except those explicitly granted by the     *
# *     UUPC/extended license agreement.                               *
# *--------------------------------------------------------------------*

#     $Id: rnews.mak 1.6 1994/01/01 19:14:29 ahd Exp $
#
#     Revision history:
#     $Log: rnews.mak $
# Revision 1.6  1994/01/01  19:14:29  ahd
# Annual Copyright Update
#
# Revision 1.5  1993/10/31  11:57:13  ahd
# Add inews, genhist, and history support from Kai Uwe Rommel
#
# Revision 1.4  1993/09/20  04:36:42  ahd
# TCP/IP support from Dave Watt
# 't' protocol support
# BC++ 1.0 for OS/2 support
#
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

RNEWSOBJ = $(OBJ)\rnews.obj $(OBJ)\history.obj $(OBJ)\idx.obj \
           $(OBJ)\hdbm.obj $(OBJ)\sys.obj $(OBJ)\batch.obj

SENDBATSOBJ = $(OBJ)\sendbats.obj $(OBJ)\batch.obj $(OBJ)\sys.obj

EXPIREOBJ = $(OBJ)\expire.obj $(OBJ)\history.obj \
           $(OBJ)\idx.obj $(OBJ)\hdbm.obj

GENHISTOBJ = $(OBJ)\genhist.obj $(OBJ)\history.obj $(OBJ)\idx.obj $(OBJ)\hdbm.obj

INEWSOBJ = $(OBJ)\inews.obj

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

genhist$(PSUFFIX).exe: $(UUPCCFG)     $(GENHISTOBJ) $(LIBRARIES)
        - erase genhist.com
        $(LINKER) $(LINKOPT) @&&|
$(STARTUP)+
$(GENHISTOBJ)
$<
$(MAP)
$(LIBRARY)
|
!if !$d(__OS2__)
        tdstrip -s $<
!endif

inews$(PSUFFIX).exe: $(UUPCCFG)     $(INEWSOBJ) $(LIBRARIES)
        - erase inews.com
        $(LINKER) $(LINKOPT) @&&|
$(STARTUP)+
$(INEWSOBJ)
$<
$(MAP)
$(LIBRARY)
|
!if !$d(__OS2__)
        tdstrip -s $<
!endif

sendbats.exe: $(UUPCCFG)     $(SENDBATSOBJ) $(LIBRARIES)
        - erase sendbats.com
        $(LINKER) $(LINKOPT) @&&|
$(STARTUP)+
$(SENDBATSOBJ)
$<
$(MAP)
$(LIBRARY)
|
!if !$d(__OS2__)
        tdstrip -s $<
!endif
