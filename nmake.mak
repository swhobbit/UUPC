#       Program:        nmake.mak                      15 May 1989
#       Author:         Andrew H. Derbyshire
#                       Kendra Electronic Wonderworks
#       Function:       Build UUPC/extended from source files
#       Processor:      Microsoft nmake
#       External files: MASM            (8088 Assembler)
#                       CL              (Microsoft C 6.0 compiler)
#                       LINK or         (MS-DOS Linker)
#                       LINK386         (OS/2 Linker)
#                       Input source files.
#
#       Copyright (c) 1989-2002 by Kendra Electronic Wonderworks;
#       all rights reserved except those explicitly granted by
#       the UUPC/extended license.
#
#       nmk /nologo /s /f nmake.mak prodr
#       nmake /nologo /s /f nmake.mak prodp
#       nmake /nologo /s /f nmake.mak prodn
#
#       $Id: nmake.mak 1.164 2002/01/08 02:20:00 ahd v1-13k $
#
#       $Log: nmake.mak $
#       Revision 1.164  2002/01/08 02:20:00  ahd
#       Annual Copyright Update
#
#       Revision 1.163  2002/01/07 22:36:46  ahd
#       Create NT GUI build directory before building modules
#
#       Revision 1.162  2001/05/16 20:55:10  ahd
#       Support extra zip files for NT GUI build
#
#       Revision 1.161  2001/03/13 02:53:07  ahd
#       Drop files used for non-native DOS support
#
#       Revision 1.160  2001/03/12 13:58:31  ahd
#       Annual copyright update
#
#       Revision 1.159  2000/12/29 00:10:08  ahd
#       Correct install of icons
#
#       Revision 1.158  2000/09/15 00:21:04  ahd
#       Update to Version 1.13h
#
#       Revision 1.157  2000/07/04 22:18:50  ahd
#       Install MSVC PDB files if they exist
#
#       Revision 1.156  2000/05/12 12:28:15  ahd
#       Annual copyright update
#
#       Revision 1.155  1999/02/21 04:09:03  ahd
#       Support batch SMTP server build
#
#       Revision 1.154  1999/01/17 17:18:37  ahd
#       egin work on BSMTP program
#
#       Revision 1.153  1999/01/08 02:20:42  ahd
#       Convert currentfile() to RCSID()
#
#       Revision 1.152  1999/01/04 03:51:49  ahd
#       Annual copyright change
#
#       Revision 1.151  1998/11/05 03:28:45  ahd
#       Correct directory used for ZIP archives
#
#       Revision 1.150  1998/11/04 01:59:13  ahd
#       Support easily moving the zip archives to a different directory
#       from the rest of the build.
#
#       Revision 1.149  1998/10/28 03:18:17  ahd
#       Allow program other than LIB to handle library archives for OS/2
#       Correct OS/2 compile options for Visual C++ 3.0
#
#       Revision 1.148  1998/05/18  03:49:22  ahd
#       Delete unneeded use of the fmt command to build uucico
#
#       Revision 1.147  1998/05/17  21:11:20  ahd
#       Allow building line-length limited DOS modules in
#       different directory
#
#       Revision 1.146  1998/05/15 03:13:15  ahd
#       Allow building special GUI zip for NT
#
#       Revision 1.145  1998/05/11 03:36:17  ahd
#       Don't link incrementally
#
#       Revision 1.144  1998/05/11 01:19:08  ahd
#       Allow possible build of UUCICO w/o tapi (uucicon.exe) under NT
#       Build zip files in build tree, not source tree
#
#       Revision 1.143  1998/04/29 03:48:53  ahd
#       Support generation of main documentation file
#
#       Revision 1.142  1998/04/28 13:13:54  ahd
#       Revise name for HTML GIF file
#
#       Revision 1.141  1998/04/27 01:54:27  ahd
#       Revise icon source directory
#
#       Revision 1.140  1998/04/24 03:26:26  ahd
#       Automatically force new timestamp after building guiprod
#
#       Revision 1.139  1998/04/22 01:19:22  ahd
#       Add guiprod target
#
#       Revision 1.138  1998/04/20 02:47:11  ahd
#       TAPI/32 Windows GUI support
#
#       Revision 1.137  1998/04/19 23:55:04  ahd
#       *** empty log message ***
#
#       Revision 1.136  1998/04/19 15:28:07  ahd
#       Move version number to external file
#       Consistently reference this file (nmake.mak) via it's variable name
#
#       Revision 1.135  1998/04/19 03:58:06  ahd
#       *** empty log message ***
#
#       Revision 1.134  1998/04/08 11:30:10  ahd
#       Build with shared libraries
#
#       Revision 1.133  1998/03/16 06:38:02  ahd
#       Add build of post script zip file
#
#       Revision 1.132  1998/03/09 06:10:04  ahd
#       Lower case names to process
#
#       Revision 1.131  1998/03/09 04:28:51  ahd
#       Revise to new file name
#
#       Revision 1.130  1998/03/06 16:07:26  ahd
#       Go to version 1.12w
#
#       Revision 1.129  1998/03/03 07:44:31  ahd
#       CORRECT BUILD OF FIRST SOURCE ARCHIVE
#
#       Revision 1.128  1998/03/03 03:04:04  ahd
#       First working POP3 server (w/o delete)
#
#       Revision 1.127  1998/03/01 19:36:36  ahd
#       First compiling POP3 server which accepts user id/password
#
#       Revision 1.126  1998/03/01 01:21:57  ahd
#       Annual Copyright Update
#

.SUFFIXES: .c .asm .zip .obj .dll .ico .bat .cmd .com .exe

# *--------------------------------------------------------------------*
# *     Note:   By default, this file generates code for 80286         *
# *             systems and above.  If you are generating              *
# *             UUPC/extended for a 8088 system, remove the /G2        *
# *             option from the CCOPT= line, below                     *
# *--------------------------------------------------------------------*

# *--------------------------------------------------------------------*
# *        Version information shared with Borland C Makefile          *
# *--------------------------------------------------------------------*

!include "nmakever.mak"

# *--------------------------------------------------------------------*
# *                    Determine the OS de jour.                       *
# *--------------------------------------------------------------------*

!ifndef OS

!if "$(CC)" == "icc"
OS=OS/2_32_Bit
!else

!ifdef OS2_SHELL
OS=OS/2_16_Bit
!else

!ifdef WINDIR
OS=WIN95
!else
OS=DOS
!endif
!endif
!endif

!endif

# *--------------------------------------------------------------------*
# *      Pull in our OS specific defines from the external file        *
# *--------------------------------------------------------------------*

!if "$(OS)" == "Windows_NT"
!include "nmakewnt.mak"
!endif

