/*--------------------------------------------------------------------*/
/*    Program:    regsetup.c             27 March 1994                */
/*    Author:     David M. Watt                                       */
/*    Internet:   dmwatt@smersh.watt.com                              */
/*    Function:   Copies UUPC's configuration info into the registry. */
/*    Language:   Visual C++ 2.2/Win32                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1999 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Usage:      regsetup [-s] [-u] [-c]                             */
/*                Where:                                              */
/*                                                                    */
/*                -s       specifies that the system configuration    */
/*                         file should be copied into                 */
/*                         HKEY_LOCAL_MACHINE                         */
/*                         (Both the UUPCSYSRC and UUPCUSRRC          */
/*                         settings are copied into the system        */
/*                         registry.)                                 */
/*                -u       specifies that the user configuration      */
/*                         file should be copied into                 */
/*                         HKEY_CURRENT_USER.  UUPCUSRRC is copied    */
/*                         here as well.                              */
/*                -c       clears the current contents of the UUPC/   */
/*                         Extended key under both LOCAL_MACHINE and  */
/*                         CURRENT_USER                               */
/*                                                                    */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: regsetup.c 1.10 1998/04/27 01:55:28 ahd v1-13f ahd $
 *
 *    $Log: regsetup.c $
 *    Revision 1.10  1998/04/27 01:55:28  ahd
 *    Make copyright unconditional, and match configur.c version
 *
 *    Revision 1.9  1998/03/01 01:36:39  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.8  1997/04/24 01:31:21  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.7  1996/01/07 14:14:40  ahd
 *    Bring up level to new format of configuration tables
 *
 *    Revision 1.6  1996/01/01 21:18:06  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.5  1995/03/09 00:31:26  dmwatt
 *    Don't reference NULL ptrs in configuration table
 *
 *    Revision 1.4  1995/02/14 04:38:42  ahd
 *    Correct problems with directory processing under NT
 *
 *    Revision 1.3  1995/02/07 01:29:14  dmwatt
 *    Clean up compile errors caused by ahd's VC++ warnings cleanup
 *
 *    Revision 1.2  1995/01/07 16:22:49  ahd
 *    Change KWBoolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.1  1994/04/24 20:24:43  dmwatt
 *    Initial revision
 *
 */

#include "uupcmoah.h"

static const char rcsid[] =
         "$Id: regsetup.c 1.10 1998/04/27 01:55:28 ahd v1-13f ahd $";

/*--------------------------------------------------------------------*/
/*                        System include file                         */
/*--------------------------------------------------------------------*/

#include <windows.h>
#include <io.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "getopt.h"
#include "pushpop.h"
#include "timestmp.h"
#include "pnterr.h"

/*--------------------------------------------------------------------*/
/*                        Typedefs and macros                         */
/*--------------------------------------------------------------------*/

#define BASEHIVE "Software\\Kendra Electronic Wonderworks\\UUPC/extended"

/*--------------------------------------------------------------------*/
/*                  Prototypes and global variables                   */
/*--------------------------------------------------------------------*/

currentfile();

HKEY CreateHive(HKEY topLevel);

void PutRegistry(HKEY hBaseHive, char *subKeyName, char *keyName, char *keyValue);

void ClearRegistry(void);

void DeleteTree(HKEY hTreeBase);

KWBoolean regconfigure( CONFIGBITS program, HKEY hSystemHive, HKEY hUserHive);

void Usage(void);

/*--------------------------------------------------------------------*/
/*       C o p y T a b l e                                            */
/*                                                                    */
/*       Copy the system settings into the system hive                */
/*--------------------------------------------------------------------*/

