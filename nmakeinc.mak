#       Copyright (c) 1989-1995 by Kendra Electronic Wonderworks;
#       all rights reserved except those explicitly granted by
#       the UUPC/extended license.

#       NMAKEINC.MAK automatically generated on 16 Sep 1995 at 07:20:03
#       by D:\OS2BIN\dependc.cmd.

#       DO NOT EDIT THIS FILE BY HAND -- CHANGES WILL BE LOST
#       (Submit permanent changes to help@kew.com)

#      $Id: nmakeinc.mak 1.17 1995/09/16 12:22:41 ahd v1-12p $
#
#      $Log: nmakeinc.mak $
#      Revision 1.17  1995/09/16 12:22:41  ahd
#      New automatically generated revision
#

$(OBJ)\arbmath.obj : lib\arbmath.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\arbmath.h

$(OBJ)\arpadate.obj : lib\arpadate.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\arpadate.h

$(OBJ)\bugout.obj : lib\bugout.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\catcher.h lib\timestmp.h\
        lib\winutil.h

$(OBJ)\catcher.obj : lib\catcher.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h winsock\winsock.h\
        lib\timestmp.h lib\catcher.h lib\safeio.h lib\uutypes.h lib\pwinsock.h lib\winutil.h

$(OBJ)\chdir.obj : lib\chdir.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h

$(OBJ)\checkptr.obj : lib\checkptr.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h

$(OBJ)\configur.obj : lib\configur.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\timestmp.h\
        lib\pushpop.h lib\setstdin.h lib\pnterr.h

$(OBJ)\creat.obj : lib\creat.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h

$(OBJ)\dater.obj : lib\dater.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\dater.h

$(OBJ)\dos2unix.obj : lib\dos2unix.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\dos2unix.h\
        lib\uundir.h

$(OBJ)\execute.obj : lib\execute.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\execute.h\
        lib\winutil.h lib\pnterr.h lib\pos2err.h

$(OBJ)\expath.obj : lib\expath.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\expath.h lib\hostable.h\
        lib\security.h lib\usertabl.h lib\pushpop.h

$(OBJ)\export.obj : lib\export.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\arbmath.h lib\export.h\
        lib\import.h lib\usertabl.h lib\hostable.h lib\security.h

$(OBJ)\filebkup.obj : lib\filebkup.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h

$(OBJ)\fopen.obj : lib\fopen.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\ssleep.h

$(OBJ)\getargs.obj : lib\getargs.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h

$(OBJ)\getdta.obj : lib\getdta.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\getdta.h

$(OBJ)\getopt.obj : lib\getopt.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\getopt.h

$(OBJ)\getseq.obj : lib\getseq.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\getseq.h

$(OBJ)\hostable.obj : lib\hostable.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\hostable.h\
        lib\security.h

$(OBJ)\hostatus.obj : lib\hostatus.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\hostable.h\
        lib\hostatus.h lib\security.h lib\timestmp.h lib\stater.h

$(OBJ)\hostrset.obj : lib\hostrset.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\hostable.h\
        lib\hostatus.h lib\hostrset.h lib\security.h lib\timestmp.h

$(OBJ)\imfile.obj : lib\imfile.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\imfile.h lib\execute.h

$(OBJ)\import.obj : lib\import.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\pos2err.h lib\import.h\
        lib\arbmath.h lib\hostable.h lib\usertabl.h lib\security.h lib\pnterr.h

$(OBJ)\kanjicnv.obj : lib\kanjicnv.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\kanjicnv.h

$(OBJ)\lock.obj : lib\lock.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\dater.h lib\stater.h\
        lib\lock.h lib\import.h

$(OBJ)\logger.obj : lib\logger.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\dater.h lib\expath.h\
        lib\logger.h lib\timestmp.h

$(OBJ)\mkdir.obj : lib\mkdir.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h

$(OBJ)\mkfilenm.obj : lib\mkfilenm.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h

$(OBJ)\mkmbox.obj : lib\mkmbox.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h

$(OBJ)\mktempnm.obj : lib\mktempnm.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h

$(OBJ)\mktime2.obj : lib\mktime2.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h

