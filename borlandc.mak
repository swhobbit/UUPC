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
# *   Changes Copyright (c) 1989-1994 by Kendra Electronic             *
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
# *             erase in a MOVE.BAT file.  This is slower than a true  *
# *             move program (which just renames between directories), *
# *             but life is hard.                                      *
# *--------------------------------------------------------------------*
#
#     $Id: makefile 1.62 1994/05/04 02:00:57 ahd Exp $
#
#     Revision history:
#     $Log: makefile $
#         Revision 1.62  1994/05/04  02:00:57  ahd
#         Allow disabling TDSTRIP
#
#         Revision 1.61  1994/03/20  00:54:41  ahd
#         Go to version 1.12j
#
#         Revision 1.60  1994/03/12  14:27:25  ahd
#         Allow easy use of short path names to speed compiles
#
#         Revision 1.59  1994/03/12  13:40:47  ahd
#         Add NMAKE included files to ZIP dependents
#
#         Revision 1.58  1994/03/06  12:24:47  ahd
#         Update makefile names for non-Borland compilers
#
#         Revision 1.57  1994/02/23  04:16:05  ahd
#         Delete TC pre-compiled headers at end of build
#
#         Revision 1.56  1994/02/20  19:03:21  ahd
#         IBM C/Set 2 Conversion, memory leak cleanup
#
#         Revision 1.55  1994/02/13  04:44:52  ahd
#         Force UUCP to be .exe file
#
#         Revision 1.54  1994/01/18  13:27:08  ahd
#         Go to version 1.12i
#
#         Revision 1.53  1994/01/06  12:45:03  ahd
#         Version 1.12h
#
#         Revision 1.52  1994/01/02  04:32:04  ahd
#         Go to version 1.12g
#
#         Revision 1.51  1994/01/01  18:56:23  ahd
#         Annual copyright update
#
#         Revision 1.50  1993/12/29  02:44:54  ahd
#         Fix those darn pif files
#
#         Revision 1.49  1993/12/26  16:57:41  ahd
#         Correct .PIF file defines
#
#         Revision 1.48  1993/12/26  14:35:35  ahd
#         Add IBM OS/2 Makefile, Windows PIF files
#
#         Revision 1.47  1993/12/24  05:11:38  ahd
#         Use short path for OS/2 make
#
#         Revision 1.46  1993/12/06  01:58:05  ahd
#         Delete IOCTL define
#
#         Revision 1.45  1993/12/02  02:24:39  ahd
#         Rename .DEF files so they stay around
#
#         Revision 1.45  1993/12/02  02:24:39  ahd
#         Rename .DEF files so they stay around
#
#         Revision 1.44  1993/11/30  04:17:36  ahd
#         Version 1.12f
#
#         Revision 1.43  1993/11/21  04:45:26  ahd
#         Add UUTRAF
#         Reorder module builds
#         Optimize OS/2 deletes
#
#         Revision 1.42  1993/11/20  14:48:14  ahd
#         Up error code for ZIP to 18
#
#         Revision 1.41  1993/10/31  21:32:16  ahd
#         Add inews, genhist to DOS build
#
#         Revision 1.40  1993/10/31  11:56:20  ahd
#         Add inews, genhist to build
#         Delete FMT as special module name
#         Add 00readme.now to source archives in proper input directory
#
#         Revision 1.39  1993/10/30  22:26:26  ahd
#         Correct directory for source scripts
#
#         Revision 1.38  1993/10/28  00:17:24  ahd
#         Go to version 1.12e
#
#         Revision 1.37  1993/10/26  12:19:29  ahd
#         Also archive source copies of script files
#
#         Revision 1.36  1993/10/24  23:23:50  ahd
#         Correct build of uuport
#
#         Revision 1.35  1993/10/16  15:12:06  ahd
#         Break source into four archives
#
#         Revision 1.34  1993/10/13  01:47:08  ahd
#         Go to 1.12d
#
#         Revision 1.33  1993/10/05  12:27:28  ahd
#         Begin the adventure of 1.12c
#
#         Revision 1.32  1993/10/04  15:51:54  ahd
#         Use only root name for adding 00readme.now
#
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

#.silent

#       The memory model to be built, and other environment
#       specific information

WINMODEL=m

!if $d(__OS2__)
MODEL=2
SUFFIX   = $(MODEL)
PSUFFIX  = 2
DEFFILE  = $(SRCSLASH)UUPCOS23.DEF
ENVIRONMENT=OS/2 32 bit
!elif $d(WINDOWS)
MODEL    = $(WINMODEL)
SUFFIX   = w$(MODEL)
PSUFFIX  = w
DEFFILE  = $(SRCSLASH)UUPCWIND.DEF
!else
!if !$d(MODEL)
MODEL    = s
!endif          #
SUFFIX   = $(MODEL)
PSUFFIX  =
DEFFILE  =
ENVIRONMENT=MS-DOS
!endif
!if !$d(TDSTRIP)
TDSTRIP=tdstrip
!endif

#       Silly hack to allow back slash as last character in variable
!if !$d(SRCSLASH) && !$d(SHORTPATH)
!if $d(__OS2__)
SRCSLASH=
!else
SRC      = e:/src/uupc/
SRCSLASH = $(SRC:/=\)
!endif
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

VERS = 1.12j

# *--------------------------------------------------------------------*
# *                           Directories                              *
# *--------------------------------------------------------------------*

!if !$d(TMP)
TMP     = \TMP
!endif

