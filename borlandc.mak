# *--------------------------------------------------------------------*
# *     Program:        MAKEFILE                15 May 1989            *
# *     Author:         Andrew H. Derbyshire                           *
# *                     Kendra Electronic Wonderworks                  *
# *                     P.O. Box 132                                   *
# *                     Arlington, MA 02174                            *
# *     Function:       Build UUPC/extended from source files          *
# *     Processor:      Borland MAKE version 3.5                       *
# *     External files: MASM or TASM    (8088 Assembler)               *
# *                     BCC             (Borland C++ 3.1 compiler)     *
# *                     LINK or TLINK   (MS-DOS Linker)                *
# *                     TDSTRIP         (Turbo debugger utility).      *
# *                     MOVE.COM        (Rename files between          *
# *                                     directories)                   *
# *                     Input source files.                            *
# *                                                                    *
# *   Changes Copyright (c) 1989-1993 by Kendra Electronic             *
# *   Wonderworks.                                                     *
# *                                                                    *
# *   All rights reserved except those explicitly granted by the       *
# *   UUPC/extended license agreement.                                 *
# *                                                                    *
# *     Note:   This file and the associated sources really do         *
# *             require the Borland C++ 3.1 compiler; various          *
# *             functions within the source call ANSI functions        *
# *             not available in older versions of the compiler.       *
# *                                                                    *
# *     Note:   MOVE can be created by putting a COPY followed by a    *
# *             ERASE in a MOVE.BAT file.  This is slower than a true  *
# *             move program (which just renames between directories), *
# *             but life is hard.                                      *
# *--------------------------------------------------------------------*
#
#     $Id: Makefile 1.9 1992/12/30 12:40:31 plummer Exp $
#
#     Revision history:
#     $Log: Makefile $
#       Revision 1.9  1992/12/30  12:40:31  plummer
#       Update version
#
#       Revision 1.9  1992/12/30  12:40:31  plummer
#       Update version
#
#       Revision 1.8  1992/12/18  13:05:00  ahd
#       Drop system aliases
#
#       Revision 1.7  1992/12/11  12:43:53  ahd
#       Add aliases file to list of samples
#
#       Revision 1.6  1992/12/04  00:59:25  ahd
#       Add copyright statements
#
#       Revision 1.5  1992/11/29  22:39:15  ahd
#       Add *.txt files to documentation archive (for RFC's)
#
#       Revision 1.4  1992/11/23  02:43:59  ahd
#       Add scripts (batch files) to RCS
#       Change expire.com and rnews.com to exe files
#
#       Revision 1.3  1992/11/22  21:04:42  ahd
#       Add normalize() to build
#
#       Revision 1.2  1992/11/18  01:34:57  ahd
#       Drop -k flag
#
#       Revision 1.1  1992/11/10  04:11:14  ahd
#       Initial revision
#
#       Revision 1.2  1992/05/03  17:19:14  ahd
#       Add library inclusion to generation of small .COM files
#

.autodepend

.nosilent

#       The memory model to be built.

!if !$d(MODEL)
MODEL   = s
!endif

!if !$d(WINMODE)
WINMODE=D
!endif

!if $(WINMODE) == "W"
SUFFIX  = $(WINMODE)$(MODEL)
!else
SUFFIX  = $(MODEL)
!endif

# *--------------------------------------------------------------------*
# *   Our release number.  This is updated each time we ship it        *
# *   out to some unsuspecting user, or sometimes when we do a         *
# *   major fix even if we don't ship it out to the lusers.  ANY       *
# *   VERSION NOT DONE ON kendra SHOULD BE TAGGED AS N.NNAm, WHERE     *
# *   N.NNA IS THE NUMBER UUPC was shipped as, and 'm' any string      *
# *   to indentify your special version.  Just upping the              *
# *   distributed version number will confuse you AND me.              *
# *--------------------------------------------------------------------*

VERS = 1.11y