$(OBJ)\ndir.obj : lib\ndir.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\uundir.h lib\getdta.h\
        lib\dos2unix.h lib\uundir.h lib\getdta.h

$(OBJ)\ndirnt.obj : lib\ndirnt.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\uundir.h lib\dos2unix.h\
        lib\uundir.h

$(OBJ)\ndiros2.obj : lib\ndiros2.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\uundir.h\
        lib\dos2unix.h lib\uundir.h

$(OBJ)\ndirwin.obj : lib\ndirwin.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\uundir.h\
        lib\dos2unix.h lib\uundir.h

$(OBJ)\normaliz.obj : lib\normaliz.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h

$(OBJ)\pnterr.obj : lib\pnterr.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\pnterr.h

$(OBJ)\pos2err.obj : lib\pos2err.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\pos2err.h

$(OBJ)\printerr.obj : lib\printerr.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h

$(OBJ)\printmsg.obj : lib\printmsg.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\dater.h lib\logger.h

$(OBJ)\pushpop.obj : lib\pushpop.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\pushpop.h

$(OBJ)\pwinsock.obj : lib\pwinsock.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h winsock\winsock.h\
        lib\pwinsock.h

$(OBJ)\readnext.obj : lib\readnext.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\readnext.h\
        lib\uundir.h lib\hostable.h lib\security.h

$(OBJ)\remove2.obj : lib\remove2.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\pos2err.h

$(OBJ)\rename.obj : lib\rename.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h

$(OBJ)\safeio.obj : lib\safeio.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\safeio.h lib\uutypes.h

$(OBJ)\safeout.obj : lib\safeout.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\safeio.h lib\uutypes.h

$(OBJ)\scrsize.obj : lib\scrsize.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\scrsize.h

$(OBJ)\scrsize2.obj : lib\scrsize2.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\scrsize.h

$(OBJ)\scrsiznt.obj : lib\scrsiznt.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\scrsize.h\
        lib\pnterr.h

$(OBJ)\security.obj : lib\security.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\hostable.h\
        lib\security.h lib\usertabl.h lib\expath.h

$(OBJ)\setstdin.obj : lib\setstdin.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\setstdin.h

$(OBJ)\ssleep.obj : lib\ssleep.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\ssleep.h lib\winutil.h\
        lib\pos2err.h lib\safeio.h lib\uutypes.h lib\catcher.h

$(OBJ)\stater.obj : lib\stater.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h

$(OBJ)\strlwr.obj : lib\strlwr.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h

$(OBJ)\strpool.obj : lib\strpool.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h

$(OBJ)\timestmp.obj : lib\timestmp.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\timestmp.h\
        lib\win32ver.h lib\winutil.h lib\ssleep.h lib\title.h

$(OBJ)\title.obj : lib\title.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\title.h lib\timestmp.h

$(OBJ)\title2.obj : lib\title2.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\timestmp.h lib\title.h

$(OBJ)\titlen.obj : lib\titlen.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\title.h lib\timestmp.h

$(OBJ)\trumpet.obj : lib\trumpet.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\ssleep.h

$(OBJ)\usertabl.obj : lib\usertabl.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\expath.h\
        lib\usertabl.h lib\hostable.h lib\security.h lib\pushpop.h

$(OBJ)\usrcatch.obj : lib\usrcatch.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\timestmp.h\
        lib\usrcatch.h

$(OBJ)\validcmd.obj : lib\validcmd.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\hostable.h\
        lib\security.h lib\usertabl.h lib\expath.h

$(OBJ)\winutil.obj : lib\winutil.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\winutil.h

$(OBJ)\address.obj : mail\address.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h mail\address.h\
        lib\hostable.h lib\security.h

$(OBJ)\alias.obj : mail\alias.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\hostable.h lib\security.h\
        lib\usertabl.h mail\alias.h mail\address.h lib\expath.h

$(OBJ)\deliver.obj : mail\deliver.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\imfile.h\
        mail\address.h mail\deliver.h lib\expath.h lib\execute.h lib\getseq.h lib\kanjicnv.h lib\hostable.h lib\import.h\
        lib\pushpop.h lib\security.h lib\stater.h lib\usertabl.h mail\sysalias.h lib\timestmp.h lib\trumpet.h lib\arpadate.h

