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
#     $Id: makefile 1.31 1993/10/04 15:41:24 ahd Exp $
#
#     Revision history:
#     $Log: makefile $
#         Revision 1.31  1993/10/04  15:41:24  ahd
#         Drop unneeded variables from definition file for nested make commands
#         Include 00readme.now in all archives
#         Generate final release names for all archives
#
#         Revision 1.30  1993/10/04  03:56:57  ahd
#         Build doc zip seperately
#
#         Revision 1.29  1993/10/02  23:45:02  ahd
#         Don't build rnews for windows
#
#         Revision 1.28  1993/10/02  19:06:24  ahd
#         Use full path names to aid debugging
#
#         Revision 1.27  1993/10/01  01:24:32  ahd
#         Force source rebuild more often
#
#         Revision 1.26  1993/09/29  13:17:11  ahd
#         Correct deletion of debugger (*.TDS) files under Windows
#
#         Revision 1.25  1993/09/29  04:47:51  ahd
#         Add unique signal handler for port suspending
#
#         Revision 1.24  1993/09/27  02:41:37  ahd
#         Use full source path for files to aid in DOS debugging
#
#         Revision 1.23  1993/09/27  00:44:01  ahd
#         Make build slient
#
#         Revision 1.22  1993/09/23  03:26:11  ahd
#         Make mail medium model
#
#         Revision 1.21  1993/09/20  04:35:34  ahd
#         Borland C++ for OS/2 1.0 support
#         TCP/IP support from Dave Watt
#         't' protocol support
#
#         Revision 1.20  1993/08/03  12:19:38  ahd
#         Upper case names
#
#         Revision 1.19  1993/08/02  03:21:28  ahd
#         Rename zip files
#         use ZIP instead of PKZIP
#         explicitly use MAKER for nested makes (need MAKE for master to
#         keep ZIP from running of out DOS memory)
#
#         Revision 1.18  1993/07/31  16:20:41  ahd
#         Clean-up in support of Windows versions
#
#         Revision 1.17  1993/07/22  23:18:38  ahd
#         Rename UUPCLIBW
#
#         Revision 1.16  1993/06/06  15:03:51  ahd
#         Move to release 1.12a
#
#         Revision 1.15  1993/05/30  16:05:22  ahd
#         Del workfile to correct pkzip archiving too much
#
#         Revision 1.14  1993/05/16  13:17:21  ahd
#         Rename nmake to mscmake
#
#         Revision 1.13  1993/04/19  13:16:08  ahd
#         Begin version 1.11z
#
#         Revision 1.12  1993/04/16  02:43:27  ahd
#         Do not archive bad rn source
#
#         Revision 1.11  1993/04/16  02:31:47  ahd
#         Drop broken rn/newsetup from build for now
#
#         Revision 1.10  1993/04/11  21:09:10  ahd
#         Drop GNU oriented Makefile
#
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

.autodepend

.silent

#       The memory model to be built, and other environment
#       specific information

WINMODEL=l

!if $d(__OS2__)
MODEL=2
SUFFIX   = $(MODEL)
PSUFFIX  = 2
DEFFILE  = $(TMP)\UUPCOS23.TMP
!elif $d(WINDOWS)
MODEL    = $(WINMODEL)
SUFFIX   = w$(MODEL)
PSUFFIX  = w
DEFFILE  = $(TMP)\UUPCWIND.TMP
!else
!if !$d(MODEL)
MODEL    = s
!endif
SUFFIX   = $(MODEL)
PSUFFIX  =
DEFFILE  =
!endif

#       Silly hack to allow back slash as last character in variable
SRC      = e:/src/uupc/
SRCSLASH = $(SRC:/=\)

# *--------------------------------------------------------------------*
# *   Our release number.  This is updated each time we ship it        *
# *   out to some unsuspecting user, or sometimes when we do a         *
# *   major fix even if we don't ship it out to the lusers.  ANY       *
# *   VERSION NOT DONE ON kendra SHOULD BE TAGGED AS N.NNAm, WHERE     *
# *   N.NNA IS THE NUMBER UUPC was shipped as, and 'm' any string      *
# *   to indentify your special version.  Just upping the              *
# *   distributed version number will confuse you AND me.              *
# *--------------------------------------------------------------------*

VERS = 1.12b

# *--------------------------------------------------------------------*
# *                           Directories                              *
# *--------------------------------------------------------------------*

!if !$d(TMP)
TMP     = \TMP
!endif


DOCS    = $(SRCSLASH)DOCS
CONF    = \UUPC
LIB     = $(SRCSLASH)LIB
OBJ     = $(SRCSLASH)OBJBC$(SUFFIX)
MAIL    = $(SRCSLASH)MAIL
RNEWS   = $(SRCSLASH)RNEWS
RN      = $(SRCSLASH)RN
UTIL    = $(SRCSLASH)UTIL
UUCICO  = $(SRCSLASH)UUCICO
TEST    = $(SRCSLASH)TEST
UUCP    = $(SRCSLASH)UUCP
WINWORD = $(SRCSLASH)WINWORD
SCRIPT  = $(SRCSLASH)SCRIPTS
MAP     =
.path.obj  = $(OBJ)

!if !$d(ARCHIVE)
ARCHIVE = $(SRCSLASH)$(VERS)
!endif

!if !$d(PROD)
!if $d(__OS2__)
PROD    = \UUPC\OS2BIN32
!else
PROD    = \UUPC\BIN
!endif
!endif

!if !$d(WINPROD)
WINPROD = \UUPC\WINBIN
!endif

FMT     = $(PROD)\FMT.COM

!if !$d(BORLANDC)
!if $d(__OS2__)
BORLANDC = \BCOS2
!else
BORLANDC = \BORLANDC
!endif
!endif

!if !$d(BLIB)
BLIB =  $(BORLANDC)\LIB
!endif

!if !$d(BINC)
BINC =  $(BORLANDC)\INCLUDE
!endif