# *--------------------------------------------------------------------*
# *   Hard Drive letter, and directories on the hard drive to be       *
# *   used.  The link files are also affected by this parameter.       *
# *--------------------------------------------------------------------*

!if !$d(TMP)
TMP     = \tmp
!endif

!if d$(SRC)
!error can't handle this!
!else
SRC     = .
!endif


DOCS    = $(SRCSLASH)docs
CONF    = \uupc
LIB     = $(SRCSLASH)lib
OBJ     = $(SRCSLASH)bcobj$(SUFFIX)
MAIL    = $(SRCSLASH)mail
RNEWS   = $(SRCSLASH)rnews
RN      = $(SRCSLASH)rn
UTIL    = $(SRCSLASH)util
UUCICO  = $(SRCSLASH)uucico
TEST    = $(SRCSLASH)test
UUCP    = $(SRCSLASH)uucp
WINWORD = $(SRCSLASH)winword
SCRIPT = $(SRCSLASH)scripts
MAP     = NUL
.path.obj  = $(OBJ)

!if !$d(ARCHIVE)
ARCHIVE=$(SRCSLASH)$(VERS)
!endif

!if !$d(PROD)
PROD    = \uupc\bin
!endif

FMT     = $(PROD)\fmt.com

!if !$d(WINPROD)
WINPROD    = \uupc\winbin
!endif

!if $(WINMODE) == "W"
PROD    =       $(WINPROD)
!endif

!if !$d(BORLANDC)
BORLANDC = \borlandc
!endif

!if !$d(BLIB)
BLIB =  $(BORLANDC)\lib
!endif

!if !$d(BINC)
BINC =  $(BORLANDC)\include
!endif

UUPCCFG  = $(TMP)\uupc$(SUFFIX).cfg

UUPCDEFS = $(TMP)\uupc$(SUFFIX).mak
UUPCDEFM=$(TMP)\uupcm.mak

COMMON  = $(UUPCCFG) $(UUPCDEFS)

# *--------------------------------------------------------------------*
# *        The names of various object files that we create.           *
# *--------------------------------------------------------------------*

TIMESTMP = $(OBJ)\timestmp.obj

WORKFILE = $(TMP)\UUPCTEMP.TMP

# *--------------------------------------------------------------------*
# *                Final locations of various targets                  *
# *--------------------------------------------------------------------*

BUGSDOC = $(WINWORD)\bugs.doc
CHNGDOC = $(WINWORD)\changes.doc
CMDSDOC = $(WINWORD)\commands.doc
HOW2DOC = $(WINWORD)\howtoget.doc
INSTDOC = $(WINWORD)\install.doc
MAILDOC = $(WINWORD)\mail.doc
READDOC = $(WINWORD)\readme.doc
REGRDOC = $(WINWORD)\register.doc
STATDOC = $(WINWORD)\uustat.doc
TILDDOC = $(WINWORD)\tilde.doc
DOCW    = $(BUGSDOC) $(CHNGDOC) $(CMDSDOC) $(HOW2DOC) $(INSTDOC) \
          $(MAILDOC) $(READDOC) $(REGRDOC) $(STATDOC) $(TILDDOC)
BUGSLST = $(WINWORD)\bugs.prn
CHNGLST = $(WINWORD)\changes.prn
CMDSLST = $(WINWORD)\commands.prn
INSTLST = $(WINWORD)\install.prn
HOW2LST = $(WINWORD)\howtoget.prn
MAILLST = $(WINWORD)\mail.prn
READLST = $(WINWORD)\readme.prn
REGRLST = $(WINWORD)\register.prn
STATLST = $(WINWORD)\uustat.prn
TILDLST = $(WINWORD)\tilde.prn
DOCSLIST= $(BUGSLST) $(CHNGLST) $(CMDSLST) $(HOW2LST) $(INSTLST) \
          $(MAILLST) $(READLST) $(REGRLST) $(STATLST) $(TILDLST)
LIVEFILES = $(DOCS)\active.kew $(DOCS)\permissn.kew $(DOCS)\aliases.kew \
            $(DOCS)\kendra.rc $(DOCS)\ahd.rc $(DOCS)\hostpath.kew

