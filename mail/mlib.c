/*--------------------------------------------------------------------*/
/*       m l i b . c                                                  */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1994 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: mlib.c 1.9 1994/01/01 19:13:04 ahd Exp $
 *
 *    Revision history:
 *    $Log: mlib.c $
 * Revision 1.9  1994/01/01  19:13:04  ahd
 * Annual Copyright Update
 *
 * Revision 1.8  1993/12/23  03:16:03  rommel
 * OS/2 32 bit support for additional compilers
 *
 * Revision 1.7  1993/11/13  17:43:26  ahd
 * Normalize external command processing
 *
 * Revision 1.6  1993/09/20  04:39:51  ahd
 * OS/2 2.x support
 *
 * Revision 1.5  1993/08/11  02:31:12  ahd
 * Use standard denormalize() macro for slash --> back slashes
 *
 * Revision 1.4  1993/08/03  03:11:49  ahd
 * Further Windows 3.x fixes
 *
 * Revision 1.3  1993/07/31  16:26:01  ahd
 * Changes in support of Robert Denny's Windows support
 *
 *
 * 13 May 89      Use PC format path names for editor
 * 01 Oct 89      Make Console_fgets use far pointers
 *                Alter Console_fgets and Is_Console to type boolean
 *
 * 29 Jul 90      Use PC format path names for pager
 */

/*
   ibmpc/mlib.c   by <skl@van-bc.UUCP>   August/87

   Mailer UA system-dependent library

   Services to provide in mlib.c:

   Get a single character from the console.
   Invoke the local editor on a given file.
   Determine if a given file stream points to the console.
   Get a line from the console.
   Invoke a local pager on a given file.

*/

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"

#include <conio.h>
#include <io.h>

#ifndef __32BIT__
#include <dos.h>
#endif

#if defined(WIN32)
#include <windows.h>
#endif

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "execute.h"

/*--------------------------------------------------------------------*/
/*                    Set up for console services                     */
/*--------------------------------------------------------------------*/

#if defined(FAMILYAPI) || defined(BIT32ENV) || defined(_Windows)
#define SIMPLE_CONSOLE_FGETS
#else

#define MULTIPLEX 0x2f        /* 8086 DOS Interrupt for multiplexing */

static int DOSRead( char *buff, const int buflen);

static boolean DOSKeyActive( void );

static int DOSKeyRead( char *buff , int buflen );

#endif

/*--------------------------------------------------------------------*/
/*       G e t _ O n e                                                */
/*                                                                    */
/*       Get a single character from the console                      */
/*--------------------------------------------------------------------*/

int Get_One()
{

   return getch();

} /*Get_One*/

/*--------------------------------------------------------------------*/
/*       I n v o k e                                                  */
/*                                                                    */
/*       Invoke the user's editor or pager to handle a text file      */
/*--------------------------------------------------------------------*/

int Invoke(const char *ecmd,
           const char *filename,
           const boolean unique )
{
   char command[FILENAME_MAX*2 + 1];
   char tempname[FILENAME_MAX];

   int rc;

   if (ecmd == nil(char))
   {
      printf("Invoke: No program specified to invoke.\n");
      return 1;
   }

/*--------------------------------------------------------------------*/
/*                         Format the command                         */
/*--------------------------------------------------------------------*/

   strcpy(tempname,filename);
   denormalize( tempname );

   sprintf(command, ecmd, tempname);

/*--------------------------------------------------------------------*/
/*          Execute command, report results if interesting.           */
/*--------------------------------------------------------------------*/

   rc = executeCommand(command, NULL, NULL, TRUE, unique );
   if( rc )
   {
      printf("Invoke: \"%s\" failed, exit code %d.\n",
              command,
              rc );

#if defined(__OS2__) || defined(FAMILYAPI)
      if ( ! unique )
         printf("Try setting NewEditorWindow or NewPagerWindow\n");
#endif

      return(2);
   }

   return 0;

} /* Invoke */

/*--------------------------------------------------------------------*/
/*       I s _ C o n s o l e                                          */
/*                                                                    */
/*       Determine if stream is from the console                      */
/*                                                                    */
/*       Note:  isatty() actually returns if the stream is a          */
/*       character device, thus causing device NUL to appear          */
/*       interactive; this is not acceptable, but there is not a      */
/*       trivial fix.                                                 */
/*--------------------------------------------------------------------*/