CONF    = \UUPC
DOCS    = $(SRCSLASH)DOCS
LIB     = $(SRCSLASH)LIB
MAIL    = $(SRCSLASH)MAIL
OBJ     = $(SRCSLASH)OBJBC$(SUFFIX)
HDRCACHE= $(SRCSLASH)TCDEF$(SUFFIX).SYM
RN      = $(SRCSLASH)RN
RNEWS   = $(SRCSLASH)RNEWS
SCRIPT  = $(SRCSLASH)SCRIPTS
TEST    = $(SRCSLASH)TEST
UTIL    = $(SRCSLASH)UTIL
UUCICO  = $(SRCSLASH)UUCICO
UUCP    = $(SRCSLASH)UUCP
UUTRAF  = $(SRCSLASH)UUTRAF
WINWORD = $(SRCSLASH)WINWORD

!if !$d(WINSOCK)
WINSOCK = WINSOCK
!endif

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

!if $d(__OS2__)
UUPCDEFM=$(UUPCDEFS)
!else
UUPCDEFM=$(TMP)\uupcm.mak
UUPCDEFW=$(TMP)\uupcw$(WINMODEL).mak
!endif

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
README=     docs\00readme.now   # Can't have full path

REQCOMM = $(PROD)\mail.exe $(PROD)\rmail.exe $(PROD)\uucico.exe\
          $(PROD)\uuxqt.exe
SCRIPTS=  $(PROD)\su.bat $(PROD)\uuclean.bat $(PROD)\uuio.bat \
          $(PROD)\waiting.bat
OPTCOMM = $(PROD)\uucp.exe $(PROD)\uuname.com $(PROD)\uupoll.exe\
          $(PROD)\uustat.com $(PROD)\uusub.com $(PROD)\uuport.com \
          $(PROD)\uux.com $(PROD)\fmt.com $(PROD)\gensig.com \
          $(PROD)\novrstrk.com

!if $d(__OS2__)
REQUIRED  = $(REQCOMM:.com=.exe)
OPTIONAL= $(OPTCOMM:.com=.exe) \
          $(SCRIPTS:.bat=.cmd) $(PROD)\mailchek.cmd $(PROD)\getuupc.cmd
!else
OPTIONAL= $(OPTCOMM) $(PROD)\comm34.com $(SCRIPTS) $(PROD)\uuclean.pif
REQUIRED  = $(REQCOMM) $(PROD)\rmail.pif $(PROD)\uuxqt.pif

# *--------------------------------------------------------------------*
# *      Of the "required" modules, we only build UUCICO under         *
# *      Windows because the MAIL program is ugly and we prefer        *
# *      UUXQT run RMAIL and RNEWS as normal DOS programs.             *
# *--------------------------------------------------------------------*

#WREQUIRED=$(WINPROD)\mail.exe $(WINPROD)\rmail.exe $(WINPROD)\uucico.exe\
#          $(WINPROD)\uuxqt.exe
WREQUIRED=$(WINPROD)\uucico.exe
WOPTIONAL=$(WINPROD)\uuname.exe $(WINPROD)\uupoll.exe $(WINPROD)\uustat.exe\
          $(WINPROD)\uusub.exe $(WINPROD)\uuport.exe
WNEWS     = #   No modules for news
#WNEWS    =$(WINPROD)\expire.exe  $(WINPROD)\uutraf.exe \
#         $(WINPROD)\rnews.exe $(WINPROD)\rn.exe $(WINPROD)\newsetup.exe
!endif

NEWS    = $(PROD)\EXPIRE.EXE $(PROD)\GENHIST.EXE $(PROD)\INEWS.EXE \
          $(PROD)\RNEWS.EXE $(PROD)\UUTRAF.EXE
#         $(PROD)\rn.exe $(PROD)\newsetup.exe

INSTALL = $(REQUIRED) $(OPTIONAL) $(NEWS) $(WREQUIRED) $(WOPTIONAL) $(WNEWS)

SAMPLES = $(DOCS)\SYSTEMS $(DOCS)\PASSWD $(DOCS)\HOSTPATH \
          $(DOCS)\personal.rc $(DOCS)\uupc.rc \
          $(DOCS)\personal.sig $(DOCS)\aliases.txt \
          $(DOCS)\mail.ico $(DOCS)\uucico.ico $(DOCS)\up-pif.dvp
SAMPLEX = SYSTEMS PASSWD HOSTPATH *.RC *.MDM PERSONAL.SIG ALIASES.TXT *.ICO

MAKEFILE = MAKEFILE

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
#WNEWZIPV = $(ARCHIVE)\$(ZIPPREFIX)W3.ZIP
!endif

DOCZIPV = $(ARCHIVE)\$(ZIPPREFIX)AD.ZIP
WFWZIPV = $(ARCHIVE)\$(ZIPPREFIX)AW.ZIP
PSZIPV  = $(ARCHIVE)\$(ZIPPREFIX)AP.ZIP
SRCZIPV1 = $(ARCHIVE)\$(ZIPPREFIX)S1.ZIP
SRCZIPV2 = $(SRCZIPV1:S1=S2)
SRCZIPV3 = $(SRCZIPV1:S1=S3)
SRCZIPV4 = $(SRCZIPV1:S1=S4)

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
MAKER=maker -DTDSTRIP=$(TDSTRIP)
!endif

# *--------------------------------------------------------------------*
# *      High speed delete command for OS/2, bypasses archiving        *
# *--------------------------------------------------------------------*

