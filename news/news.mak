# *--------------------------------------------------------------------*
# *     n e w s . m a k                                                *
# *                                                                    *
# *     Changes Copyright (c) 1989-2002 by Kendra Electronic           *
# *     Wonderworks.                                                   *
# *                                                                    *
# *     All rights reserved except those explicitly granted by the     *
# *     UUPC/extended license agreement.                               *
# *--------------------------------------------------------------------*

#     $Id: news.mak 1.18 2001/03/12 13:58:31 ahd v1-13k $
#
#     Revision history:
#     $Log: news.mak $
#     Revision 1.18  2001/03/12 13:58:31  ahd
#     Annual copyright update
#
#     Revision 1.17  2000/05/12 12:21:16  ahd
#     Annual copyright update
#
#     Revision 1.16  1999/01/04 03:51:49  ahd
#     Annual copyright change
#
#     Revision 1.15  1998/03/01 01:29:50  ahd
#     Annual Copyright Update
#
#     Revision 1.14  1997/04/24 01:40:33  ahd
#     Annual Copyright Update
#
#     Revision 1.13  1996/01/01 21:10:51  ahd
#     Annual Copyright Update
#
#     Revision 1.12  1995/09/24 19:05:42  ahd
#     Correct internal name
#
#     Revision 1.11  1995/02/21 02:45:44  ahd
#     Move active.c, importng.c to news directory, add news index
#     cache support, create news subroutine library, and use BCC
#     to drive tlink.
#
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
