#       Copyright (c) 1989-1994 by Kendra Electronic Wonderworks;
#       all rights reserved except those explicitly granted by
#       the UUPC/extended license.

#       NMAKEINC.MAK automatically generated on 25 May 1994 at 18:05:23

#       DO NOT EDIT THIS FILE BY HAND -- CHANGES WILL BE LOST
#       (Submit permanent changes to help@kew.com)

#      $Id$
#
#      $Log$

$(OBJ)\active.obj : lib\active.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\timestmp.h lib\active.h\
        lib\importng.h lib\getopt.h
         $(CC) $(CCOPT) lib\active.c

$(OBJ)\arbmath.obj : lib\arbmath.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\arbmath.h
         $(CC) $(CCOPT) lib\arbmath.c

$(OBJ)\arpadate.obj : lib\arpadate.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\arpadate.h
         $(CC) $(CCOPT) lib\arpadate.c

$(OBJ)\bugout.obj : lib\bugout.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\catcher.h lib\timestmp.h\
        lib\winutil.h
         $(CC) $(CCOPT) lib\bugout.c

$(OBJ)\catcher.obj : lib\catcher.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h winsock\winsock.h\
        lib\timestmp.h lib\catcher.h lib\safeio.h lib\uutypes.h lib\pwinsock.h lib\winutil.h
         $(CC) $(CCOPT) lib\catcher.c

$(OBJ)\chdir.obj : lib\chdir.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h
         $(CC) $(CCOPT) lib\chdir.c

$(OBJ)\checkptr.obj : lib\checkptr.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h
         $(CC) $(CCOPT) lib\checkptr.c

$(OBJ)\configur.obj : lib\configur.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\timestmp.h\
        lib\pushpop.h lib\setstdin.h lib\pnterr.h
         $(CC) $(CCOPT) lib\configur.c

$(OBJ)\creat.obj : lib\creat.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h
         $(CC) $(CCOPT) lib\creat.c

$(OBJ)\dater.obj : lib\dater.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\dater.h
         $(CC) $(CCOPT) lib\dater.c

$(OBJ)\dos2unix.obj : lib\dos2unix.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\dos2unix.h\
        lib\uundir.h
         $(CC) $(CCOPT) lib\dos2unix.c

$(OBJ)\execute.obj : lib\execute.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\execute.h\
        lib\winutil.h lib\pnterr.h lib\pos2err.h
         $(CC) $(CCOPT) lib\execute.c

$(OBJ)\expath.obj : lib\expath.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\expath.h lib\hostable.h\
        lib\security.h lib\usertabl.h lib\pushpop.h
         $(CC) $(CCOPT) lib\expath.c

$(OBJ)\export.obj : lib\export.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\arbmath.h lib\export.h\
        lib\import.h lib\usertabl.h lib\hostable.h lib\security.h
         $(CC) $(CCOPT) lib\export.c

$(OBJ)\filebkup.obj : lib\filebkup.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h
         $(CC) $(CCOPT) lib\filebkup.c

$(OBJ)\fopen.obj : lib\fopen.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\ssleep.h
         $(CC) $(CCOPT) lib\fopen.c

$(OBJ)\getargs.obj : lib\getargs.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h
         $(CC) $(CCOPT) lib\getargs.c

$(OBJ)\getdta.obj : lib\getdta.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\getdta.h
         $(CC) $(CCOPT) lib\getdta.c

$(OBJ)\getopt.obj : lib\getopt.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\getopt.h
         $(CC) $(CCOPT) lib\getopt.c

$(OBJ)\getseq.obj : lib\getseq.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\getseq.h
         $(CC) $(CCOPT) lib\getseq.c

$(OBJ)\hostable.obj : lib\hostable.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\hostable.h\
        lib\security.h
         $(CC) $(CCOPT) lib\hostable.c

$(OBJ)\hostatus.obj : lib\hostatus.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\hostable.h\
        lib\hostatus.h lib\security.h lib\timestmp.h lib\stater.h
         $(CC) $(CCOPT) lib\hostatus.c

$(OBJ)\hostrset.obj : lib\hostrset.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\hostable.h\
        lib\hostatus.h lib\hostrset.h lib\security.h lib\timestmp.h
         $(CC) $(CCOPT) lib\hostrset.c

