# *--------------------------------------------------------------------*
# *     Program:        borlandc.mak            15 May 1989            *
# *     Author:         Andrew H. Derbyshire                           *
# *                     Kendra Electronic Wonderworks                  *
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
# *   Changes Copyright (c) 1989-2002 by Kendra Electronic             *
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
#     $Id: borlandc.mak 1.96 2001/03/13 02:53:07 ahd v1-13k $
#
#     Revision history:
#     $Log: borlandc.mak $
#     Revision 1.96  2001/03/13 02:53:07  ahd
#     Drop files used for non-native DOS support
#
#     Revision 1.95  2001/03/12 13:58:31  ahd
#     Annual copyright update
#
#     Revision 1.94  2000/05/12 12:08:51  ahd
#     Annual copyright update
#
#     Revision 1.93  1999/01/04 03:51:49  ahd
#     Annual copyright change
#
#     Revision 1.92  1998/05/11 01:19:08  ahd
#     Allow possible build of UUCICO w/o tapi (uucicon.exe) under NT
#     Build zip files in build tree, not source tree
#
#     Revision 1.91  1998/04/29 03:48:53  ahd
#     Revise to use new location for icons
#
#     Revision 1.90  1998/04/19 15:28:07  ahd
#     Move version number to external file
#
#     Revision 1.89  1998/04/19 03:58:06  ahd
#     *** empty log message ***
#
#     Revision 1.88  1998/03/09 04:28:51  ahd
#     Go to version 1.12w
#
#     Revision 1.87  1998/03/01 01:22:17  ahd
#     Annual Copyright Update
#
#     Revision 1.86  1998/01/03 05:26:26  ahd
#     Support current aux files in release process better
#     Move to version 1.12v
#
#     Revision 1.85  1997/12/22 16:46:45  ahd
#     Delete BC++ OS/2 support (the compiler stank)
#     Move to 1.12u
#     Make RMAIL a medium model executable
#
#     Revision 1.84  1997/04/24 01:00:57  ahd
#     Annual Copyright Update
#
#     Revision 1.83  1996/01/20 13:03:45  ahd
#     Shorten up 'default' SRCSLASH path to be no path all, allowing
#     more compiles at a time under BC++ 3.1
#
#     Revision 1.82  1996/01/01 20:48:04  ahd
#     Annual Copyright Update
#
#     Revision 1.81  1995/12/12 13:47:08  ahd
#     Update to 1.12r
#
#     Revision 1.80  1995/11/30 12:44:23  ahd
#     Clean up for build of 1.12q samples
#
#     Revision 1.79  1995/11/08 01:02:37  ahd
#     Use B for Borland DOS executable ZIP files
#
#     Revision 1.78  1995/09/24 19:05:05  ahd
#     Update to 1.12p
#
#     Revision 1.77  1995/03/08 02:56:22  ahd
#     Update version number
#
#     Revision 1.76  1995/02/23 04:25:47  ahd
#     Add installw, to build only Windows
#
#     Revision 1.75  1995/02/21 02:44:51  ahd
#     Further updates for renamed news directory
#
#     Revision 1.74  1995/02/12 23:25:53  ahd
#     split rnews into rnews/news, rename rnews dir to news, make inews COM file
#
#     Revision 1.73  1995/01/14 14:07:57  ahd
#     Add sentbats.exe to prod build list
#
#     Revision 1.72  1995/01/08 21:00:31  ahd
#     Make RNEWS use medium model -- it doesn't fit in small model any more
#
#     Revision 1.71  1995/01/05 01:53:32  ahd
#     Drop unneeded WINDOWSOPT
#
#     Revision 1.70  1995/01/03 05:29:13  ahd
#     Use short source name for faster compiles
#

.autodepend

#.silent


# *--------------------------------------------------------------------*
# *          Version information, shared with nmake builds             *
# *--------------------------------------------------------------------*