REQUIRED= $(PROD)\mail.exe $(PROD)\rmail.exe $(PROD)\uucico.exe\
          $(PROD)\uuxqt.exe
SCRIPTS=  $(PROD)\su.bat $(PROD)\uuclean.bat $(PROD)\uuio.bat \
          $(PROD)\waiting.bat
OPTIONAL= $(PROD)\uucp.com $(PROD)\uuname.com $(PROD)\uupoll.exe\
          $(PROD)\uustat.com $(PROD)\uusub.com $(PROD)\comm34.com\
          $(PROD)\uux.com $(FMT) $(PROD)\gensig.com $(PROD)\novrstrk.com \
          $(SCRIPTS)
NEWS    = $(PROD)\rnews.exe $(PROD)\expire.exe $(PROD)\rn.exe\
          $(PROD)\newsetup.exe

INSTALL = $(REQUIRED) $(OPTIONAL) $(NEWS)

SAMPLES = $(DOCS)\systems $(DOCS)\passwd $(DOCS)\hostpath \
          $(DOCS)\personal.rc $(DOCS)\uupc.rc \
          $(DOCS)\personal.sig $(DOCS)\aliases.txt \
          $(DOCS)\mail.ico $(DOCS)\uucico.ico $(DOCS)\up-pif.dvp
SAMPLEX = systems passwd hostpath *.rc *.mdm personal.sig aliases.txt *.ico

MAKEFILE = $(SRCSLASH)makefile

STARTUP  = $(BLIB)\c0$(SUFFIX)
STARTUPT = $(BLIB)\c0t

UUPCLIB  = $(TMP)\cuupc$(SUFFIX).lib
UUPCLIBW = $(TMP)\cuupc$(SUFFIX)w.lib
LIBRARY2 = $(BLIB)\c$(SUFFIX).lib

!if $(WINMODE) == "W"
LIBRARY3 = $(BLIB)\import.lib
LIBRARY  = $(UUPCLIB)+$(UUPCLIBW)+$(LIBRARY2)+$(LIBRARY3)
!else
LIBRARY  = $(UUPCLIB)+$(LIBRARY2)
!endif
LIBRARIES= $(LIBRARY:+= )

COMMON  = $(UUPCCFG) $(UUPCDEFS) $(UUPCLIB)

REQZIPV = $(ARCHIVE)\uupcreq.zip
OPTZIPV = $(ARCHIVE)\uupcopt.zip
NEWZIPV = $(ARCHIVE)\uupcnews.zip
DOCZIPV = $(ARCHIVE)\uupcdoc.zip
WINZIPV = $(ARCHIVE)\uupcwin.zip
SRCZIPV1 = $(ARCHIVE)\uupcsrc1.zip
SRCZIPV2 = $(ARCHIVE)\uupcsrc2.zip

# *--------------------------------------------------------------------*
# *     Various Program names and their options                        *
# *--------------------------------------------------------------------*

CC = bcc +$(UUPCCFG)
LINKER  = tlink
!if $(WINMODE) == "W"
LINKOPT =  /c /s /v /s /yx /Twe
LINKOPTT = /c /s /t /x /Twe
!else
LINKOPT =  /c /s /v /s /yx
LINKOPTT = /c /s /t /x
!endif

TASM    = tasm.exe
TASMOPT = /mx /z /zi /w3 /DUUPC
ZIP     = pkzip

# *--------------------------------------------------------------------*
# *                Begin rules for building modules.                   *
# *                                                                    *
# *        The first definition is the one built by default.           *
# *--------------------------------------------------------------------*

# *--------------------------------------------------------------------*
# *     test builds the basic UUPC modules in the source directory.    *
# *--------------------------------------------------------------------*

test:    mail.exe uucico.exe\
         rmail.exe uuxqt.exe uucp.exe uupoll.exe\
         uux.exe uustat.exe uusub.exe\
         rnews.exe expire.exe

