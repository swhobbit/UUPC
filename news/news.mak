# *--------------------------------------------------------------------*
# *     r n e w s . m a k                                              *
# *                                                                    *
# *     Changes Copyright (c) 1989-1995 by Kendra Electronic           *
# *     Wonderworks.                                                   *
# *                                                                    *
# *     All rights reserved except those explicitly granted by the     *
# *     UUPC/extended license agreement.                               *
# *--------------------------------------------------------------------*

#     $Id: news.mak 1.10 1995/02/12 23:27:23 ahd Exp $
#
#     Revision history:
#     $Log: news.mak $
#     Revision 1.10  1995/02/12 23:27:23  ahd
#     split rnews into rnews/news, rename rnews dir to news, make inews COM file
#
#     Revision 1.9  1995/01/08 21:01:03  ahd
#     Drop batch.obj from objects for rnews
#
#     Revision 1.8  1994/12/31 03:34:53  ahd
#     First pass of integrating Mike McLagan's news SYS file suuport
#
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

LIBNEWSLIST=\
    $(OBJ)\active.obj $(OBJ)\cache.obj $(OBJ)\hdbm.obj          \
    $(OBJ)\history.obj $(OBJ)\idx.obj $(OBJ)\importng.obj $(OBJ)\sys.obj

LIBNEWS=$(LIB)\UUPC$(MODEL)N.lib

.c.obj:
  $(CC) $(CCX) -I$:{ $<}

.obj.com:
        $(CC) -c- -mt -lt -v- $(CCX) -n$(@D) -e$@ $< $(UUPCLIB)

.obj.exe:
        $(CC) -c- -m$(MODEL) -v- $(CCX) -n$(@D) -e$@ $< $(UUPCLIB)

.path.c = $(NEWS)

.path.obj = $(OBJ)

SENDBATSOBJ = $(OBJ)\sendbats.obj $(OBJ)\batch.obj

expire.exe:   expire.obj $(LIBNEWS) $(UUPCLIB)
        $(CC) -e$@ -v -n$(@D) -c- $**
!if !$d(__OS2__)
        -tdstrip -s $<
!endif

genhist.exe:   genhist.obj $(LIBNEWS) $(UUPCLIB)
        $(CC) -e$@ -v -n$(@D) -c- $**
!if !$d(__OS2__)
        -tdstrip -s $<
!endif

newsrun.exe:   newsrun.obj $(LIBNEWS) $(UUPCLIB)
        $(CC) -e$@ -v -n$(@D) -c- $**
!if !$d(__OS2__)
        -tdstrip -s $<
!endif

sendbats.exe:   $(SENDBATSOBJ) $(LIBNEWS) $(UUPCLIB)
        $(CC) -e$@ -v -n$(@D) -c- $**
!if !$d(__OS2__)
        -tdstrip -s $<
!endif

$(LIBNEWS): $(LIBNEWSLIST)
        &TLIB /C /E $< -+$?