!include "nmakever.mak"

#       The memory model to be built, and other environment
#       specific information

WINMODEL=m

#SRC=\src\uupc/
#       Silly hack to allow back slash as last character in variable
!if !$d(SRCSLASH) && !$d(SHORTPATH)
!if !$d(SRC)
SRC      = ./
!else
SRCSLASH = $(SRC:/=\)
!endif
!endif

!if $d(WINDOWS)
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


# *--------------------------------------------------------------------*
# *                           Directories                              *
# *--------------------------------------------------------------------*

!if !$d(TMP)
TMP     = \tmp
!endif

CONF    = $(PRODDRIVE)\uupc
DOCS    = $(SRCSLASH)docs
LIB     = $(SRCSLASH)lib
MAIL    = $(SRCSLASH)mail
!if !$d(OBJ)
OBJ     = $(SRCSLASH)objbc$(SUFFIX)
!endif
HDRCACHE= $(SRCSLASH)tcdef$(SUFFIX).sym
RN      = $(SRCSLASH)rn
NEWS    = $(SRCSLASH)news
SCRIPT  = $(SRCSLASH)scripts
TEST    = $(SRCSLASH)test
UTIL    = $(SRCSLASH)util
UUCICO  = $(SRCSLASH)uucico
UUCP    = $(SRCSLASH)uucp
UUTRAF  = $(SRCSLASH)uutraf
WINWORD = $(SRCSLASH)winword
ICONS   = $(SRCSLASH)icons.win

!if !$d(WINSOCK)
WINSOCK = WINSOCK
!endif

MAP     =

.path.obj  = $(OBJ)

!if !$d(ARCHIVE)
ARCHIVE = $(SRCSLASH)$(VERS)
!endif

!if !$d(PROD)
PROD    = $(PRODDRIVE)\uupc\bin
!endif

!if !$d(WINPROD)
WINPROD = $(PRODDRIVE)\uupc\winbin
!endif

!if !$d(BORLANDC)
BORLANDC = \BORLANDC
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

LISTFILES = $(WINWORD)\uupcmstr.prn $(WINWORD)\changes.txt \
            $(WINWORD)\howtoget.txt \
            $(WINWORD)\license.txt $(WINWORD)\mail.hlp \
            $(WINWORD)\register.txt $(WINWORD)\tilde.hlp

README=     docs\00readme.now

REQCOMM = $(PROD)\mail.exe $(PROD)\rmail.exe $(PROD)\uucico.exe\
          $(PROD)\uuxqt.exe
SCRIPTS=  $(PROD)\su.bat $(PROD)\uuclean.bat $(PROD)\uuio.bat \
          $(PROD)\waiting.bat
OPTCOMM = $(PROD)\uucp.exe $(PROD)\uuname.com $(PROD)\uupoll.exe\
          $(PROD)\uustat.com $(PROD)\uusub.com $(PROD)\uuport.com \
          $(PROD)\uux.com $(PROD)\fmt.com $(PROD)\gensig.com \
          $(PROD)\novrstrk.com $(PROD)\fromwho.com

OPTIONAL= $(OPTCOMM) $(PROD)\comm34.com $(SCRIPTS)
REQUIRED  = $(REQCOMM)

# *--------------------------------------------------------------------*
# *      Of the "required" modules, we only build UUCICO under         *
# *      Windows because the MAIL program is ugly and we prefer        *
# *      UUXQT run RMAIL and RNEWS as normal DOS programs.             *
# *--------------------------------------------------------------------*

WREQUIRED=$(WINPROD)\uucico.exe
WOPTIONAL=$(WINPROD)\uuname.exe $(WINPROD)\uupoll.exe $(WINPROD)\uustat.exe\
          $(WINPROD)\uusub.exe $(WINPROD)\uuport.exe
WNEWS     = #   No modules for news