$(OBJ)\mail.obj : mail\mail.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h mail\address.h mail\alias.h\
        lib\dater.h lib\expath.h lib\getopt.h mail\mail.h mail\mailblib.h mail\maillib.h lib\imfile.h mail\mailsend.h\
        mail\mlib.h lib\pushpop.h lib\stater.h lib\timestmp.h lib\arpadate.h lib\title.h lib\winutil.h

$(OBJ)\mailblib.obj : mail\mailblib.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h mail\address.h\
        mail\mail.h mail\maillib.h lib\imfile.h mail\mailblib.h mail\mailsend.h mail\alias.h lib\expath.h lib\execute.h

$(OBJ)\maillib.obj : mail\maillib.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h mail\address.h\
        mail\mlib.h mail\alias.h mail\mail.h mail\maillib.h lib\imfile.h lib\scrsize.h

$(OBJ)\mailsend.obj : mail\mailsend.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\arpadate.h\
        lib\expath.h lib\execute.h mail\mlib.h mail\alias.h mail\mail.h mail\maillib.h lib\imfile.h mail\mailblib.h\
        mail\mailsend.h lib\safeio.h lib\uutypes.h mail\address.h

$(OBJ)\mlib.obj : mail\mlib.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\execute.h

$(OBJ)\rmail.obj : mail\rmail.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h mail\address.h lib\arpadate.h\
        lib\imfile.h mail\deliver.h lib\getopt.h lib\hostable.h lib\logger.h lib\security.h lib\usertabl.h lib\timestmp.h\
        lib\catcher.h lib\winutil.h

$(OBJ)\sysalias.obj : mail\sysalias.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\imfile.h\
        mail\deliver.h mail\sysalias.h

$(OBJ)\active.obj : news\active.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h news\active.h

$(OBJ)\activeo.obj : news\activeo.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\timestmp.h\
        news\active.h news\importng.h lib\getopt.h

$(OBJ)\batch.obj : news\batch.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\timestmp.h lib\import.h\
        lib\getseq.h news\batch.h news\sys.h lib\execute.h lib\imfile.h lib\stater.h news\sys.h

$(OBJ)\cache.obj : news\cache.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h news\cache.h

$(OBJ)\expire.obj : news\expire.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h news\active.h lib\dater.h\
        lib\getopt.h news\history.h lib\import.h news\importng.h lib\logger.h lib\uundir.h lib\pushpop.h lib\stater.h\
        lib\timestmp.h

$(OBJ)\genhist.obj : news\genhist.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h news\active.h lib\dater.h\
        lib\getopt.h news\history.h lib\import.h news\importng.h lib\logger.h lib\uundir.h lib\pushpop.h lib\stater.h\
        lib\timestmp.h

$(OBJ)\hdbm.obj : news\hdbm.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h news\hdbm.h news\idx.h

$(OBJ)\history.obj : news\history.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\timestmp.h\
        news\active.h news\history.h news\importng.h news\hdbm.h

$(OBJ)\idx.obj : news\idx.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h news\idx.h news\cache.h

$(OBJ)\importng.obj : news\importng.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\import.h\
        news\importng.h

$(OBJ)\inews.obj : news\inews.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\getopt.h lib\getseq.h\
        lib\import.h lib\logger.h lib\timestmp.h lib\execute.h lib\arpadate.h lib\hostable.h

$(OBJ)\newsrun.obj : news\newsrun.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h news\active.h\
        lib\getopt.h lib\getseq.h news\history.h lib\hostable.h lib\import.h lib\imfile.h news\importng.h lib\logger.h\
        lib\timestmp.h lib\stater.h lib\execute.h news\batch.h news\sys.h news\sys.h

$(OBJ)\rnews.obj : news\rnews.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\getopt.h lib\logger.h\
        lib\timestmp.h lib\getseq.h lib\execute.h

$(OBJ)\sendbats.obj : news\sendbats.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h news\active.h\
        lib\getopt.h lib\getseq.h news\history.h lib\import.h news\importng.h lib\logger.h lib\timestmp.h lib\title.h\
        news\batch.h news\sys.h news\sys.h lib\getopt.h