$(OBJ)\import.obj : lib\import.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\import.h lib\arbmath.h\
        lib\hostable.h lib\usertabl.h lib\security.h lib\pnterr.h
         $(CC) $(CCOPT) lib\import.c

$(OBJ)\importng.obj : lib\importng.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\import.h\
        lib\importng.h
         $(CC) $(CCOPT) lib\importng.c

$(OBJ)\kanjicnv.obj : lib\kanjicnv.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\kanjicnv.h
         $(CC) $(CCOPT) lib\kanjicnv.c

$(OBJ)\lock.obj : lib\lock.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\dater.h lib\stater.h\
        lib\lock.h lib\import.h
         $(CC) $(CCOPT) lib\lock.c

$(OBJ)\logger.obj : lib\logger.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\dater.h lib\expath.h\
        lib\logger.h lib\timestmp.h
         $(CC) $(CCOPT) lib\logger.c

$(OBJ)\mkdir.obj : lib\mkdir.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h
         $(CC) $(CCOPT) lib\mkdir.c

$(OBJ)\mkfilenm.obj : lib\mkfilenm.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h
         $(CC) $(CCOPT) lib\mkfilenm.c

$(OBJ)\mkmbox.obj : lib\mkmbox.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h
         $(CC) $(CCOPT) lib\mkmbox.c

$(OBJ)\mktempnm.obj : lib\mktempnm.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h
         $(CC) $(CCOPT) lib\mktempnm.c

$(OBJ)\mktime2.obj : lib\mktime2.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h
         $(CC) $(CCOPT) lib\mktime2.c

$(OBJ)\ndir.obj : lib\ndir.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\uundir.h lib\getdta.h\
        lib\dos2unix.h lib\uundir.h lib\getdta.h
         $(CC) $(CCOPT) lib\ndir.c

$(OBJ)\ndirnt.obj : lib\ndirnt.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\uundir.h lib\dos2unix.h\
        lib\uundir.h
         $(CC) $(CCOPT) lib\ndirnt.c

$(OBJ)\ndiros2.obj : lib\ndiros2.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\uundir.h\
        lib\dos2unix.h lib\uundir.h
         $(CC) $(CCOPT) lib\ndiros2.c

$(OBJ)\ndirwin.obj : lib\ndirwin.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\uundir.h\
        lib\dos2unix.h lib\uundir.h
         $(CC) $(CCOPT) lib\ndirwin.c

$(OBJ)\normaliz.obj : lib\normaliz.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h
         $(CC) $(CCOPT) lib\normaliz.c

$(OBJ)\pnterr.obj : lib\pnterr.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\pnterr.h
         $(CC) $(CCOPT) lib\pnterr.c

$(OBJ)\pos2err.obj : lib\pos2err.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\pos2err.h
         $(CC) $(CCOPT) lib\pos2err.c

$(OBJ)\printerr.obj : lib\printerr.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h
         $(CC) $(CCOPT) lib\printerr.c

$(OBJ)\printmsg.obj : lib\printmsg.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\dater.h lib\logger.h
         $(CC) $(CCOPT) lib\printmsg.c

$(OBJ)\pushpop.obj : lib\pushpop.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\pushpop.h
         $(CC) $(CCOPT) lib\pushpop.c

$(OBJ)\pwinsock.obj : lib\pwinsock.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h winsock\winsock.h\
        lib\pwinsock.h
         $(CC) $(CCOPT) lib\pwinsock.c

$(OBJ)\readnext.obj : lib\readnext.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\readnext.h\
        lib\uundir.h lib\hostable.h lib\security.h
         $(CC) $(CCOPT) lib\readnext.c

$(OBJ)\rename.obj : lib\rename.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h
         $(CC) $(CCOPT) lib\rename.c

$(OBJ)\safeio.obj : lib\safeio.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\safeio.h lib\uutypes.h
         $(CC) $(CCOPT) lib\safeio.c

$(OBJ)\safeout.obj : lib\safeout.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\safeio.h lib\uutypes.h
         $(CC) $(CCOPT) lib\safeout.c

$(OBJ)\scrsize.obj : lib\scrsize.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\scrsize.h
         $(CC) $(CCOPT) lib\scrsize.c

$(OBJ)\scrsize2.obj : lib\scrsize2.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\scrsize.h
         $(CC) $(CCOPT) lib\scrsize2.c