NEWSPGM = $(PROD)\EXPIRE.EXE $(PROD)\GENHIST.EXE $(PROD)\INEWS.COM \
          $(PROD)\NEWSRUN.EXE \
          $(PROD)\RNEWS.COM $(PROD)\SENDBATS.EXE $(PROD)\UUTRAF.EXE

INSTALL = $(REQUIRED) $(OPTIONAL) $(NEWSPGM) $(WREQUIRED) $(WOPTIONAL) $(WNEWS)

SAMPLES = $(DOCS)\SYSTEMS $(DOCS)\PASSWD $(DOCS)\HOSTPATH \
          $(DOCS)\personal.rc $(DOCS)\uupc.rc \
          $(DOCS)\personal.sig $(DOCS)\nickname.txt \
          $(ICONS)\mail.ico $(ICONS)\uucico.ico
SAMPLEX = SYSTEMS PASSWD HOSTPATH *.RC *.MDM PERSONAL.SIG ALIASES.TXT *.ICO

MAKEFILE = BORLANDC.MAK

UUPCLIB  = $(TMP)\CUUPC$(SUFFIX).LIB
UUPCLIBW = $(TMP)\CUUPCW$(SUFFIX).LIB

!if $d(WINDOWS)
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

REQZIPV = $(ARCHIVE)\$(ZIPPREFIX)B1.ZIP
OPTZIPV = $(ARCHIVE)\$(ZIPPREFIX)B2.ZIP
NEWZIPV = $(ARCHIVE)\$(ZIPPREFIX)B3.ZIP
WREQZIPV = $(ARCHIVE)\$(ZIPPREFIX)W1.ZIP
WOPTZIPV = $(ARCHIVE)\$(ZIPPREFIX)W2.ZIP
#WNEWZIPV = $(ARCHIVE)\$(ZIPPREFIX)W3.ZIP

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

!if $d(WINDOWS)
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

MAKER=maker -DTDSTRIP=$(TDSTRIP)

# *--------------------------------------------------------------------*
# *      High speed delete command for OS/2, bypasses archiving        *
# *--------------------------------------------------------------------*

!if !$d(ERASE)
ERASE=DEL
!endif

# *--------------------------------------------------------------------*
# *                Begin rules for building modules.                   *
# *                                                                    *
# *        The first definition is the one built by default.           *
# *--------------------------------------------------------------------*

# *--------------------------------------------------------------------*
# *     test builds the basic UUPC modules in the source directory.    *
# *--------------------------------------------------------------------*

test:    comm34.com expire.exe fmt.com fromwho.com genhist.exe          \
         gensig.com inews.exe mail.exe novrstrk.com rmail.exe rnews.exe \
         newsrun.exe sendbats.exe                                       \
         uucico.exe uucp.exe uuname.exe uupoll.exe uuport.exe           \
         uustat.exe uusub.exe uutraf.exe uux.exe uuxqt.exe

# *--------------------------------------------------------------------*
# *      Note that we don't actually install all the Windows           *
# *      modules we build for testing.  This, like the unused news     *
# *      modules definitions, are really for furture expandsion.       *
# *--------------------------------------------------------------------*

windows: expirew.exe mailw.exe rmailw.exe rnewsw.exe  newsrun.exe       \
         sendbatw.exe uucicow.exe                                       \
         uucpw.exe uupollw.exe uuportw.exe uustatw.exe uusubw.exe       \
         uutrafw.exe uuxqtw.exe uuxw.exe

# *--------------------------------------------------------------------*
# *   Prod build the basic modules and then moves them to $(PROD)      *
# *--------------------------------------------------------------------*

prod:   $(INSTALL)
        - $(ERASE) $(TIMESTMP)
        - $(ERASE) $(UUPCLIB)
        - $(ERASE) *.sym

winprod:  commonw $(WREQUIRED) $(WOPTIONAL) $(WNEWS)
        - del *.tds
        - $(ERASE) *.sym