!if !$d(UUPCCFG)
UUPCCFG  = $(TMP)\UUPC$(SUFFIX).CFG
!endif

!if !$d(UUPCDEFS)
UUPCDEFS=$(TMP)\uupc$(SUFFIX).mak
!endif

UUPCDEFM=$(TMP)\uupcm.mak
UUPCDEFW=$(TMP)\uupcw$(WINMODEL).mak

COMMON  = $(UUPCCFG) $(UUPCDEFS) $(DEFFILE) $(LIBRARIES)

# *--------------------------------------------------------------------*
# *        The names of various object files that we create.           *
# *--------------------------------------------------------------------*

TIMESTMP = $(OBJ)\TIMESTMP.OBJ

WORKFILE = $(TMP)\UUPCTEMP.TMP

# *--------------------------------------------------------------------*
# *                Final locations of various targets                  *
# *--------------------------------------------------------------------*

LIVEFILES = $(DOCS)\ACTIVE.KEW $(DOCS)\PERMISSN.KEW $(DOCS)\ALIASES.KEW \
            $(DOCS)\kendra.rc $(DOCS)\ahd.rc $(DOCS)\hostpath.kew

LISTFILES = $(WINWORD)\advanced.prn $(WINWORD)\changes.prn \
            $(WINWORD)\commands.prn $(WINWORD)\files.prn \
            $(WINWORD)\howtoget.prn $(WINWORD)\install.prn \
            $(WINWORD)\license.prn $(WINWORD)\mail.prn \
            $(WINWORD)\register.prn $(WINWORD)\tilde.prn
README=     docs\00readme.now

REQUIRED= $(PROD)\mail.exe $(PROD)\rmail.exe $(PROD)\uucico.exe\
          $(PROD)\uuxqt.exe
SCRIPTS=  $(PROD)\su.bat $(PROD)\uuclean.bat $(PROD)\uuio.bat \
          $(PROD)\waiting.bat
OPTCOMM = $(PROD)\uucp.com $(PROD)\uuname.com $(PROD)\uupoll.exe\
          $(PROD)\uustat.com $(PROD)\uusub.com\
          $(PROD)\uuport.com \
          $(PROD)\uux.com $(FMT) $(PROD)\gensig.com $(PROD)\novrstrk.com

!if $d(__OS2__)
OPTIONAL= $(OPTCOMM:.com=.exe) \
          $(SCRIPTS:.bat=.cmd) $(PROD)\mailchek.cmd $(PROD)\getuupc.cmd
!else
OPTIONAL= $(OPTCOMM) $(PROD)\comm34.com $(SCRIPTS)

# *--------------------------------------------------------------------*
# *      Of the "required" modules, we only build UUCICO under         *
# *      Windows because the MAIL program is ugly and we prefer        *
# *      UUXQT run RMAIL and RNEWS as normal DOS programs.             *
# *--------------------------------------------------------------------*

#WREQUIRED=$(WINPROD)\mail.exe $(WINPROD)\rmail.exe $(WINPROD)\uucico.exe\
#          $(WINPROD)\uuxqt.exe
WREQUIRED=$(WINPROD)\uucico.exe
WOPTIONAL=$(WINPROD)\uucp.exe $(WINPROD)\uuname.exe $(WINPROD)\uupoll.exe\
          $(WINPROD)\uustat.exe $(WINPROD)\uusub.exe $(WINPROD)\uux.exe
WNEWS    =$(WINPROD)\expire.exe
#         $(WINPROD)\rnews.exe $(WINPROD)\rn.exe $(WINPROD)\newsetup.exe
!endif

NEWS    = $(PROD)\RNEWS.EXE $(PROD)\EXPIRE.EXE
#         $(PROD)\rn.exe $(PROD)\newsetup.exe

INSTALL = $(REQUIRED) $(OPTIONAL) $(NEWS) $(WREQUIRED) $(WOPTIONAL) $(WNEWS)

SAMPLES = $(DOCS)\SYSTEMS $(DOCS)\PASSWD $(DOCS)\HOSTPATH \
          $(DOCS)\personal.rc $(DOCS)\uupc.rc \
          $(DOCS)\personal.sig $(DOCS)\aliases.txt \
          $(DOCS)\mail.ico $(DOCS)\uucico.ico $(DOCS)\up-pif.dvp
SAMPLEX = SYSTEMS PASSWD HOSTPATH *.RC *.MDM PERSONAL.SIG ALIASES.TXT *.ICO

MAKEFILE = $(SRCSLASH)MAKEFILE

UUPCLIB  = $(TMP)\CUUPC$(SUFFIX).LIB
UUPCLIBW = $(TMP)\CUUPCW$(SUFFIX).LIB

!if $d(__OS2__)
STARTUP  = $(BLIB)\C02.obj
LIBRARY2 = $(BLIB)\C2.LIB
LIBRARY3 = $(BLIB)\os2.LIB
LIBRARY  = $(UUPCLIB)+$(LIBRARY2)+$(LIBRARY3)
!elif $d(WINDOWS)
STARTUP  = $(BLIB)\C0$(SUFFIX)
LIBRARY2 = $(BLIB)\C$(SUFFIX).LIB
LIBRARY3 = $(BLIB)\IMPORT.LIB
LIBRARY  = $(UUPCLIB)+$(LIBRARY2)+$(LIBRARY3)
!else
STARTUP  = $(BLIB)\C0$(SUFFIX)
STARTUPT = $(BLIB)\C0T
LIBRARY2 = $(BLIB)\C$(SUFFIX).LIB
LIBRARY  = $(UUPCLIB)+$(LIBRARY2)
!endif

LIBRARIES= $(LIBRARY:+= )

COMMON  = $(UUPCCFG) $(UUPCDEFS) $(DEFFILE) $(LIBRARIES)

ZIPPREFIXX=UPC$(VERS:.=)
ZIPPREFIX=$(ZIPPREFIXX:C1=C)