boolean Is_Console(FILE *stream)
{

   return isatty(fileno(stream));

} /*Is_Console*/

#ifdef SIMPLE_CONSOLE_FGETS

/*--------------------------------------------------------------------*/
/*       C o n s o l e _ f g e t s                                    */
/*                                                                    */
/*       Read a full line from the console under non-DOS systems      */
/*--------------------------------------------------------------------*/

boolean Console_fgets(char *buff, int buflen, char *prompt)
{

   if (bflag[F_DOSKEY] )
   {
     printmsg(0,"DOSKEY support not available, option disabled");
     bflag[F_DOSKEY] = FALSE;
   }

   fputs(prompt, stdout);
   fflush(stdout);

   return (fgets(buff, buflen, stdin) != nil(char)) ? TRUE : FALSE;

} /*Console_fgets*/

#else

/*--------------------------------------------------------------------*/
/*       C o n s o l e _ f g e t s                                    */
/*                                                                    */
/*       Get a line of input from the local console                   */
/*                                                                    */
/*       This is a hook to allow for using the local system's         */
/*       facility for input line editing.  We call DOS to perform     */
/*       a line read, thus allowing utilities like DOSEDIT or CED     */
/*       to do their fancy work.                                      */
/*--------------------------------------------------------------------*/

boolean Console_fgets(char *buff, int buflen, char *prompt)
{
   static boolean eof = FALSE;    /* pending EOF flag  */

   char *eofptr;

   if (eof) {           /* have a pending EOF?  */
      eof = FALSE;      /* no more pending EOF  */
      return FALSE;     /* signal the EOF    */
   }

/*--------------------------------------------------------------------*/
/*      Prompt the user, read the data, and then go to a new line     */
/*--------------------------------------------------------------------*/

   fputs(prompt, stdout);
   fflush(stdout);

   if ( DOSKeyActive() )
      buflen = DOSKeyRead( buff, buflen );
   else
      buflen = DOSRead( buff, buflen );

    putchar('\n');

/*--------------------------------------------------------------------*/
/*             Determine if we hit end of file on the read            */
/*--------------------------------------------------------------------*/

   if ( buflen == -1 )
   {
      *buff = '\0';
      return FALSE;
   }

/*--------------------------------------------------------------------*/
/*                        Terminate the buffer                        */
/*--------------------------------------------------------------------*/

   buff[buflen] = '\n';
   buff[buflen + 1] = '\0';

   if ((eofptr = strchr(buff, '\x1a')) == nil(char))
      return TRUE;      /* an ordinary successful read   */
   else if (eofptr == buff)
   {
      return FALSE;     /* signal EOF right away      */
   }
   else {
      eof = TRUE;       /* we now have a pending EOF  */
      *eofptr = '\0';
      return TRUE;      /* read successful but EOF next  */
   } /* else */

} /*Console_fgets*/

/*--------------------------------------------------------------------*/
/*       D O S R e a d                                                */
/*                                                                    */
/*       Read from console under DOS without DOSKEY.  We use DOS      */
/*       services rather than the C library to insure we such hooks   */
/*       are CED are used if the user installed them.                 */
/*--------------------------------------------------------------------*/

static int DOSRead( char *buff, const int buflen)
{
   union REGS regs;
   struct SREGS sregs;

   struct {
      unsigned char maximum, actual;
      char buffer[255];
   } request;

   char far *p = (char far *) &request;

/*--------------------------------------------------------------------*/
/*            Set up the address of our read buffer for DOS           */
/*--------------------------------------------------------------------*/

   sregs.ds = FP_SEG( p );    /* Use segment of the buffer           */
   regs.x.dx = (unsigned int)(&request);
   request.maximum = (unsigned char) min( buflen - 1,
                                          sizeof request.buffer);
   regs.h.ah = 0x0a;          /* Buffered keyboard input             */

/*--------------------------------------------------------------------*/
/*                  Invoke the buffered console read                  */
/*--------------------------------------------------------------------*/

   intdosx(&regs, &regs, &sregs);

/*--------------------------------------------------------------------*/
/*                        Now return the result                       */
/*--------------------------------------------------------------------*/

   memcpy( buff, request.buffer, request.actual );
   return (unsigned int) request.actual;

} /* DOSRead */

