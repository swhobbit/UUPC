# *--------------------------------------------------------------------*
# *     u u c i c o . m a k                                            *
# *                                                                    *
# *     Changes Copyright (c) 1989-2002 by Kendra Electronic           *
# *     Wonderworks.                                                   *
# *                                                                    *
# *     All rights reserved except those explicitly granted by the     *
# *     UUPC/extended license agreement.                               *
# *--------------------------------------------------------------------*

#     $Id: uucico.mak 1.27 2001/03/12 13:58:31 ahd v1-13k $
#
#     Revision history:
#     $Log: uucico.mak $
#     Revision 1.27  2001/03/12 13:58:31  ahd
#     Annual copyright update
#
#     Revision 1.26  2000/05/12 12:08:51  ahd
#     Annual copyright update
#
#     Revision 1.25  1999/01/04 03:51:49  ahd
#     Annual copyright change
#
#     Revision 1.24  1998/03/01 01:38:52  ahd
#     Annual Copyright Update
#
#     Revision 1.23  1997/04/24 01:38:30  ahd
#     Annual Copyright Update
#
#     Revision 1.22  1996/01/01 21:25:45  ahd
#     Annual Copyright Update
#
#     Revision 1.21  1994/12/22 00:42:28  ahd
#     Annual Copyright Update
#
#     Revision 1.20  1994/10/02 20:46:04  ahd
#     Suppress blow up assigning icon uucico.exe
#
# Revision 1.19  1994/05/04  02:01:23  ahd
# Allow disable of TDSTRIP
#
# Revision 1.18  1994/01/01  19:24:50  ahd
# Annual Copyright Update
#
# Revision 1.18  1994/01/01  19:24:50  ahd
# Annual Copyright Update
#
# Revision 1.17  1993/12/02  13:47:46  ahd
# 'e' protocol support
#
# Revision 1.16  1993/10/28  00:51:58  ahd
# Add icon to UUCICOW.EXE
#
# Revision 1.15  1993/10/03  20:36:50  ahd
# Use real suspend module under OS/2
#
# Revision 1.14  1993/10/02  23:27:59  ahd
# Add continue character
#
# Revision 1.13  1993/10/02  23:12:12  dmwatt
# Add TCP/Ip error messages
#
# Revision 1.12  1993/09/29  13:17:37  ahd
# Correct build of uuport.com under DOS
#
# Revision 1.11  1993/09/27  00:44:27  ahd
# Add uuport build for OS/2.
#
# Revision 1.10  1993/09/25  03:07:09  ahd
# Add OS/2 priority function
#
# Revision 1.9  1993/09/24  03:42:24  ahd
# Add OS/2 named pipes support
#
# Revision 1.8  1993/09/20  04:36:42  ahd
# TCP/IP support from Dave Watt
# 't' protocol support
# BC++ 1.0 for OS/2 support
#
# Revision 1.7  1993/08/02  03:22:25  ahd
# Chaanges in support of Robert Denny's Windows 3.x support
#
# Revision 1.6  1993/07/31  16:21:21  ahd
# Windows 3.x support
#

.nosilent

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

.c.com:
        $(CC) -I$: -c- -mt -lt -v- $(CCX) -n$(SRC) $< $(UUPCLIB)
        del $(@:.com=.obj)

# *--------------------------------------------------------------------*
# *             *Implicit Rules*                                       *
# *--------------------------------------------------------------------*

.path.asm = $(UUCICO)
.path.c   = $(UUCICO)

UUCICOCOM = \
         $(OBJ)\checktim.obj $(OBJ)\commlib.obj $(OBJ)\dcp.obj \
         $(OBJ)\dcpfpkt.obj $(OBJ)\dcpgpkt.obj $(OBJ)\dcplib.obj \
         $(OBJ)\dcpstats.obj $(OBJ)\dcpsys.obj $(OBJ)\dcpxfer.obj \
         $(OBJ)\modem.obj $(OBJ)\nbstime.obj $(OBJ)\script.obj \
         $(OBJ)\uucico.obj

!if $d(__OS2__)
UUCICOOBJ = $(UUCICOCOM) \
         $(OBJ)\dcpepkt.obj $(OBJ)\dcptpkt.obj $(OBJ)\prtyos2.obj \
         $(OBJ)\suspend2.obj $(OBJ)\ulibnmp.obj $(OBJ)\ulibos2.obj

!elif $d(WINDOWS)

# *--------------------------------------------------------------------*
# *      Delete ulibip.obj and pswerr.obj if you don't have WINSOCK.H. *
# *      You also need to define NOTCPIP to commlib.c so it doesn't    *
# *      try to include the suite's definition.                        *
# *--------------------------------------------------------------------*

UUCICOOBJ = $(UUCICOCOM) \
         $(OBJ)\dcpepkt.obj $(OBJ)\dcptpkt.obj $(OBJ)\pwserr.obj \
         $(OBJ)\suspend.obj $(OBJ)\ulibip.obj $(OBJ)\ulibwin.obj

!else
UUCICOOBJ = $(UUCICOCOM) \
         $(OBJ)\comm.obj $(OBJ)\fossil.obj $(OBJ)\suspend.obj \
         $(OBJ)\ulib.obj $(OBJ)\ulib14.obj $(OBJ)\ulibfs.obj

!endif

# *--------------------------------------------------------------------*
# *                Begin rules for building modules.                   *
# *                                                                    *
# *        The first definition is the one built by default.           *
# *--------------------------------------------------------------------*

uucico$(PSUFFIX).exe: $(COMMON) $(UUCICOOBJ) $(LIBRARIES)
        $(LINKER) $(LINKOPT) @&&|
$(STARTUP)+
$(UUCICOOBJ)
$<
$(MAP)
$(LIBRARY)
$(DEFFILE)
|
!if $(WINDOWSXX)
        rc -I$(UUCICO) &&|
uucicow  ICON uucicow.ico
| $<
!endif
!if !$d(__OS2__)
        $(TDSTRIP) -s $<
!endif

uuport$(PSUFFIX).exe: $(COMMON) $(OBJ)\uuport.obj $(LIBRARIES)
        $(LINKER) $(LINKOPT) @&&|
$(STARTUP)+
$(OBJ)\uuport.obj
$<
$(MAP)
$(LIBRARY)
$(DEFFILE)
|
!if !$d(__OS2__)
        $(TDSTRIP) -s $<
!endif

$(OBJ)\comm.obj: $(UUCICO)\comm.asm