$(OBJ)\scrsiznt.obj : lib\scrsiznt.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\scrsize.h\
        lib\pnterr.h
         $(CC) $(CCOPT) lib\scrsiznt.c

$(OBJ)\security.obj : lib\security.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\hostable.h\
        lib\security.h lib\usertabl.h lib\expath.h
         $(CC) $(CCOPT) lib\security.c

$(OBJ)\setstdin.obj : lib\setstdin.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\setstdin.h
         $(CC) $(CCOPT) lib\setstdin.c

$(OBJ)\ssleep.obj : lib\ssleep.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\ssleep.h lib\winutil.h\
        lib\pos2err.h lib\safeio.h lib\uutypes.h lib\catcher.h
         $(CC) $(CCOPT) lib\ssleep.c

$(OBJ)\stater.obj : lib\stater.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h
         $(CC) $(CCOPT) lib\stater.c

$(OBJ)\strlwr.obj : lib\strlwr.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h
         $(CC) $(CCOPT) lib\strlwr.c

$(OBJ)\strpool.obj : lib\strpool.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h
         $(CC) $(CCOPT) lib\strpool.c

$(OBJ)\timestmp.obj : lib\timestmp.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\timestmp.h\
        lib\win32ver.h lib\winutil.h lib\ssleep.h lib\title.h
         $(CC) $(CCOPT) lib\timestmp.c

$(OBJ)\title.obj : lib\title.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h
         $(CC) $(CCOPT) lib\title.c

$(OBJ)\title2.obj : lib\title2.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\timestmp.h
         $(CC) $(CCOPT) lib\title2.c

$(OBJ)\titlen.obj : lib\titlen.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h
         $(CC) $(CCOPT) lib\titlen.c

$(OBJ)\trumpet.obj : lib\trumpet.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\ssleep.h
         $(CC) $(CCOPT) lib\trumpet.c

$(OBJ)\usertabl.obj : lib\usertabl.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\expath.h\
        lib\usertabl.h lib\hostable.h lib\security.h lib\pushpop.h
         $(CC) $(CCOPT) lib\usertabl.c

$(OBJ)\usrcatch.obj : lib\usrcatch.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\timestmp.h\
        lib\usrcatch.h
         $(CC) $(CCOPT) lib\usrcatch.c

$(OBJ)\validcmd.obj : lib\validcmd.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\hostable.h\
        lib\security.h lib\usertabl.h lib\expath.h
         $(CC) $(CCOPT) lib\validcmd.c

$(OBJ)\winutil.obj : lib\winutil.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\winutil.h
         $(CC) $(CCOPT) lib\winutil.c

$(OBJ)\address.obj : mail\address.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h mail\address.h\
        lib\hostable.h lib\security.h
         $(CC) $(CCOPT) mail\address.c

$(OBJ)\alias.obj : mail\alias.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\hostable.h lib\security.h\
        lib\usertabl.h mail\alias.h mail\address.h lib\expath.h
         $(CC) $(CCOPT) mail\alias.c

$(OBJ)\deliver.obj : mail\deliver.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h mail\address.h\
        mail\deliver.h lib\expath.h lib\execute.h lib\getseq.h lib\kanjicnv.h lib\hostable.h lib\import.h lib\pushpop.h\
        lib\security.h lib\stater.h lib\usertabl.h mail\sysalias.h lib\timestmp.h lib\trumpet.h
         $(CC) $(CCOPT) mail\deliver.c

$(OBJ)\mail.obj : mail\mail.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h mail\address.h mail\alias.h\
        lib\dater.h lib\expath.h lib\getopt.h mail\mail.h mail\mailblib.h mail\maillib.h mail\mailsend.h mail\mlib.h\
        lib\pushpop.h lib\stater.h lib\timestmp.h lib\arpadate.h lib\title.h lib\winutil.h
         $(CC) $(CCOPT) mail\mail.c

$(OBJ)\mailblib.obj : mail\mailblib.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h mail\address.h\
        mail\mail.h mail\maillib.h mail\mailblib.h mail\mailsend.h mail\alias.h lib\expath.h lib\execute.h
         $(CC) $(CCOPT) mail\mailblib.c

$(OBJ)\maillib.obj : mail\maillib.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h mail\address.h\
        mail\mlib.h mail\alias.h mail\mail.h mail\maillib.h lib\scrsize.h
         $(CC) $(CCOPT) mail\maillib.c

