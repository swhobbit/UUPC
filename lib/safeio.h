/*--------------------------------------------------------------------*/
/*    s a f e i o . h                                                 */
/*                                                                    */
/*    Console I/O functions for use during interrupt processing       */
/*--------------------------------------------------------------------*/

#include "uutypes.h"        /* Include UUPC/extended types           */

int  safein( void );
boolean safepeek( void );
void safeout( char *str );
void safeflush( void );

#if defined(WIN32)
void InitConsoleInputHandle(void);
void InitConsoleOutputHandle(void);
#endif