/*--------------------------------------------------------------------*/
/*       D O S K e y A c t i v e                                      */
/*                                                                    */
/*       Determine if the DOS Key command line editor is active       */
/*--------------------------------------------------------------------*/

static boolean DOSKeyActive( void )
{
   static boolean first_pass = TRUE;
   static boolean active = FALSE;

   if ( first_pass )
   {
      first_pass = FALSE;
      if ((_osmajor > 4) )
      {
         union REGS regs;

#ifdef __TURBOC__
         if ( getvect( MULTIPLEX ) == NULL )
#else
         if ( _dos_getvect( MULTIPLEX ) == NULL )
#endif
            printmsg(0,"Multiplex interrupt not installed???\n");
         else {
            regs.x.ax = 0x4800;     /* Request for DOS Key active */
            int86( MULTIPLEX , &regs, &regs );
            if ( regs.h.al != 0x00 )
               active = TRUE;
         }
      } /* if (_osmajor > 4 ) */
   } /* if ( first_pass ) */

/*--------------------------------------------------------------------*/
/*                          Return to caller                          */
/*--------------------------------------------------------------------*/

   if ( bflag[F_DOSKEY] && ! active )
   {
     printmsg(0,"DOSKEY support not installed, option disabled");
     bflag[F_DOSKEY] = FALSE;
   }

   return active;

} /* DOSKeyActive */

/*--------------------------------------------------------------------*/
/*    D O S K e y R e a d                                             */
/*                                                                    */
/*    Read a line from the terminal using DOS Key                     */
/*--------------------------------------------------------------------*/

static int DOSKeyRead( char *buff , int buflen )
{
   union REGS regs;
   struct SREGS sregs;

   struct {
      unsigned char maximum, actual;
      char buffer[126];
   } request;

   char far *p = (char far *) &request;

/*--------------------------------------------------------------------*/
/*                   Set up for the DOSKEY read call                  */
/*--------------------------------------------------------------------*/

   sregs.ds = FP_SEG( p );    /* Use segment of the buffer           */
   regs.x.dx = (unsigned int)(&request);
   regs.x.ax = 0x4810;
   request.maximum = (unsigned char) min( buflen - 1, sizeof request );

/*--------------------------------------------------------------------*/
/*                      Issue the call to DOSKEY                      */
/*--------------------------------------------------------------------*/

   int86x( MULTIPLEX, &regs, &regs, &sregs );

/*--------------------------------------------------------------------*/
/*                          Check the result                          */
/*--------------------------------------------------------------------*/

   if ( regs.x.ax == 0 )      /* Function succeed?                */
   {
      buflen = request.actual;
      memcpy( buff, request.buffer , buflen );
   } /* if ( regs.x.ax == 0 ) */
   else {                        /* Function failed, report it    */
      printmsg(0,"DOSKEY read failed!");
      buflen = -1;
   } /* else */

/*--------------------------------------------------------------------*/
/*                          Return to caller                          */
/*--------------------------------------------------------------------*/

   return buflen;

} /* DOSKeyRead */

#endif

/*--------------------------------------------------------------------*/
/*       C l e a r                                                    */
/*                                                                    */
/*       Clear the screen                                             */
/*--------------------------------------------------------------------*/

void ClearScreen()
{

#ifdef __TURBOC__

   clrscr();

#elif defined(WIN32)

   long mode;
   static COORD coord = {0, 0};
   static HANDLE cons_hnd = INVALID_HANDLE_VALUE;

   if (cons_hnd == INVALID_HANDLE_VALUE)
   {
      cons_hnd = GetStdHandle(STD_OUTPUT_HANDLE);
      GetConsoleMode(cons_hnd, &mode);
      mode |= ENABLE_PROCESSED_OUTPUT;
      SetConsoleMode(cons_hnd, mode);
   }

   SetConsoleCursorPosition(cons_hnd, coord);
   FillConsoleOutputCharacter(cons_hnd, 0x20, 60*132, coord, &mode);

#else
   fputs("\033[2J", stdout);        /* ANSI Erase screen           */
   fflush(stdout);
#endif

} /* ClearScreen */
