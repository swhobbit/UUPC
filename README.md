## INTRODUCTION

  This is the READ ME file for UUPC/extended 1.13k by Kendra Electronic
  Wonderworks, and was last edited (only on GitHub) 
  on 24 June 2018 for the release made on January 7, 2002.  This same file
  is included in all the UUPC/extended archives.

## DOWNLOADING BINARIES

Our web site for binary downloads of the current relase can be found at http://www.kew.com/pub/uupc/1.13k/

## BOILERPLATE

  UUPC/extended is a UUCP clone for DOS, OS/2, and Windows 95/NT/2000/
  XP.  The package includes MAIL and UUCP interfaces, a news backend
  (delivery program) and associated utilities.  UUPC/extended does not
  currently include a news reader.

  The software and associated documentation are copyrighted; individuals
  or corporations may use UUPC/extended on as many systems as they
  desire for as long as they want without paying a fee, but it is not
  public domain software.  Restrictions exist on how UUPC/extended may
  be copied or modified, primarily to insure the source remains
  available and that others are not charged for the software except for
  reasonable copying costs.  See LICENSE.TXT in this archive for the
  details.

## ABOUT THE RELEASE

  There was no release 1.13i.

  UUPC/extended 1.13k is the latest major release for SMTP/POP3 support
  for the 32 bit environments Windows NT and OS/2.  This support allows
  full access for POP/SMTP clients such as Netscape Communicator and
  Microsoft Outlook Express not directly connected to the Internet.

  UUPC/extended 1.13k corrects a defect long lines crashed RMAIL when
  acting as an SMTP client.

## ABOUT DOCUMENTATION

  The ASCII documents are in the file UPC???AD.ZIP file, assuming the
  current release is 1.???.  That is, the second three characters of the
  archive name are the last three digits of the release number.

  (The documents may be older than the release.  For example. the
  documentation archive UPC12BAD.ZIP shared space with releases as late
  as 1.12n).  The Word for Windows archive (UPC???AP.ZIP) may be newer
  than the base documentation archive, in which case it may have
  information on newer features.  If you do not have Word for Windows
  but do have Windows, you can download a free viewer from
  www.microsoft.com or via our listserv.

  The ASCII documents include overstrikes for printing on a printer.  To
  read the document on-line, extract the program NOVRSTRK from the
  UUPC/extended optional programs archive and issue the following
  command:

        NOVRSTRK filename.PRN filename.TXT

  Where filename is the name of the document you want to read.  You can
  then browse the terminal ready version of the document, UUPCDOCS.TXT.
  However, given that these documents are over 200 pages long, you'll
  want to either print it or have a good search function handy.

  READ THE DOCUMENTATION, including our copyright and license agreement,
  and the installation section on "Who should use UUPC/extended".  The
  same license is included in all archives, just like this document.

## NOTES FOR WINDOWS 3.1 USERS

  We have dropped support for Windows 3.1.  Please fall back to DOS if
  you are unable to move up to Windows 95 or above.

## NOTES

### NOTES FOR WINDOWS 9x USERS

  Use the Windows NT archives under Windows 95/98.  We actually tested
  this, honest.

  One known bug under Windows 95 is that the 'abort, send?' prompt in
  mail gets the response character listed as the first character on the
  following input line as well.  We're looking into it.

### NOTES FOR OS/2 USERS.

  The OS/2 32-bit executables are now stable, having been built with the
  IBM C++ compiler rather than Borland C++ for OS/2.  We no longer ship
  the 16 bit OS/2 archives.

  We now include a DLL for the OS/2 version, this must be placed in a
  directory in your LIBPATH.  The easiest thing may be to simply add the
  OS/2 UUPC/extended binaries directory to your LIBPATH.

  NOTE:  We have dropped the UUCICON.EXE/RMAILN.EXE modules which
         did not require TCP/IP.  You must install OS/2 dial up or LAN
         TCP/IP support.

### WHAT TO DO NEXT

  If you don't have a copy of HOWTOGET.TXT, either on its own or as a
  part of the main reference guide in the UPC???AD.ZIP, get a copy of it
  to determine what files you need for your environment, then read the
  documentation to learn how to install UUPC/extended.

  Once you are up and running, the documentation also includes
  instructions on how to stay current via the UUPC-Announce, UUPC-Info
  and the new UUPC-Info-digest mailing lists, and interesting stuff.
  Enjoy.

-ahd-

Internet:       uupc-help@kew.com