!if $d(__OS2__)
REQZIPV = $(ARCHIVE)\$(ZIPPREFIX)21.ZIP
OPTZIPV = $(ARCHIVE)\$(ZIPPREFIX)22.ZIP
NEWZIPV = $(ARCHIVE)\$(ZIPPREFIX)23.ZIP
!else
REQZIPV = $(ARCHIVE)\$(ZIPPREFIX)D1.ZIP
OPTZIPV = $(ARCHIVE)\$(ZIPPREFIX)D2.ZIP
NEWZIPV = $(ARCHIVE)\$(ZIPPREFIX)D3.ZIP
WREQZIPV = $(ARCHIVE)\$(ZIPPREFIX)W1.ZIP
WOPTZIPV = $(ARCHIVE)\$(ZIPPREFIX)W2.ZIP
WNEWZIPV = $(ARCHIVE)\$(ZIPPREFIX)W3.ZIP
!endif

DOCZIPV = $(ARCHIVE)\$(ZIPPREFIX)AD.ZIP
WFWZIPV = $(ARCHIVE)\$(ZIPPREFIX)AW.ZIP
PSZIPV  = $(ARCHIVE)\$(ZIPPREFIX)AP.ZIP
SRCZIPV1 = $(ARCHIVE)\$(ZIPPREFIX)S1.ZIP
SRCZIPV2 = $(ARCHIVE)\$(ZIPPREFIX)S2.ZIP

# *--------------------------------------------------------------------*
# *     Various Program names and their options                        *
# *--------------------------------------------------------------------*

CC = BCC +$(UUPCCFG)
LINKER  = TLINK

!if $d(__OS2__)
LINKOPTN = /c /x /Toe /ap /B:0x10000
!elif $d(WINDOWS)
LINKOPTN = /c /x /yx /Twe
LINKOPTT = /c /x /t /Twe
!else
LINKOPTN = /c /x /yx
LINKOPTT = /c /x /t
!endif

LINKOPTD = $(LINKOPTN) /v

TASM    = tasm.exe
TASMOPT = /mx /z /zi /w3 /DUUPC
ZIP     = zip
ZIPOPT1 = -o -9 -u $< -@
ZIPOPT2 = -j $(ZIPOPT1)

!if $d(__OS2__)
MAKER=make
!else
MAKER=maker
!endif

# *--------------------------------------------------------------------*
# *                Begin rules for building modules.                   *
# *                                                                    *
# *        The first definition is the one built by default.           *
# *--------------------------------------------------------------------*

# *--------------------------------------------------------------------*
# *     test builds the basic UUPC modules in the source directory.    *
# *--------------------------------------------------------------------*

!if $d(__OS2__)
test:    mail$(PSUFFIX).exe uucico$(PSUFFIX).exe rmail$(PSUFFIX).exe\
         uuxqt$(PSUFFIX).exe uucp$(PSUFFIX).exe uupoll$(PSUFFIX).exe\
         uuport$(PSUFFIX).exe\
         uux$(PSUFFIX).exe uustat$(PSUFFIX).exe uusub$(PSUFFIX).exe\
         uuname$(PSUFFIX).exe rnews$(PSUFFIX).exe expire$(PSUFFIX).exe
!else

test:    mail.exe uucico.exe\
         rmail.exe uuxqt.exe uucp.exe uupoll.exe\
         uux.exe uustat.exe uusub.exe\
         rnews.exe expire.exe

windows: mailw.exe uucicow.exe\
         rmailw.exe uuxqtw.exe uucpw.exe uupollw.exe\
         uuxw.exe uustatw.exe uusubw.exe\
         rnewsw.exe expirew.exe
!endif

# *--------------------------------------------------------------------*
# *   Prod build the basic modules and then moves them to $(PROD)      *
# *--------------------------------------------------------------------*

!if $d(__OS2__)
prod:   $(REQUIRED:.com=.exe) $(OPTIONAL:.com=.exe) $(NEWS:.com=.exe)
        - erase $(TIMESTMP)
        - erase $(UUPCLIB)
!else
prod:   $(INSTALL)
        - erase $(TIMESTMP)
        - erase $(UUPCLIB)
!endif

winprod:  commonw $(WREQUIRED) $(WOPTIONAL) $(WNEWS)
        - del *.tds

required: $(REQUIRED)

# *--------------------------------------------------------------------*
# *     Install builds the UUPC modules, installs the executables      *
# *     in the production directory, formats the documentation files,  *
# *     and creates PKZIP files for giving away.                       *
# *--------------------------------------------------------------------*


!if $d(__OS2__)
install:
        $(MAKER) -DNDEBUG=1 installx
!else
install: installx $(WREQZIPV) $(WOPTZIPV) $(WNEWZIPV)
!endif

doczip:  $(DOCZIPV)

installx: $(INSTALL) $(REQZIPV) $(OPTZIPV) $(NEWZIPV) \
          $(SRCZIPV1) $(SRCZIPV2) regen
        - erase $(WORKFILE)
        @echo Installed UUPC and created ZIP files $(REQZIPV),
        @echo $(WREQZIPV), $(WOPTZIPV), $(WNEWZIPV),
        @echo $(OPTZIPV), $(NEWZIPV),
        @echo $(SRCZIPV1), $(SRCZIPV2)
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

!if $d(__OS2__)
$(PROD)\mail.exe: mail$(PSUFFIX).exe
        - erase $<
        move $? $<
        - erase $(?B: =.tds)

$(PROD)\uucp.exe: uucp$(PSUFFIX).exe
        - erase $<
        move $? $<
        - erase $(?B: =.tds)

$(PROD)\uusub.exe: uusub$(PSUFFIX).exe
        - erase $<
        move $? $<
        - erase $(?B: =.tds)

$(PROD)\uuxqt.exe: uuxqt$(PSUFFIX).exe
        - erase $<
        move $? $<
        - erase $(?B: =.tds)