!if "$(OS)" == "WIN95"
!include "nmakew95.mak"
!endif

!if "$(OS)" == "OS/2_32_Bit"
!include "nmakos22.mak"
!endif

!if "$(OS)" == "OS/2_16_Bit"
!include "nmakos21.mak"
OS2=1
!endif

!if "$(OS)" == "DOS"
!include "nmakedos.mak"
!endif

!ifndef ZIPID
!error ZIPID not defined, check that environment file was included...
!endif

!ifndef BIGMODEL
BIGMODEL=L
!endif

# *--------------------------------------------------------------------*
# *            User specified include file (if specified)              *
# *--------------------------------------------------------------------*

!ifdef UUPCNMK
!include $(UUPCNMK)
!endif

!ifndef LIBPROG
LIBPROG=lib
!endif

# *--------------------------------------------------------------------*
# *   Various directories.  The TMP directory is not defined here;     *
# *   rather we expect it to be defined as part of the user's          *
# *   environment variables                                            *
# *--------------------------------------------------------------------*

MAIL    = $(SRCSLASH)mail
UTIL    = $(SRCSLASH)util
UUCP    = $(SRCSLASH)uucp
NEWS    = $(SRCSLASH)news
UULIB   = $(SRCSLASH)lib
TEST    = $(SRCSLASH)test
UUCICO  = $(SRCSLASH)uucico
UUTRAF  = $(SRCSLASH)uutraf
SCRIPTS = $(SRCSLASH)scripts
DOCS    = $(SRCSLASH)docs

SAMPLES = $(DOCS)\SYSTEMS $(DOCS)\PASSWD $(DOCS)\HOSTPATH \
          $(DOCS)\personal.rc $(DOCS)\uupc.rc \
          $(DOCS)\personal.sig $(DOCS)\nickname.txt \
          $(ICONS)\mail.ico $(ICONS)\uucico.ico

LISTFILES = $(WINWORD)\uupcmstr.prn $(WINWORD)\changes.txt \
            $(WINWORD)\howtoget.txt \
            $(WINWORD)\mail.hlp \
            $(WINWORD)\register.txt $(WINWORD)\tilde.hlp

!ifndef UUPCNMK
WINWORD = $(SRCSLASH)winword
!endif

!ifndef OBJROOT
OBJROOT=$(TMP)
!endif

!ifndef OBJ
OBJ=$(OBJROOT)obj$(ZIPID)$(MODEL)
!endif

!ifndef SBR
SBR     = $(SRCSLASH)sbr
!endif

!ifndef LIBDIR
LIBDIR=$(OBJ)
!endif

!ifndef ZIPROOT
ZIPROOT=$(OBJROOT)
!endif

!ifndef ZIPDIR
ZIPDIR=$(ZIPROOT)$(VERS)
!endif

# *--------------------------------------------------------------------*
# *                        Various file names                          *
# *--------------------------------------------------------------------*

!ifndef ZIPPREFIX
ZIPPREFIX=$(ZIPDIR)\upc$(VERS:1.=)
!endif

LIBNEWS  = $(LIBDIR)\uupc$(ZIPID)$(MODEL)n.lib
LIBCOMM  = $(LIBDIR)\uupc$(ZIPID)$(MODEL)c.lib
LIBFILE  = $(LIBDIR)\uupc$(ZIPID)$(MODEL)f.lib

WORKFILE = $(TMP)\UUPCMAKE.TMP
!ifdef OS2
DEFFILE  = $(LIBDIR)\UUPCOS$(ZIPID)$(MODEL).DEF
!endif

ZIPE1 = $(ZIPPREFIX)$(ZIPID)1.zip
ZIPE2 = $(ZIPPREFIX)$(ZIPID)2.zip
ZIPE3 = $(ZIPPREFIX)$(ZIPID)3.zip
ZIPEG = $(ZIPPREFIX)$(ZIPID)G.zip

WORDZIP  = $(ZIPPREFIX)aw.zip
HTMLZIP  = $(ZIPPREFIX)ah.zip
PSZIP    = $(ZIPPREFIX)ap.zip
DOCSZIP  = $(ZIPPREFIX)ad.zip

SRCZIPV1 = $(ZIPPREFIX)s1.zip
SRCZIPV2 = $(SRCZIPV1:s1=s2)
SRCZIPV3 = $(SRCZIPV1:s1=s3)
SRCZIPV4 = $(SRCZIPV1:s1=s4)

!ifndef INSTALLCMD
INSTALLCMD=copy
!endif

# *--------------------------------------------------------------------*
# *        The names of various object files that we create.           *
# *--------------------------------------------------------------------*

TIMESTMP = $(OBJ)\timestmp.obj

LIBFILELIST =\
    $(OBJ)\abspath.obj $(OBJ)\arbmath.obj $(OBJ)\arpadate.obj   \
    $(OBJ)\configur.obj $(OBJ)\expath.obj $(OBJ)\export.obj     \
    $(OBJ)\filebkup.obj $(OBJ)\getopt.obj $(OBJ)\getseq.obj     \
    $(OBJ)\hostable.obj $(OBJ)\hostatus.obj $(OBJ)\hostrset.obj \
    $(OBJ)\imfile.obj $(OBJ)\import.obj   \
    $(OBJ)\kanjicnv.obj $(OBJ)\lock.obj $(OBJ)\pushpop.obj      \
    $(OBJ)\security.obj $(OBJ)\ssleep.obj $(OBJ)\stater.obj     \
    $(OBJ)\trumpet.obj $(OBJ)\usertabl.obj $(OBJ)\validcmd.obj

LIBCOMMLIST =\
    $(OBJ)\bugout.obj $(OBJ)\catcher.obj $(OBJ)\chdir.obj       \
    $(OBJ)\checkptr.obj $(OBJ)\dater.obj $(OBJ)\dos2unix.obj    \
    $(OBJ)\execute.obj $(OBJ)\fopen.obj $(OBJ)\getargs.obj      \
    $(OBJ)\getdrive.obj $(OBJ)\logger.obj $(OBJ)\localfnm.obj   \
    $(OBJ)\mkdir.obj      \
    $(OBJ)\mkfilenm.obj $(OBJ)\mkmbox.obj $(OBJ)\mktempnm.obj   \
    $(OBJ)\normaliz.obj $(OBJ)\printerr.obj $(OBJ)\printmsg.obj \
    $(OBJ)\readnext.obj $(OBJ)\rename.obj $(OBJ)\safeio.obj     \
    $(OBJ)\safeout.obj $(OBJ)\strpool.obj $(OBJ)\usrcatch.obj   \
    $(LIBOSLIST) $(TIMESTMP)