# *--------------------------------------------------------------------*
# *   Prod build the basic modules and then moves them to $(PROD)      *
# *--------------------------------------------------------------------*

prod:   $(INSTALL)
        - erase $(TIMESTMP)
        - erase $(UUPCLIB)

required: $(REQUIRED)

# *--------------------------------------------------------------------*
# *     Install builds the UUPC modules, installs the executables      *
# *     in the production directory, formats the documentation files,  *
# *     and creates PKZIP files for giving away.                       *
# *--------------------------------------------------------------------*

install: $(INSTALL) $(REQZIPV) $(OPTZIPV) $(NEWZIPV) \
         $(SRCZIPV1) $(SRCZIPV2) \
         $(DOCZIPV) $(WINZIPV) regen
        - erase $(WORKFILE)
        @echo Installed UUPC and created ZIP files $(REQZIPV),
        @echo $(OPTZIPV), $(NEWZIPV),
        @echo $(SRCZIPV1), $(SRCZIPV2),
        @echo $(DOCZIPV), and $(WINZIPV)
        - erase $(TIMESTMP)
        - erase $(UUPCLIB)

# *--------------------------------------------------------------------*
# *                 Build just the source .ZIP file.                   *
# *--------------------------------------------------------------------*

szip:   $(SRCZIPV1) $(SRCZIPV2)

#       Do a purge of various temporary output files.

clean:
        wipeinfo $(SRCSLASH)*.com /n /s /batch
        wipeinfo $(SRCSLASH)*.exe /n /s /batch
        wipeinfo $(SRCSLASH)*.tds /n /s /batch
        wipeinfo $(SRCSLASH)*.lib /n /s /batch
        wipeinfo $(SRCSLASH)*.obj /n /s /batch
        wipeinfo $(SRCSLASH)*.o   /n /s /batch
        wipeinfo $(SRCSLASH)*.map /n /s /batch

# *--------------------------------------------------------------------*
# *           The production copies of executables follow.             *
# *--------------------------------------------------------------------*

$(PROD)\mail.exe: mail.exe
        - erase $<
        move $? $<
        - erase $&.tds

$(PROD)\uucp.com: uucp.com
        - erase $<
        move $? $<
        - erase $&.tds

$(PROD)\uusub.com: uusub.com
        - erase $<
        move $? $<
        - erase $&.tds

$(PROD)\uuxqt.exe: uuxqt.exe
        - erase $<
        move $? $<
        - erase $&.tds

$(PROD)\uupoll.exe: uupoll.exe
        - erase $<
        move $? $<
        - erase $&.tds

$(PROD)\rmail.exe: rmail.exe
        - erase $<
        move $? $<
        - erase $&.tds

$(PROD)\uuname.exe: uuname.exe
        - erase $<
        move $? $<
        - erase $&.tds

$(PROD)\uustat.com: uustat.com
        - erase $<
        move $? $<
        - erase $&.tds

$(PROD)\uux.com: uux.com
        - erase $<
        move $? $<
        - erase $&.tds

$(PROD)\comm34.com: comm34.com
        - erase $<
        move $? $<
        - erase $&.tds

$(PROD)\uuname.com: uuname.com
        - erase $<
        move $? $<
        - erase $&.tds

$(FMT): fmt.com
        - erase $<
        move $? $<
        - erase $&.tds

$(PROD)\gensig.com: gensig.com
        - erase $<
        move $? $<
        - erase $&.tds

$(PROD)\novrstrk.com: novrstrk.com
        - erase $<
        move $? $<
        - erase $&.tds

$(PROD)\uucico.exe: uucico.exe
        - erase $<
        move $? $<
        - erase $&.tds

$(PROD)\rnews.exe: rnews.exe
        - erase $<
        move $? $<
        - erase $&.tds

$(PROD)\rn.exe: rn.exe
        - erase $<
        move $? $<
        - erase $&.tds

$(PROD)\newsetup.exe: newsetup.exe
        - erase $<
        move $? $<
        - erase $&.tds