void CopyTable(HKEY hSystemHive,
               char *subKey,
               CONFIGTABLE *table,
               const size_t tableSize )
{
   size_t subscript;

   for (subscript = 0; subscript < tableSize; subscript ++ )
   {
      if (table[subscript].flag & B_OBSOLETE)
      {
         /* Skip obsolete stuff */
         continue;
      }
      else if (table[subscript].loc &&
               *((char **)(table[subscript].loc)) == NULL)
      {
         /* Skip uninitialized */
         continue;
      }
      else if (table[subscript].flag & B_BOOLEAN)
      {
         /* For now, take it easy:  leave out KWBooleans */
         continue;
      }
      else if (table[subscript].flag & B_CHAR)
      {
         /* characters */
             char buf[2];
             buf[1] = '\0';
             buf[0] = *((char *)table[subscript].loc);
             PutRegistry(hSystemHive, subKey, table[subscript].sym, buf);
      }
      else if (table[subscript].flag & (B_SHORT|B_LONG))
      {
         char buf[BUFSIZ];

         if (table[subscript].flag & B_LONG)
            sprintf(buf, "%ld", *((long *) table[subscript].loc));
         else
            sprintf(buf, "%hu", *((KEWSHORT *) table[subscript].loc));
         PutRegistry(hSystemHive, subKey, table[subscript].sym, buf);
      }
      else if (table[subscript].flag & (B_LIST | B_CLIST))
      {

/*--------------------------------------------------------------------*/
/*       Set delimiter to either space or colon (B_CLIST -> colon)    */
/*--------------------------------------------------------------------*/

         char delimiter[2];
         char buf[BUFSIZ];
         char **el = (char **)(*((char **)(table[subscript].loc)));
         int i;

         delimiter[1] = '\0';

         if (table[subscript].flag & B_CLIST)
            delimiter[0] = ':';
         else
            delimiter[0] = ' ';

         i = 0;
         *buf = '\0';

         while (el[i] != NULL)
         {
            strcat(buf, el[i]);
            i++;
            if (el[i] != NULL)
                           strcat(buf, delimiter);
                        else
                           break;

         }
         PutRegistry(hSystemHive, subKey, table[subscript].sym, buf);
      } else
      {
         /* All that's left is strings */
               if (table[subscript].loc)
                  PutRegistry(hSystemHive,
                              subKey,
                              table[subscript].sym,
                              *((char **)(table[subscript].loc)));
      }
   }

} /* CopyTable */

/*--------------------------------------------------------------------*/
/*    m a i n                                                         */
/*                                                                    */
/*    main program                                                    */
/*--------------------------------------------------------------------*/

 void main( int argc , char *argv[] )
 {

   int option;
   KWBoolean doSystem = KWFalse;
   KWBoolean doUser = KWFalse;
   KWBoolean doClear = KWFalse;
   HKEY hSystemHive = INVALID_HANDLE_VALUE;
   HKEY hUserHive = INVALID_HANDLE_VALUE;

   banner(argv);

   while((option = getopt(argc, argv, "suc")) != EOF)
   switch(option)
   {

/*--------------------------------------------------------------------*/
/*   Copy the contents of UUPC.RC into HKEY_LOCAL_MACHINE             */
/*--------------------------------------------------------------------*/
      case 's':
         doSystem = KWTrue;
         break;

/*--------------------------------------------------------------------*/
/*   Copy the contents of [userid].RC into HKEY_CURRENT_USER          */
/*--------------------------------------------------------------------*/

      case 'u':
         doUser = KWTrue;
         break;

      case 'c':
         doClear = KWTrue;
         break;
   }

   if (!doSystem && !doUser && !doClear)
      Usage();

   if (doSystem)
   {
      hSystemHive = CreateHive(HKEY_LOCAL_MACHINE);
      if (INVALID_HANDLE_VALUE == hSystemHive)
         panic();
   }

   if (doUser)
   {
      hUserHive = CreateHive(HKEY_CURRENT_USER);
      if (INVALID_HANDLE_VALUE == hUserHive)
         panic();
   }

   if (doClear)
   {
      ClearRegistry();
   }

   if (!regconfigure( B_ALL, hSystemHive, hUserHive ))
      panic();

   exit(0);
}

HKEY CreateHive(HKEY topLevel)
{
   LONG result;
   HKEY hResultKey = INVALID_HANDLE_VALUE;
   DWORD disposition;

   result = RegCreateKeyEx(topLevel, BASEHIVE, 0, "", REG_OPTION_NON_VOLATILE,
      KEY_ALL_ACCESS, NULL, &hResultKey, &disposition);

   if (result != ERROR_SUCCESS)
   {
      printNTerror("RegCreateKeyEx", result);
      return hResultKey;
   }

   return hResultKey;

} /* CreateHive */