$(OBJ)\sys.obj : news\sys.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\timestmp.h news\active.h\
        lib\hostable.h news\sys.h lib\stater.h lib\arpadate.h

$(OBJ)\dlltest.obj : test\dlltest.c uupcdll.h

$(OBJ)\testimp.obj : test\testimp.c lib\lib.h lib\import.h lib\export.h lib\timestmp.h

$(OBJ)\testscrt.obj : test\testscrt.c lib\lib.h lib\hlib.h script.h

$(OBJ)\testsec.obj : test\testsec.c lib\lib.h lib\expath.h lib\hlib.h lib\hostable.h lib\usertabl.h lib\security.h

$(OBJ)\testulib.obj : test\testulib.c lib\lib.h lib\hlib.h ulib.h lib\catcher.h lib\timestmp.h comm.h

$(OBJ)\tprot.obj : test\tprot.c uucp.h

$(OBJ)\comm34.obj : util\comm34.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\timestmp.h

$(OBJ)\fmt.obj : util\fmt.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\timestmp.h

$(OBJ)\fromwho.obj : util\fromwho.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\timestmp.h\
        lib\getopt.h

$(OBJ)\gensig.obj : util\gensig.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\timestmp.h

$(OBJ)\install.obj : util\install.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\timestmp.h

$(OBJ)\novrstrk.obj : util\novrstrk.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\timestmp.h

$(OBJ)\regsetup.obj : util\regsetup.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\getopt.h\
        lib\pushpop.h lib\timestmp.h lib\pnterr.h

$(OBJ)\uupcdll.obj : util\uupcdll.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\hlib.h

$(OBJ)\uupoll.obj : util\uupoll.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\getopt.h lib\timestmp.h\
        lib\ssleep.h lib\arpadate.h lib\safeio.h lib\uutypes.h lib\dater.h lib\execute.h lib\title.h

$(OBJ)\catcheru.obj : uucico\catcheru.c lib\catcher.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h\
        winsock\winsock.h lib\timestmp.h lib\catcher.h lib\safeio.h lib\uutypes.h lib\pwinsock.h lib\winutil.h

$(OBJ)\checktim.obj : uucico\checktim.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h uucico\checktim.h

$(OBJ)\commlib.obj : uucico\commlib.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h uucico\commlib.h\
        uucico\dcp.h lib\hostable.h lib\usertabl.h lib\security.h uucico\modem.h uucico\ulib.h uucico\ulibfs.h uucico\ulib14.h\
        uucico\ulibip.h uucico\ulibnmp.h

$(OBJ)\commlibn.obj : uucico\commlibn.c uucico\commlib.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h\
        uucico\commlib.h uucico\dcp.h lib\hostable.h lib\usertabl.h lib\security.h uucico\modem.h uucico\ulib.h\
        uucico\ulibfs.h uucico\ulib14.h uucico\ulibip.h uucico\ulibnmp.h

$(OBJ)\dcp.obj : uucico\dcp.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\catcher.h uucico\checktim.h\
        uucico\dcp.h uucico\dcplib.h uucico\dcpstats.h uucico\dcpsys.h uucico\dcpxfer.h lib\expath.h lib\getopt.h\
        lib\hostable.h lib\hostatus.h lib\lock.h lib\logger.h uucico\modem.h lib\security.h lib\ssleep.h uucico\suspend.h\
        uucico\commlib.h lib\title.h lib\execute.h lib\pushpop.h lib\winutil.h

$(OBJ)\dcpepkt.obj : uucico\dcpepkt.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h uucico\dcp.h\
        uucico\dcpepkt.h uucico\dcpsys.h lib\hostable.h lib\security.h lib\ssleep.h uucico\modem.h uucico\commlib.h

$(OBJ)\dcpfpkt.obj : uucico\dcpfpkt.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h uucico\dcp.h\
        uucico\dcpfpkt.h uucico\dcpsys.h lib\hostable.h lib\security.h lib\ssleep.h uucico\modem.h uucico\commlib.h