$(PROD)\expire.exe: expire.exe
        - erase $<
        move $? $<
        - erase $&.tds

$(PROD)\su.bat: $(SCRIPT)\su.bat
        copy $? $<

$(PROD)\uuio.bat: $(SCRIPT)\uuio.bat
        copy $? $<

$(PROD)\uuclean.bat: $(SCRIPT)\uuclean.bat
        copy $? $<

$(PROD)\waiting.bat: $(SCRIPT)\waiting.bat
        copy $? $<

# *--------------------------------------------------------------------*
# *                         The .ZIP files.                            *
# *--------------------------------------------------------------------*

# *--------------------------------------------------------------------*
# *                           UUPCSRC.ZIP                              *
# *--------------------------------------------------------------------*

$(SRCZIPV1): $(MAKEFILE) $(SRCSLASH)nmake $(TIMESTMP) $(FMT)
        - mkdir $:.
        $(FMT) -0 &&!
$(MAKEFILE)
nmake
lib\*.c
lib\*.h
lib\*.mak
mail\*.c
mail\*.h
mail\*.mak
uucico\*.asm
uucico\*.c
uucico\*.h
uucico\*.mak
uucp\*.c
uucp\*.h
uucp\*.mak
!       $(WORKFILE)
       -12 $(ZIP) -o- -P -u $< @$(WORKFILE)
       pkbanner $<  &&!
Kendra Electronic Wonderworks: UUPC/extended $(VERS) source files (1 of 2)

Changes and Compilation Copyright (c) 1989-1993 by Kendra Electronic
Wonderworks.  May be distributed freely if original source and documentation
files are included.  Please direct all questions on UUPC/extended to
help@kew.com.
!

$(SRCZIPV2): $(FMT)
        - mkdir $:.
        $(FMT) -0 &&!
rn\*.c
rn\*.h
rn\*.mak
rnews\*.c
rnews\*.h
rnews\*.mak
test\*.c
test\*.h
test\*.mak
util\*.c
util\*.h
util\*.mak
!       $(WORKFILE)
       -12 $(ZIP) -o- -P -u $< @$(WORKFILE)
       pkbanner $<  &&!
Kendra Electronic Wonderworks: UUPC/extended $(VERS) source files (2 of 2)

Changes and Compilation Copyright (c) 1989-1993 by Kendra Electronic
Wonderworks.  May be distributed freely if original source and documentation
files are included.  Please direct all questions on UUPC/extended to
help@kew.com.
!

# *--------------------------------------------------------------------*
# *       UUPCWIN.ZIP - Word for Windows unformatted documents         *
# *--------------------------------------------------------------------*

$(WINZIPV):  $(DOCW) $(FMT) $(MAKEFILE)
        - mkdir $:.
        $(FMT) -0 &&!
$(DOCW)
winword\manual.dot
!       $(WORKFILE)
       -12 $(ZIP) -o- -u $< @$(WORKFILE)
       pkbanner $<  &&!
Kendra Electronic Wonderworks: UUPC/extended $(VERS) Unformatted Documents

Changes and Compilation Copyright (c) 1989-1993 by Kendra Electronic
Wonderworks.  May be distributed freely if original source and documentation
files are included.  Please direct all questions on UUPC/extended to
help@kew.com.
!

# *--------------------------------------------------------------------*
# *                UUPCREQ.ZIP - Required Executables                  *
# *--------------------------------------------------------------------*

$(REQZIPV):  $(REQUIRED) $(FMT) $(MAKEFILE)
        - mkdir $:.
        $(FMT) -0 &&!
$(REQUIRED)
!       $(WORKFILE)
       -12 $(ZIP) -o- -p -u $< @$(WORKFILE)
       pkbanner $<  &&!
Kendra Electronic Wonderworks: UUPC/extended $(VERS) DOS executables (1 of 3)

Changes and Compilation Copyright (c) 1989-1993 by Kendra Electronic
Wonderworks.  May be distributed freely if original source and documentation
files are included.  Please direct all questions on UUPC/extended to
help@kew.com.
!