$(PROD)\uupoll.exe: uupoll$(PSUFFIX).exe
        - erase $<
        move $? $<
        - erase $(?B: =.tds)

$(PROD)\uuport.exe: uuport$(PSUFFIX).exe
        - erase $<
        move $? $<
        - erase $(?B: =.tds)

$(PROD)\rmail.exe: rmail$(PSUFFIX).exe
        - erase $<
        move $? $<
        - erase $(?B: =.tds)

$(PROD)\uustat.exe: uustat$(PSUFFIX).exe
        - erase $<
        move $? $<
        - erase $(?B: =.tds)

$(PROD)\uux.exe: uux$(PSUFFIX).exe
        - erase $<
        move $? $<
        - erase $(?B: =.tds)

$(PROD)\uuname.exe: uuname$(PSUFFIX).exe
        - erase $<
        move $? $<
        - erase $(?B: =.tds)

$(FMT): fmt$(PSUFFIX).exe
        - erase $<
        move $? $<
        - erase $(?B: =.tds)

$(PROD)\gensig.exe: gensig$(PSUFFIX).exe
        - erase $<
        move $? $<
        - erase $(?B: =.tds)

$(PROD)\novrstrk.exe: novrstrk.exe
        - erase $<
        move $? $<
        - erase $(?B: =.tds)

$(PROD)\uucico.exe: uucico$(PSUFFIX).exe
        - erase $<
        move $? $<
        - erase $(?B: =.tds)

$(PROD)\rnews.exe: rnews$(PSUFFIX).exe
        - erase $<
        move $? $<
        - erase $(?B: =.tds)

$(PROD)\rn.exe: rn$(PSUFFIX).exe
        - erase $<
        move $? $<
        - erase $(?B: =.tds)

$(PROD)\newsetup.exe: newsetup$(PSUFFIX).exe
        - erase $<
        move $? $<
        - erase $(?B: =.tds)

$(PROD)\expire.exe: expire$(PSUFFIX).exe
        - erase $<
        move $? $<
        - erase $(?B: =.tds)

$(PROD)\su.cmd: $(SCRIPT)\su.cmd
        copy $? $<

$(PROD)\uuio.cmd: $(SCRIPT)\uuio.cmd
        copy $? $<

$(PROD)\uuclean.cmd: $(SCRIPT)\uuclean.cmd
        copy $? $<

$(PROD)\waiting.cmd: $(SCRIPT)\waiting.cmd
        copy $? $<

$(PROD)\getuupc.cmd: $(SCRIPT)\getuupc.cmd
        copy $? $<

$(PROD)\mailchek.cmd: $(SCRIPT)\mailchek.cmd
        copy $? $<

!else

$(PROD)\mail.exe: mail.exe
        - erase $<
        move $? $<
        - erase $(?B: =.tds)

$(PROD)\uucp.com: uucp.com
        - erase $<
        move $? $<
        - erase $(?B: =.tds)

$(PROD)\uusub.com: uusub.com
        - erase $<
        move $? $<
        - erase $(?B: =.tds)

$(PROD)\uuxqt.exe: uuxqt.exe
        - erase $<
        move $? $<
        - erase $(?B: =.tds)

$(PROD)\uupoll.exe: uupoll.exe
        - erase $<
        move $? $<
        - erase $(?B: =.tds)

$(PROD)\rmail.exe: rmail.exe
        - erase $<
        move $? $<
        - erase $(?B: =.tds)

$(PROD)\uustat.com: uustat.com
        - erase $<
        move $? $<
        - erase $(?B: =.tds)

$(PROD)\uux.com: uux.com
        - erase $<
        move $? $<
        - erase $(?B: =.tds)

$(PROD)\comm34.com: comm34.com
        - erase $<
        move $? $<
        - erase $(?B: =.tds)

$(PROD)\uuname.com: uuname.com
        - erase $<
        move $? $<
        - erase $(?B: =.tds)

$(FMT): fmt.com
        - erase $<
        move $? $<
        - erase $(?B: =.tds)

$(PROD)\gensig.com: gensig.com
        - erase $<
        move $? $<
        - erase $(?B: =.tds)

$(PROD)\novrstrk.com: novrstrk.com
        - erase $<
        move $? $<
        - erase $(?B: =.tds)

$(PROD)\uucico.exe: uucico.exe
        - erase $<
        move $? $<
        - erase $(?B: =.tds)

$(PROD)\uuport.com: uuport.com
        - erase $<
        move $? $<
        - erase $(?B: =.tds)

$(PROD)\rnews.exe: rnews.exe
        - erase $<
        move $? $<
        - erase $(?B: =.tds)

$(PROD)\rn.exe: rn.exe
        - erase $<
        move $? $<
        - erase $(?B: =.tds)

$(PROD)\newsetup.exe: newsetup.exe
        - erase $<
        move $? $<
        - erase $(?B: =.tds)

$(PROD)\expire.exe: expire.exe
        - erase $<
        move $? $<
        - erase $(?B: =.tds)

$(PROD)\su.bat: $(SCRIPT)\su.bat
        copy $? $<

$(PROD)\uuio.bat: $(SCRIPT)\uuio.bat
        copy $? $<

$(PROD)\uuclean.bat: $(SCRIPT)\uuclean.bat
        copy $? $<

$(PROD)\waiting.bat: $(SCRIPT)\waiting.bat
        copy $? $<

# *--------------------------------------------------------------------*
# *                       Windows executables                          *
# *--------------------------------------------------------------------*

$(WINPROD)\mail.exe: mailw.exe
        - erase $<
        move $? $<
        - erase $(?B: =.tds)

$(WINPROD)\uucp.exe: uucpw.exe
        - erase $<
        move $? $<
        - erase $(?B: =.tds)

$(WINPROD)\uusub.exe: uusubw.exe
        - erase $<
        move $? $<
        - erase $(?B: =.tds)

