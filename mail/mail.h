#ifndef __MAIL
#define __MAIL

/*--------------------------------------------------------------------*/
/*       m a i l . h                                                  */
/*                                                                    */
/*       Common defines and data structures for UUPC/extended         */
/*       Mail User Agent                                              */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-2001 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: mail.h 1.15 2000/05/12 12:42:31 ahd v1-13g $
 *
 *    Revision history:
 *    $Log: mail.h $
 *    Revision 1.15  2000/05/12 12:42:31  ahd
 *    Annual copyright update
 *
 *    Revision 1.14  1999/01/04 03:56:09  ahd
 *    Annual copyright change
 *
 *    Revision 1.13  1998/03/03 03:55:05  ahd
 *    Routines to handle messages within a POP3 mailbox
 *
 *    Revision 1.12  1998/03/01 01:31:05  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.11  1997/11/21 18:16:32  ahd
 *    Command processing stub SMTP daemon
 *
 *    Revision 1.10  1997/04/24 01:10:45  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.9  1996/11/19 00:25:20  ahd
 *    Revise LDESC declare
 *
 *    Revision 1.8  1996/01/01 21:04:30  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.7  1994/12/22 00:20:16  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.6  1994/03/05 21:15:49  ahd
 *    Revamp header copyrights, line buffer lengths
 *
 */

#define  MISSING  -1L      /* Line not found in this mail header   ahd */
#define  UIDL_HEADER       "X-UIDL:"

#ifdef BIT32ENV
#define LSIZE  1024
#else
#define LSIZE  256
#endif

#define MAXADDRS 300

/*--------------------------------------------------------------------*/
/*    Structure for tracking mail in the mailbox.                     */
/*--------------------------------------------------------------------*/

typedef enum {
      M_UNREAD,
      M_READ,
      M_SAVED,
      M_FORWARDED,
      M_ANSWERED,
      M_DELETED }
      MSTATUS;

typedef struct _LDESC
{
   MSTATUS status;         /* status of this message  */
   long adr;               /* address of From line    */
   long date;              /* address of Date: line   */
   long subject;           /* address of Subject: line   */
   long from;              /* address of From: line   */
   long replyto;           /* address of Reply-To: line   */
   long lines;             /* number of lines         */
} LDESC;

/*--------------------------------------------------------------------*/
/*                        Enumerated Verb list                        */
/*--------------------------------------------------------------------*/

typedef enum {  M_ALIAS,
                M_COPY,
                M_DEBUG,
                M_DELETE,
                M_DELETEQ,
                M_DOWN,
                M_EMPTY,
                M_EXIT,
                M_EXTPRINT,
                M_EXTTYPE,
                M_FORWARD,
                M_FASTHELP,
                M_GOTO,
                M_HEADERS,
                M_HELP,
                M_INTPRINT,
                M_INTTYPE,
                M_INVALID,
                M_MAIL,
                M_NOOP,
                M_QUIT,
                M_REPLY,
                M_SAVE,
                M_SET,
                M_STATUS,
                M_SYSTEM,
                M_UNDELETE,
                M_UP,
                M_WRITE
                } ACTION;

/*--------------------------------------------------------------------*/
/*                        Bit flags for verbs                         */
/*--------------------------------------------------------------------*/

#define AUTOPRINT     0x0001
#define FILE_OP       0x0002
#define KEWSHORT_OP   0x0004
#define LETTER_OP     0x0008
#define NO_OPERANDS   0x0010
#define NOPRINT       0x0020
#define POSITION      0x0040
#define START_TOP     0x0080
#define STRING_OP     0x0100
#define TOKEN_OP      0x0200
#define USER_OP       0x0400
#define NODISPLAY     0x0800  /* Do not display during help */
#define NOAUTOHEADER  0x0100  /* Do not issue header command after cmd */

/*--------------------------------------------------------------------*/
/*               Options for copying mail between files               */
/*--------------------------------------------------------------------*/

typedef enum { noheader,         /* Don't copy RFC-822 header or sep lines */
               fromheader,       /* Generate one line "From" description  */
               ignoresome,       /* Copy header, but not ignorelist       */
               autoresent,       /* Prefix selected Resent- headers w/-X  */
               nocontinue,       /* Do not copy next line if contination  */
               noseperator,      /* Copy all but binary seperator lines   */
               seperators        /* Copy everything!                */
             } copyopt;

#endif
