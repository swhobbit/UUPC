/*--------------------------------------------------------------------*/
/*       p r t y n t . c                                              */
/*                                                                    */
/*       Set task priority for NT tasks under UUPC/extended           */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1993 by Kendra Electronic                 */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <windows.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "pnterr.h"
#include "commlib.h"

/*--------------------------------------------------------------------*/
/*                          Local variables                           */
/*--------------------------------------------------------------------*/

currentfile();

static boolean restore = FALSE;

/*--------------------------------------------------------------------*/
/*       s e t P r t y                                                */
/*                                                                    */
/*       Set priority to configuration defined value                  */
/*--------------------------------------------------------------------*/

void setPrty( const KEWSHORT priorityIn, const KEWSHORT prioritydeltaIn )
{
/*--------------------------------------------------------------------*/
/*                     Up our processing priority                     */
/*--------------------------------------------------------------------*/
   HANDLE hProcess;
   BOOL rc;

   hProcess = GetCurrentProcess();
   rc = SetPriorityClass(hProcess, HIGH_PRIORITY_CLASS);

   if (!rc)
   {
      DWORD err = GetLastError();

      printNTerror("setprty", err);
   }

   restore = TRUE;
}

/*--------------------------------------------------------------------*/
/*       r e s e t P r t y                                            */
/*                                                                    */
/*       Restore priority saved by SetPrty                            */
/*--------------------------------------------------------------------*/

void resetPrty( void )
{
   HANDLE hProcess;
   BOOL rc;

   if ( !restore )
      return;

/*--------------------------------------------------------------------*/
/*                           Lower priority                           */
/*--------------------------------------------------------------------*/

   hProcess = GetCurrentProcess();
   rc = SetPriorityClass(hProcess, NORMAL_PRIORITY_CLASS);

   if (!rc)
   {
      DWORD err = GetLastError();
      printNTerror("resetPrty", err);
   }

}