$(WINPROD)\uuxqt.exe: uuxqtw.exe
        - erase $<
        move $? $<
        - erase $(?B: =.tds)

$(WINPROD)\uupoll.exe: uupollw.exe
        - erase $<
        move $? $<
        - erase $(?B: =.tds)

$(WINPROD)\rmail.exe: rmailw.exe
        - erase $<
        move $? $<
        - erase $(?B: =.tds)

$(WINPROD)\uustat.exe: uustatw.exe
        - erase $<
        move $? $<
        - erase $(?B: =.tds)

$(WINPROD)\uux.exe: uuxw.exe
        - erase $<
        move $? $<
        - erase $(?B: =.tds)

$(WINPROD)\uuname.exe: uunamew.exe
        - erase $<
        move $? $<
        - erase $(?B: =.tds)

$(WINPROD)\uucico.exe: uucicow.exe
        - erase $<
        move $? $<
        - erase $(?B: =.tds)

$(WINPROD)\rnews.exe: rnewsw.exe
        - erase $<
        move $? $<
        - erase $(?B: =.tds)

$(WINPROD)\rn.exe: rnw.exe
        - erase $<
        move $? $<
        - erase $(?B: =.tds)

$(WINPROD)\newsetup.exe: newsetupw.exe
        - erase $<
        move $? $<
        - erase $(?B: =.tds)

$(WINPROD)\expire.exe: expirew.exe
        - erase $<
        move $? $<
        - erase $(?B: =.tds)
!endif

# *--------------------------------------------------------------------*
# *                         The .ZIP files.                            *
# *--------------------------------------------------------------------*

# *--------------------------------------------------------------------*
# *                           UUPCSRC.ZIP                              *
# *--------------------------------------------------------------------*

$(SRCZIPV1): $(MAKEFILE) $(SRCSLASH)mscmake \
             $(UUPCCFG) $(UUPCDEFS) $(DEFFILE) $(README)
        - mkdir $:.
       -12 $(ZIP) $(ZIPOPT1) < &&%
MSCMAKE
LIB\*.C
LIB\*.H
LIB\*.MAK
MAIL\*.C
MAIL\*.H
MAIL\*.MAK
UUCICO\*.ASM
UUCICO\*.C
UUCICO\*.H
UUCICO\*.MAK
UUCP\*.C
UUCP\*.H
UUCP\*.MAK
%
       -12 $(ZIP) $(ZIPOPT2) < &&%
$(README)
$(MAKEFILE)
%
       zip -z $< <  &&%
UUPC/extended $(VERS) source files (1 of 2)

Changes and Compilation Copyright (c) 1989-1993 by Kendra Electronic
Wonderworks.  May be distributed freely if original source and documentation
files are included.  Please direct all questions on UUPC/extended to
help@kew.com.
.
%

$(SRCZIPV2): $(MAKEFILE) \
             $(UUPCCFG) $(UUPCDEFS) $(DEFFILE) $(README)
        - mkdir $:.
       -12 $(ZIP) $(ZIPOPT1) < &&%
RNEWS\*.C
RNEWS\*.H
RNEWS\*.MAK
TEST\*.C
TEST\*.H
TEST\*.MAK
UTIL\*.C
UTIL\*.H
UTIL\*.MAK
%
       -12 $(ZIP) $(ZIPOPT2) < &&%
$(README)
%
       zip -z $< <  &&%
UUPC/extended $(VERS) source files (2 of 2)

Changes and Compilation Copyright (c) 1989-1993 by Kendra Electronic
Wonderworks.  May be distributed freely if original source and documentation
files are included.  Please direct all questions on UUPC/extended to
help@kew.com.
%

# *--------------------------------------------------------------------*
# *       UUPCWIN.ZIP - Word for Windows unformatted documents         *
# *--------------------------------------------------------------------*

$(WFWZIPV):  $(WINWORD)\uupcuser.doc $(WINWORD)\manual.dot $(README)
        - mkdir $:.
       -12 $(ZIP) $(ZIPOPT2) < &&%
$?
%
       zip -z $< < &&%
UUPC/extended $(VERS) Word for Windows Document Source

Changes and Compilation Copyright (c) 1989-1993 by Kendra Electronic
Wonderworks.  May be distributed freely if original source and documentation
files are included.  Please direct all questions on UUPC/extended to
help@kew.com.
%

# *--------------------------------------------------------------------*
# *                UUPCREQ.ZIP - Required Executables                  *
# *--------------------------------------------------------------------*

$(REQZIPV):  $(REQUIRED) $(README)
        - mkdir $:.
       -12 $(ZIP) $(ZIPOPT2) < &&%
$(REQUIRED)
$(README)
%
       zip -z $< <  &&%
UUPC/extended $(VERS) DOS executables (1 of 3)

Changes and Compilation Copyright (c) 1989-1993 by Kendra Electronic
Wonderworks.  May be distributed freely if original source and documentation
files are included.  Please direct all questions on UUPC/extended to
help@kew.com.
%

# *--------------------------------------------------------------------*
# *                UUPCOPT.ZIP - Optional Executables                  *
# *--------------------------------------------------------------------*

$(OPTZIPV):  $(OPTIONAL) $(README)
        - mkdir $:.
       -12 $(ZIP) $(ZIPOPT2) < &&%
$?
%
       zip -z $< <  &&%
UUPC/extended $(VERS) DOS executables (2 of 3)

Changes and Compilation Copyright (c) 1989-1993 by Kendra Electronic
Wonderworks.  May be distributed freely if original source and documentation
files are included.  Please direct all questions on UUPC/extended to
help@kew.com.
%

# *--------------------------------------------------------------------*
# *                 UUPC/extended - NEWS executables                   *
# *--------------------------------------------------------------------*

$(NEWZIPV):  $(NEWS) $(README)
        - mkdir $:.
       -12 $(ZIP) $(ZIPOPT2) < &&%
$?
%
       zip -z $< <  &&%