$(OBJ)\mailsend.obj : mail\mailsend.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\arpadate.h\
        lib\expath.h lib\execute.h mail\mlib.h mail\alias.h mail\mail.h mail\maillib.h mail\mailblib.h mail\mailsend.h\
        lib\safeio.h lib\uutypes.h mail\address.h
         $(CC) $(CCOPT) mail\mailsend.c

$(OBJ)\mlib.obj : mail\mlib.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\execute.h
         $(CC) $(CCOPT) mail\mlib.c

$(OBJ)\rmail.obj : mail\rmail.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h mail\address.h lib\arpadate.h\
        mail\deliver.h lib\getopt.h lib\hostable.h lib\logger.h lib\security.h lib\usertabl.h lib\timestmp.h lib\catcher.h\
        lib\winutil.h
         $(CC) $(CCOPT) mail\rmail.c

$(OBJ)\sysalias.obj : mail\sysalias.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h mail\deliver.h\
        mail\sysalias.h
         $(CC) $(CCOPT) mail\sysalias.c

$(OBJ)\expire.obj : rnews\expire.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\active.h lib\dater.h\
        lib\getopt.h rnews\history.h lib\import.h lib\importng.h lib\logger.h lib\uundir.h lib\pushpop.h lib\stater.h\
        lib\timestmp.h
         $(CC) $(CCOPT) rnews\expire.c

$(OBJ)\genhist.obj : rnews\genhist.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\active.h lib\dater.h\
        lib\getopt.h rnews\history.h lib\import.h lib\importng.h lib\logger.h lib\uundir.h lib\pushpop.h lib\stater.h\
        lib\timestmp.h
         $(CC) $(CCOPT) rnews\genhist.c

$(OBJ)\hdbm.obj : rnews\hdbm.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h rnews\hdbm.h rnews\idx.h
         $(CC) $(CCOPT) rnews\hdbm.c

$(OBJ)\history.obj : rnews\history.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\timestmp.h\
        lib\active.h rnews\history.h lib\importng.h rnews\hdbm.h
         $(CC) $(CCOPT) rnews\history.c

$(OBJ)\idx.obj : rnews\idx.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h rnews\idx.h
         $(CC) $(CCOPT) rnews\idx.c

$(OBJ)\inews.obj : rnews\inews.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\getopt.h lib\getseq.h\
        lib\import.h lib\logger.h lib\timestmp.h lib\execute.h lib\arpadate.h lib\hostable.h
         $(CC) $(CCOPT) rnews\inews.c

$(OBJ)\rnews.obj : rnews\rnews.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\active.h lib\getopt.h\
        lib\getseq.h rnews\history.h lib\import.h lib\importng.h lib\logger.h lib\timestmp.h lib\execute.h
         $(CC) $(CCOPT) rnews\rnews.c

$(OBJ)\comm34.obj : util\comm34.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\timestmp.h
         $(CC) $(CCOPT) util\comm34.c

$(OBJ)\fmt.obj : util\fmt.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\timestmp.h
         $(CC) $(CCOPT) util\fmt.c

$(OBJ)\gensig.obj : util\gensig.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\timestmp.h
         $(CC) $(CCOPT) util\gensig.c

$(OBJ)\install.obj : util\install.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\timestmp.h
         $(CC) $(CCOPT) util\install.c

$(OBJ)\novrstrk.obj : util\novrstrk.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\timestmp.h
         $(CC) $(CCOPT) util\novrstrk.c

$(OBJ)\regsetup.obj : util\regsetup.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\getopt.h\
        lib\pushpop.h lib\timestmp.h lib\pnterr.h
         $(CC) $(CCOPT) util\regsetup.c

$(OBJ)\uupcdll.obj : util\uupcdll.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h
         $(CC) $(CCOPT) util\uupcdll.c

$(OBJ)\uupoll.obj : util\uupoll.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\getopt.h lib\timestmp.h\
        lib\ssleep.h lib\arpadate.h lib\safeio.h lib\uutypes.h lib\dater.h lib\execute.h lib\title.h
         $(CC) $(CCOPT) util\uupoll.c