!if !$d(ERASE)
!if $d(__OS2__)
ERASE=DEL /F
!else
ERASE=DEL
!endif
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

test:    expire$(PSUFFIX).exe fmt$(PSUFFIX).exe genhist$(PSUFFIX).exe   \
         gensig$(PSUFFIX).exe inews$(PSUFFIX).exe mail$(PSUFFIX).exe    \
         novrstr$(PSUFFIX).exe rmail$(PSUFFIX).exe rnews$(PSUFFIX).exe \
         uucico$(PSUFFIX).exe uucp$(PSUFFIX).exe uuname$(PSUFFIX).exe   \
         uupoll$(PSUFFIX).exe uuport$(PSUFFIX).exe uustat$(PSUFFIX).exe \
         uusub$(PSUFFIX).exe uutraf$(PSUFFIX).exe uux$(PSUFFIX).exe     \
         uuxqt$(PSUFFIX).exe

!else

test:    comm34.com expire.exe fmt.com genhist.exe gensig.com           \
         inews.exe mail.exe novrstrk.com rmail.exe rnews.exe            \
         uucico.exe uucp.exe uuname.exe uupoll.exe uuport.exe           \
         uustat.exe uusub.exe uutraf.exe uux.exe uuxqt.exe

# *--------------------------------------------------------------------*
# *      Note that we don't actually install all the Windows           *
# *      modules we build for testing.  This, like the unused news     *
# *      modules definitions, are really for furture expandsion.       *
# *--------------------------------------------------------------------*

windows: expirew.exe mailw.exe rmailw.exe rnewsw.exe uucicow.exe        \
         uucpw.exe uupollw.exe uuportw.exe uustatw.exe uusubw.exe       \
         uutrafw.exe uuxqtw.exe uuxw.exe

!endif

# *--------------------------------------------------------------------*
# *   Prod build the basic modules and then moves them to $(PROD)      *
# *--------------------------------------------------------------------*

!if $d(__OS2__)
prod:   $(REQUIRED:.com=.exe) $(OPTIONAL:.com=.exe) $(NEWS:.com=.exe)
        - $(ERASE) $(TIMESTMP)
        - $(ERASE) $(UUPCLIB)
        - $(ERASE) *.sym
!else
prod:   $(INSTALL)
        - $(ERASE) $(TIMESTMP)
        - $(ERASE) $(UUPCLIB)
        - $(ERASE) *.sym
!endif

winprod:  commonw $(WREQUIRED) $(WOPTIONAL) $(WNEWS)
        - del *.tds
        - $(ERASE) *.sym

required: $(REQUIRED)

# *--------------------------------------------------------------------*
# *     Install builds the UUPC modules, installs the executables      *
# *     in the production directory, formats the documentation files,  *
# *     and creates PKZIP files for giving away.                       *
# *--------------------------------------------------------------------*

!if $d(__OS2__)
install:
        $(MAKER) -DNDEBUG=1 installx \
        $(SRCZIPV1) $(SRCZIPV2) $(SRCZIPV3) $(SRCZIPV4)
!else
install: installx $(WREQZIPV) $(WOPTZIPV) $(WNEWZIPV)
        - del *.sym
!endif

doczip:  $(DOCZIPV)

installx: $(INSTALL) $(REQZIPV) $(OPTZIPV) $(NEWZIPV) \
           regen
        - $(ERASE) $(WORKFILE)
        @echo Installed UUPC and created ZIP files $(REQZIPV),
        @echo $(WREQZIPV), $(WOPTZIPV), $(WNEWZIPV),
        @echo $(OPTZIPV), $(NEWZIPV)
        - $(ERASE) $(TIMESTMP)
        - $(ERASE) $(UUPCLIB)

# *--------------------------------------------------------------------*
# *                 Build just the source .ZIP file.                   *
# *--------------------------------------------------------------------*

szip:   $(SRCZIPV1) $(SRCZIPV2) $(SRCZIPV3) $(SRCZIPV4)

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

$(PROD)\expire.exe: expire$(PSUFFIX).exe
        - $(ERASE) $<
        move $? $<
!if !$d(__OS2__)
        - $(ERASE) $(?B: =.tds)
!endif

$(PROD)\fmt.exe: fmt$(PSUFFIX).exe
        - $(ERASE) $<
        move $? $<
!if !$d(__OS2__)
        - $(ERASE) $(?B: =.tds)
!endif

$(PROD)\gensig.exe: gensig$(PSUFFIX).exe
        - $(ERASE) $<
        move $? $<
!if !$d(__OS2__)
        - $(ERASE) $(?B: =.tds)
!endif

$(PROD)\genhist.exe: genhist$(PSUFFIX).exe
        - $(ERASE) $<
        move $? $<
!if !$d(__OS2__)
        - $(ERASE) $(?B: =.tds)
!endif

$(PROD)\inews.exe: inews$(PSUFFIX).exe
        - $(ERASE) $<
        move $? $<
!if !$d(__OS2__)
        - $(ERASE) $(?B: =.tds)
!endif

$(PROD)\mail.exe: mail$(PSUFFIX).exe
        - $(ERASE) $<
        move $? $<
!if !$d(__OS2__)
        - $(ERASE) $(?B: =.tds)
!endif

$(PROD)\newsetup.exe: newsetup$(PSUFFIX).exe
        - $(ERASE) $<
        move $? $<
!if !$d(__OS2__)
        - $(ERASE) $(?B: =.tds)
!endif

$(PROD)\novrstrk.exe: novrstr$(PSUFFIX).exe
        - $(ERASE) $<
        move $? $<