UUPC/extended $(VERS) DOS executables (3 of 3)

Changes and Compilation Copyright (c) 1989-1993 by Kendra Electronic
Wonderworks.  May be distributed freely if original source and documentation
files are included.  Please direct all questions on UUPC/extended to
help@kew.com.
%

# *--------------------------------------------------------------------*
# *        UUPCDOC.ZIP - Formatted (Human readable) documents          *
# *--------------------------------------------------------------------*

docs: $(DOCZIPV) $(WFWZIPV) $(PSZIPV)

$(DOCZIPV): $(SAMPLES) $(LIVEFILES) $(LISTFILES) $(README)
        - mkdir $:.
       -12 $(ZIP) $(ZIPOPT2) < &&%
$?
%
       -12 $(ZIP) $(ZIPOPT2) < &&%
$(DOCS)\*.mdm
$(DOCS)\*.inf
$(DOCS)\*.txt
$(DOCS)\*.spb
%
       zip -z $< < &&%
UUPC/extended $(VERS) documents and sample files

Changes and Compilation Copyright (c) 1989-1993 by Kendra Electronic
Wonderworks.  May be distributed freely if original source and documentation
files are included.  Please direct all questions on UUPC/extended to
help@kew.com.
%

$(PSZIPV): $(LISTFILES:.prn=.ps) $(README)
        - mkdir $:.
       -12 $(ZIP) $(ZIPOPT2) < &&%
$?
%
       zip -z $< < &&%
UUPC/extended $(VERS) documents in PostScript format

Changes and Compilation Copyright (c) 1989-1993 by Kendra Electronic
Wonderworks.  May be distributed freely if original source and documentation
files are included.  Please direct all questions on UUPC/extended to
help@kew.com.
%

# *--------------------------------------------------------------------*
# *                           Common build                             *
# *--------------------------------------------------------------------*

common: $(.path.obj) $(COMMON)  $(DEFFILE)

commonm:
        @echo $(MAKER) -f$(MAKEFILE) -DMODEL=m common
        $(MAKER) -f$(MAKEFILE) -DMODEL=m common

commonw:
        $(MAKER) -f$(MAKEFILE) -DWINDOWS common

# *--------------------------------------------------------------------*
# *                     Link our various modules                       *
# *--------------------------------------------------------------------*

!if $d(__OS2__)

mail$(PSUFFIX).exe: common
        $(MAKER) -f$(MAIL)\mail.mak -DUUPCDEFS=$(UUPCDEFS) $<

rmail$(PSUFFIX).exe: common
        $(MAKER) -f$(MAIL)\mail.mak -DUUPCDEFS=$(UUPCDEFS) $<

uucp$(PSUFFIX).exe: common
        $(MAKER) -f$(UUCP)\uucp.mak -DUUPCDEFS=$(UUPCDEFS) $<

uuname$(PSUFFIX).exe: common
        $(MAKER) -f$(UUCP)\uucp.mak -DUUPCDEFS=$(UUPCDEFS) $<

uustat$(PSUFFIX).exe: common
        $(MAKER) -f$(UUCP)\uucp.mak -DUUPCDEFS=$(UUPCDEFS) $<

uusub$(PSUFFIX).exe: common
        $(MAKER) -f$(UUCP)\uucp.mak -DUUPCDEFS=$(UUPCDEFS) $<

uux$(PSUFFIX).exe: common
        $(MAKER) -f$(UUCP)\uucp.mak -DUUPCDEFS=$(UUPCDEFS) $<

uuxqt$(PSUFFIX).exe: common
        $(MAKER) -f$(UUCP)\uucp.mak -DUUPCDEFS=$(UUPCDEFS) $<

fmt$(PSUFFIX).exe: common
        $(MAKER) -f$(UTIL)\util.mak -DUUPCDEFS=$(UUPCDEFS) $<

gensig$(PSUFFIX).exe: common
        $(MAKER) -f$(UTIL)\util.mak -DUUPCDEFS=$(UUPCDEFS) $<

novrstrk.exe: common
        $(MAKER) -f$(UTIL)\util.mak -DUUPCDEFS=$(UUPCDEFS) $<

uupoll$(PSUFFIX).exe: common
        $(MAKER) -f$(UTIL)\util.mak -DUUPCDEFS=$(UUPCDEFS) $<

uuport$(PSUFFIX).exe: common
        $(MAKER) -f$(UUCICO)\uucico.mak -DUUPCDEFS=$(UUPCDEFS) $<

expire$(PSUFFIX).exe: common
        $(MAKER) -f$(RNEWS)\rnews.mak -DUUPCDEFS=$(UUPCDEFS) $<

rnews$(PSUFFIX).exe: common
        $(MAKER) -f$(RNEWS)\rnews.mak -DUUPCDEFS=$(UUPCDEFS) $<

uucico$(PSUFFIX).exe: common
        $(MAKER) -f$(UUCICO)\UUCICO.mak -DUUPCDEFS=$(UUPCDEFS) $<

rn$(PSUFFIX).exe: common
        $(MAKER) -f$(RN)\RN.mak -DUUPCDEFS=$(UUPCDEFS) $<

newsetup$(PSUFFIX).exe: common
        $(MAKER) -f$(RN)\RN.mak -DUUPCDEFS=$(UUPCDEFM) $<

!else
mail.exe: commonm
        $(MAKER) -f$(MAIL)\mail.mak -DUUPCDEFS=$(UUPCDEFM) $<

rmail.exe: common
        $(MAKER) -f$(MAIL)\mail.mak -DUUPCDEFS=$(UUPCDEFS) $<

uucp.exe: common
        $(MAKER) -f$(UUCP)\uucp.mak -DUUPCDEFS=$(UUPCDEFS) $<

uuname.exe: common
        $(MAKER) -f$(UUCP)\uucp.mak -DUUPCDEFS=$(UUPCDEFS) $<