required: $(REQUIRED)

# *--------------------------------------------------------------------*
# *     Install builds the UUPC modules, installs the executables      *
# *     in the production directory, formats the documentation files,  *
# *     and creates PKZIP files for giving away.                       *
# *--------------------------------------------------------------------*

install: installx $(WREQZIPV) $(WOPTZIPV) $(WNEWZIPV)
        - del *.sym

doczip:  $(DOCZIPV)

installw: $(WREQZIPV) $(WOPTZIPV) $(WNEWZIPV)

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
        - $(ERASE) $(?B: =.tds)

$(PROD)\fmt.exe: fmt$(PSUFFIX).exe
        - $(ERASE) $<
        move $? $<
        - $(ERASE) $(?B: =.tds)

$(PROD)\gensig.exe: gensig$(PSUFFIX).exe
        - $(ERASE) $<
        move $? $<
        - $(ERASE) $(?B: =.tds)

$(PROD)\genhist.exe: genhist$(PSUFFIX).exe
        - $(ERASE) $<
        move $? $<
        - $(ERASE) $(?B: =.tds)

$(PROD)\inews.exe: inews$(PSUFFIX).exe
        - $(ERASE) $<
        move $? $<
        - $(ERASE) $(?B: =.tds)

$(PROD)\mail.exe: mail$(PSUFFIX).exe
        - $(ERASE) $<
        move $? $<
        - $(ERASE) $(?B: =.tds)

$(PROD)\newsrun.exe: newsrun$(PSUFFIX).exe
        - $(ERASE) $<
        move $? $<
        - $(ERASE) $(?B: =.tds)

$(PROD)\newsetup.exe: newsetup$(PSUFFIX).exe
        - $(ERASE) $<
        move $? $<
        - $(ERASE) $(?B: =.tds)

$(PROD)\novrstrk.exe: novrstr$(PSUFFIX).exe
        - $(ERASE) $<
        move $? $<
        - $(ERASE) $(?B: =.tds)

$(PROD)\rmail.exe: rmail$(PSUFFIX).exe
        - $(ERASE) $<
        move $? $<
        - $(ERASE) $(?B: =.tds)

$(PROD)\rn.exe: rn$(PSUFFIX).exe
        - $(ERASE) $<
        move $? $<
        - $(ERASE) $(?B: =.tds)

$(PROD)\rnews.exe: rnews$(PSUFFIX).exe
        - $(ERASE) $<
        move $? $<
        - $(ERASE) $(?B: =.tds)

$(PROD)\sendbats.exe: sendbats$(PSUFFIX).exe
        - $(ERASE) $<
        move $? $<
        - $(ERASE) $(?B: =.tds)

$(PROD)\uucico.exe: uucico$(PSUFFIX).exe
        - $(ERASE) $<
        move $? $<
        - $(ERASE) $(?B: =.tds)

$(PROD)\uucp.exe: uucp$(PSUFFIX).exe
        - $(ERASE) $<
        move $? $<
        - $(ERASE) $(?B: =.tds)

$(PROD)\uuname.exe: uuname$(PSUFFIX).exe
        - $(ERASE) $<
        move $? $<
        - $(ERASE) $(?B: =.tds)

$(PROD)\uupoll.exe: uupoll$(PSUFFIX).exe
        - $(ERASE) $<
        move $? $<
        - $(ERASE) $(?B: =.tds)

$(PROD)\uuport.exe: uuport$(PSUFFIX).exe
        - $(ERASE) $<
        move $? $<
        - $(ERASE) $(?B: =.tds)

$(PROD)\uustat.exe: uustat$(PSUFFIX).exe
        - $(ERASE) $<
        move $? $<
        - $(ERASE) $(?B: =.tds)

$(PROD)\uusub.exe: uusub$(PSUFFIX).exe
        - $(ERASE) $<
        move $? $<
        - $(ERASE) $(?B: =.tds)