$(OBJ)\dcpgpkt.obj : uucico\dcpgpkt.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h uucico\dcp.h\
        uucico\dcpsys.h uucico\dcpgpkt.h lib\hostable.h lib\security.h uucico\commlib.h uucico\modem.h lib\catcher.h

$(OBJ)\dcplib.obj : uucico\dcplib.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\arpadate.h\
        uucico\dcp.h uucico\dcplib.h uucico\dcpsys.h lib\execute.h lib\hostable.h lib\import.h uucico\modem.h lib\pushpop.h\
        lib\security.h lib\ssleep.h uucico\commlib.h lib\usertabl.h lib\timestmp.h

$(OBJ)\dcpstats.obj : uucico\dcpstats.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h uucico\dcp.h\
        uucico\dcpstats.h lib\stater.h lib\hostable.h lib\hostatus.h lib\security.h lib\timestmp.h lib\ssleep.h lib\lock.h

$(OBJ)\dcpsys.obj : uucico\dcpsys.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h uucico\checktim.h\
        uucico\dcp.h uucico\dcpfpkt.h uucico\dcpgpkt.h uucico\dcptpkt.h uucico\dcpepkt.h uucico\dcplib.h uucico\dcpsys.h\
        lib\export.h lib\hostable.h lib\hostatus.h uucico\modem.h lib\lock.h uucico\nbstime.h lib\uundir.h lib\ssleep.h\
        lib\security.h uucico\commlib.h

$(OBJ)\dcptpkt.obj : uucico\dcptpkt.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h winsock\winsock.h\
        uucico\dcp.h uucico\dcptpkt.h uucico\dcpsys.h lib\hostable.h lib\security.h lib\ssleep.h uucico\modem.h\
        uucico\commlib.h lib\pwinsock.h

$(OBJ)\dcpxfer.obj : uucico\dcpxfer.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h uucico\dcp.h\
        uucico\dcpsys.h uucico\dcpxfer.h lib\expath.h lib\hostable.h lib\import.h lib\security.h uucico\modem.h\
        uucico\commlib.h

$(OBJ)\fossil.obj : uucico\fossil.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h uucico\fossil.h

$(OBJ)\modem.obj : uucico\modem.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\hostable.h\
        lib\catcher.h uucico\checktim.h uucico\commlib.h lib\dater.h uucico\dcp.h uucico\dcpsys.h uucico\modem.h\
        uucico\script.h lib\security.h lib\ssleep.h uucico\suspend.h lib\usrcatch.h lib\title.h

$(OBJ)\nbstime.obj : uucico\nbstime.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\title.h\
        lib\arpadate.h uucico\dcp.h uucico\dcpsys.h lib\hostable.h uucico\nbstime.h uucico\script.h lib\security.h\
        uucico\commlib.h lib\catcher.h lib\pnterr.h

$(OBJ)\prtynt.obj : uucico\prtynt.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\pnterr.h\
        uucico\commlib.h

$(OBJ)\prtyos2.obj : uucico\prtyos2.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\pos2err.h

$(OBJ)\psos2err.obj : uucico\psos2err.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h uucico\psos2err.h

$(OBJ)\pwserr.obj : uucico\pwserr.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h winsock\winsock.h\
        uucico\pwserr.h

$(OBJ)\script.obj : uucico\script.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h uucico\dcp.h\
        uucico\dcpsys.h lib\hostable.h uucico\modem.h uucico\script.h lib\security.h lib\ssleep.h lib\catcher.h lib\usrcatch.h\
        uucico\commlib.h

$(OBJ)\suspend.obj : uucico\suspend.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\hostable.h\
        uucico\dcp.h lib\security.h uucico\suspend.h

$(OBJ)\suspend2.obj : uucico\suspend2.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\hostable.h\
        lib\security.h uucico\dcp.h uucico\dcpsys.h lib\safeio.h lib\uutypes.h uucico\modem.h lib\catcher.h lib\pos2err.h\
        uucico\suspend.h lib\ssleep.h lib\usrcatch.h

$(OBJ)\suspendn.obj : uucico\suspendn.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\hostable.h\
        lib\security.h uucico\dcp.h uucico\dcpsys.h lib\safeio.h lib\uutypes.h uucico\modem.h lib\catcher.h lib\pnterr.h\
        uucico\suspend.h lib\ssleep.h