!if !$d(__OS2__)
        - $(ERASE) $(?B: =.tds)
!endif

$(PROD)\rmail.exe: rmail$(PSUFFIX).exe
        - $(ERASE) $<
        move $? $<
!if !$d(__OS2__)
        - $(ERASE) $(?B: =.tds)
!endif

$(PROD)\rn.exe: rn$(PSUFFIX).exe
        - $(ERASE) $<
        move $? $<
!if !$d(__OS2__)
        - $(ERASE) $(?B: =.tds)
!endif

$(PROD)\rnews.exe: rnews$(PSUFFIX).exe
        - $(ERASE) $<
        move $? $<
!if !$d(__OS2__)
        - $(ERASE) $(?B: =.tds)
!endif

$(PROD)\uucico.exe: uucico$(PSUFFIX).exe
        - $(ERASE) $<
        move $? $<
!if !$d(__OS2__)
        - $(ERASE) $(?B: =.tds)
!endif

$(PROD)\uucp.exe: uucp$(PSUFFIX).exe
        - $(ERASE) $<
        move $? $<
!if !$d(__OS2__)
        - $(ERASE) $(?B: =.tds)
!endif

$(PROD)\uuname.exe: uuname$(PSUFFIX).exe
        - $(ERASE) $<
        move $? $<
!if !$d(__OS2__)
        - $(ERASE) $(?B: =.tds)
!endif

$(PROD)\uupoll.exe: uupoll$(PSUFFIX).exe
        - $(ERASE) $<
        move $? $<
!if !$d(__OS2__)
        - $(ERASE) $(?B: =.tds)
!endif

$(PROD)\uuport.exe: uuport$(PSUFFIX).exe
        - $(ERASE) $<
        move $? $<
!if !$d(__OS2__)
        - $(ERASE) $(?B: =.tds)
!endif

$(PROD)\uustat.exe: uustat$(PSUFFIX).exe
        - $(ERASE) $<
        move $? $<
!if !$d(__OS2__)
        - $(ERASE) $(?B: =.tds)
!endif

$(PROD)\uusub.exe: uusub$(PSUFFIX).exe
        - $(ERASE) $<
        move $? $<
!if !$d(__OS2__)
        - $(ERASE) $(?B: =.tds)
!endif

$(PROD)\uutraf.exe: uutraf$(PSUFFIX).exe
        - $(ERASE) $<
        move $? $<
!if !$d(__OS2__)
        - $(ERASE) $(?B: =.tds)
!endif

$(PROD)\uux.exe: uux$(PSUFFIX).exe
        - $(ERASE) $<
        move $? $<
!if !$d(__OS2__)
        - $(ERASE) $(?B: =.tds)
!endif

$(PROD)\uuxqt.exe: uuxqt$(PSUFFIX).exe
        - $(ERASE) $<
        move $? $<
!if !$d(__OS2__)
        - $(ERASE) $(?B: =.tds)
!endif

$(PROD)\getuupc.cmd: $(SCRIPT)\getuupc.cmd
        copy $? $<

$(PROD)\mailchek.cmd: $(SCRIPT)\mailchek.cmd
        copy $? $<

$(PROD)\su.cmd: $(SCRIPT)\su.cmd
        copy $? $<

$(PROD)\uuclean.cmd: $(SCRIPT)\uuclean.cmd
        copy $? $<

$(PROD)\uuio.cmd: $(SCRIPT)\uuio.cmd
        copy $? $<

$(PROD)\waiting.cmd: $(SCRIPT)\waiting.cmd
        copy $? $<

!if !$d(__OS2__)

$(PROD)\uusub.com: uusub.com
        - $(ERASE) $<
        move $? $<
        - $(ERASE) $(?B: =.tds)

$(PROD)\uustat.com: uustat.com
        - $(ERASE) $<
        move $? $<
        - $(ERASE) $(?B: =.tds)

$(PROD)\uux.com: uux.com
        - $(ERASE) $<
        move $? $<
        - $(ERASE) $(?B: =.tds)

$(PROD)\comm34.com: comm34.com
        - $(ERASE) $<
        move $? $<
        - $(ERASE) $(?B: =.tds)

$(PROD)\uuname.com: uuname.com
        - $(ERASE) $<
        move $? $<
        - $(ERASE) $(?B: =.tds)

$(PROD)\fmt.com: fmt.com
        - $(ERASE) $<
        move $? $<
        - $(ERASE) $(?B: =.tds)

$(PROD)\gensig.com: gensig.com
        - $(ERASE) $<
        move $? $<
        - $(ERASE) $(?B: =.tds)

$(PROD)\novrstrk.com: novrstrk.com
        - $(ERASE) $<
        move $? $<
        - $(ERASE) $(?B: =.tds)

$(PROD)\uuport.com: uuport.com
        - $(ERASE) $<
        move $? $<
        - $(ERASE) $(?B: =.tds)

$(PROD)\su.bat: $(SCRIPT)\su.bat
        copy $? $<

$(PROD)\uuio.bat: $(SCRIPT)\uuio.bat
        copy $? $<

$(PROD)\uuclean.bat: $(SCRIPT)\uuclean.bat
        copy $? $<

$(PROD)\waiting.bat: $(SCRIPT)\waiting.bat
        copy $? $<

$(PROD)\uuclean.pif: $(DOCS)\uuclean.pif
        copy $? $<

$(PROD)\uuxqt.pif: $(DOCS)\uuxqt.pif
        copy $? $<