# *--------------------------------------------------------------------*
# *                UUPCOPT.ZIP - Optional Executables                  *
# *--------------------------------------------------------------------*

$(OPTZIPV):  $(OPTIONAL) $(FMT) $(MAKEFILE)
        - mkdir $:.
        $(FMT) -0 &&!
$(OPTIONAL)
!       $(WORKFILE)
       -12 $(ZIP) -o- -p -u $< @$(WORKFILE)
       pkbanner $<  &&!
Kendra Electronic Wonderworks: UUPC/extended $(VERS) DOS executables (2 of 3)

Changes and Compilation Copyright (c) 1989-1993 by Kendra Electronic
Wonderworks.  May be distributed freely if original source and documentation
files are included.  Please direct all questions on UUPC/extended to
help@kew.com.
!

# *--------------------------------------------------------------------*
# *                 UUPC/extended - NEWS executables                   *
# *--------------------------------------------------------------------*

$(NEWZIPV):  $(NEWS) $(FMT) $(MAKEFILE)
        - mkdir $:.
        $(FMT) -0 &&!
$(NEWS)
!       $(WORKFILE)
       -12 $(ZIP) -o- -p -u $< @$(WORKFILE)
       pkbanner $<  &&!
Kendra Electronic Wonderworks: UUPC/extended $(VERS) DOS executables (3 of 3)

Changes and Compilation Copyright (c) 1989-1993 by Kendra Electronic
Wonderworks.  May be distributed freely if original source and documentation
files are included.  Please direct all questions on UUPC/extended to
help@kew.com.
!

# *--------------------------------------------------------------------*
# *        UUPCDOC.ZIP - Formatted (Human readable) documents          *
# *--------------------------------------------------------------------*

docs: $(DOCZIPV) $(WINZIPV)

$(DOCZIPV):  $(SAMPLES) $(DOCSLIST) $(FMT) $(MAKEFILE) $(LIVEFILES)
        - mkdir $:.
        $(FMT) -0 &&!
$(DOCSLIST)
$(SAMPLES)
$(DOCS)\*.mdm
$(DOCS)\*.inf
$(DOCS)\*.txt
$(DOCS)\*.spb
$(LIVEFILES)
!       $(WORKFILE)
       -12 $(ZIP) -o- -p -u $< @$(WORKFILE)
       pkbanner $< &&!
Kendra Electronic Wonderworks: UUPC/extended $(VERS) documents and sample files

Changes and Compilation Copyright (c) 1989-1993 by Kendra Electronic
Wonderworks.  May be distributed freely if original source and documentation
files are included.  Please direct all questions on UUPC/extended to
help@kew.com.
!

# *--------------------------------------------------------------------*
# *                     Link our various modules                       *
# *--------------------------------------------------------------------*

mail.exe: common
        $(MAKE) -f$(MAIL)\mail.mak -DUUPCDEFS=$(UUPCDEFS) $<

rmail.exe: common
        $(MAKE) -f$(MAIL)\mail.mak -DUUPCDEFS=$(UUPCDEFS) $<

uucp.exe: common
        $(MAKE) -f$(UUCP)\uucp.mak -DUUPCDEFS=$(UUPCDEFS) $<

uucp.com: common
        $(MAKE) -f$(UUCP)\uucp.mak -DUUPCDEFS=$(UUPCDEFS) $<

uuname.exe: common
        $(MAKE) -f$(UUCP)\uucp.mak -DUUPCDEFS=$(UUPCDEFS) $<

uuname.com: common
        $(MAKE) -f$(UUCP)\uucp.mak -DUUPCDEFS=$(UUPCDEFS) $<

uustat.exe: common
        $(MAKE) -f$(UUCP)\uucp.mak -DUUPCDEFS=$(UUPCDEFS) $<

uustat.com: common
        $(MAKE) -f$(UUCP)\uucp.mak -DUUPCDEFS=$(UUPCDEFS) $<