uustat.exe: common
        $(MAKER) -f$(UUCP)\uucp.mak -DUUPCDEFS=$(UUPCDEFS) $<

uusub.exe: common
        $(MAKER) -f$(UUCP)\uucp.mak -DUUPCDEFS=$(UUPCDEFS) $<

uux.exe: common
        $(MAKER) -f$(UUCP)\uucp.mak -DUUPCDEFS=$(UUPCDEFS) $<

uuxqt.exe: common
        $(MAKER) -f$(UUCP)\uucp.mak -DUUPCDEFS=$(UUPCDEFS) $<

uupoll.exe: common
        $(MAKER) -f$(UTIL)\util.mak -DUUPCDEFS=$(UUPCDEFS) $<

expire.exe: common
        $(MAKER) -f$(RNEWS)\rnews.mak -DUUPCDEFS=$(UUPCDEFS) $<

rnews.exe: common
        $(MAKER) -f$(RNEWS)\rnews.mak -DUUPCDEFS=$(UUPCDEFS) $<

uucico.exe: commonm
        $(MAKER) -f$(UUCICO)\UUCICO.mak -DUUPCDEFS=$(UUPCDEFM) $<

uuport.com: commonm
        $(MAKER) -f$(UUCICO)\UUCICO.mak -DUUPCDEFS=$(UUPCDEFM) $<

rn.exe: commonm
        $(MAKER) -f$(RN)\RN.mak -DUUPCDEFS=$(UUPCDEFM) $<

newsetup.exe: commonm
        $(MAKER) -f$(RN)\RN.mak -DUUPCDEFS=$(UUPCDEFM) $<

uucp.com: common
        $(MAKER) -f$(UUCP)\uucp.mak -DUUPCDEFS=$(UUPCDEFS) $<

uuname.com: common
        $(MAKER) -f$(UUCP)\uucp.mak -DUUPCDEFS=$(UUPCDEFS) $<

uustat.com: common
        $(MAKER) -f$(UUCP)\uucp.mak -DUUPCDEFS=$(UUPCDEFS) $<

uusub.com: common
        $(MAKER) -f$(UUCP)\uucp.mak -DUUPCDEFS=$(UUPCDEFS) $<

uux.com: common
        $(MAKER) -f$(UUCP)\uucp.mak -DUUPCDEFS=$(UUPCDEFS) $<

comm34.com: common
        $(MAKER) -f$(UTIL)\util.mak -DUUPCDEFS=$(UUPCDEFS) $<

fmt.com: common
        $(MAKER) -f$(UTIL)\util.mak -DUUPCDEFS=$(UUPCDEFS) $<

gensig.com: common
        $(MAKER) -f$(UTIL)\util.mak -DUUPCDEFS=$(UUPCDEFS) $<

novrstrk.com: common
        $(MAKER) -f$(UTIL)\util.mak -DUUPCDEFS=$(UUPCDEFS) $<

# *--------------------------------------------------------------------*
# *                     Link our Windows modules                       *
# *--------------------------------------------------------------------*

mailw.exe: commonw
        $(MAKER) -f$(MAIL)\mail.mak -DUUPCDEFS=$(UUPCDEFW) $<

rmailw.exe: commonw
        $(MAKER) -f$(MAIL)\mail.mak -DUUPCDEFS=$(UUPCDEFW) $<

uucpw.exe: commonw
        $(MAKER) -f$(UUCP)\uucp.mak -DUUPCDEFS=$(UUPCDEFW) $<

uunamew.exe: commonw
        $(MAKER) -f$(UUCP)\uucp.mak -DUUPCDEFS=$(UUPCDEFW) $<

uunamew.com: commonw
        $(MAKER) -f$(UUCP)\uucp.mak -DUUPCDEFS=$(UUPCDEFW) $<

uustatw.exe: commonw
        $(MAKER) -f$(UUCP)\uucp.mak -DUUPCDEFS=$(UUPCDEFW) $<

uusubw.exe: commonw
        $(MAKER) -f$(UUCP)\uucp.mak -DUUPCDEFS=$(UUPCDEFW) $<

uuxw.exe: commonw
        $(MAKER) -f$(UUCP)\uucp.mak -DUUPCDEFS=$(UUPCDEFW) $<

uuxqtw.exe: commonw
        $(MAKER) -f$(UUCP)\uucp.mak -DUUPCDEFS=$(UUPCDEFW) $<

uupollw.exe: commonw
        $(MAKER) -f$(UTIL)\util.mak -DUUPCDEFS=$(UUPCDEFW) $<

expirew.exe: commonw
        $(MAKER) -f$(RNEWS)\rnews.mak -DUUPCDEFS=$(UUPCDEFW) $<

rnewsw.exe: commonw
        $(MAKER) -f$(RNEWS)\rnews.mak -DUUPCDEFS=$(UUPCDEFW) $<

uucicow.exe: commonw
        $(MAKER) -f$(UUCICO)\UUCICO.mak -DUUPCDEFS=$(UUPCDEFW) $<

# *--------------------------------------------------------------------*
# *               WUUPCREQ.ZIP - Required Windows Executables          *
# *--------------------------------------------------------------------*

$(WREQZIPV):  $(WREQUIRED) $(README)
        - mkdir $:.
       -12 $(ZIP) $(ZIPOPT2) < &&%
$?
%
       zip -z $< <  &&%
UUPC/extended $(VERS) Windows 3.x executables (1 of 3)

Special thanks to Robert B. Denny for performing the Windows 3.x port.

Changes and Compilation Copyright (c) 1989-1993 by Kendra Electronic
Wonderworks.  May be distributed freely if original source and documentation
files are included.  Please direct all questions on UUPC/extended to
help@kew.com.
%

# *--------------------------------------------------------------------*
# *               WUUPCOPT.ZIP - Optional Windows Executables          *
# *--------------------------------------------------------------------*

$(WOPTZIPV):  $(WOPTIONAL) $(README)
        - mkdir $:.
       -12 $(ZIP) $(ZIPOPT2) < &&%