$(OBJ)\catcheru.obj : uucico\catcheru.c lib\catcher.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h\
        winsock\winsock.h lib\timestmp.h lib\catcher.h lib\safeio.h lib\uutypes.h lib\pwinsock.h lib\winutil.h
         $(CC) $(CCOPT) uucico\catcheru.c

$(OBJ)\checktim.obj : uucico\checktim.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h uucico\checktim.h
         $(CC) $(CCOPT) uucico\checktim.c

$(OBJ)\commlib.obj : uucico\commlib.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h uucico\commlib.h\
        uucico\dcp.h lib\hostable.h lib\usertabl.h lib\security.h uucico\modem.h uucico\ulib.h uucico\ulibfs.h uucico\ulib14.h\
        uucico\ulibip.h uucico\ulibnmp.h
         $(CC) $(CCOPT) uucico\commlib.c

$(OBJ)\commlibn.obj : uucico\commlibn.c uucico\commlib.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h\
        uucico\commlib.h uucico\dcp.h lib\hostable.h lib\usertabl.h lib\security.h uucico\modem.h uucico\ulib.h\
        uucico\ulibfs.h uucico\ulib14.h uucico\ulibip.h uucico\ulibnmp.h
         $(CC) $(CCOPT) uucico\commlibn.c

$(OBJ)\dcp.obj : uucico\dcp.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\catcher.h uucico\checktim.h\
        uucico\dcp.h uucico\dcplib.h uucico\dcpstats.h uucico\dcpsys.h uucico\dcpxfer.h lib\expath.h lib\getopt.h\
        lib\hostable.h lib\hostatus.h lib\lock.h lib\logger.h uucico\modem.h lib\security.h lib\ssleep.h uucico\suspend.h\
        uucico\commlib.h lib\title.h lib\execute.h lib\winutil.h
         $(CC) $(CCOPT) uucico\dcp.c

$(OBJ)\dcpepkt.obj : uucico\dcpepkt.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h uucico\dcp.h\
        uucico\dcpepkt.h uucico\dcpsys.h lib\hostable.h lib\security.h lib\ssleep.h uucico\modem.h uucico\commlib.h
         $(CC) $(CCOPT) uucico\dcpepkt.c

$(OBJ)\dcpfpkt.obj : uucico\dcpfpkt.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h uucico\dcp.h\
        uucico\dcpfpkt.h uucico\dcpsys.h lib\hostable.h lib\security.h lib\ssleep.h uucico\modem.h uucico\commlib.h
         $(CC) $(CCOPT) uucico\dcpfpkt.c

$(OBJ)\dcpgpkt.obj : uucico\dcpgpkt.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h uucico\dcp.h\
        uucico\dcpsys.h uucico\dcpgpkt.h lib\hostable.h lib\security.h uucico\commlib.h uucico\modem.h lib\catcher.h
         $(CC) $(CCOPT) uucico\dcpgpkt.c

$(OBJ)\dcplib.obj : uucico\dcplib.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\arpadate.h\
        uucico\dcp.h uucico\dcplib.h uucico\dcpsys.h lib\execute.h lib\hostable.h lib\import.h uucico\modem.h lib\pushpop.h\
        lib\security.h lib\ssleep.h uucico\commlib.h lib\usertabl.h lib\timestmp.h
         $(CC) $(CCOPT) uucico\dcplib.c

$(OBJ)\dcpstats.obj : uucico\dcpstats.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h uucico\dcp.h\
        uucico\dcpstats.h lib\stater.h lib\hostable.h lib\hostatus.h lib\security.h lib\timestmp.h lib\ssleep.h lib\lock.h
         $(CC) $(CCOPT) uucico\dcpstats.c

$(OBJ)\dcpsys.obj : uucico\dcpsys.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h uucico\checktim.h\
        uucico\dcp.h uucico\dcpfpkt.h uucico\dcpgpkt.h uucico\dcptpkt.h uucico\dcpepkt.h uucico\dcplib.h uucico\dcpsys.h\
        lib\export.h lib\hostable.h lib\hostatus.h uucico\modem.h lib\lock.h uucico\nbstime.h lib\uundir.h lib\ssleep.h\
        lib\security.h uucico\commlib.h
         $(CC) $(CCOPT) uucico\dcpsys.c

