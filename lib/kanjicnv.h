/*
 * kanjicnv.h
 * Japanese conversion routines for UUPC/Extended
 *
 * Based on Oimo source file stdfunc.c
 * Programmed by Shigeki Matsushima and Dai Yokota 4-MAY-1989
 * Modified by Kenji Rikitake 13-AUG-1991
 * Copyright (c) 1988, 1991 by Shigeki Matsushima, Dai Yokota, and
 * Kenji Rikitake. All rights reserved.
 */

int fputs_shiftjis(unsigned char *buf, FILE *fp);

int fputs_jis7bit(unsigned char *buf, FILE *fp);
