/*--------------------------------------------------------------------*/
/*       u s r c a t c h e r . c                                      */
/*                                                                    */
/*       Ctrl-Break handler for UUPC/extended                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1993 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: catcher.c 1.2 1993/09/20 04:38:11 ahd Exp $
 *
 *    Revision history:
 *    $Log: catcher.c $
 */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <signal.h>
#include <process.h>
#include <stdlib.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "timestmp.h"
#include "usrcatch.h"

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

int raised = 0;

/*--------------------------------------------------------------------*/
/*    u s r 1 h a n d l e r                                           */
/*                                                                    */
/*    Handles SIGUSR interrupt; from MicroSoft Programmer's           */
/*    Workbench QuickHelp samples                                     */
/*--------------------------------------------------------------------*/

void
#ifdef __TURBOC__
__cdecl
#endif
usrhandler( int sig )
{
   raised = sig;

/*--------------------------------------------------------------------*/
/*    The handler pointer must be reset to our handler since by       */
/*    default it is reset to the system handler.                      */
/*--------------------------------------------------------------------*/

#ifdef __OS2__
      signal( sig , (void (__cdecl *)(int))usrhandler );
#else
      signal( sig , usrhandler );
#endif

} /* usrhandler */