$(PROD)\uutraf.exe: uutraf$(PSUFFIX).exe
        - $(ERASE) $<
        move $? $<
        - $(ERASE) $(?B: =.tds)

$(PROD)\uux.exe: uux$(PSUFFIX).exe
        - $(ERASE) $<
        move $? $<
        - $(ERASE) $(?B: =.tds)

$(PROD)\uuxqt.exe: uuxqt$(PSUFFIX).exe
        - $(ERASE) $<
        move $? $<
        - $(ERASE) $(?B: =.tds)

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

$(PROD)\fromwho.com: fromwho.com
        - $(ERASE) $<
        move $? $<
        - $(ERASE) $(?B: =.tds)

$(PROD)\gensig.com: gensig.com
        - $(ERASE) $<
        move $? $<
        - $(ERASE) $(?B: =.tds)

$(PROD)\inews.com: inews.com
        - $(ERASE) $<
        move $? $<
        - $(ERASE) $(?B: =.tds)

$(PROD)\novrstrk.com: novrstrk.com
        - $(ERASE) $<
        move $? $<
        - $(ERASE) $(?B: =.tds)

$(PROD)\rnews.com: rnews.com
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

$(WINPROD)\newsrun.exe: rnewsrunw.exe
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

# *--------------------------------------------------------------------*
# *                         The .ZIP files.                            *
# *--------------------------------------------------------------------*

# *--------------------------------------------------------------------*
# *                           UUPCSRC.ZIP                              *
# *--------------------------------------------------------------------*

$(SRCZIPV1):  $(MAKEFILE) NMAKE.MAK NMAKEDOS.MAK NMAKEWNT.MAK \
             NMAKOS21.MAK NMAKOS22.MAK \
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
%
       zip -z $< <  &&%
UUPC/extended $(VERS) source files (1 of 4)

Changes and Compilation Copyright (c) 1989-2002 by Kendra Electronic
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

Changes and Compilation Copyright (c) 1989-2002 by Kendra Electronic
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

Changes and Compilation Copyright (c) 1989-2002 by Kendra Electronic
Wonderworks.  May be distributed freely if original source and documentation
files are included.  Please direct all questions on UUPC/extended to
help@kew.com.
.
%

$(SRCZIPV4): $(MAKEFILE) \
             $(UUPCCFG) $(UUPCDEFS) $(DEFFILE) $(README)
        - mkdir $:.
       -18 $(ZIP) $(ZIPOPT1) < &&%
NEWS\*.C
NEWS\*.H
NEWS\*.MAK
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

Changes and Compilation Copyright (c) 1989-2002 by Kendra Electronic
Wonderworks.  May be distributed freely if original source and documentation
files are included.  Please direct all questions on UUPC/extended to
help@kew.com.
%

# *--------------------------------------------------------------------*
# *       UUPCWIN.ZIP - Word for Windows unformatted documents         *
# *--------------------------------------------------------------------*

$(WFWZIPV):  $(WINWORD)\uupcmstr.doc \
             $(WINWORD)\uupcindx.doc \
             $(WINWORD)\manual.dot  \
             $(README)
        - mkdir $:.
       -18 $(ZIP) $(ZIPOPT2) < &&%
$?
%
       zip -z $< < &&%
UUPC/extended $(VERS) Word for Windows Document Source

Changes and Compilation Copyright (c) 1989-2002 by Kendra Electronic
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

Changes and Compilation Copyright (c) 1989-2002 by Kendra Electronic
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

Changes and Compilation Copyright (c) 1989-2002 by Kendra Electronic
Wonderworks.  May be distributed freely if original source and documentation
files are included.  Please direct all questions on UUPC/extended to
help@kew.com.
%

# *--------------------------------------------------------------------*
# *                 UUPC/extended - NEWS executables                   *
# *--------------------------------------------------------------------*

