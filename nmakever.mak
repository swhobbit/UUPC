# *--------------------------------------------------------------------*
# *      n m a k e v e r . m a k                                       *
# *                                                                    *
# *      Version information for UUPC/extended builds                  *
# *--------------------------------------------------------------------*

# *--------------------------------------------------------------------*
# *      Changes Copyright (c) 1989-1998 by Kendra Electronic          *
# *      Wonderworks.                                                  *
# *                                                                    *
# *      All rights reserved except those explicitly granted by        *
# *      the UUPC/extended license agreement.                          *
# *--------------------------------------------------------------------*

# *--------------------------------------------------------------------*
# *                         RCS Information                            *
# *--------------------------------------------------------------------*

#        $Id: nmakever.mak 1.5 1998/11/07 03:42:11 ahd v1-13e $
#
#        $Log: nmakever.mak $
#        Revision 1.5  1998/11/07 03:42:11  ahd
#        Up version
#
#        Revision 1.4  1998/08/29 14:16:41  ahd
#        Version 1.13d begins
#
#        Revision 1.3  1998/07/27 01:03:15  ahd
#        Go to version 1.13c
#
# Revision 1.2  1998/04/29  03:48:53  ahd
# Version 1.13b
#
#        Revision 1.1  1998/04/19 15:28:07  ahd
#        Initial revision
#

# *--------------------------------------------------------------------*
# *   Our release number.  This is updated each time we ship it        *
# *   out to some unsuspecting user, or sometimes when we do a         *
# *   major fix even if we don't ship it out to the lusers.  ANY       *
# *   VERSION NOT DONE ON kendra SHOULD BE TAGGED AS N.NNAm, WHERE     *
# *   N.NNA IS THE NUMBER UUPC was shipped as, and 'm' any string      *
# *   to indentify your special version.  Just upping the              *
# *   distributed version number will confuse you AND me.              *
# *--------------------------------------------------------------------*

!ifndef VERS
VERS = 1.13f
!endif