LIBNEWSLIST=\
    $(OBJ)\active.obj $(OBJ)\cache.obj $(OBJ)\hdbm.obj          \
    $(OBJ)\history.obj $(OBJ)\idx.obj $(OBJ)\importng.obj $(OBJ)\sys.obj

# *--------------------------------------------------------------------*
# *     Determine if we're building .COM or .EXE files under DOS       *
# *--------------------------------------------------------------------*

!ifndef COMEXE
COMEXE=exe                      # our default is safer exe files
!endif

!if "$(MODEL)" == "T"
COMEXET=com
COMEXE=com
!else
COMEXET=exe
!endif

!ifndef COMEXE
!if "$(MODEL)" == "S"
COMEXE=com
!endif
!endif

# *--------------------------------------------------------------------*
# *             Object file lists for specific programs                *
# *--------------------------------------------------------------------*

MAILOBJ = $(OBJ)\mail.obj $(OBJ)\address.obj $(OBJ)\mailblib.obj \
          $(OBJ)\maillib.obj $(OBJ)\mlib.obj $(OBJ)\mailsend.obj \
          $(OBJ)\nickname.obj $(OBJ)\mailnewm.obj

!ifdef SMTP
RMAILS_DELIVERS = $(OBJ)\delivers.obj $(OBJ)\commlibt.obj \
                  $(OBJ)\ulibip.obj $(RMAIL_DELIVERS_SUPPORT)
!endif
RMAILOBJ = $(OBJ)\rmail.obj   \
           $(OBJ)\address.obj \
           $(OBJ)\deliver.obj \
           $(OBJ)\deliverm.obj   \
           $(OBJ)\sysalias.obj

SENDBATSOBJ = $(OBJ)\sendbats.obj $(OBJ)\batch.obj

UUCICOOBJ1 =$(OBJ)\checktim.obj $(OBJ)\commlib.obj $(OBJ)\dcp.obj  \
         $(OBJ)\dcpfpkt.obj $(OBJ)\dcpgpkt.obj $(OBJ)\dcplib.obj   \
         $(OBJ)\dcpstats.obj $(OBJ)\dcpsys.obj $(OBJ)\dcpxfer.obj  \
         $(OBJ)\modem.obj $(OBJ)\script.obj     \
         $(OBJ)\uucico.obj

UUSMTPDOBJ=$(OBJ)\smtpd.obj      \
           $(OBJ)\smtpclnt.obj   \
           $(OBJ)\smtpcmds.obj   \
           $(OBJ)\smtpcmmn.obj   \
           $(OBJ)\smtpdns.obj    \
           $(OBJ)\smtplwc.obj    \
           $(OBJ)\smtpnetw.obj   \
           $(OBJ)\smtpnett.obj   \
           $(OBJ)\smtpserv.obj   \
           $(OBJ)\smtprecv.obj   \
           $(OBJ)\smtputil.obj   \
           $(OBJ)\smtpverb.obj   \
           $(OBJ)\address.obj    \
           $(OBJ)\deliverm.obj   \
           $(OBJ)\delivern.obj   \
           $(OBJ)\sysalias.obj   \
           $(OBJ)\memstr.obj     \
           $(RMAIL_DELIVERS_SUPPORT)

BSMTPOBJ=  $(OBJ)\smtpd.obj       \
           $(OBJ)\smtpclnt.obj   \
           $(OBJ)\smtpcmds.obj   \
           $(OBJ)\smtpcmmn.obj   \
           $(OBJ)\smtplwc.obj    \
           $(OBJ)\smtpnetb.obj   \
           $(OBJ)\smtpnetw.obj   \
           $(OBJ)\smtprecv.obj   \
           $(OBJ)\smtpserv.obj   \
           $(OBJ)\smtputil.obj   \
           $(OBJ)\smtpverb.obj   \
           $(OBJ)\address.obj    \
           $(OBJ)\deliver.obj    \
           $(OBJ)\deliverm.obj   \
           $(OBJ)\sysalias.obj   \
           $(OBJ)\memstr.obj     \
           $(RMAILS_DELIVERS)

UUPOPDOBJ=  $(OBJ)\smtpd.obj      \
            $(OBJ)\smtpclnt.obj   \
            $(OBJ)\smtpcmmn.obj   \
            $(OBJ)\smtpdns.obj    \
            $(OBJ)\smtpnetw.obj   \
            $(OBJ)\smtpnett.obj   \
            $(OBJ)\smtpserv.obj   \
            $(OBJ)\smtpverb.obj   \
            $(OBJ)\pop3lwc.obj    \
            $(OBJ)\pop3cmds.obj   \
            $(OBJ)\pop3user.obj   \
            $(OBJ)\pop3mbox.obj   \
            $(OBJ)\mailnewm.obj   \
            $(OBJ)\memstr.obj     \
            $(RMAIL_DELIVERS_SUPPORT)

RNOBJ1=$(OBJ)\addng.obj $(OBJ)\art.obj $(OBJ)\artio.obj $(OBJ)\artsrch.obj\
       $(OBJ)\backpage.obj $(OBJ)\bits.obj $(OBJ)\cheat.obj $(OBJ)\final.obj
RNOBJ2=$(OBJ)\head.obj $(OBJ)\help.obj $(OBJ)\init.obj $(OBJ)\intrp.obj\
       $(OBJ)\kfile.obj $(OBJ)\last.obj $(OBJ)\ng.obj $(OBJ)\ngdata.obj
RNOBJ3=$(OBJ)\ngsrch.obj $(OBJ)\ngstuff.obj $(OBJ)\only.obj $(OBJ)\rcln.obj\
       $(OBJ)\rcstuff.obj $(OBJ)\respond.obj $(OBJ)\rn.obj $(OBJ)\search.obj\
       $(OBJ)\sw.obj $(OBJ)\term.obj $(OBJ)\util.obj $(OBJ)\uudecode.obj \
       $(OBJ)\rnpclib.obj
RNOBJ=$(RNOBJ1) $(RNOBJ2) $(RNOBJ3)

NEWSETUPOBJ=$(OBJ)\newsetup.obj $(OBJ)\rnlib.obj

UUTRAFOBJ = $(OBJ)\uutraf.obj $(OBJ)\version.obj $(OBJ)\alist.obj\
            $(OBJ)\list.obj $(OBJ)\llist.obj

# *--------------------------------------------------------------------*
# *      Lists of programs to built for various pseudo targets         *
# *--------------------------------------------------------------------*

EXEC =\
    expire.exe fmt.$(COMEXE) fromwho.$(COMEXE) genhist.exe              \
    gensig.$(COMEXE) inews.$(COMEXE) mail.exe newsrun.exe               \
    novrstrk.$(COMEXE) rmail.exe rnews.$(COMEXE) sendbats.exe uucico.exe\
    uucp.exe uuname.$(COMEXE) uupoll.exe uuport.$(COMEXE)         \
    uustat.exe uusub.$(COMEXE) uutraf.exe uux.exe uuxqt.exe \
    $(EXTRAT)

