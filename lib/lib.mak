#     $Header: E:\src\uupc\LIB\RCS\LIB.MAK 1.3 1992/11/23 02:46:06 ahd Exp $
#
#     Revision history:
#     $Log: LIB.MAK $
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

LIBLST1=+$(OBJ)\arbmath.obj+$(OBJ)\arpadate.obj+$(OBJ)\bugout.obj\
        +$(OBJ)\catcher.obj+$(OBJ)\chdir.obj+$(OBJ)\checkptr.obj\
        +$(OBJ)\configur.obj+$(OBJ)\dater.obj+$(OBJ)\expath.obj
LIBLST2=+$(OBJ)\export.obj+$(OBJ)\filebkup.obj+$(OBJ)\fopen.obj\
        +$(OBJ)\getargs.obj+$(OBJ)\getopt.obj+$(OBJ)\getseq.obj\
        +$(OBJ)\hostable.obj+$(OBJ)\hostatus.obj
LIBLST3=+$(OBJ)\hostrset.obj+$(OBJ)\import.obj+$(OBJ)\importng.obj\
        +$(OBJ)\kanjicnv.obj+$(OBJ)\lock.obj+$(OBJ)\logger.obj\
        +$(OBJ)\mkdir.obj+$(OBJ)\mkfilenm.obj+$(OBJ)\mkmbox.obj
LIBLST4=+$(OBJ)\mktempnm.obj+$(OBJ)\ndir.obj+$(OBJ)\printerr.obj\
        +$(OBJ)\printmsg.obj+$(OBJ)\pushpop.obj+$(OBJ)\readnext.obj\
        +$(OBJ)\rename.obj+$(OBJ)\safeio.obj+$(OBJ)\normaliz.obj
LIBLST5=+$(OBJ)\safeout.obj+$(OBJ)\security.obj+$(OBJ)\ssleep.obj\
        +$(OBJ)\stater.obj+$(OBJ)\usertabl.obj+$(OBJ)\validcmd.obj\
        +$(OBJ)\scrsize.obj+$(OBJ)\strpool.obj\
        +$(TIMESTMP)

LIBALL = $(LIBLST1:+= ) $(LIBLST2:+= ) $(LIBLST3:+= ) $(LIBLST4:+= ) $(LIBLST5:+= )

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

$(UUPCLIB): $(LIBALL) $(LIB)\lib.mak
        erase $<
        TLIB /C /E $< @&&|
$(LIBLST1) &
$(LIBLST2) &
$(LIBLST3) &
$(LIBLST4) &
$(LIBLST5)
$&
|
        - erase $(TEMP)\$&.BAK

# *--------------------------------------------------------------------*
# *               We don't optimize the sleep routine!                 *
# *--------------------------------------------------------------------*

ssleep.obj: ssleep.c
        $(CC) -c $(CCX) -Od -I$: { $<}