$(PROD)\rmail.pif: $(DOCS)\rmail.pif
        copy $? $<

# *--------------------------------------------------------------------*
# *                       Windows executables                          *
# *--------------------------------------------------------------------*

!if "$(PSUFFIX)" != "w"

$(WINPROD)\expire.exe: expirew.exe
        - $(ERASE) $<
        move $? $<
        - $(ERASE) $(?B: =.tds)

$(WINPROD)\mail.exe: mailw.exe
        - $(ERASE) $<
        move $? $<
        - $(ERASE) $(?B: =.tds)

$(WINPROD)\newsetup.exe: newsetupw.exe
        - $(ERASE) $<
        move $? $<
        - $(ERASE) $(?B: =.tds)

$(WINPROD)\rmail.exe: rmailw.exe
        - $(ERASE) $<
        move $? $<
        - $(ERASE) $(?B: =.tds)

$(WINPROD)\rn.exe: rnw.exe
        - $(ERASE) $<
        move $? $<
        - $(ERASE) $(?B: =.tds)

$(WINPROD)\rnews.exe: rnewsw.exe
        - $(ERASE) $<
        move $? $<
        - $(ERASE) $(?B: =.tds)

$(WINPROD)\uucico.exe: uucicow.exe
        - $(ERASE) $<
        move $? $<
        - $(ERASE) $(?B: =.tds)

$(WINPROD)\uucp.exe: uucpw.exe
        - $(ERASE) $<
        move $? $<
        - $(ERASE) $(?B: =.tds)

$(WINPROD)\uuname.exe: uunamew.exe
        - $(ERASE) $<
        move $? $<
        - $(ERASE) $(?B: =.tds)

$(WINPROD)\uupoll.exe: uupollw.exe
        - $(ERASE) $<
        move $? $<
        - $(ERASE) $(?B: =.tds)

$(WINPROD)\uuport.exe: uuportw.exe
        - $(ERASE) $<
        move $? $<
        - $(ERASE) $(?B: =.tds)

$(WINPROD)\uustat.exe: uustatw.exe
        - $(ERASE) $<
        move $? $<
        - $(ERASE) $(?B: =.tds)

$(WINPROD)\uusub.exe: uusubw.exe
        - $(ERASE) $<
        move $? $<
        - $(ERASE) $(?B: =.tds)

$(WINPROD)\uutraf.exe: uutrafw.exe
        - $(ERASE) $<
        move $? $<
        - $(ERASE) $(?B: =.tds)

$(WINPROD)\uux.exe: uuxw.exe
        - $(ERASE) $<
        move $? $<
        - $(ERASE) $(?B: =.tds)

$(WINPROD)\uuxqt.exe: uuxqtw.exe
        - $(ERASE) $<
        move $? $<
        - $(ERASE) $(?B: =.tds)
!endif

!endif

# *--------------------------------------------------------------------*
# *                         The .ZIP files.                            *
# *--------------------------------------------------------------------*

# *--------------------------------------------------------------------*
# *                           UUPCSRC.ZIP                              *
# *--------------------------------------------------------------------*

$(SRCZIPV1):  $(MAKEFILE) NMAKE.MAK NMAKEDOS.MAK NMAKEWNT.MAK \
             NMAKOS21.MAK NMAKOS22.MAK \
             $(DOCS)\rmail.pif $(DOCS)\uuxqt.pif $(DOCS)\uuclean.pif \
             $(UUPCCFG) $(UUPCDEFS) $(DEFFILE) $(README)
        - mkdir $:.
       -18 $(ZIP) $(ZIPOPT1) < &&%
*.MAK
LIB\*.C
LIB\*.H
LIB\*.MAK
WINSOCK\*.H
$(README)
$(MAKEFILE)
DOCS\*.pif
%
       zip -z $< <  &&%
UUPC/extended $(VERS) source files (1 of 4)

Changes and Compilation Copyright (c) 1989-1994 by Kendra Electronic
Wonderworks.  May be distributed freely if original source and documentation
files are included.  Please direct all questions on UUPC/extended to
help@kew.com.
.
%

$(SRCZIPV2): $(MAKEFILE) $(SRCSLASH)nmake.mak \
             $(UUPCCFG) $(UUPCDEFS) $(DEFFILE) $(README)
        - mkdir $:.
       -18 $(ZIP) $(ZIPOPT1) < &&%
UUCICO\*.ASM
UUCICO\*.C
UUCICO\*.H
UUCICO\*.MAK
$(README)
%
       zip -z $< <  &&%
UUPC/extended $(VERS) source files (2 of 4)

Changes and Compilation Copyright (c) 1989-1994 by Kendra Electronic
Wonderworks.  May be distributed freely if original source and documentation
files are included.  Please direct all questions on UUPC/extended to
help@kew.com.
.
%

$(SRCZIPV3): $(MAKEFILE) $(SRCSLASH)nmake.mak \
             $(UUPCCFG) $(UUPCDEFS) $(DEFFILE) $(README)
        - mkdir $:.
       -18 $(ZIP) $(ZIPOPT1) < &&%
MAIL\*.C
MAIL\*.H
MAIL\*.MAK
UUCP\*.C
UUCP\*.H
UUCP\*.MAK
$(README)
%
       zip -z $< <  &&%
UUPC/extended $(VERS) source files (3 of 4)

Changes and Compilation Copyright (c) 1989-1994 by Kendra Electronic
Wonderworks.  May be distributed freely if original source and documentation
files are included.  Please direct all questions on UUPC/extended to
help@kew.com.
.
%