$(NEWZIPV):  $(NEWSPGM) $(README)
        - mkdir $:.
       -18 $(ZIP) $(ZIPOPT2) < &&%
$?
%
       zip -z $< <  &&%
UUPC/extended $(VERS) $(ENVIRONMENT) executables (3 of 3)

Changes and Compilation Copyright (c) 1989-2002 by Kendra Electronic
Wonderworks.  May be distributed freely if original source and documentation
files are included.  Please direct all questions on UUPC/extended to
help@kew.com.
%

# *--------------------------------------------------------------------*
# *        UUPCDOC.ZIP - Formatted (Human readable) documents          *
# *--------------------------------------------------------------------*

docs: $(DOCZIPV) $(WFWZIPV) $(PSZIPV) $(HTMLZIPV)

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

Changes and Compilation Copyright (c) 1989-2002 by Kendra Electronic
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

Changes and Compilation Copyright (c) 1989-2002 by Kendra Electronic
Wonderworks.  May be distributed freely if original source and documentation
files are included.  Please direct all questions on UUPC/extended to
help@kew.com.
%

# *--------------------------------------------------------------------*
# *                           Common build                             *
# *--------------------------------------------------------------------*

common: $(.path.obj) $(COMMON)  $(DEFFILE)

commonm:
        $(MAKER) -f$(MAKEFILE) -DMODEL=m common

commonw:
        $(MAKER) -f$(MAKEFILE) -DWINDOWS common

# *--------------------------------------------------------------------*
# *                     Link our various modules                       *
# *--------------------------------------------------------------------*

expire$(PSUFFIX).exe: common
        $(MAKER) -f$(NEWS)\news.mak -DUUPCDEFS=$(UUPCDEFS) $<

fmt$(PSUFFIX).exe: common
        $(MAKER) -f$(UTIL)\util.mak -DUUPCDEFS=$(UUPCDEFS) $<

fromwho$(PSUFFIX).exe: common
        $(MAKER) -f$(UTIL)\util.mak -DUUPCDEFS=$(UUPCDEFS) $<

genhist$(PSUFFIX).exe: commonm
        $(MAKER) -f$(NEWS)\news.mak -DUUPCDEFS=$(UUPCDEFM) $<

gensig$(PSUFFIX).exe: common
        $(MAKER) -f$(UTIL)\util.mak -DUUPCDEFS=$(UUPCDEFS) $<

inews$(PSUFFIX).exe: common
        $(MAKER) -f$(NEWS)\news.mak -DUUPCDEFS=$(UUPCDEFS) $<

mail$(PSUFFIX).exe: commonm
        $(MAKER) -f$(MAIL)\mail.mak -DUUPCDEFS=$(UUPCDEFM) $<

newsrun$(PSUFFIX).exe: commonm
        $(MAKER) -f$(NEWS)\news.mak -DUUPCDEFS=$(UUPCDEFM) $<

newsetup$(PSUFFIX).exe: commonm
        $(MAKER) -f$(RN)\RN.mak -DUUPCDEFS=$(UUPCDEFM) $<

novrstr$(PSUFFIX).exe: common
        $(MAKER) -f$(UTIL)\util.mak -DUUPCDEFS=$(UUPCDEFS) $<

rmail$(PSUFFIX).exe: commonm
        $(MAKER) -f$(MAIL)\mail.mak -DUUPCDEFS=$(UUPCDEFM) $<

rn$(PSUFFIX).exe: commonm
        $(MAKER) -f$(RN)\RN.mak -DUUPCDEFS=$(UUPCDEFS) $<

rnews$(PSUFFIX).exe: common
        $(MAKER) -f$(NEWS)\news.mak -DUUPCDEFS=$(UUPCDEFS) $<

sendbats$(PSUFFIX).exe: common
        $(MAKER) -f$(NEWS)\news.mak -DUUPCDEFS=$(UUPCDEFS) $<

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

