#ifndef FOSSIL_H
#define FOSSIL_H

/*--------------------------------------------------------------------*/
/*       f o s s i l . h                                              */
/*                                                                    */
/*       UUPC/extended defines for accessing FOSSIL functions via     */
/*       INT 14 under MS-DOS.                                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1990-1994 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: FOSSIL.H 1.4 1993/11/06 17:57:46 rhg Exp $
 *
 *    Revision history:
 *    $Log: FOSSIL.H $
 * Revision 1.4  1993/11/06  17:57:46  rhg
 * Drive Drew nuts by submitting cosmetic changes mixed in with bug fixes
 *
 * Revision 1.4  1993/11/06  17:57:46  rhg
 * Drive Drew nuts by submitting cosmetic changes mixed in with bug fixes
 *
 * Revision 1.3  1993/10/12  01:35:12  ahd
 * Normalize comments to PL/I style
 *
 * Revision 1.2  1993/05/30  15:27:22  ahd
 * Correct CTS high macro
 *
 * Revision 1.1  1993/05/30  00:16:35  ahd
 * Initial revision
 *
 */

/*--------------------------------------------------------------------*/
/*       FOSSIL function calls, taken from "Fundamentals of FOSSIL    */
/*       Implementation and Use:  Version 5, February 11, 1988 by     */
/*       Rick Moore of Solar Wind Computing.                          */
/*--------------------------------------------------------------------*/

#define FS_INTERRUPT 0x14        /* Fossil uses INT 14                 */

/*--------------------------------------------------------------------*/
/*       Function calls numbers, placed in AH before interrupt        */
/*--------------------------------------------------------------------*/

#define FS_SPEED     0x00        /* Set speed, parity, stop bits, char len  */
#define FS_XMIT1     0x01        /* Transmit one character (blocking write)  */
#define FS_RECV1     0x02        /* Receive one character (blocking read)  */
#define FS_STATPORT  0x03        /* Return port status                 */
#define FS_OPENPORT  0x04        /* Initialize port                    */
#define FS_CLOSPORT  0x05        /* Close port                        */
#define FS_DTR       0x06        /* Raise/lower DTR                    */
#define FS_TIMESTAT  0x07        /* Return system timer parameters     */
#define FS_XMITWAIT  0x08        /* Wait for output queue to transmit  */
#define FS_XMITPURG  0X09        /* Purge output queue w/o transmit   */
#define FS_RECVPURG  0x0a        /* Purge unread input queue           */
#define FS_XMIT1NOW  0x0b        /* Transmit one character (nonblocking)  */
#define FS_PEEK1     0x0c        /* Non-destructive port read         */
#define FS_KEYBPEEK  0x0d        /* Non-destructive keyboard read     */
#define FS_KEYBREAD  0x0e        /* Destructive keyboard read (blocking)  */
#define FS_FLOWCNTL  0x0f        /* Enable/disable flow control        */
#define FS_CNTLCHEK  0x10        /* Enable/disable Cntl-C/Cntl-K checking  */
#define FS_SCURSOR   0x11        /* Set screeen cursor location        */
#define FS_RCURSOR   0x12        /* Read screen cursor location        */
#define FS_ANSICHAR  0x13        /* Write character with ANSI control  */
#define FS_WATCHDOG  0x14        /* Auto-reboot on lost carrier        */
#define FS_PUTCHAR   0x15        /* BIOS level screen put              */
#define FS_TIMECHN   0x16        /* Insert/delete items on timer chain  */
#define FS_REBOOT    0x17        /* Perform Vulcan nerve pinch on system  */
#define FS_READBLOK  0x18        /* Read block of data from port      */
#define FS_WRITBLOK  0x19        /* Write block of data to port        */
#define FS_BREAK     0x1A        /* Enable/disable break on port      */
#define FS_DRIVINFO  0x1B        /* Get driver information            */

typedef struct _FS_INFO {        /* Data returned by FS_DRIVINFO      */
   short size;                   /* Data bytes returned               */
   char  version;                /* FOSSIL specification version used  */
   char  revision;               /* Driver revision level              */
   char  UUFAR *id;              /* Pointer to ASCII driver id        */
   short inputSize;              /* Input buffer size                 */
   short inputFree;              /* Bytes free in input queue         */
   short outputSize;             /* Bytes queued for output;           */
   short outputFree;             /* Bytes free in output queue        */
   char  width;                  /* Screen width in characters        */
   char  height;                 /* Screen height in characters        */
   char  baudmask;               /* Baud rate in format used by FS_SPEED  */
} FS_INFO;

/*--------------------------------------------------------------------*/
/*              Define macros to perform basic functions              */
/*--------------------------------------------------------------------*/

#define FSSetSpeed( speed, parity, stopBits, charLen ) \
         FossilCntl(FS_SPEED, (unsigned char) \
                    ((speed << 5) | (parity << 3) | \
                    (stopBits << 2) | charLen))

#define FS_NO_PARITY    0
#define FS_NO_PARITYX   1
#define FS_ODD_PARITY   2
#define FS_EVEN_PARITY  3

#define FS_STOPBIT_1    0
#define FS_STOPBIT_15   1           /* For five bit character only     */
#define FS_STOPBIT_2    1           /* For all but five bit characters  */

#define FS_CHARLEN_5    0
#define FS_CHARLEN_6    1
#define FS_CHARLEN_7    2
#define FS_CHARLEN_8    3

#define FSOpen( )           FossilCntl(FS_OPENPORT, 0)

#define FS_COOKIE    0x1954      /* Success result from FS_OPENPORT   */

#define FSClose( )          FossilCntl(FS_CLOSPORT, 0)

#define FSStatus( )         FossilCntl(FS_STATPORT, 0)

#define FS_STAT_OUTPEMPT   0x4000   /* Output buffer empty            */
#define FS_STAT_OUTPROOM   0x2000   /* Output buffer not full         */
#define FS_STAT_OVERRUN    0x0200   /* Input buffer overrun           */
#define FS_STAT_INQUEUED   0x0100   /* Characters in input buffer     */
#define FS_STAT_DCD        0x0080   /* Data carrier detected           */

#define FSFlushXmit( )      FossilCntl(FS_XMITPURG, 0)

#define FSFlushRecv( )      FossilCntl(FS_RECVPURG, 0)

#define FSFlowControl(type) FossilCntl(FS_FLOWCNTL, (unsigned char) (0xf0 | (type)))

#define FS_NOFLOW   0x00
#define FS_XONXMIT  0x01
#define FS_CTSRTS   0x04
#define FS_XONRECV  0x08

#define FSDTR( onoroff )    FossilCntl(FS_DTR, onoroff)

#define FSBreak( onoroff )  FossilCntl(FS_BREAK, onoroff)

/*--------------------------------------------------------------------*/
/*        Define information for routines and data in fossil.c        */
/*--------------------------------------------------------------------*/

extern short portNum;        /* Must be set by openline()              */

short FossilCntl( const char function, const unsigned char info );

#endif