$(SRCZIPV4): $(MAKEFILE) \
             $(UUPCCFG) $(UUPCDEFS) $(DEFFILE) $(README)
        - mkdir $:.
       -18 $(ZIP) $(ZIPOPT1) < &&%
RNEWS\*.C
RNEWS\*.H
RNEWS\*.MAK
TEST\*.C
TEST\*.H
TEST\*.MAK
UTIL\*.C
UTIL\*.H
UTIL\*.MAK
SCRIPTS\*.CMD
SCRIPTS\*.BAT
UUTRAF\*.C
UUTRAF\*.H
UUTRAF\*.MAK
UUTRAF\CREDITS
$(README)
%
       zip -z $< <  &&%
UUPC/extended $(VERS) source files (4 of 4)

Changes and Compilation Copyright (c) 1989-1994 by Kendra Electronic
Wonderworks.  May be distributed freely if original source and documentation
files are included.  Please direct all questions on UUPC/extended to
help@kew.com.
%

# *--------------------------------------------------------------------*
# *       UUPCWIN.ZIP - Word for Windows unformatted documents         *
# *--------------------------------------------------------------------*

$(WFWZIPV):  $(WINWORD)\uupcuser.doc $(WINWORD)\manual.dot $(README)
        - mkdir $:.
       -18 $(ZIP) $(ZIPOPT2) < &&%
$?
%
       zip -z $< < &&%
UUPC/extended $(VERS) Word for Windows Document Source

Changes and Compilation Copyright (c) 1989-1994 by Kendra Electronic
Wonderworks.  May be distributed freely if original source and documentation
files are included.  Please direct all questions on UUPC/extended to
help@kew.com.
%

# *--------------------------------------------------------------------*
# *                UUPCREQ.ZIP - Required Executables                  *
# *--------------------------------------------------------------------*

$(REQZIPV):  $(REQUIRED) $(README)
        - mkdir $:.
       -18 $(ZIP) $(ZIPOPT2) < &&%
$(REQUIRED)
$(README)
%
       zip -z $< <  &&%
UUPC/extended $(VERS) $(ENVIRONMENT) executables (1 of 3)

Changes and Compilation Copyright (c) 1989-1994 by Kendra Electronic
Wonderworks.  May be distributed freely if original source and documentation
files are included.  Please direct all questions on UUPC/extended to
help@kew.com.
%

# *--------------------------------------------------------------------*
# *                UUPCOPT.ZIP - Optional Executables                  *
# *--------------------------------------------------------------------*

$(OPTZIPV):  $(OPTIONAL) $(README)
        - mkdir $:.
       -18 $(ZIP) $(ZIPOPT2) < &&%
$?
%
       zip -z $< <  &&%
UUPC/extended $(VERS) $(ENVIRONMENT) executables (2 of 3)

Changes and Compilation Copyright (c) 1989-1994 by Kendra Electronic
Wonderworks.  May be distributed freely if original source and documentation
files are included.  Please direct all questions on UUPC/extended to
help@kew.com.
%

# *--------------------------------------------------------------------*
# *                 UUPC/extended - NEWS executables                   *
# *--------------------------------------------------------------------*

$(NEWZIPV):  $(NEWS) $(README)
        - mkdir $:.
       -18 $(ZIP) $(ZIPOPT2) < &&%
$?
%
       zip -z $< <  &&%
UUPC/extended $(VERS) $(ENVIRONMENT) executables (3 of 3)

Changes and Compilation Copyright (c) 1989-1994 by Kendra Electronic
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
       -18 $(ZIP) $(ZIPOPT2) < &&%
$?
%
       -18 $(ZIP) $(ZIPOPT2) < &&%
$(DOCS)\*.mdm
$(DOCS)\*.inf
$(DOCS)\*.txt
$(DOCS)\*.spb
%
       zip -z $< < &&%
UUPC/extended $(VERS) documents and sample files

Changes and Compilation Copyright (c) 1989-1994 by Kendra Electronic
Wonderworks.  May be distributed freely if original source and documentation
files are included.  Please direct all questions on UUPC/extended to
help@kew.com.
%

$(PSZIPV): $(LISTFILES:.prn=.ps) $(README)
        - mkdir $:.
       -18 $(ZIP) $(ZIPOPT2) < &&%
$?
%
       zip -z $< < &&%
UUPC/extended $(VERS) documents in PostScript format

Changes and Compilation Copyright (c) 1989-1994 by Kendra Electronic
Wonderworks.  May be distributed freely if original source and documentation
files are included.  Please direct all questions on UUPC/extended to
help@kew.com.
%

# *--------------------------------------------------------------------*
# *                           Common build                             *
# *--------------------------------------------------------------------*

common: $(.path.obj) $(COMMON)  $(DEFFILE)

!if $d(__OS2__)
commonm:        common
        @REM    I'm a dummy command

!else
commonm:
        $(MAKER) -f$(MAKEFILE) -DMODEL=m common

commonw:
        $(MAKER) -f$(MAKEFILE) -DWINDOWS common
!endif

# *--------------------------------------------------------------------*
# *                     Link our various modules                       *
# *--------------------------------------------------------------------*

expire$(PSUFFIX).exe: common
        $(MAKER) -f$(RNEWS)\rnews.mak -DUUPCDEFS=$(UUPCDEFS) $<

fmt$(PSUFFIX).exe: common
        $(MAKER) -f$(UTIL)\util.mak -DUUPCDEFS=$(UUPCDEFS) $<

