/*--------------------------------------------------------------------*/
/*    d e l i v e r . h                                               */
/*                                                                    */
/*   Externally known functions in deliver.c, delivery subroutines    */
/*                in rmail component of UUPC/extended                 */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1997 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: deliver.h 1.19 1997/04/24 01:10:40 ahd Exp $
 *
 *    $Log: deliver.h $
 *    Revision 1.19  1997/04/24 01:10:40  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.18  1996/11/19 00:25:20  ahd
 *    Externalize functions
 *
 *    Revision 1.17  1996/01/01 21:04:46  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.16  1995/09/11 00:24:49  ahd
 *    Delete non-existent variable
 *
 *    Revision 1.15  1995/01/09 01:43:38  ahd
 *    Optimize UUCP remote queuing
 *
 *    Revision 1.14  1995/01/08 19:54:01  ahd
 *    Add in-memory files to RMAIL
 *
 *    Revision 1.13  1995/01/07 16:19:37  ahd
 *    Change KWBoolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.12  1994/12/22 00:20:10  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.11  1994/12/09 03:54:15  ahd
 *    Support recursely processing system aliases file for same address
 *
 * Revision 1.10  1994/01/24  03:17:50  ahd
 * Annual Copyright Update
 *
 * Revision 1.9  1994/01/01  19:13:31  ahd
 * Annual Copyright Update
 *
 * Revision 1.8  1993/12/07  05:02:09  ahd
 * rename fromUser and fromNode
 *
 * Revision 1.7  1993/11/13  17:52:31  ahd
 * Add call grading support
 *
 * Revision 1.6  1993/10/12  01:35:12  ahd
 * Normalize comments to PL/I style
 *
 * Revision 1.5  1993/09/20  04:51:31  ahd
 * TCP Support from Dave Watt
 * 't' protocol support
 * OS/2 2.x support (BC++ 1.0 for OS/2 support)
 *
 * Revision 1.4  1993/06/13  14:12:29  ahd
 * Save invoked program name and use it for recursive calls
 *
 * Revision 1.3  1993/04/11  00:36:13  ahd
 * Global edits for year, TEXT, etc.
 *
 * Revision 1.2  1992/12/04  01:03:49  ahd
 * Add system alias support
 *
 */

#define POSTMASTER "postmaster"

size_t Deliver( IMFILE *imf,           /* Input file name            */
                char *address,         /* Target address             */
                KWBoolean validate);   /* Validate/forward local mail*/

size_t DeliverRemote( IMFILE *imf,        /* Input file name          */
                    const char *address,  /* Target address           */
                    const char *path);

#ifdef TCPIP
size_t DeliverSMTP( IMFILE *imf,          /* Input file name          */
                    const char *address,  /* Target address           */
                    const char *path);
#endif

size_t Bounce( IMFILE *imf,
               const char *text,
               const char *data,
               const char *address,
               const KWBoolean validate );

extern KEWSHORT hops;

extern KWBoolean remoteMail;

extern char fromUser[];
extern char fromNode[];

extern char *ruser;
extern char *rnode;
extern char *uuser;
extern char grade;

extern char *myProgramName;
