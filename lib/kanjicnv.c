/*
 * kanjicnv.c
 * Japanese conversion routines for UUPC/Extended
 *
 * Based on Oimo source file stdfunc.c
 * Programmed by Shigeki Matsushima and Dai Yokota 4-MAY-1989
 * Modified by Kenji Rikitake 13-AUG-1991
 * Copyright (c) 1988, 1991 by Shigeki Matsushima, Dai Yokota, and
 * Kenji Rikitake. All rights reserved.
 *
 */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <time.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "kanjicnv.h"

/*
 * On Japanese support:
 *
 * Japanese MS-DOS uses a 2byte Kanji (Japanese ideogram) code called
 * "Shift-JIS". This cannot be delivered via SMTP since Shift-JIS maps
 * its first byte from 0x80-0x9f and 0xe0-0xfc.
 * JUNET requests all hosts to send Kanji in a 7bit subset of ISO2022.
 * This is commonly called "JIS 7bit".
 *
 * To provide Japanese functionality, you need to convert
 * all remote delivery messages to JIS 7bit, and
 * all local delivery messages to Shift-JIS.
 */

/*--------------------------------------------------------------------*/
/*                           Global defines                           */
/*--------------------------------------------------------------------*/

#define SEQ_TO_JIS90 "\033$B"
#define SEQ_TO_ASCII "\033(B"

/*--------------------------------------------------------------------*/
/*                               Macros                               */
/*--------------------------------------------------------------------*/

/* check if c is the first byte of Shift-JIS Kanji */

#define  iskanji(c)  ((unsigned char)(c) >= 0x81 && \
          (unsigned char)(c) <= 0x9f || \
          (unsigned char)(c) >= 0xe0 && \
          (unsigned char)(c) <= 0xfc)

/* check if c is the second byte of Shift-JIS Kanji */

#define  iskanji2(c) ((unsigned char)(c) >= 0x40 && \
          (unsigned char)(c) <= 0x7e || \
          (unsigned char)(c) >= 0x80 && \
          (unsigned char)(c) <= 0xfc)

/*--------------------------------------------------------------------*/
/*    f p u t s _ s h i f t j i s                                     */
/*                                                                    */
/*    fputs() with conversion from JIS 7bit to Shift-JIS              */
/*--------------------------------------------------------------------*/

int fputs_shiftjis(unsigned char *buf, FILE *fp)
{
   int shiftin = FALSE;
   unsigned char hi, lo;

   while (*buf) {
      if ((*buf == '\033') && (*(buf+1) == '$') &&
         ((*(buf+2) == 'B') || (*(buf+2) == '@'))) {
         shiftin = TRUE;
         buf += 3;
         }
      else if ((*buf == '\033') && (*(buf+1) == '(')
          && ((*(buf+2) == 'J') || (*(buf+2) == 'B') ||
              (*(buf+2) == 'H'))) {
         shiftin = FALSE;
         buf += 3;
         }
      else if (shiftin) {
         hi = *buf++;
         if ((lo = *buf++) == '\0')
            break;
         if (hi & 1) lo += 0x1f;
         else lo += 0x7d;
         if (lo >= 0x7f) lo++;
         hi = (hi - 0x21 >> 1) + 0x81;
         if (hi > 0x9f) hi += 0x40;
         if (EOF == fputc(hi, fp)) {
            return EOF;
            }
         if (EOF == fputc(lo, fp)) {
            return EOF;
            }
         }
      else {
         if (EOF == fputc(*buf, fp)) {
            return EOF;
            }
         buf++;
         }
      }
   return 0;
} /* fputs_shiftjis */

/*--------------------------------------------------------------------*/
/*    fputs_jis7bit                                                   */
/*                                                                    */
/*    fputs() with conversion from Shift-JIS to JIS 7bit              */
/*--------------------------------------------------------------------*/

int fputs_jis7bit(unsigned char *buf, FILE *fp)
{
   int kanjiflag = FALSE;
   unsigned char hi, lo;
   int i;

   while (*buf) {
      if (iskanji(*buf) && iskanji2(*(buf+1))) {
         if (kanjiflag == FALSE) {
            kanjiflag = TRUE;
            if (0 != (i = fputs(SEQ_TO_JIS90, fp))) {
               return i;
               }
            }
         hi = *buf++;
         if ((lo = *buf++) == '\0')
            break;
         hi -= (hi <= 0x9f) ? 0x71 : 0xb1;
         hi = hi * 2 + 1;
         if (lo > 0x7f) lo -= 1;
         if (lo >= 0x9e) {
            lo -= 0x7d;
            hi += 1;
            }
         else {
            lo -= 0x1f;
            }
         if (EOF == fputc(hi, fp)) {
            return EOF;
            }
         if (EOF == fputc(lo, fp)) {
            return EOF;
            }
         }
      else {
         if (kanjiflag == TRUE) {
            kanjiflag = FALSE;
            if (0 != (i = fputs(SEQ_TO_ASCII, fp))) {
               return i;
               }
            }
         if (EOF == fputc(*buf, fp)) {
            return EOF;
            }
         buf++;
         }
      }
   if (kanjiflag) {
      if (0 != (i = fputs(SEQ_TO_ASCII, fp))) {
         return i;
         }
      }
   return 0;
} /* fputs_jis7bit */