genhist$(PSUFFIX).exe: commonm
        $(MAKER) -f$(RNEWS)\rnews.mak -DUUPCDEFS=$(UUPCDEFM) $<

gensig$(PSUFFIX).exe: common
        $(MAKER) -f$(UTIL)\util.mak -DUUPCDEFS=$(UUPCDEFS) $<

inews$(PSUFFIX).exe: common
        $(MAKER) -f$(RNEWS)\rnews.mak -DUUPCDEFS=$(UUPCDEFS) $<

mail$(PSUFFIX).exe: commonm
        $(MAKER) -f$(MAIL)\mail.mak -DUUPCDEFS=$(UUPCDEFM) $<

newsetup$(PSUFFIX).exe: commonm
        $(MAKER) -f$(RN)\RN.mak -DUUPCDEFS=$(UUPCDEFM) $<

novrstr$(PSUFFIX).exe: common
        $(MAKER) -f$(UTIL)\util.mak -DUUPCDEFS=$(UUPCDEFS) $<

rmail$(PSUFFIX).exe: common
        $(MAKER) -f$(MAIL)\mail.mak -DUUPCDEFS=$(UUPCDEFS) $<

rn$(PSUFFIX).exe: commonm
        $(MAKER) -f$(RN)\RN.mak -DUUPCDEFS=$(UUPCDEFS) $<

rnews$(PSUFFIX).exe: common
        $(MAKER) -f$(RNEWS)\rnews.mak -DUUPCDEFS=$(UUPCDEFS) $<

uucp$(PSUFFIX).exe: common
        $(MAKER) -f$(UUCP)\uucp.mak -DUUPCDEFS=$(UUPCDEFS) $<

uucico$(PSUFFIX).exe: commonm
        $(MAKER) -f$(UUCICO)\UUCICO.mak -DUUPCDEFS=$(UUPCDEFM) $<

uuname$(PSUFFIX).exe: common
        $(MAKER) -f$(UUCP)\uucp.mak -DUUPCDEFS=$(UUPCDEFS) $<

uupoll$(PSUFFIX).exe: common
        $(MAKER) -f$(UTIL)\util.mak -DUUPCDEFS=$(UUPCDEFS) $<

uuport$(PSUFFIX).exe: common
        $(MAKER) -f$(UUCICO)\uucico.mak -DUUPCDEFS=$(UUPCDEFS) $<

uustat$(PSUFFIX).exe: common
        $(MAKER) -f$(UUCP)\uucp.mak -DUUPCDEFS=$(UUPCDEFS) $<

uusub$(PSUFFIX).exe: common
        $(MAKER) -f$(UUCP)\uucp.mak -DUUPCDEFS=$(UUPCDEFS) $<

uutraf$(PSUFFIX).exe: common
        $(MAKER) -f$(UUTRAF)\uutraf.mak -DUUPCDEFS=$(UUPCDEFS) $<

uux$(PSUFFIX).exe: common
        $(MAKER) -f$(UUCP)\uucp.mak -DUUPCDEFS=$(UUPCDEFS) $<

uuxqt$(PSUFFIX).exe: common
        $(MAKER) -f$(UUCP)\uucp.mak -DUUPCDEFS=$(UUPCDEFS) $<

!if !$d(__OS2__)

comm34.com: common
        $(MAKER) -f$(UTIL)\util.mak -DUUPCDEFS=$(UUPCDEFS) $<

fmt.com: common
        $(MAKER) -f$(UTIL)\util.mak -DUUPCDEFS=$(UUPCDEFS) $<

gensig.com: common
        $(MAKER) -f$(UTIL)\util.mak -DUUPCDEFS=$(UUPCDEFS) $<

novrstrk.com: common
        $(MAKER) -f$(UTIL)\util.mak -DUUPCDEFS=$(UUPCDEFS) $<

uuname.com: common
        $(MAKER) -f$(UUCP)\uucp.mak -DUUPCDEFS=$(UUPCDEFS) $<

uuport.com: common
        $(MAKER) -f$(UUCICO)\UUCICO.mak -DUUPCDEFS=$(UUPCDEFS) $<

uustat.com: common
        $(MAKER) -f$(UUCP)\uucp.mak -DUUPCDEFS=$(UUPCDEFS) $<

uusub.com: common
        $(MAKER) -f$(UUCP)\uucp.mak -DUUPCDEFS=$(UUPCDEFS) $<

uux.com: common
        $(MAKER) -f$(UUCP)\uucp.mak -DUUPCDEFS=$(UUPCDEFS) $<

# *--------------------------------------------------------------------*
# *                     Link our Windows modules                       *
# *--------------------------------------------------------------------*

!if "$(PSUFFIX)" != "w"
expirew.exe: commonw
        $(MAKER) -f$(RNEWS)\rnews.mak -DUUPCDEFS=$(UUPCDEFW) $<

mailw.exe: commonw
        $(MAKER) -f$(MAIL)\mail.mak -DUUPCDEFS=$(UUPCDEFW) $<

rmailw.exe: commonw
        $(MAKER) -f$(MAIL)\mail.mak -DUUPCDEFS=$(UUPCDEFW) $<

rnewsw.exe: commonw
        $(MAKER) -f$(RNEWS)\rnews.mak -DUUPCDEFS=$(UUPCDEFW) $<

uucicow.exe: commonw
        $(MAKER) -f$(UUCICO)\UUCICO.mak -DUUPCDEFS=$(UUPCDEFW) $<

