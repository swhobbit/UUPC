@echo off
start /min /n uucico -h %1 -m tcpip -U
rem      $Id: uucpd.cmd 1.3 1994/05/09 02:13:02 ahd v1-13f $
rem
rem      $Log: uucpd.cmd $
rem      Revision 1.3  1994/05/09 02:13:02  ahd
rem      Don't display started command
rem
rem     Revision 1.2  1994/04/27  00:05:45  ahd
rem     Automatically run UUXQT after completion
rem
rem     Revision 1.1  1994/04/26  03:32:25  ahd
rem     Initial revision
