#     $Header: E:\SRC\UUPC\lib\RCS\lib.mak 1.6 1993/04/05 04:31:55 ahd Exp ahd $
#
#     Revision history:
#     $Log: lib.mak $
#    Revision 1.6  1993/04/05  04:31:55  ahd
#    Add time stamp, size to data returned by directory searches
#
#    Revision 1.5  1993/03/06  22:47:34  ahd
#    Move active into shared library
#
#       Revision 1.4  1992/11/27  14:37:34  ahd
#       Add scrsize() to build
#
#       Revision 1.3  1992/11/23  02:46:06  ahd
#       Addd strpool and normalize to build list
#
#       Revision 1.2  1992/11/17  13:47:42  ahd
#       Drop type of input file
#

!include $(UUPCDEFS)

.c.obj:
  $(CC) -c $(CCX) -I$: { $<}

.asm.obj:
        $(TASM) $(TASMOPT) $<,$(OBJ)\$&;

.path.c   = $(LIB)

#       The names of various object files that we create.

LIBLST1= $(OBJ)\active.obj\
         $(OBJ)\arbmath.obj $(OBJ)\arpadate.obj $(OBJ)\bugout.obj\
         $(OBJ)\catcher.obj $(OBJ)\chdir.obj $(OBJ)\checkptr.obj\
         $(OBJ)\configur.obj $(OBJ)\dater.obj $(OBJ)\dos2unix.obj\
         $(OBJ)\expath.obj
LIBLST2= $(OBJ)\export.obj $(OBJ)\filebkup.obj $(OBJ)\fopen.obj\
         $(OBJ)\getargs.obj $(OBJ)\getopt.obj $(OBJ)\getseq.obj\
         $(OBJ)\hostable.obj $(OBJ)\hostatus.obj
LIBLST3= $(OBJ)\hostrset.obj $(OBJ)\import.obj $(OBJ)\importng.obj\
         $(OBJ)\kanjicnv.obj $(OBJ)\lock.obj $(OBJ)\logger.obj\
         $(OBJ)\mkdir.obj $(OBJ)\mkfilenm.obj $(OBJ)\mkmbox.obj
LIBLST4= $(OBJ)\mktempnm.obj $(OBJ)\ndir.obj $(OBJ)\printerr.obj\
         $(OBJ)\printmsg.obj $(OBJ)\pushpop.obj $(OBJ)\readnext.obj\
         $(OBJ)\rename.obj $(OBJ)\safeio.obj $(OBJ)\normaliz.obj
LIBLST5= $(OBJ)\safeout.obj $(OBJ)\security.obj $(OBJ)\ssleep.obj\
         $(OBJ)\stater.obj $(OBJ)\usertabl.obj $(OBJ)\validcmd.obj\
         $(OBJ)\scrsize.obj $(OBJ)\strpool.obj\
         $(TIMESTMP)

LIBALL = $(LIBLST1) $(LIBLST2) $(LIBLST3) $(LIBLST4) $(LIBLST5)

# *--------------------------------------------------------------------*
# *     Force a regeneration of the time stamp/version module.         *
# *--------------------------------------------------------------------*

regen:  $(LIB)\timestmp.c
        - erase $(TIMESTMP)

# *--------------------------------------------------------------------*
# *     The timestamp module has a reference to this MAKEFILE,         *
# *     which insures whenever we change the version number the        *
# *     time stamp gets re-generated.                                  *
# *--------------------------------------------------------------------*

$(TIMESTMP): $(LIB)\timestmp.c $(UUPCCFG) $(REGEN) \
                $(MAKEFILE) \
                $(LIB)\lib.mak \
                $(MAIL)\mail.mak \
                $(UUCP)\uucp.mak \
                $(UUCICO)\uucico.mak \
                $(UTIL)\util.mak

# *--------------------------------------------------------------------*
# *     Common library build                                           *
# *--------------------------------------------------------------------*

$(UUPCLIB): $(LIBALL)
        &TLIB /C /E $< -+$?
        - erase $(TEMP)\$&.BAK

$(UUPCLIBW): $(LIBALL)
         &TLIB /C /E $< -+$?
        - erase $(TEMP)\$&.BAK

# *--------------------------------------------------------------------*
# *               We don't optimize the sleep routine!                 *
# *--------------------------------------------------------------------*

ssleep.obj: ssleep.c
        $(CC) -c $(CCX) -Od -I$: { $<}