INSTALL1= $(PROD)\mail.exe $(PROD)\rmail.exe $(PROD)\uuxqt.exe    \
          $(PROD)\uucico.exe $(PROD)\uuport.$(COMEXE) $(EXTRA1)

INSTALL2=\
    $(PROD)\uucp.exe $(PROD)\uuname.$(COMEXE) $(PROD)\uupoll.exe  \
    $(PROD)\uustat.exe $(PROD)\uusub.$(COMEXE)                    \
    $(PROD)\uux.exe $(PROD)\gensig.$(COMEXE)                      \
    $(PROD)\novrstrk.$(COMEXE) $(PROD)\fmt.$(COMEXE)                    \
    $(PROD)\fromwho.$(COMEXE) $(EXTRA2)

INSTALL3=\
    $(PROD)\expire.exe $(PROD)\genhist.exe $(PROD)\inews.$(COMEXE)      \
    $(PROD)\newsrun.exe $(PROD)\rnews.$(COMEXE) $(PROD)\sendbats.exe    \
    $(PROD)\uutraf.exe $(EXTRA3)

INSTALLRS=$(PROD)\su.bat $(PROD)\uuio.bat $(PROD)\uuclean.bat\
          $(PROD)\waiting.bat

INSTALLW=$(WINWORD)\uupcmstr.doc  \
         $(WINWORD)\uupcindx.doc  \
         $(WINWORD)\manual.dot
INSTALLH=$(WINWORD)\uupcmstr.htm  \
         $(WINWORD)\img00001.gif
INSTALLP=$(WINWORD)\uupcmstr.ps

!if "$(OS)" == "Windows_NT"
INSTALLS= $(INSTALLRS) $(EXTRAS)
!else
!if "$(OS)" == "Windows_95"
INSTALLS= $(INSTALLRS) $(EXTRAS)
!else
!if "$(OS)" == "DOS"
INSTALLS= $(INSTALLRS) $(EXTRAS)
!else
INSTALLS= $(INSTALLRS:.bat=.cmd) $(EXTRAS)
!endif
!endif
!endif

INSTALL= $(INSTALL1) $(INSTALL2) $(INSTALL3) $(INSTALLS)

# *--------------------------------------------------------------------*
# *   Source browser files; because of space limitations, these        *
# *   are not always generated unless you change the C compiler        *
# *   options.                                                         *
# *--------------------------------------------------------------------*

# *--------------------------------------------------------------------*
# *     This needs to be expanded to *all* the OBJ files ... but       *
# *     not today, as the Wonderworks doesn't use it.                  *
# *--------------------------------------------------------------------*
SBRS = $(LIBDOSLIST:obj=sbr)

# *--------------------------------------------------------------------*
# *                   And, of course, our makefile                     *
# *--------------------------------------------------------------------*

!ifndef MAKEFILE
MAKEFILE = $(SRCSLASH)nmake.mak
!endif

README=         docs\00readme.now winword\license.txt
ZIPHEAD=        docs\ziphead.txt

# *--------------------------------------------------------------------*
# *                      C compiler and options                        *
# *--------------------------------------------------------------------*

!ifndef CC
CC      = cl
!endif

{$(UULIB)}.c{$(OBJ)}.obj:
        $(CC) $(CCOPT) $<

{$(MAIL)}.c{$(OBJ)}.obj:
        $(CC) $(CCOPT) $<

{$(RN)}.c{$(OBJ)}.obj:
        $(CC) $(CCOPT) /W3 $<

{$(NEWS)}.c{$(OBJ)}.obj:
        $(CC) $(CCOPT) $<

{$(TEST)}.c{$(OBJ)}.obj:
        $(CC) $(CCOPT) $<

{$(UUCICO)}.c{$(OBJ)}.obj:
        $(CC) $(CCOPT) $<

{$(UUCICO)}.asm{$(OBJ)}.obj:
        $(MASM) $(MASMOPTS) $(UUCICO)\$(@B) ,$@;

{$(UUCP)}.c{$(OBJ)}.obj:
        $(CC) $(CCOPT) $<

{$(UTIL)}.c{$(OBJ)}.obj:
        $(CC) $(CCOPT) $<

{$(UUTRAF)}.c{$(OBJ)}.obj:
        $(CC) -DSYSVR3 -DUUPC -I$(UUTRAF) $(CCOPT) /W1 $<

# *--------------------------------------------------------------------*
# *                Rules for copying into production                   *
# *--------------------------------------------------------------------*

# *--------------------------------------------------------------------*
# *                         Prod .EXE files                            *
# *--------------------------------------------------------------------*

.exe{$(PROD)}.exe:
!if "$(OS)" == "OS/2_32_Bit"
        dllrname /Q /R $? $(RTLDLL:.dll=)=$(DLLNAME:.dll=)
!endif
        $(INSTALLCMD) $? $@
        if exist $(?R).pdb $(INSTALLCMD) $(?R).pdb $(@R).pdb
!ifndef KEWKEEPEXE
        - @if exist $? $(ERASE) $?
        - @if exist $(?R).pdb $(ERASE) $(?R).pdb
!endif
        - @if exist $(?R).ilk $(ERASE) $(?R).ilk
        - @if exist $(?R).dbg $(ERASE) $(?R).dbg
        - @if exist $(?R).map $(ERASE) $(?R).map

# *--------------------------------------------------------------------*
# *                         PROD .COM files                            *
# *--------------------------------------------------------------------*

.com{$(PROD)}.com:
        - @if exist $(?R).dbg $(ERASE) $(?R).dbg
        - @if exist $(?R).map $(ERASE) $(?R).map
        - $(ERASE) $@
        $(INSTALLCMD) $? $@

.dll{$(PROD)}.dll:
        $(INSTALLCMD) $? $@

{$(SCRIPTS)}.bat{$(PROD)}.bat:
        copy $? $@

!if "$(OS)" == "Windows_NT"

{$(SCRIPTS)}.bat{$(PROD)}.cmd:
        copy $? $@
!else

{$(SCRIPTS)}.cmd{$(PROD)}.cmd:
        copy $? $@
!endif

{$(ICONS)}.ico{$(PROD)}.ico:
        copy $? $@

# *--------------------------------------------------------------------*
# *            ZIP, for creating the distributed archive               *
# *--------------------------------------------------------------------*

!ifndef ZIP
ZIP     = zip
!endif

!ifndef ZIPOPT1
ZIPOPT1 = -o -9 -u -@
!endif

!ifndef ZIPOPT2
ZIPOPT2 = -j $(ZIPOPT1)
!endif