KWBoolean regconfigure( CONFIGBITS program, HKEY hSystemHive, HKEY hUserHive)
{
   char *sysrc, *usrrc;
   FILE *fp;
   KWBoolean success;
   char buf[BUFSIZ];
   size_t subscript;
   char *s, *ptr;

   static char *envlist[] = { "EDITOR",   "EDITOR",
                              "HOME",     "HOME",
                              "NAME",     "NAME",
                              "MAILBOX",  "MAILBOX",
                              "LOGNAME",  "MAILBOX",  /* Same as rcs   */
                              "USERNAME", "MAILBOX",  /* Useful for NT */
                              "TEMP",     "TEMPDIR",
                              "TMP",      "TEMPDIR",
                              NULL } ;

   typedef struct _DEFAULTS {
      char **value;
      char *literal;
      KWBoolean path;
   } DEFAULTS;

   static DEFAULTS deflist[] = {
        {&E_maildir,      "mail"    , KWTrue },
        {&E_newsdir,      "news"    , KWTrue },
        {&E_pubdir,       "public"  , KWTrue },
        {&E_spooldir,     "spool"   , KWTrue },
        {&E_tempdir,      "tmp"     , KWTrue },
        {&E_systems,      "systems" , KWTrue },
        {&E_passwd,       "passwd"  , KWTrue },
        {&E_permissions,  "permissn", KWTrue },
        {&E_tz,           "tz"      , KWFalse},
        { NULL  }
        } ;

/*--------------------------------------------------------------------*/
/*                  Determine the active environment                  */
/*--------------------------------------------------------------------*/

#if !defined(__TURBOC__) && !defined(BIT32ENV)
   if (_osmode != DOS_MODE)
      active_env = ENV_OS2 | ENV_BIT16;
#endif

   if (!getrcnames(&sysrc, &usrrc))
      return KWFalse;

/*--------------------------------------------------------------------*/
/*          Extract selected variables from our environment           */
/*--------------------------------------------------------------------*/

   subscript = 0;

   while( envlist[subscript] != NULL )
   {
      s = getenv( envlist[subscript++] );

      if (s != NULL )
      {
         sprintf(buf,"%s=%s",envlist[subscript], s );

         if ( !processconfig( buf,
                              SYSTEM_CONFIG,
                              program,
                              rcTable,
                              rcTableSize,
                              configFlags,
                              configFlagsSize ))
         {
            printmsg(0,"Internal error: Invalid keyword %s",
                       envlist[subscript]  );
            panic();
         }
      } /* if (sysrc != NULL ) */

      subscript++;            /* Step to next environment var in list */
   }

/*--------------------------------------------------------------------*/
/*       If the user specified a user name in the environment but     */
/*       not a UUPCUSRRC variable, build a UUPCUSRRC variable         */
/*--------------------------------------------------------------------*/

   if ((usrrc == nil(char)) && ( E_mailbox != NULL ))
   {                             /* Can build from the environment?  */
      strcpy( buf, E_mailbox );
      strcat( buf, ".rc" );
      usrrc = newstr( buf );
      printmsg(2,"Using UUPCUSRRC=%s", usrrc );
   }

/*--------------------------------------------------------------------*/
/*          Determine configuration directory from UUPCSYSRC          */
/*--------------------------------------------------------------------*/

   E_confdir = normalize( sysrc );     /* Make 'em all slashes        */

   s = strrchr( E_confdir, '/' );      /* Get end of path component   */

   if ( s == NULL )                    /* There WAS one, right?       */
   {                                   /* Er, no, sorry.              */
      printmsg(0,"No path name in UUPCSYSRC: %s", sysrc);
      panic();
   }

   *(s+1) = '\0';                      /* Terminate for Config Directory */
   E_confdir = newstr(normalize( E_confdir ));
                                       /* Drop trailing slash unless  */
                                       /* root directory and save     */

/*--------------------------------------------------------------------*/
/*      Put the UUPCSYSRC environment variable into the registry      */
/*--------------------------------------------------------------------*/

   for (ptr = sysrc; *ptr != '\0'; ptr++)       /* Convert to slashes */
      if (*ptr == '\\')
         *ptr = '/';

   PutRegistry(hSystemHive, NULL, SYSRCSYM, sysrc);

/*--------------------------------------------------------------------*/
/*               Process the system configuration file                */
/*--------------------------------------------------------------------*/

   if ((fp = FOPEN(sysrc, "r",TEXT_MODE)) == nil(FILE))
   {
      printmsg(0, "Cannot open system configuration file \"%s\"", sysrc);
      printerr(sysrc);
      return KWFalse;
   }

   PushDir( E_confdir );

   success = getconfig(fp,
                       SYSTEM_CONFIG,
                       program,
                       rcTable,
                       rcTableSize,
                       configFlags,
                       configFlagsSize );

   fclose(fp);
   if (!success)
   {
      PopDir();
      return KWFalse;
   }

/*--------------------------------------------------------------------*/
/*                     Fill in derived parameters                     */
/*--------------------------------------------------------------------*/

   subscript = 0;

   while( deflist[subscript].value != NULL )
   {
      if ( *(deflist[subscript].value) == NULL )
         *(deflist[subscript].value) = deflist[subscript].path ?
                     newstr( normalize(deflist[subscript].literal) ) :
                     deflist[subscript].literal;
      subscript++;
   }

/*--------------------------------------------------------------------*/
/*      Copy the system settings into the system hive                 */
/*--------------------------------------------------------------------*/

   CopyTable(hSystemHive, sysrc, rcTable, rcTableSize);

/*--------------------------------------------------------------------*/
/*                Process the user configuration value                */
/*--------------------------------------------------------------------*/

   if (usrrc != nil(char))
   {
      for (ptr = usrrc; *ptr != '\0'; ptr++)    /* Convert to slashes */
         if (*ptr == '\\')
            *ptr = '/';

/*--------------------------------------------------------------------*/
/*      Put the UUPCUSRRC environment variable into the registry      */
/*--------------------------------------------------------------------*/

      PutRegistry(hUserHive, NULL, USRRCSYM, usrrc);

/*--------------------------------------------------------------------*/
/*       Put UUPCUSRRC into both:  the LOCAL_MACHINE one will be      */
/*       used by services.                                            */
/*--------------------------------------------------------------------*/

      PutRegistry(hSystemHive, NULL, USRRCSYM, usrrc);

      if ((fp = FOPEN(usrrc, "r",TEXT_MODE)) == nil(FILE))
      {
         printmsg(0, "Cannot open user configuration file \"%s\"", usrrc);
         PopDir();
         return KWFalse;
      }

/*--------------------------------------------------------------------*/
/*  NULL out the table (ignore memory leaks for now)                  */
/*--------------------------------------------------------------------*/

   for ( subscript = 0; subscript < rcTableSize; subscript++ )
   {
      if (!rcTable[subscript].loc)
           continue;     /* Skip variables we don't save */
      if (rcTable[subscript].flag & B_OBSOLETE)
         continue;      /* Skip obsolete stuff */
      if (rcTable[subscript].loc &&
          *((char **)(rcTable[subscript].loc)) == NULL)
         continue;  /* Skip uninitialized */

/*--------------------------------------------------------------------*/
/*       For now, take it easy:  leave out KWBooleans, shorts,        */
/*       longs, and lists                                             */
/*--------------------------------------------------------------------*/

     if (rcTable[subscript].flag & B_BOOLEAN)
         continue;

      if (rcTable[subscript].flag & (B_SHORT|B_LONG))
         *((char **)(rcTable[subscript].loc)) = NULL;

      if (rcTable[subscript].flag & (B_LIST | B_CLIST))
         *((char **)(rcTable[subscript].loc)) = NULL;

      /* All that's left is strings */

      *((char **)(rcTable[subscript].loc)) = NULL;

   } /* for ( subscript = 0; subscript < rcTableSize; subscript++ ) */

   success = getconfig(fp,
                        USER_CONFIG,
                        program,
                        rcTable,
                        rcTableSize,
                        configFlags,
                        configFlagsSize );
   fclose(fp);

/*--------------------------------------------------------------------*/
/*      Copy the user settings into the user hive                     */
/*--------------------------------------------------------------------*/

      CopyTable(hUserHive, usrrc, rcTable, rcTableSize );

      CopyTable(hSystemHive, usrrc, rcTable, rcTableSize);
                  /* Put a copy into the system archive for services */

      if (!success)
      {
         PopDir();
         return KWFalse;
      }

   }

/*--------------------------------------------------------------------*/
/*                       Display our copyright                        */
/*                                                                    */
/*      We do unconditionally because this program is rarely run      */
/*--------------------------------------------------------------------*/

      fprintf(stderr,
"Changes and Compilation Copyright (c) 1989-1999 by Kendra Electronic\n"
"Wonderworks.  May be freely distributed for reasonable copying fee\n"
"if original documentation and source is included.  See license for\n"
"details and restrictions.\n");

   PopDir();

   return success;

} /*configure*/

