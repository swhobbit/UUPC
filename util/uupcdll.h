#ifndef UUPCDLL_H
#define UUPCDLL_H
/*--------------------------------------------------------------------*/
/*    u u p c d l l . c                                               */
/*                                                                    */
/*    A DLL for calling UUPC functions externally under Windows NT    */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Copyright (c) David M. Watt 1994, All Right Reserved            */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1994 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *       $Id: uupcdll.h 1.1 1994/05/23 22:49:49 ahd Exp $
 *       $Log: uupcdll.h $
 * Revision 1.1  1994/05/23  22:49:49  ahd
 * Initial revision
 *
 *
 */



BOOL UUPCInit(void);
/*
        UUPCInit() initializes the UUPC DLL.  (At the moment, it does
        nothing.)
*/

BOOL UUPCGetParm(char *parmName, char *buf, int len);
/*
        UUPCGetParm() gets the UUPC system parameter named parmName out
        of the registry.  It only works if you've run REGSETUP -s on a
        correctly configured UUPC/extended system first.
*/

BOOL UUPCGetNewsSpoolSize(char *system, long *count, long *bytes);
/*
        Returns the total number of files and their total size in the 'D'
        directory under SpoolDir for the given system.
*/

BOOL UUPCSendMail(char *message);
/*
        Sends a message to a user using UUPC's RMAIL program.  The message
        must have at least a To: line and a From: line, followed by two
        newlines, in order for UUPCSendMail to consider it a valid message.
        It does <no> further checking for the validity of the message itself,
        but will return an error if RMAIL had a problem delivering it for
        some reason.
*/

#endif