# *--------------------------------------------------------------------*
# *                Begin rules for building modules.                   *
# *--------------------------------------------------------------------*

# *--------------------------------------------------------------------*
# *        The first definition is the one built by default.           *
# *--------------------------------------------------------------------*

test:   $(OBJ) $(EXEC)

news:   $(OBJ) expire.exe genhist.exe inews.$(COMEXE) newsrun.exe \
        rnews.$(COMEXE) sendbats.exe

mail:   $(OBJ) mail.exe rmail.exe

!ifdef GUI
INSTALLG= $(PROD)\uupoll.exe $(PROD)\uucico.exe $(PROD)\uusmtpd.exe $(PROD)\uupopd.exe
GUILIST= uupoll.exe uucico.exe uusmtpd.exe uupopd.exe

gui: $(GUILIST)
         @echo Done building $**

guiprod: $(INSTALLG) regen
         @echo Done building $(**:regen=)

gzip:    $(OBJ) $(ZIPEG) regen

!else
gui:
         $(MAKE) -f $(MAKEFILE) GUI=1 $@

guiprod:
         $(MAKE) -f $(MAKEFILE) NODEBUG=1 GUI=1 $@

gzip:
         $(MAKE) -f $(MAKEFILE) NODEBUG=1 GUI=1 $@

!endif

prod:   $(OBJ) $(EXEC) $(INSTALL) regen

qprod:  $(OBJ) $(INSTALL)

install:        zip

zip:    clean $(OBJ) qzip

!ifdef NODEBUG
qzip:           ezip szip wzip regen
        @echo Build complete.
!else
qzip:
        $(MAKE) -f $(MAKEFILE) NODEBUG=1  qzip
!endif

ezip:   $(ZIPE1) $(ZIPE2) $(ZIPE3) $(EXTRAZIP)

# *--------------------------------------------------------------------*
# *                 Build just the source .ZIP file.                   *
# *--------------------------------------------------------------------*

szip:   $(SRCZIPV1) $(SRCZIPV2) $(SRCZIPV3) $(SRCZIPV4)

wzip:   $(WORDZIP) $(HTMLZIP) $(PSZIP) $(DOCSZIP)

# *--------------------------------------------------------------------*
# *          Do a purge of various temporary output files.             *
# *--------------------------------------------------------------------*

clean:  xclean
        - $(ERASE) $(SRCSLASH)*.bak
        - $(ERASE) *.map
        - $(ERASE) $(WORKFILE)

# *--------------------------------------------------------------------*
# *      Do a clean of just executable files and related files.        *
# *--------------------------------------------------------------------*

xclean:
        - $(ERASE) $(OBJ)\*.obj
        - $(ERASE) $(OBJ)\*.exe
        - $(ERASE) $(LIBCOMM)
        - $(ERASE) $(SRCSLASH)*.obj
        - $(ERASE) $(SRCSLASH)*.exe
        - $(ERASE) $(SBR)\*.sbr
!ifndef NORECURSION
        $(MAKE) -f $(MAKEFILE) NORECURSION=1 MODEL=$(BIGMODEL) $@
        $(MAKE) -f $(MAKEFILE) NORECURSION=1 MODEL=T $@
!endif

# *--------------------------------------------------------------------*
# *               The commands to actually link files                  *
# *--------------------------------------------------------------------*

comm34.$(COMEXET):   $(OBJ)\comm34.obj $(LIBFILE) $(LIBCOMM) $(DEFFILE)
        $(CC) $(LDOPT) $** $(OTHERLIBS)

inews.$(COMEXET):    $(OBJ)\inews.obj $(LIBFILE) $(LIBCOMM) $(DEFFILE)
        $(CC) $(LDOPT) $** $(OTHERLIBS)

!if "$(MODEL)" != "S"
expire.$(COMEXET):  $(OBJ)\expire.obj $(LIBNEWS) $(LIBFILE) $(LIBCOMM) $(DEFFILE)
        $(CC) $(LDOPT) $** $(OTHERLIBS)
!endif

fmt.$(COMEXET):      $(OBJ)\fmt.obj $(LIBFILE) $(LIBCOMM) $(DEFFILE)
        $(CC) $(LDOPT) $** $(OTHERLIBS)

fromwho.$(COMEXET):  $(OBJ)\fromwho.obj $(LIBFILE) $(LIBCOMM) $(DEFFILE)
        $(CC) $(LDOPT) $** $(OTHERLIBS)

!if "$(MODEL)" != "S"
genhist.$(COMEXET):  $(OBJ)\genhist.obj $(LIBNEWS) $(LIBFILE) $(LIBCOMM) $(DEFFILE)
        $(CC) $(LDOPT) $** $(OTHERLIBS)
!endif

gensig.$(COMEXET):   $(OBJ)\gensig.obj $(LIBFILE) $(LIBCOMM) $(DEFFILE)
        $(CC) $(LDOPT) $** $(OTHERLIBS)

!if "$(MODEL)" != "S"
newsrun.$(COMEXET):  $(OBJ)\newsrun.obj $(LIBNEWS) $(LIBFILE) $(LIBCOMM) $(DEFFILE)
        $(CC) $(LDOPT) $** $(OTHERLIBS)
!endif

novrstrk.$(COMEXET): $(OBJ)\novrstrk.obj $(LIBFILE) $(LIBCOMM) $(DEFFILE)
        $(CC) $(LDOPT) $** $(OTHERLIBS)

rnews.$(COMEXET):    $(OBJ)\rnews.obj $(LIBFILE) $(LIBCOMM) $(DEFFILE)
        $(CC) $(LDOPT) $** $(OTHERLIBS)

uucp.$(COMEXET):     $(OBJ)\uucp.obj $(LIBFILE) $(LIBCOMM) $(DEFFILE)
        $(CC) $(LDOPT) $** $(OTHERLIBS)

uuname.$(COMEXET):   $(OBJ)\uuname.obj $(LIBFILE) $(LIBCOMM) $(DEFFILE)
        $(CC) $(LDOPT) $** $(OTHERLIBS)

uupoll.$(COMEXET):   $(OBJ)\uupoll.obj $(LIBFILE) $(LIBCOMM) $(DEFFILE)
        $(CC) $(LDOPT) $** $(OTHERLIBS)

uuport.$(COMEXET):   $(OBJ)\uuport.obj $(LIBFILE) $(LIBCOMM) $(DEFFILE)
        $(CC) $(LDOPT) $** $(OTHERLIBS)

uustat.$(COMEXET):   $(OBJ)\uustat.obj $(LIBFILE) $(LIBCOMM) $(DEFFILE)
        $(CC) $(LDOPT) $** $(OTHERLIBS)