void PutRegistry(HKEY hBaseHive,
                 char *subKeyName,
                 char *keyName,
                 char *keyValue)
{
   HKEY hDestKey;
   LONG result;
   HKEY hResultKey = INVALID_HANDLE_VALUE;
   DWORD disposition;

   if (hBaseHive == INVALID_HANDLE_VALUE)
      return;

   if (subKeyName == NULL)
      hDestKey = hBaseHive;
   else
   {
      result = RegCreateKeyEx(hBaseHive, subKeyName, 0, "",
         REG_OPTION_NON_VOLATILE,
         KEY_ALL_ACCESS, NULL, &hDestKey, &disposition);

      if (result != ERROR_SUCCESS)
      {
         printNTerror("RegCreateKeyEx", result);
         return;
      }
   }

   result = RegSetValueEx(hDestKey,
      keyName,
      0,
      REG_SZ,
      keyValue,
      strlen(keyValue));
   if (result != ERROR_SUCCESS)
   {
      printmsg(0, "Could not set key %s to %s", keyName, keyValue);
      printNTerror("RegSetValueEx", result);
      return;
   }

   if (subKeyName != NULL)
      RegCloseKey(hDestKey);

   return;

} /* PutRegistry */

void ClearRegistry(void)
{
   LONG result;
   HKEY hSystemKey = INVALID_HANDLE_VALUE;
   HKEY hUserKey = INVALID_HANDLE_VALUE;

   result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, BASEHIVE, 0,
      KEY_ALL_ACCESS, &hSystemKey);

   if (result == ERROR_SUCCESS)
   {
      DeleteTree(hSystemKey);
      RegDeleteValue(hSystemKey, SYSRCSYM);
      RegDeleteValue(hSystemKey, USRRCSYM);
      RegCloseKey(hSystemKey);
   }

   result = RegOpenKeyEx(HKEY_CURRENT_USER, BASEHIVE, 0,
      KEY_ALL_ACCESS, &hUserKey);

   if (result == ERROR_SUCCESS)
   {
      DeleteTree(hUserKey);
      RegDeleteValue(hUserKey, SYSRCSYM);
      RegDeleteValue(hUserKey, USRRCSYM);
      RegCloseKey(hUserKey);
   }

} /* ClearRegistry */

void DeleteTree(HKEY hTreeBase)
{
   DWORD index = 0;
   char buf[BUFSIZ];
   DWORD bufsize = BUFSIZ;
   DWORD result;

   while (ERROR_NO_MORE_ITEMS != RegEnumKey(hTreeBase, index, buf, bufsize))
   {
      result = RegDeleteKey(hTreeBase, buf);

      if (result != ERROR_SUCCESS)
      {
         HKEY subkey;

         result = RegOpenKeyEx(hTreeBase, buf, 0, KEY_ALL_ACCESS, &subkey);

         if (result != ERROR_SUCCESS)
         {
            printmsg(0, "Trouble opening a subkey!\n");
            panic();
         }
         else
         {
            DeleteTree(subkey);
         }
      }
      else
         index++;
   }

} /* DeleteTree */

void Usage(void)
{
   printmsg(0, "regsetup: must specify -c, -s and/or -u\n"
               "      -c: clear UUPC's current registry contents\n"
               "      -s: copy UUPC.RC (system) configuration to registry\n"
               "      -u: copy [userid].RC (user) configuration to registry\n");
   exit(0);
}