$(OBJ)\ulib.obj : uucico\ulib.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h uucico\ulib.h uucico\comm.h\
        lib\ssleep.h lib\catcher.h uucico\commlib.h

$(OBJ)\ulib14.obj : uucico\ulib14.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h uucico\ulib14.h\
        uucico\comm.h lib\ssleep.h lib\catcher.h uucico\fossil.h uucico\commlib.h lib\hostable.h lib\usertabl.h lib\security.h

$(OBJ)\ulibfs.obj : uucico\ulibfs.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h uucico\ulibfs.h\
        uucico\commlib.h uucico\fossil.h lib\catcher.h lib\ssleep.h

$(OBJ)\ulibip.obj : uucico\ulibip.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h uucico\psos2err.h\
        lib\catcher.h winsock\winsock.h uucico\ulibip.h lib\catcher.h uucico\pwserr.h lib\pnterr.h lib\pwinsock.h\
        uucico\commlib.h

$(OBJ)\ulibnb.obj : uucico\ulibnb.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h ulibnb.h uucico\comm.h\
        lib\ssleep.h lib\catcher.h uucico\commlib.h

$(OBJ)\ulibnmp.obj : uucico\ulibnmp.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h uucico\ulibnmp.h\
        lib\ssleep.h lib\catcher.h uucico\commlib.h lib\pos2err.h

$(OBJ)\ulibnt.obj : uucico\ulibnt.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h uucico\ulib.h\
        lib\ssleep.h lib\catcher.h uucico\dcp.h uucico\commlib.h lib\pnterr.h uucico\suspend.h

$(OBJ)\ulibos2.obj : uucico\ulibos2.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h uucico\ulib.h\
        lib\ssleep.h lib\catcher.h lib\pos2err.h uucico\commlib.h lib\usrcatch.h

$(OBJ)\ulibwin.obj : uucico\ulibwin.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h chat.h uucico\ulib.h\
        lib\ssleep.h uucico\commlib.h lib\catcher.h

$(OBJ)\ulibwino.obj : uucico\ulibwino.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h chat.h uucico\ulib.h\
        lib\ssleep.h uucico\commlib.h

$(OBJ)\uucico.obj : uucico\uucico.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h uucico\dcp.h\
        lib\hostable.h lib\security.h lib\pushpop.h lib\timestmp.h lib\catcher.h

$(OBJ)\uuport.obj : uucico\uuport.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\timestmp.h\
        uucico\suspend.h lib\winutil.h lib\logger.h lib\ssleep.h

$(OBJ)\uucp.obj : uucp\uucp.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\expath.h lib\getopt.h\
        lib\getseq.h lib\hostable.h lib\import.h lib\uundir.h lib\security.h lib\timestmp.h lib\execute.h lib\winutil.h\
        lib\logger.h

$(OBJ)\uuname.obj : uucp\uuname.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\getopt.h lib\hostable.h\
        lib\security.h lib\timestmp.h

$(OBJ)\uustat.obj : uucp\uustat.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\dater.h lib\export.h\
        lib\getopt.h lib\getseq.h lib\hostable.h lib\hostatus.h lib\import.h lib\pushpop.h lib\readnext.h lib\security.h\
        lib\stater.h lib\timestmp.h

$(OBJ)\uusub.obj : uucp\uusub.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\hostable.h lib\dater.h\
        lib\hostrset.h lib\hostatus.h lib\getopt.h lib\security.h lib\timestmp.h

$(OBJ)\uux.obj : uucp\uux.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\getopt.h lib\getseq.h\
        lib\expath.h lib\import.h lib\pushpop.h lib\hostable.h lib\security.h lib\timestmp.h lib\winutil.h lib\logger.h

$(OBJ)\uuxqt.obj : uucp\uuxqt.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\dater.h lib\execute.h\
        lib\expath.h lib\getopt.h lib\getseq.h lib\hostable.h lib\import.h lib\lock.h lib\logger.h lib\pushpop.h\
        lib\readnext.h lib\security.h lib\timestmp.h lib\usertabl.h lib\uundir.h lib\winutil.h