$(OBJ)\dcptpkt.obj : uucico\dcptpkt.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h winsock\winsock.h\
        uucico\dcp.h uucico\dcptpkt.h uucico\dcpsys.h lib\hostable.h lib\security.h lib\ssleep.h uucico\modem.h\
        uucico\commlib.h lib\pwinsock.h
         $(CC) $(CCOPT) uucico\dcptpkt.c

$(OBJ)\dcpxfer.obj : uucico\dcpxfer.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h uucico\dcp.h\
        uucico\dcpsys.h uucico\dcpxfer.h lib\expath.h lib\hostable.h lib\import.h lib\security.h uucico\modem.h\
        uucico\commlib.h
         $(CC) $(CCOPT) uucico\dcpxfer.c

$(OBJ)\fossil.obj : uucico\fossil.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h uucico\fossil.h
         $(CC) $(CCOPT) uucico\fossil.c

$(OBJ)\modem.obj : uucico\modem.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\hostable.h\
        lib\catcher.h uucico\checktim.h uucico\commlib.h lib\dater.h uucico\dcp.h uucico\dcpsys.h uucico\modem.h\
        uucico\script.h lib\security.h lib\ssleep.h uucico\suspend.h lib\usrcatch.h lib\title.h
         $(CC) $(CCOPT) uucico\modem.c

$(OBJ)\nbstime.obj : uucico\nbstime.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\title.h\
        lib\arpadate.h uucico\dcp.h uucico\dcpsys.h lib\hostable.h uucico\nbstime.h uucico\script.h lib\security.h\
        uucico\commlib.h lib\catcher.h lib\pnterr.h
         $(CC) $(CCOPT) uucico\nbstime.c

$(OBJ)\prtynt.obj : uucico\prtynt.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\pnterr.h\
        uucico\commlib.h
         $(CC) $(CCOPT) uucico\prtynt.c

$(OBJ)\prtyos2.obj : uucico\prtyos2.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\pos2err.h
         $(CC) $(CCOPT) uucico\prtyos2.c

$(OBJ)\psos2err.obj : uucico\psos2err.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h uucico\psos2err.h
         $(CC) $(CCOPT) uucico\psos2err.c

$(OBJ)\pwserr.obj : uucico\pwserr.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h winsock\winsock.h\
        uucico\pwserr.h
         $(CC) $(CCOPT) uucico\pwserr.c

$(OBJ)\script.obj : uucico\script.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h uucico\dcp.h\
        uucico\dcpsys.h lib\hostable.h uucico\modem.h uucico\script.h lib\security.h lib\ssleep.h lib\catcher.h lib\usrcatch.h\
        uucico\commlib.h
         $(CC) $(CCOPT) uucico\script.c

$(OBJ)\suspend.obj : uucico\suspend.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\hostable.h\
        uucico\dcp.h lib\security.h uucico\suspend.h
         $(CC) $(CCOPT) uucico\suspend.c

$(OBJ)\suspend2.obj : uucico\suspend2.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\hostable.h\
        lib\security.h uucico\dcp.h uucico\dcpsys.h lib\safeio.h lib\uutypes.h uucico\modem.h lib\catcher.h lib\pos2err.h\
        uucico\suspend.h lib\ssleep.h lib\usrcatch.h
         $(CC) $(CCOPT) uucico\suspend2.c

$(OBJ)\suspendn.obj : uucico\suspendn.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\hostable.h\
        lib\security.h uucico\dcp.h uucico\dcpsys.h lib\safeio.h lib\uutypes.h uucico\modem.h lib\catcher.h lib\pnterr.h\
        uucico\suspend.h lib\ssleep.h
         $(CC) $(CCOPT) uucico\suspendn.c

$(OBJ)\ulib.obj : uucico\ulib.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h uucico\ulib.h uucico\comm.h\
        lib\ssleep.h lib\catcher.h uucico\commlib.h
         $(CC) $(CCOPT) uucico\ulib.c

$(OBJ)\ulib14.obj : uucico\ulib14.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h uucico\ulib14.h\
        uucico\comm.h lib\ssleep.h lib\catcher.h uucico\fossil.h uucico\commlib.h lib\hostable.h lib\usertabl.h lib\security.h
         $(CC) $(CCOPT) uucico\ulib14.c

$(OBJ)\ulibfs.obj : uucico\ulibfs.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h uucico\ulibfs.h\
        uucico\commlib.h uucico\fossil.h lib\catcher.h lib\ssleep.h
         $(CC) $(CCOPT) uucico\ulibfs.c