uusub.exe: common
        $(MAKE) -f$(UUCP)\uucp.mak -DUUPCDEFS=$(UUPCDEFS) $<

uusub.com: common
        $(MAKE) -f$(UUCP)\uucp.mak -DUUPCDEFS=$(UUPCDEFS) $<

uux.exe: common
        $(MAKE) -f$(UUCP)\uucp.mak -DUUPCDEFS=$(UUPCDEFS) $<

uux.com: common
        $(MAKE) -f$(UUCP)\uucp.mak -DUUPCDEFS=$(UUPCDEFS) $<

uuxqt.exe: common
        $(MAKE) -f$(UUCP)\uucp.mak -DUUPCDEFS=$(UUPCDEFS) $<

comm34.com: common
        $(MAKE) -f$(UTIL)\util.mak -DUUPCDEFS=$(UUPCDEFS) $<

fmt.com: common
        $(MAKE) -f$(UTIL)\util.mak -DUUPCDEFS=$(UUPCDEFS) $<

gensig.com: common
        $(MAKE) -f$(UTIL)\util.mak -DUUPCDEFS=$(UUPCDEFS) $<

novrstrk.com: common
        $(MAKE) -f$(UTIL)\util.mak -DUUPCDEFS=$(UUPCDEFS) $<

uupoll.exe: common
        $(MAKE) -f$(UTIL)\util.mak -DUUPCDEFS=$(UUPCDEFS) $<

expire.exe: common
        $(MAKE) -f$(RNEWS)\rnews.mak -DUUPCDEFS=$(UUPCDEFS) $<

rnews.exe: common
        $(MAKE) -f$(RNEWS)\rnews.mak -DUUPCDEFS=$(UUPCDEFS) $<

common: $(.path.obj) $(COMMON)

commonm:
        $(MAKE) -f$(MAKEFILE) -DMODEL=m -DWINMODE=$(WINMODE) -DUUPCDEFS=$(UUPCDEFM) common

$(.path.obj):
        - mkdir $(.path.obj)

uucico.exe: commonm
        $(MAKE) -f$(UUCICO)\UUCICO.mak -DUUPCDEFS=$(UUPCDEFM) $<

rn.exe: commonm
        $(MAKE) -f$(RN)\RN.mak -DUUPCDEFS=$(UUPCDEFM) $<

newsetup.exe: commonm
        $(MAKE) -f$(RN)\RN.mak -DUUPCDEFS=$(UUPCDEFM) $<

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

$(TIMESTMP): $(LIB)\timestmp.c $(UUPCCFG) $(REGEN)

# *--------------------------------------------------------------------*
# *     Common library build                                           *
# *--------------------------------------------------------------------*

$(UUPCLIB):
        $(MAKE) -f $(LIB)\lib.mak -DUUPCDEFS=$(UUPCDEFS) $<
        echo Built $<

$(UUPCLIBW):
        $(MAKE) -f $(LIB)\lib.mak -DUUPCDEFS=$(UUPCDEFS) $<
        echo Built $<

# *--------------------------------------------------------------------*
# *   Live files from the Wonderworks we provide as additional         *
# *   examples                                                         *
# *--------------------------------------------------------------------*


$(DOCS)\active.kew: $(CONF)\active
        copy $? $<

$(DOCS)\permissn.kew: $(CONF)\permissn
        copy $? $<

$(DOCS)\kendra.rc: $(CONF)\uupc.rc
        copy $? $<

$(DOCS)\ahd.rc: $(CONF)\ahd.rc
        copy $? $<

$(DOCS)\hostpath.kew: $(CONF)\hostpath
        copy $? $<

# *--------------------------------------------------------------------*
# *                  *Compiler Configuration File*                     *
# *--------------------------------------------------------------------*


!if $(WINMODE) == "W"
GENERATE = -W
!endif

#       -N-     <=== disables

$(UUPCCFG): $(MAKEFILE) $(UUPCDEFS)
  copy &&|