uusub.$(COMEXET):    $(OBJ)\uusub.obj $(LIBFILE) $(LIBCOMM) $(DEFFILE)
        $(CC) $(LDOPT) $** $(OTHERLIBS)

uux.$(COMEXET):      $(OBJ)\uux.obj $(LIBFILE) $(LIBCOMM) $(DEFFILE)
        $(CC) $(LDOPT) $** $(OTHERLIBS)

uuxqt.$(COMEXET):    $(OBJ)\uuxqt.obj $(LIBFILE) $(LIBCOMM) $(DEFFILE)
        $(CC) $(LDOPT) $** $(OTHERLIBS)

# *--------------------------------------------------------------------*
# *      DOS specific module builds, which use response files to       *
# *      get around command line length limits                         *
# *--------------------------------------------------------------------*

!if "$(OS)" == "DOS"

!if "$(MODEL)" != "T"

comm34.com fmt.com fromwho.com gensig.com inews.com novrstrk.com \
        rnews.com uucp.com uuname.com uuport.com uustat.com uusub.com \
        uux.com comm34.com: always
        $(MAKE) -f $(MAKEFILE) NODEBUG=1 MODEL=T $@

!endif

!if "$(MODEL)" == "S"
expire.exe genhist.exe mail.exe  newsrun.exe rmail.exe sendbats.exe uucico.exe: always
        $(MAKE) -f $(MAKEFILE) MODEL=$(BIGMODEL) $@

!else
mail.exe:     $(MAILOBJ) $(LIBFILE) $(LIBCOMM)
       $(LINKER) $(LINKOPT) @<<
$(MAILOBJ: =+^
)
$@,,$(LIBFILE)+$(LIBCOMM);
<<

newsetup.exe:        $(NEWSETUPOBJ) $(LIBFILE) $(LIBCOMM)
        $(LINKER) $(LINKOPT) @<<
$(NEWSETUPOBJ: =+^
)
$@,,$(LIBFILE)+$(LIBCOMM);
<<

rmail.exe:     $(RMAILOBJ) $(LIBFILE) $(LIBCOMM)
        $(LINKER) $(LINKOPT) @<<
$(RMAILOBJ: =+^
)
$@,,$(LIBFILE)+$(LIBCOMM);
<<

rn.exe:        $(RNOBJ) $(LIBFILE) $(LIBCOMM)
        $(LINKER) $(LINKOPT) @<<
$(RNOBJ1: =+^
)+
$(RNOBJ2: =+^
)+
$(RNOBJ3: =+^
)
$@,,$(LIBFILE)+$(LIBCOMM);
<<

sendbats.exe:     $(SENDBATSOBJ) $(LIBNEWS) $(LIBFILE) $(LIBCOMM)
        $(LINKER) $(LINKOPT) @<<
$(SENDBATSOBJ: =+^
)
$@,,$(LIBNEWS)+$(LIBFILE)+$(LIBCOMM);
<<

uucico.exe:     $(UUCICOOBJ1) $(UUCICOOBJ3)\
                  $(LIBFILE) $(LIBCOMM)
        $(LINKER) $(LINKOPT) @<<
$(UUCICOOBJ1: =+^
)+
$(UUCICOOBJ3: =+^
)
$@,,$(LIBFILE)+$(LIBCOMM);
<<
!endif

uutraf.exe:      $(UUTRAFOBJ) $(LIBFILE) $(LIBCOMM)
        $(LINKER) $(LINKOPT) @<<
$(UUTRAFOBJ: =+^
)
$@,,$(LIBFILE)+$(LIBCOMM);
<<

testulib.exe:   $(OBJ)\testulib.obj $(OBJ)\ulib.obj $(OBJ)\comm.obj \
                $(LIBFILE) $(LIBCOMM)
        $(LINKER) $(LINKOPT) @<<
$(OBJ)\testulib.obj $(OBJ)\ulib.obj $(OBJ)\comm.obj
$@,,$(LIBFILE)+$(LIBCOMM);
<<

!else

# *--------------------------------------------------------------------*
# *      Non-DOS module builds, which exploit OS/2 and Windows/NT      *
# *      longer command line to specify all the modules on the         *
# *      command line.                                                 *
# *--------------------------------------------------------------------*

mail.exe:     $(MAILOBJ) $(LIBFILE) $(LIBCOMM) $(DEFFILE)
        $(CC) $(LDOPT) $** $(OTHERLIBS)

newsetup.exe: $(NEWSETUPOBJ) $(LIBFILE) $(LIBCOMM) $(DEFFILE)
        $(CC) $(LDOPT) $** $(OTHERLIBS)

uupopd.exe:   $(UUPOPDOBJ) \
              $(LIBFILE) $(LIBCOMM) $(DEFFILE)
        $(CC) $(LDOPT) $** $(OTHERLIBS)

rn.exe:        $(RNOBJ) $(LIBFILE) $(LIBCOMM) $(DEFFILE)
        $(CC) $(LDOPT) $** $(OTHERLIBS)

rmail.exe:     $(RMAILOBJ) $(RMAILS_DELIVERS) $(LIBFILE) $(LIBCOMM) $(DEFFILE)
        $(CC) $(LDOPT) $** $(OTHERLIBS)

rmailn.exe:    $(RMAILOBJ:deliver.=delivern.) $(LIBFILE) $(LIBCOMM) $(DEFFILE)
        $(CC) $(LDOPT) $** $(OTHERLIBS)

sendbats.exe: $(SENDBATSOBJ) $(LIBNEWS) $(LIBFILE) $(LIBCOMM) $(DEFFILE)
        $(CC) $(LDOPT) $** $(OTHERLIBS)

uucico.exe:   $(UUCICOOBJ1) $(UUCICOOBJ2) $(UUCICOOBJ3) \
              $(LIBFILE) $(LIBCOMM) $(DEFFILE)
        $(CC) $(LDOPT) $** $(OTHERLIBS)

uusmtpd.exe:   $(UUSMTPDOBJ) \
              $(LIBFILE) $(LIBCOMM) $(DEFFILE)
        $(CC) $(LDOPT) $** $(OTHERLIBS)

bsmtp.exe:   $(BSMTPOBJ) \
              $(LIBFILE) $(LIBCOMM) $(DEFFILE)
        $(CC) $(LDOPT) $** $(OTHERLIBS)

# *--------------------------------------------------------------------*
# *                  UUCICO without TCP/IP support                     *
# *--------------------------------------------------------------------*

UUCICOOBJ1A=$(UUCICOOBJ1:commlib=commlibn)
UUCICOOBJ1B=$(UUCICOOBJ1A:modem=modemn)
uucicon.exe:  $(UUCICOOBJ1B) $(UUCICOOBJ2) $(UUCICOOBJX) \
              $(LIBFILE) $(LIBCOMM) $(DEFFILE)
        $(CC) $(LDOPT) $** $(OTHERLIBS)