comm34.com: common
        $(MAKER) -f$(UTIL)\util.mak -DUUPCDEFS=$(UUPCDEFS) $<

fmt.com: common
        $(MAKER) -f$(UTIL)\util.mak -DUUPCDEFS=$(UUPCDEFS) $<

fromwho.com: common
        $(MAKER) -f$(UTIL)\util.mak -DUUPCDEFS=$(UUPCDEFS) $<

gensig.com: common
        $(MAKER) -f$(UTIL)\util.mak -DUUPCDEFS=$(UUPCDEFS) $<

inews.com: common
        $(MAKER) -f$(NEWS)\news.mak -DUUPCDEFS=$(UUPCDEFS) $<

novrstrk.com: common
        $(MAKER) -f$(UTIL)\util.mak -DUUPCDEFS=$(UUPCDEFS) $<

rnews.com: common
        $(MAKER) -f$(NEWS)\news.mak -DUUPCDEFS=$(UUPCDEFS) $<

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
        $(MAKER) -f$(NEWS)\news.mak -DUUPCDEFS=$(UUPCDEFW) $<

mailw.exe: commonw
        $(MAKER) -f$(MAIL)\mail.mak -DUUPCDEFS=$(UUPCDEFW) $<

rmailw.exe: commonw
        $(MAKER) -f$(MAIL)\mail.mak -DUUPCDEFS=$(UUPCDEFW) $<

rnewsw.exe: commonw
        $(MAKER) -f$(NEWS)\news.mak -DUUPCDEFS=$(UUPCDEFW) $<

sendbatw.exe: commonw
        $(MAKER) -f$(NEWS)\news.mak -DUUPCDEFS=$(UUPCDEFW) $<

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

Changes and Compilation Copyright (c) 1989-2002 by Kendra Electronic
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

Changes and Compilation Copyright (c) 1989-2002 by Kendra Electronic
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

Changes and Compilation Copyright (c) 1989-2002 by Kendra Electronic
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

MODELOPT=-m$(MODEL)
OPTIMIZEOPT=-O
!if $d(WINDOWS)
GENERATEOPT=-W
CODEGENOPT=-3
!else
GENERATEOPT=-D_MSC
!endif

$(UUPCCFG): $(MAKEFILE) \
            $(LIB)\lib.mak \
            $(MAIL)\mail.mak \
            $(UUCP)\uucp.mak \
            $(UUCICO)\uucico.mak \
            $(UTIL)\util.mak
  copy &&|
$(GENERATEOPT)
$(CODEGENOPT)
$(MODELOPT)
$(OPTMIZEOPT)
-c
-d
-DUDEBUG
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
DEFFILE    = $(DEFFILE)
DOCS       = $(DOCS)
ERASE      = $(ERASE)
LIB        = $(LIB)
LIBRARIES  = $(LIBRARIES)
LIBRARY    = $(LIBRARY)
LINKER     = $(LINKER)
LINKOPTD   = $(LINKOPTD)
LINKOPTN   = $(LINKOPTN)
LINKOPTT   = $(LINKOPTT)
MAIL       = $(MAIL)
MAKEFILE   = $(MAKEFILE)
MAP        = $(MAP)
MODEL      = $(MODEL)
NEWS       = $(NEWS)
NEWSPGM    = $(NEWSPGM)
OBJ        = $(OBJ)
PSUFFIX    = $(PSUFFIX)
RN         = $(RN)
SRC        = $(SRC)
SRCSLASH   = $(SRCSLASH)
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

| $<

!if $d(WINDOWS)
$(DEFFILE): $(MAKEFILE)
  copy &&|
EXETYPE WINDOWS
DATA MOVABLE MULTIPLE
STACKSIZE 8182
HEAPSIZE  12288
Description 'UUPC/extended $(VERS), Changes Copyright (c) 1989-2002 Kendra Electronic Wonderworks, All Rights Reserved'
| $<
!endif