$(GENERATE)
-c
-d
-DUUPCV="$(VERS)"
-D?__HEAPCHECK__
-D?__CORELEFT__
-D?_DEBUG
-D?UDEBUG
-DIOCTL
-f
-G
-I$(LIB)
-I$(BINC)
-L$(BLIB)
-nbcobj$(SUFFIX)
-N
-k-
-O
-P-.C
-r
-v
-Z
-wpro
-wdcl
-weas
-will
-wpin
-wrvl
-wamb
-wamp
-wnod
-wstv
-wuse
-wsig
-wucp
-O2
-m$(MODEL)
| $<

$(UUPCDEFS): $(MAKEFILE)
  copy &&|
.path.obj  = $(.path.obj)
ARCHIVE    = $(ARCHIVE)
TASM       = $(TASM)
TASMOPT    = $(TASMOPT)
BINC       = $(BINC)
BLIB       = $(BLIB)
BORLANDC   = $(BORLANDC)
BUGSDOC    = $(BUGSDOC)
BUGSLST    = $(BUGSLST)
CC         = $(CC)
CHNGDOC    = $(CHNGDOC)
CHNGLST    = $(CHNGLST)
CMDSDOC    = $(CMDSDOC)
CMDSLST    = $(CMDSLST)
DOCS       = $(DOCS)
DOCSLIST   = $(DOCSLIST)
DOCW       = $(DOCW)
DOCZIPV    = $(DOCZIPV)
FMT        = $(FMT)
HOW2DOC    = $(HOW2DOC)
HOW2LST    = $(HOW2LST)
INSTALL    = $(INSTALL)
INSTDOC    = $(INSTDOC)
INSTLST    = $(INSTLST)
LIB        = $(LIB)
LIBRARIES  = $(LIBRARIES)
LIBRARY    = $(LIBRARY)
LIBRARY2   = $(LIBRARY2)
LINKER     = $(LINKER)
LINKOPT    = $(LINKOPT)
LINKOPTT   = $(LINKOPTT)
MAIL       = $(MAIL)
MODEL      = $(MODEL)
WINMODE    = $(WINMODE)
SUFFIX     = $(SUFFIX)
MAILDOC    = $(MAILDOC)
MAILLST    = $(MAILLST)
MAKEFILE   = $(MAKEFILE)
MAP        = $(MAP)
NEWS       = $(NEWS)
NEWZIPV    = $(NEWZIPV)
OBJ        = $(OBJ)
OPTIONAL   = $(OPTIONAL)
OPTZIPV    = $(OPTZIPV)
PROD       = $(PROD)
READDOC    = $(READDOC)
READLST    = $(READLST)
REGRDOC    = $(REGRDOC)
REGRLST    = $(REGRLST)
REQUIRED   = $(REQUIRED)
REQZIPV    = $(REQZIPV)
RNEWS      = $(RNEWS)
RN         = $(RN)
SAMPLES    = $(SAMPLES)
SAMPLEX    = $(SAMPLEX)
SRC        = $(SRC)
SRCZIPV1   = $(SRCZIPV1)
SRCZIPV2   = $(SRCZIPV2)
STARTUP    = $(STARTUP)
STARTUPT   = $(STARTUPT)
STATDOC    = $(STATDOC)
STATLST    = $(STATLST)
TEST       = $(TEST)
TILDDOC    = $(TILDDOC)
TILDLST    = $(TILDLST)
TIMESTMP   = $(TIMESTMP)
TMP        = $(TMP)
UTIL       = $(UTIL)
UUCICO     = $(UUCICO)
UUCICOCOM  = $(UUCICOCOM)
UUCP       = $(UUCP)
UUPCCFG    = $(UUPCCFG)
UUPCLIB    = $(UUPCLIB)
VERS       = $(VERS)
WINWORD    = $(WINWORD)
WINZIPV    = $(WINZIPV)
WORKFILE   = $(WORKFILE)
COMMON     = $(COMMON)
ZIP        = $(ZIP)

.autodepend

.nosilent
| $<