uutraf.exe:   $(UUTRAFOBJ) $(LIBFILE) $(LIBCOMM) $(DEFFILE)
        $(CC) $(LDOPT) $** $(OTHERLIBS)

regsetup.exe: $(OBJ)\regsetup.obj $(LIBFILE) $(LIBCOMM) $(DEFFILE)
        $(CC) $(LDOPT) $** $(OTHERLIBS)

uupcdll.dll:  $(OBJ)\uupcdll.obj uupcdll.exp
        $(CC) /LD $(LDOPT) $** $(OTHERLIBS:libcmt=msvcrt) -link -entry:DllMain -map:$(@B).map -force

uupcdll.exp:    uupcdll.def
        $(LIBPROG) /out:$(@:exp=lib)  /machine:$(CPU) /def:$(@:exp=def)

uupcdll.def : $(MAKEFILE) nmakever.mak nmakewnt.mak
        copy << $@
LIBRARY $(@B) BASE=000D0000
DESCRIPTION 'UUPC/extended $(VERS)  Copyright (c) 1989-2002 Kendra Electronic Wonderworks, All Rights Reserved'
CODE    execute,read
EXPORTS
        UUPCGetNewsSpoolSize
        UUPCGetParm
        UUPCInit
        UUPCSendMail
<<

dlltest.exe:  $(OBJ)/dlltest.obj
        $(CC) $(LDOPT) $** $(OTHERLIBS)

!endif

# *--------------------------------------------------------------------*
# *                          The zip files                             *
# *--------------------------------------------------------------------*

$(ZIPE1):  $(INSTALL1) $(README)
!if "$(OS)" != "OS/2_16_Bit"
       - if not exist $(@D) mkdir $(@D)
!endif
       -12 $(ZIP) $(ZIPOPT2) $@ < <<
       $(?: =^
       )
<<
       copy << +$(ZIPHEAD) $(WORKFILE)
Kendra Electronic Wonderworks: UUPC/extended $(VERS) $(OS:_= ) executables (1 of 3)
<<
       $(ZIP) -z $@  < $(WORKFILE)

$(ZIPE2):  $(INSTALL2) $(INSTALLS) $(README)
!if "$(OS)" != "OS/2_16_Bit"
       - if not exist $(@D) mkdir $(@D)
!endif
       -12 $(ZIP) $(ZIPOPT2) $@ < <<
       $(?: =^
       )
<<
       copy << +$(ZIPHEAD) $(WORKFILE)
Kendra Electronic Wonderworks: UUPC/extended $(VERS) $(OS:_= ) executables (2 of 3)
<<
       $(ZIP) -z $@  < $(WORKFILE)

$(ZIPE3):  $(INSTALL3) $(README)
!if "$(OS)" != "OS/2_16_Bit"
       - if not exist $(@D) mkdir $(@D)
!endif
       -12 $(ZIP) $(ZIPOPT2) $@ < <<
       $(?: =^
       )
<<
       copy << +$(ZIPHEAD) $(WORKFILE)
Kendra Electronic Wonderworks: UUPC/extended $(VERS) $(OS:_= ) executables (3 of 3)
<<
       $(ZIP) -z $@  < $(WORKFILE)

$(ZIPEG):  $(INSTALLG) $(README)
!if "$(OS)" != "OS/2_16_Bit"
       - if not exist $(@D) mkdir $(@D)
!endif
       -12 $(ZIP) $(ZIPOPT2) $@ < <<
       $(?: =^
       )
<<
       copy << +$(ZIPHEAD) $(WORKFILE)
Kendra Electronic Wonderworks: UUPC/extended $(VERS) $(OS:_= ) GUI mode executables
<<
       $(ZIP) -z $@  < $(WORKFILE)

# *--------------------------------------------------------------------*
# *                           UUPCSRC.ZIP                              *
# *--------------------------------------------------------------------*

$(SRCZIPV1):  borlandc.mak NMAKE.MAK NMAKEDOS.MAK NMAKEWNT.MAK \
             NMAKOS21.MAK NMAKOS22.MAK \
             $(README)
!if "$(OS)" != "OS/2_16_Bit"
       - if not exist $(@D) mkdir $(@D)
!endif
       -18 $(ZIP) $(ZIPOPT1) $@ < <<
*.MAK
LIB\*.C
LIB\*.H
LIB\*.MAK
WINSOCK\*.H
$(README)
icons.win\*.ico
icons.os2\*.ico
<<
       copy << +$(ZIPHEAD) $(WORKFILE)
Kendra Electronic Wonderworks: UUPC/extended $(VERS) source (1 of 4)
<<
       $(ZIP) -z $@  < $(WORKFILE)

$(SRCZIPV2): borlandc.mak $(MAKEFILE) nmakever.mak $(README)
!if "$(OS)" != "OS/2_16_Bit"
       - if not exist $(@D) mkdir $(@D)
!endif
       -18 $(ZIP) $(ZIPOPT1) $@ < <<
UUCICO\*.ASM
UUCICO\*.C
UUCICO\*.H
UUCICO\*.MAK
$(README)
<<
       copy << +$(ZIPHEAD) $(WORKFILE)
Kendra Electronic Wonderworks: UUPC/extended $(VERS) source (2 of 4)
<<
       $(ZIP) -z $@  < $(WORKFILE)

$(SRCZIPV3): borlandc.mak $(MAKEFILE) nmakever.mak $(README)
!if "$(OS)" != "OS/2_16_Bit"
       - if not exist $(@D) mkdir $(@D)
!endif
       -18 $(ZIP) $(ZIPOPT1) $@ < <<
MAIL\*.C
MAIL\*.H
MAIL\*.MAK
UUCP\*.C
UUCP\*.H
UUCP\*.MAK
$(README)
<<
       copy << +$(ZIPHEAD) $(WORKFILE)
Kendra Electronic Wonderworks: UUPC/extended $(VERS) source (3 of 4)
<<
       $(ZIP) -z $@  < $(WORKFILE)

$(SRCZIPV4): borlandc.mak $(MAKEFILE) nmakever.mak $(README)
!if "$(OS)" != "OS/2_16_Bit"
       - if not exist $(@D) mkdir $(@D)
!endif
       -18 $(ZIP) $(ZIPOPT1) $@ < <<
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
<<
       copy << +$(ZIPHEAD) $(WORKFILE)
Kendra Electronic Wonderworks: UUPC/extended $(VERS) source (4 of 4)
<<
       $(ZIP) -z $@  < $(WORKFILE)

