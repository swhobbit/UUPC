/*--------------------------------------------------------------------*/
/*    c a t c h e r . c                                               */
/*                                                                    */
/*    Ctrl-Break handler for UUPC/extended                            */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Since C I/O functions are not safe inside signal routines,      */
/*    the code uses conditionals to use system-level DOS and OS/2     */
/*    services.  Another option is to set global flags and do any     */
/*    I/O operations outside the signal handler.                      */
/*--------------------------------------------------------------------*/

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
#include "catcher.h"
#include "safeio.h"

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

boolean terminate_processing = FALSE;
boolean interactive_processing = TRUE;
boolean norecovery = TRUE;

int panic_rc = 69;

#define INVALID_CHAR '*'

/*--------------------------------------------------------------------*/
/*    c t r l c h a n d l e r                                         */
/*                                                                    */
/*    Handles SIGINT (CTRL+C) interrupt; from MicroSoft Programmer's  */
/*    Workbench QuickHelp samples                                     */
/*--------------------------------------------------------------------*/

void ctrlchandler( void )
{
    int ch = INVALID_CHAR;

/*--------------------------------------------------------------------*/
/*                  Disallow CTRL+C during handler.                   */
/*--------------------------------------------------------------------*/

    signal( SIGINT, SIG_IGN );

/*--------------------------------------------------------------------*/
/*          Don't ask if the program doesn't think we should          */
/*--------------------------------------------------------------------*/

    if ( ! interactive_processing )
    {

      safeout( "\r\n" );
      safeout( compilen );
      panic_rc = 100;
      terminate_processing = interactive_processing = TRUE;
      safeout(": *** Termination in progress ***\r\n");
      signal( SIGINT, ctrlchandler );
      return;
    }

    if ( terminate_processing )
      safeout( "Termination already in progress ... answer Y to SCRAM program");

/*--------------------------------------------------------------------*/
/*                   Ask user if he/she/it is sure                    */
/*--------------------------------------------------------------------*/

   while ( ch == INVALID_CHAR )
   {
      safeout( "\r\n" );
      safeout( compilen );
      safeout( ": Abort processing? (Y/N) " );
      safeflush();            /* Flush any queued characters         */
      ch = safein();

      switch( ch )
      {

         case 'y':
         case 'Y':
            if ( terminate_processing || norecovery )
            {
               safeout("\n\rProgram aborted.\r\n");
               _exit(100);
            }

            terminate_processing = TRUE;  /* Controlled shutdown  */
            panic_rc = 100;
            safeout("\n\r*** Termination in progress ***\r\n");
            break;

        case 'N':
        case 'n':
           safeout("\r\nResuming execution\r\n");
           break;

        default:
           safeout(" -- Invalid response\a");
           ch = INVALID_CHAR;
           break;

      } /* switch  */
   } /* for */

/*--------------------------------------------------------------------*/
/*    The CTRL+C interrupt must be reset to our handler since by      */
/*    default it is reset to the system handler.                      */
/*--------------------------------------------------------------------*/

    signal( SIGINT, ctrlchandler );
} /* catcher */