uucpw.exe: commonw
        $(MAKER) -f$(UUCP)\uucp.mak -DUUPCDEFS=$(UUPCDEFW) $<

uunamew.exe: commonw
        $(MAKER) -f$(UUCP)\uucp.mak -DUUPCDEFS=$(UUPCDEFW) $<

uupollw.exe: commonw
        $(MAKER) -f$(UTIL)\util.mak -DUUPCDEFS=$(UUPCDEFW) $<

uuportw.exe: commonw
        $(MAKER) -f$(UUCICO)\uucico.mak -DUUPCDEFS=$(UUPCDEFW) $<

uustatw.exe: commonw
        $(MAKER) -f$(UUCP)\uucp.mak -DUUPCDEFS=$(UUPCDEFW) $<

uusubw.exe: commonw
        $(MAKER) -f$(UUCP)\uucp.mak -DUUPCDEFS=$(UUPCDEFW) $<

uutrafw.exe: commonw
        $(MAKER) -f$(UUTRAF)\uutraf.mak -DUUPCDEFS=$(UUPCDEFW) $<

uuxw.exe: commonw
        $(MAKER) -f$(UUCP)\uucp.mak -DUUPCDEFS=$(UUPCDEFW) $<

uuxqtw.exe: commonw
        $(MAKER) -f$(UUCP)\uucp.mak -DUUPCDEFS=$(UUPCDEFW) $<
!endif

# *--------------------------------------------------------------------*
# *               WUUPCREQ.ZIP - Required Windows Executables          *
# *--------------------------------------------------------------------*

$(WREQZIPV):  $(WREQUIRED) $(README)
        - mkdir $:.
       -18 $(ZIP) $(ZIPOPT2) < &&%
$?
%
       zip -z $< <  &&%
UUPC/extended $(VERS) Windows 3.x executables (1 of 3)

Special thanks to Robert B. Denny for performing the Windows 3.x port.

Changes and Compilation Copyright (c) 1989-1994 by Kendra Electronic
Wonderworks.  May be distributed freely if original source and documentation
files are included.  Please direct all questions on UUPC/extended to
help@kew.com.
%

# *--------------------------------------------------------------------*
# *               WUUPCOPT.ZIP - Optional Windows Executables          *
# *--------------------------------------------------------------------*

$(WOPTZIPV):  $(WOPTIONAL) $(README)
        - mkdir $:.
       -18 $(ZIP) $(ZIPOPT2) < &&%
$?
%
       zip -z $< <  &&%
UUPC/extended $(VERS) Windows 3.x executables (2 of 3)

Special thanks to Robert B. Denny for performing the Windows 3.x port.

Changes and Compilation Copyright (c) 1989-1994 by Kendra Electronic
Wonderworks.  May be distributed freely if original source and documentation
files are included.  Please direct all questions on UUPC/extended to
help@kew.com.
%

# *--------------------------------------------------------------------*
# *                 UUPC/extended - NEWS executables                   *
# *--------------------------------------------------------------------*

!ifdef 0
$(WNEWZIPV):  $(WNEWS) $(README)
        - mkdir $:.
       -18 $(ZIP) $(ZIPOPT2) < &&%
$?
%
       zip -z $< <  &&%
UUPC/extended $(VERS) Windows 3.x executables (3 of 3)

Special thanks to Robert B. Denny for performing the Windows 3.x port.

Changes and Compilation Copyright (c) 1989-1994 by Kendra Electronic
Wonderworks.  May be distributed freely if original source and documentation
files are included.  Please direct all questions on UUPC/extended to
help@kew.com.
%
!endif

!endif

$(.path.obj):
        - mkdir $(.path.obj)

# *--------------------------------------------------------------------*
# *     Force a regeneration of the time stamp/version module.         *
# *--------------------------------------------------------------------*

regen:  $(LIB)\timestmp.c
        - $(ERASE) $(TIMESTMP)

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
-f
-G
-H=$(HDRCACHE)
-I$(LIB)
-I$(BINC)
-I$(WINSOCK)
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
DOCS       = $(DOCS)
DEFFILE    = $(DEFFILE)
ERASE      = $(ERASE)
LIB        = $(LIB)
LIBRARIES  = $(LIBRARIES)
LIBRARY    = $(LIBRARY)
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
PSUFFIX    = $(PSUFFIX)
RN         = $(RN)
RNEWS      = $(RNEWS)
STARTUP    = $(STARTUP)
STARTUPT   = $(STARTUPT)
SUFFIX     = $(SUFFIX)
TASM       = $(TASM)
TASMOPT    = $(TASMOPT)
TDSTRIP    = $(TDSTRIP)
TEST       = $(TEST)
TIMESTMP   = $(TIMESTMP)
TMP        = $(TMP)
UTIL       = $(UTIL)
UUCICO     = $(UUCICO)
UUCP       = $(UUCP)
UUPCCFG    = $(UUPCCFG)
UUPCLIB    = $(UUPCLIB)
UUTRAF     = $(UUTRAF)
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
Description 'UUPC/extended $(VERS), Changes Copyright (c) 1989-1994 Kendra Electronic Wonderworks, All Rights Reserved'
| $<
!endif

!if $d(__OS2__)
$(DEFFILE): $(MAKEFILE)
  copy &&|
DATA MULTIPLE
STACKSIZE 32760
EXETYPE OS2
Description 'UUPC/extended $(VERS), Changes Copyright (c) 1989-1994 Kendra Electronic Wonderworks, All Rights Reserved'
| $<
!endif