# *--------------------------------------------------------------------*
# *                    Word for Windows ZIP file                       *
# *--------------------------------------------------------------------*

$(WORDZIP):  $(INSTALLW) $(README)
!if "$(OS)" != "OS/2_16_Bit"
       - if not exist $(@D) mkdir $(@D)
!endif
       -12 $(ZIP) $(ZIPOPT2) $@ < <<
       $(?: =^
       )
<<
       copy << +$(ZIPHEAD) $(WORKFILE)
Kendra Electronic Wonderworks: UUPC/extended $(VERS) Documentation (Word for Windows)
<<
       $(ZIP) -z $@  < $(WORKFILE)

$(HTMLZIP):  $(INSTALLH) $(README)
!if "$(OS)" != "OS/2_16_Bit"
       - if not exist $(@D) mkdir $(@D)
!endif
       -12 $(ZIP) $(ZIPOPT2) $@ < <<
       $(?: =^
       )
<<
       copy << +$(ZIPHEAD) $(WORKFILE)
Kendra Electronic Wonderworks: UUPC/extended $(VERS) Documentation (HTML)
<<
       $(ZIP) -z $@  < $(WORKFILE)

$(PSZIP):  $(INSTALLP) $(README)
!if "$(OS)" != "OS/2_16_Bit"
       - if not exist $(@D) mkdir $(@D)
!endif
       -12 $(ZIP) $(ZIPOPT2) $@ < <<
       $(?: =^
       )
<<
       copy << +$(ZIPHEAD) $(WORKFILE)
Kendra Electronic Wonderworks: UUPC/extended $(VERS) Documentation (Post Script)
<<
       $(ZIP) -z $@  < $(WORKFILE)

$(DOCSZIP):  $(LISTFILES) $(SAMPLES) $(README)
!if "$(OS)" != "OS/2_16_Bit"
       - if not exist $(@D) mkdir $(@D)
!endif
       -12 $(ZIP) $(ZIPOPT2) $@ < <<
       $(?: =^
       )
$(DOCS)\*.mdm
<<
       copy << +$(ZIPHEAD) $(WORKFILE)
Kendra Electronic Wonderworks: UUPC/extended $(VERS) Documentation and Samples
<<
       $(ZIP) -z $@  < $(WORKFILE)

# *--------------------------------------------------------------------*
# *                     Handle shared libraries                        *
# *--------------------------------------------------------------------*

!ifdef DLLNAME

!if "$(DLLPROD)" == "$(DLLDIR)"
!error Prod DLL directory "$(DLLPROD)" must not equal input directory
!endif

$(DLLPROD)\$(DLLNAME):      $(DLLDIR)\$(RTLDLL)
        - erase $@
        copy $? $@
        dllrname /n /q $@ $(?B)=$(@B)
        @echo Binding messages from  $(MSGFILE) to $@
        msgbind <<
>$@
<$(MSGFILE)
*
<<
!endif

# *--------------------------------------------------------------------*
# *                    Build the object libraries                      *
# *--------------------------------------------------------------------*

!ifdef WIN32ENV

$(LIBCOMM): $(LIBCOMMLIST)
         $(LIBPROG) /out:$@ @<<
$(LIBCOMMLIST)
<<

$(LIBFILE): $(LIBFILELIST)
         $(LIBPROG) /out:$@ @<<
$(LIBFILELIST)
<<

$(LIBNEWS): $(LIBNEWSLIST)
         $(LIBPROG) /out:$@ @<<
$(LIBNEWSLIST)
<<

!else
$(LIBFILE): $(LIBFILELIST)
        $(LIBPROG) /nologo /noignorecase $@ @<<
-+$(?: =&^
-+)
nul;
<<

$(LIBCOMM): $(LIBCOMMLIST)
        $(LIBPROG) /nologo /noignorecase $@ @<<
-+$(?: =&^
-+)
nul;
<<

$(LIBNEWS): $(LIBNEWSLIST)
        $(LIBPROG) /nologo /noignorecase $@ @<<
-+$(?: =&^
-+)
nul;
<<

!endif

# *--------------------------------------------------------------------*
# *     The timestamp module has a reference to this MAKEFILE,         *
# *     which insures whenever we change the version number the        *
# *     time stamp gets re-generated.                                  *
# *--------------------------------------------------------------------*

$(TIMESTMP): $(MAKEFILE) $(REGEN) \
             nmakedos.mak nmakos22.mak nmakewnt.mak nmakos21.mak

# *--------------------------------------------------------------------*
# *     Header file with our version number for timestmp.c             *
# *--------------------------------------------------------------------*

$(UULIB)\win32ver.h: $(MAKEFILE) nmakever.mak $(REGEN)
        copy << $@
/*--------------------------------------------------------------------*/
/*         DO NOT EDIT -- AUTOMATICALLY GENERATED BY MAKEFILE         */
/*--------------------------------------------------------------------*/

#define UUPCV "$(VERS)"         /* UUPC/extended version number */
<<

# *--------------------------------------------------------------------*
# *     Force a regeneration of the time stamp/version module.         *
# *--------------------------------------------------------------------*

regen:  $(UULIB)\timestmp.c
        - if exist $(LIBCOMM) $(ERASE) $(LIBCOMM)
        - if exist $(LIBFILE) $(ERASE) $(LIBFILE)
        - if exist $(LIBNEWS) $(ERASE) $(LIBNEWS)
        - $(ERASE) $(OBJ)\timestmp.obj

# *--------------------------------------------------------------------*
# *         Generate the source browser database for the PWB           *
# *--------------------------------------------------------------------*

$(MAKEFILE).bsc: $(SBRS)
        pwbrmake @<<
/o $(MAKEFILE).bsc $(SBRS)
<<

$(OBJ):
       - if not exist $@\*.* mkdir $@

always:
        @rem no operation

# *--------------------------------------------------------------------*
# *      Definition file.  Built dynamically so it automatically       *
# *      includes the correct version nuber.                           *
# *--------------------------------------------------------------------*

!ifdef DEFFILE
$(DEFFILE):     $(MAKEFILE) nmakos22.mak nmakos21.mak
       copy << $@
NAME WINDOWCOMPAT NEWFILES
DESCRIPTION 'UUPC/extended $(VERS), Changes Copyright (c) 1989-2002 Kendra Electronic Wonderworks, All Rights Reserved'
PROTMODE
STACKSIZE $(STACKSIZE)
SEGMENTS
  '_MSGSEG32' CLASS 'CODE' LOADONCALL
<<
!endif

# *--------------------------------------------------------------------*
# *      Lastly, include the rules for building the object             *
# *      files.  This line can be deleted, but then you lose the       *
# *      automatic recompiles when header files are updated.           *
# *--------------------------------------------------------------------*

!include "nmakeinc.mak"