$?
%
       zip -z $< <  &&%
UUPC/extended $(VERS) Windows 3.x executables (2 of 3)

Special thanks to Robert B. Denny for performing the Windows 3.x port.

Changes and Compilation Copyright (c) 1989-1993 by Kendra Electronic
Wonderworks.  May be distributed freely if original source and documentation
files are included.  Please direct all questions on UUPC/extended to
help@kew.com.
%

# *--------------------------------------------------------------------*
# *                 UUPC/extended - NEWS executables                   *
# *--------------------------------------------------------------------*

$(WNEWZIPV):  $(WNEWS) $(README)
        - mkdir $:.
       -12 $(ZIP) $(ZIPOPT2) < &&%
$?
%
       zip -z $< <  &&%
UUPC/extended $(VERS) Windows 3.x executables (3 of 3)

Special thanks to Robert B. Denny for performing the Windows 3.x port.

Changes and Compilation Copyright (c) 1989-1993 by Kendra Electronic
Wonderworks.  May be distributed freely if original source and documentation
files are included.  Please direct all questions on UUPC/extended to
help@kew.com.
%
!endif

$(.path.obj):
        - mkdir $(.path.obj)

# *--------------------------------------------------------------------*
# *     Force a regeneration of the time stamp/version module.         *
# *--------------------------------------------------------------------*

regen:  $(LIB)\timestmp.c
        - erase $(TIMESTMP)

# *--------------------------------------------------------------------*
# *     Common library build                                           *
# *--------------------------------------------------------------------*

$(UUPCLIB):  DUMMY
        $(MAKER) -f $(LIB)\lib.mak -DUUPCDEFS=$(UUPCDEFS) $<

DUMMY:
        @rem hello world

$(UUPCLIBW):
        $(MAKER) -f $(LIB)\lib.mak -DUUPCDEFS=$(UUPCDEFW) $<

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



!if $d(__OS2__)
OPTIMIZEOPT=-O2
!else
MODELOPT=-m$(MODEL)
OPTIMIZEOPT=-O
!if $d(WINDOWS)
GENERATEOPT=-W
CODEGENOPT=-3
!else
GENERATEOPT=-D_MSC
!endif
!endif

$(UUPCCFG): $(MAKEFILE) \
            $(LIB)\lib.mak \
            $(MAIL)\mail.mak \
            $(UUCP)\uucp.mak \
            $(UUCICO)\uucico.mak \
            $(UTIL)\util.mak
  copy &&|
$(GENERATEOPT)
$(WINDOWSOPT)
$(CODEGENOPT)
$(MODELOPT)
$(OPTMIZEOPT)
-c
-d
-DUUPCV="$(VERS)"
-DIOCTL
-f
-G
-I$(LIB)
-I$(BINC)
-L$(BLIB)
-n$(OBJ)
-N
-k-
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
-P-
| $<

!if $d(WINDOWS)
WINDOWSDEF = WINDOWS=1
!endif

$(UUPCDEFS): $(MAKEFILE)
  copy &&|
.path.obj  = $(.path.obj)
ARCHIVE    = $(ARCHIVE)
BINC       = $(BINC)
BLIB       = $(BLIB)
BORLANDC   = $(BORLANDC)
CC         = $(CC)
COMMON     = $(COMMON)
DOCS       = $(DOCS)
DOCSLIST   = $(DOCSLIST)
DOCW       = $(DOCW)
DEFFILE    = $(DEFFILE)
FMT        = $(FMT)
INSTALL    = $(INSTALL)
LIB        = $(LIB)
LIBRARIES  = $(LIBRARIES)
LIBRARY    = $(LIBRARY)
LIBRARY2   = $(LIBRARY2)
LINKER     = $(LINKER)
LINKOPTN   = $(LINKOPTN)
LINKOPTD   = $(LINKOPTD)
LINKOPTT   = $(LINKOPTT)
MAIL       = $(MAIL)
MAKEFILE   = $(MAKEFILE)
MAP        = $(MAP)
MODEL      = $(MODEL)
NEWS       = $(NEWS)
OBJ        = $(OBJ)
OPTIONAL   = $(OPTIONAL)
PSUFFIX    = $(PSUFFIX)
REQUIRED   = $(REQUIRED)
RN         = $(RN)
RNEWS      = $(RNEWS)
SAMPLES    = $(SAMPLES)
SAMPLEX    = $(SAMPLEX)
SRC        = $(SRC)
STARTUP    = $(STARTUP)
STARTUPT   = $(STARTUPT)
SUFFIX     = $(SUFFIX)
TASM       = $(TASM)
TASMOPT    = $(TASMOPT)
TEST       = $(TEST)
TIMESTMP   = $(TIMESTMP)
TMP        = $(TMP)
UTIL       = $(UTIL)
UUCICO     = $(UUCICO)
UUCP       = $(UUCP)
UUPCCFG    = $(UUPCCFG)
UUPCLIB    = $(UUPCLIB)
VERS       = $(VERS)
WINWORD    = $(WINWORD)
WORKFILE   = $(WORKFILE)
$(WINDOWSDEF)

.autodepend

.silent
| $<

!if $d(WINDOWS)
$(DEFFILE): $(MAKEFILE)
  copy &&|
EXETYPE WINDOWS
DATA MOVABLE MULTIPLE
STACKSIZE 8182
HEAPSIZE  12288
Description 'UUPC/extended $(VERS), Changes Copyright (c) 1989-1993 Kendra Electronic Wonderworks, All Rights Reserved'
| $<
!endif

!if $d(__OS2__)
$(DEFFILE): $(MAKEFILE)
  copy &&|
DATA MULTIPLE
STACKSIZE 32760
EXETYPE OS2
Description 'UUPC/extended $(VERS), Changes Copyright (c) 1989-1993 Kendra Electronic Wonderworks, All Rights Reserved'
| $<
!endif
