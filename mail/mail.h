#ifndef __MAIL
#define __MAIL

/*--------------------------------------------------------------------*/
/*    Common defines and data structures for UUPC/extended            */
/*                                                                    */
/*    Copyright (c) 1989, 1991 Andrew H. Derbyshire                   */
/*--------------------------------------------------------------------*/

#define  MISSING  -1L      /* Line not found in this mail header   ahd     */

#define LSIZE  256

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

static struct ldesc {
   MSTATUS status;         /* status of this message  */
   long adr;               /* address of From line    */
   long date;              /* address of Date: line   */
   long subject;           /* address of Subject: line   */
   long from;              /* address of From: line   */
   long replyto;           /* address of Reply-To: line   */
   long lines;             /* number of lines         */
};

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

/*--------------------------------------------------------------------*/
/*               Options for copying mail between files               */
/*--------------------------------------------------------------------*/

typedef enum { noheader,         /* Don't copy RFC-822 header or sep lines */
               fromheader,       /* Generate one line "From" description   */
               noreceived,       /* Copy header, but not ignorelist        */
               nocontinue,       /* Do not copy next line if contination   */
               noseperator,      /* Copy all but binary seperator lines    */
               seperators } copyopt;   /* Copy everything!                 */

#endif