$(OBJ)\ulibip.obj : uucico\ulibip.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h uucico\psos2err.h\
        lib\catcher.h winsock\winsock.h uucico\ulibip.h lib\catcher.h uucico\pwserr.h lib\pnterr.h lib\pwinsock.h\
        uucico\commlib.h
         $(CC) $(CCOPT) uucico\ulibip.c

$(OBJ)\ulibnb.obj : uucico\ulibnb.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h ulibnb.h uucico\comm.h\
        lib\ssleep.h lib\catcher.h uucico\commlib.h
         $(CC) $(CCOPT) uucico\ulibnb.c

$(OBJ)\ulibnmp.obj : uucico\ulibnmp.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h uucico\ulibnmp.h\
        lib\ssleep.h lib\catcher.h uucico\commlib.h lib\pos2err.h
         $(CC) $(CCOPT) uucico\ulibnmp.c

$(OBJ)\ulibnt.obj : uucico\ulibnt.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h uucico\ulib.h\
        lib\ssleep.h lib\catcher.h uucico\dcp.h uucico\commlib.h lib\pnterr.h uucico\suspend.h
         $(CC) $(CCOPT) uucico\ulibnt.c

$(OBJ)\ulibos2.obj : uucico\ulibos2.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h uucico\ulib.h\
        lib\ssleep.h lib\catcher.h lib\pos2err.h uucico\commlib.h lib\usrcatch.h
         $(CC) $(CCOPT) uucico\ulibos2.c

$(OBJ)\ulibwin.obj : uucico\ulibwin.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h uucico\ulib.h\
        lib\ssleep.h uucico\commlib.h
         $(CC) $(CCOPT) uucico\ulibwin.c

$(OBJ)\uucico.obj : uucico\uucico.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h uucico\dcp.h\
        lib\hostable.h lib\security.h lib\pushpop.h lib\timestmp.h lib\catcher.h
         $(CC) $(CCOPT) uucico\uucico.c

$(OBJ)\uuport.obj : uucico\uuport.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\timestmp.h\
        uucico\suspend.h lib\winutil.h lib\logger.h
         $(CC) $(CCOPT) uucico\uuport.c

$(OBJ)\uucp.obj : uucp\uucp.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\expath.h lib\getopt.h\
        lib\getseq.h lib\hostable.h lib\import.h lib\uundir.h lib\security.h lib\timestmp.h lib\execute.h lib\winutil.h\
        lib\logger.h
         $(CC) $(CCOPT) uucp\uucp.c

$(OBJ)\uuname.obj : uucp\uuname.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\getopt.h lib\hostable.h\
        lib\security.h lib\timestmp.h
         $(CC) $(CCOPT) uucp\uuname.c

$(OBJ)\uustat.obj : uucp\uustat.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\dater.h lib\export.h\
        lib\getopt.h lib\getseq.h lib\hostable.h lib\hostatus.h lib\import.h lib\pushpop.h lib\readnext.h lib\security.h\
        lib\stater.h lib\timestmp.h
         $(CC) $(CCOPT) uucp\uustat.c

$(OBJ)\uusub.obj : uucp\uusub.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\hostable.h lib\dater.h\
        lib\hostrset.h lib\hostatus.h lib\getopt.h lib\security.h lib\timestmp.h
         $(CC) $(CCOPT) uucp\uusub.c

$(OBJ)\uux.obj : uucp\uux.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\getopt.h lib\getseq.h\
        lib\expath.h lib\import.h lib\pushpop.h lib\hostable.h lib\security.h lib\timestmp.h lib\winutil.h lib\logger.h
         $(CC) $(CCOPT) uucp\uux.c

$(OBJ)\uuxqt.obj : uucp\uuxqt.c lib\uupcmoah.h lib\uutypes.h lib\confvars.h lib\lib.h lib\hlib.h lib\dater.h lib\execute.h\
        lib\expath.h lib\getopt.h lib\getseq.h lib\hostable.h lib\import.h lib\lock.h lib\logger.h lib\pushpop.h\
        lib\readnext.h lib\security.h lib\timestmp.h lib\usertabl.h lib\uundir.h lib\winutil.h
         $(CC) $(CCOPT) uucp\uuxqt.c

