# *--------------------------------------------------------------------*
# *     u u c p . m a k                                                *
# *                                                                    *
# *     Changes Copyright (c) 1989-2001 by Kendra Electronic           *
# *     Wonderworks.                                                   *
# *                                                                    *
# *     All rights reserved except those explicitly granted by the     *
# *     UUPC/extended license agreement.                               *
# *--------------------------------------------------------------------*

#     $Id: uucp.mak 1.12 2000/05/12 12:08:51 ahd v1-13g $
#
#     Revision history:
#     $Log: uucp.mak $
#     Revision 1.12  2000/05/12 12:08:51  ahd
#     Annual copyright update
#
#     Revision 1.11  1999/01/04 03:51:49  ahd
#     Annual copyright change
#
#     Revision 1.10  1998/03/01 01:46:06  ahd
#     Annual Copyright Update
#
#     Revision 1.9  1997/04/24 01:41:44  ahd
#     Annual Copyright Update
#
#     Revision 1.8  1996/01/01 21:34:51  ahd
#     Annual Copyright Update
#
#     Revision 1.7  1995/02/12 23:27:23  ahd
#     split rnews into rnews/news, rename rnews dir to news, make inews COM file
#
#     Revision 1.6  1994/12/22 00:45:39  ahd
#     Annual Copyright Update
#
#     Revision 1.5  1994/01/01 19:28:13  ahd
#     Annual Copyright Update
#
# Revision 1.4  1993/09/20  04:36:42  ahd
# TCP/IP support from Dave Watt
# 't' protocol support
# BC++ 1.0 for OS/2 support
#
# Revision 1.4  1993/09/20  04:36:42  ahd
# TCP/IP support from Dave Watt
# 't' protocol support
# BC++ 1.0 for OS/2 support
#
# Revision 1.3  1993/08/02  03:22:25  ahd
# Chaanges in support of Robert Denny's Windows 3.x support
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

.c.obj:
  $(CC) $(CCX) -I$: -n$(OBJ) $<

.asm.obj:
        $(TASM) $(TASMOPT) $<,$(OBJ)\$&;

.obj.com:
        $(CC) -c- -mt -lt -v- $(CCX) -n$(@D) -e$@ $< $(UUPCLIB)

.path.c = $(UUCP)

UUCPOBJ = $(OBJ)\uucp.obj

UUSTATOBJ = $(OBJ)\uustat.obj

UUSUBOBJ = $(OBJ)\uusub.obj

UUXQTOBJ   = $(OBJ)\uuxqt.obj

UUXOBJ = $(OBJ)\uux.obj

UUNAMEOBJ = $(OBJ)\uuname.obj

uucp$(PSUFFIX).exe: $(UUPCCFG)     $(UUCPOBJ) $(LIBRARIES)
        - erase uucp.com
        $(LINKER) $(LINKOPT) @&&|
$(STARTUP)+
$(UUCPOBJ)
$<
$(MAP)
$(LIBRARY)
$(DEFFILE)
|
!if !$d(__OS2__)
        tdstrip -s $<
!endif

uusub$(PSUFFIX).exe: $(UUPCCFG)   $(UUSUBOBJ) $(LIBRARIES)
        - erase uusub.com
        $(LINKER) $(LINKOPT) @&&|
$(STARTUP)+
$(UUSUBOBJ)
$<
$(MAP)
$(LIBRARY)
$(DEFFILE)
|
!if !$d(__OS2__)
        tdstrip -s $<
!endif

uuxqt$(PSUFFIX).exe: $(UUPCCFG)     $(UUXQTOBJ) $(LIBRARIES)
        $(LINKER) $(LINKOPT) @&&|
$(STARTUP)+
$(UUXQTOBJ)
$<
$(MAP)
$(LIBRARY)
$(DEFFILE)
|
!if !$d(__OS2__)
        tdstrip -s $<
!endif

uustat$(PSUFFIX).exe: $(UUPCCFG)     $(UUSTATOBJ) $(LIBRARIES)
        - erase uustat.com
        $(LINKER) $(LINKOPT) @&&|
$(STARTUP)+
$(UUSTATOBJ)
$<
$(MAP)
$(LIBRARY)
$(DEFFILE)
|
!if !$d(__OS2__)
        tdstrip -s $<
!endif

uux$(PSUFFIX).exe: $(UUPCCFG)     $(UUXOBJ) $(LIBRARIES)
        - erase uux.com
        $(LINKER) $(LINKOPT) @&&|
$(STARTUP)+
$(UUXOBJ)
$<
$(MAP)
$(LIBRARY)
|
!if !$d(__OS2__)
        tdstrip -s $<
!endif

uuname$(PSUFFIX).exe: $(UUPCCFG)     $(UUNAMEOBJ) $(LIBRARIES)
        - erase uuname.com
        $(LINKER) $(LINKOPT) @&&|
$(STARTUP)+
$(UUNAMEOBJ)
$<
$(MAP)
$(LIBRARY)
$(DEFFILE)
|
!if !$d(__OS2__)
        tdstrip -s $<
!endif
