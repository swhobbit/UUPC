/*--------------------------------------------------------------------*/
/*    u u c p . c                                                     */
/*                                                                    */
/*    UUCP lookalike for IBM PC.                                      */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1997 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: uucp.c 1.24 1996/03/19 03:29:39 ahd Exp $
 *
 *    Revision history:
 *    $Log: uucp.c $
 *    Revision 1.24  1996/03/19 03:29:39  ahd
 *    Correct compile warnings
 *    Add new error trapping
 *
 *    Revision 1.23  1996/01/01 21:34:46  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.22  1995/09/26 00:37:40  ahd
 *    Use unsigned sequence number for jobs to prevent mapping errors
 *
 *    Revision 1.21  1995/02/24 00:38:58  ahd
 *    Reduce memory usage, reformat source
 *
 *    Revision 1.20  1995/02/20 17:28:43  ahd
 *    in-memory file support, 16 bit compiler clean up
 *
 *    Revision 1.19  1995/02/20 00:40:12  ahd
 *    Correct C compiler warnings
 *
 *    Revision 1.18  1995/02/12 23:37:04  ahd
 *    compiler cleanup, NNS C/news support, optimize dir processing
 *
 *    Revision 1.17  1995/01/07 16:41:26  ahd
 *    Change boolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.16  1994/12/22 00:44:15  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.15  1994/02/19 05:14:52  ahd
 *    Use standard first header
 *
 * Revision 1.14  1994/01/01  19:27:27  ahd
 * Annual Copyright Update
 *
 * Revision 1.13  1993/10/31  22:07:05  ahd
 * Issue messages if opens during copies fail
 * Don't close directory after copy failure if never open
 * Automatically build spool data directory as required
 *
 * Revision 1.12  1993/10/30  17:19:50  rhg
 * Additional clean-up for UUX support
 *
 * Revision 1.12  1993/10/30  17:19:50  rhg
 * Additional clean-up for UUX support
 *
 * Revision 1.11  1993/10/25  01:21:22  ahd
 * Force directory for destination when input is wildcarded
 *
 * Revision 1.10  1993/10/12  01:34:47  ahd
 * Normalize comments to PL/I style
 *
 * Revision 1.9  1993/10/03  20:43:08  ahd
 * Normalize comments to C++ double slash
 *
 * Revision 1.8  1993/10/02  19:07:49  ahd
 * Use 0644 permissions on files
 *
 * Revision 1.7  1993/09/20  04:48:25  ahd
 * TCP/IP support from Dave Watt
 * 't' protocol support
 * OS/2 2.x support (BC++ 1.0 for OS/2)
 *
 * Revision 1.6  1993/08/02  03:24:59  ahd
 * Further changes in support of Robert Denny's Windows 3.x support
 *
 * Revision 1.5  1993/07/31  16:27:49  ahd
 * Changes in support of Robert Denny's Windows support
 *
 * Revision 1.4  1993/04/11  00:35:46  ahd
 * Global edits for year, TEXT, etc.
 *
 * Revision 1.3  1993/04/05  04:35:40  ahd
 * Use timestamp/file size information returned by directory search
 *
 * Revision 1.2  1992/12/11  12:45:11  ahd
 * Normalize paths for files read
 *
 */

/*--------------------------------------------------------------------*/
/*                                                                    */
/*    Change history:                                                 */
/*                                                                    */
/*       02/08/81 H.A.E.Broomhall                                     */
/*                Hacked for UUPC/extended 1.09c                      */
/*       04/27/91 Drew Derbyshire                                     */
/*                Modified for UUPC/extended 1.10a                    */
/*       09/26/91 Mitch Mitchell                                      */
/*                Support for UUX                                     */
/*       01/26/92 Drew Derbyshire                                     */
/*                Various comment and error message clean up          */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                        system include files                        */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"

#include  <ctype.h>
#include  <direct.h>
#include  <fcntl.h>
#include  <io.h>
#include  <sys/stat.h>

#ifdef _Windows
#include <windows.h>
#endif

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include  "expath.h"
#include  "getopt.h"
#include  "getseq.h"
#include  "hostable.h"
#include  "import.h"
#include  "uundir.h"
#include  "security.h"
#include  "timestmp.h"
#include  "execute.h"

#ifdef _Windows
#include "winutil.h"
#include "logger.h"
#endif

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

static KWBoolean      spool_flag = KWFalse;
static char          spool_file[FILENAME_MAX] = ""; /* alt spool file name */
static KWBoolean      dir_flag = KWTrue;
static KWBoolean      xeqt_flag = KWTrue;   /* Triggered by -r option   */
static KWBoolean      j_flag = KWFalse;     /* set by -j option */
static char          grade = 'n';         /* Default grade of service  */
static KWBoolean      mail_me = KWFalse;    /* changes with -m           */
static KWBoolean      mail_them = KWFalse;  /* changes with -n           */
static char  remote_user[10];             /* user to mail with -n     */

static char  flags[16];

currentfile();

/*--------------------------------------------------------------------*/
/*                          Local prototypes                          */
/*--------------------------------------------------------------------*/

static void appendSlash( char *s );

/*--------------------------------------------------------------------*/
/*    u s a g e                                                       */
/*                                                                    */
/*    Report flags used by program                                    */
/*--------------------------------------------------------------------*/

static void usage(void)
{
   fprintf(stderr, "Usage: uucp\t[-c|-C] [-d|-f] [-gGRADE] [-j]"
                   "[-m] [-nUSER] [-r] [-sFILE]\\\n"
                   "\t\t[-xDEBUG_LEVEL] source-files destination-file\n");
} /* usage */

/*--------------------------------------------------------------------*/
/*    c p                                                             */
/*                                                                    */
/*    Copy a file                                                     */
/*--------------------------------------------------------------------*/

static int cp(char *from, char *to)
{
      int         fd_from, fd_to;
      int         nr = -1;
      char        buf[BUFSIZ]; /* faster if we alloc a big buffer   */

      if ((fd_from = open(from, O_RDONLY | O_BINARY)) == -1)
      {
         printerr(from);
         return(1);        /* failed                                   */
      }

      /* what if the target is a directory? */
      /* possible with local source & dest uucp */

      if ((fd_to = open(to, O_CREAT | O_BINARY | O_WRONLY, S_IWRITE | S_IREAD)) == -1)
      {
         printerr(to);
         close(fd_from);
         return(1);        /* failed                                   */
         /* NOTE - this assumes all the required directories exist!  */
      }

      while  ((nr = read(fd_from, buf, sizeof buf)) > 0)
      {
         if (write(fd_to, buf, (unsigned) nr) != nr)
         {
            printerr( to );
            close(fd_to);
            close(fd_from);
            return 1;
         }
      }

/*--------------------------------------------------------------------*/
/*      Close up after our copy and check for errors on the read      */
/*--------------------------------------------------------------------*/

      close(fd_to);
      close(fd_from);

      if (nr != 0)
      {
         printerr( from );
         return(1);        /* failed in copy                          */
      }

      return(0);

} /* cp */

/*--------------------------------------------------------------------*/
/*    s p l i t _ p a t h                                             */
/*                                                                    */
/*    split_path splits a path into 3 components.                     */
/*    1)  The system next in line                                     */
/*    2)  Any intermediate systems as a bang path                     */
/*    3)  The actual file name/path                                   */
/*                                                                    */
/*    It tries to be a little clever with idiots, in recognizing      */
/*    system=this machine                                             */
/*--------------------------------------------------------------------*/

static         void    split_path(char *path,
                                  char *system,
                                  char *inter,
                                  char *file)
{
      char    *p_left, *p_right, *p;

      *system = *inter = *file = '\0';    /* init to nothing           */

      for (p = path;; p = p_left + 1)
      {
         p_left = strchr(p, '!');         /* look for the first bang  */

         if (p_left == NULL)
         {                                /* not a remote path        */
            strcpy(file, p);              /* so just return filename  */
            return;
         }

         /* now check if the system was in fact us.
            If so strip it and restart */

         if (equaln(E_nodename, p, (size_t) (p_left - p)) &&
            (E_nodename[p_left - p] == '\0'))
            continue;

         p_right = strrchr(p, '!');      /* look for the last bang     */
         strcpy(file, p_right + 1);      /* and thats our filename     */
         strncpy(system, p, (size_t) (p_left - p));
                                         /* and we have a remote system*/
         system[p_left - p] = '\0';

         /* now see if there is an intermediate path */

         if (p_left != p_right)
         {                          /* yup - there is           */
            strncpy(inter, p_left + 1, (size_t) (p_right - p_left - 1));
            inter[p_right - p_left - 1] = '\0';
         }

         return;                 /* and we're done                    */

      }        /* never get here :-)  */

} /* split_path */

/*--------------------------------------------------------------------*/
/*    d o _ u u x                                                     */
/*                                                                    */
/*    Generate & execute UUX command                                  */
/*--------------------------------------------------------------------*/

int   do_uux(char *remote,
             char *src_syst,
             char *src_file,
             char *dest_syst,
             char *dest_inter,
             char *dest_file)
{
      char        xcmd[BUFSIZ];        /* buffer for assembling the UUX command  */
      int         len;

/*--------------------------------------------------------------------*/
/*                 First - lets get the basic command                 */
/*--------------------------------------------------------------------*/

      len = sprintf(xcmd, "-x%d -g%c -%c%s%s%s%s %s!uucp -C%s%s%s ",
                          debuglevel, grade,
                          spool_flag ? 'C' : 'c',
                          xeqt_flag ? "" : " -r",
                          j_flag ? " -j" : "",
                          *spool_file ? " -s" : "",
                          *spool_file ? spool_file : "",
                          remote,
                          dir_flag ? "" : " -f",
                          mail_them ? " -n" : "",
                          mail_them ? remote_user : "");
                              /* but what about mailing the guy (-m)? */

/*--------------------------------------------------------------------*/
/*                  Now we sort out the source name                   */
/*--------------------------------------------------------------------*/

      if ((*src_syst == '\0') || equal(src_syst, E_nodename))
         len += sprintf(xcmd + len, " !%s ", src_file);
      else  {
         if (!equal(remote, src_syst))
            len += sprintf(xcmd + len, " (%s!%s) ", src_syst, src_file);
         else
            len += sprintf(xcmd + len, " (%s) ", src_file);
      } /* else */

/*--------------------------------------------------------------------*/
/*                   Now to do the destination name                   */
/*--------------------------------------------------------------------*/

      if (*dest_inter != '\0')
      {
         if (*dest_syst != '\0')
            len += sprintf(xcmd + len, " (%s!%s!%s) ", dest_syst, dest_inter, dest_file);
         else
            len += sprintf(xcmd + len, " (%s!%s) ", dest_inter, dest_file);
      }
      else  {
         if ((*dest_syst == '\0') || equal(dest_syst, E_nodename))
            len += sprintf(xcmd + len, " (%s!%s) ", E_nodename, dest_file);
      }
      printmsg(2, "xcmd: %s", xcmd);

/*--------------------------------------------------------------------*/
/*                              OK - GO!                              */
/*--------------------------------------------------------------------*/

   execute("uux", xcmd, NULL, NULL, KWTrue, KWFalse);

   return(1);

} /* do_uux */

/*--------------------------------------------------------------------*/
/*    s u b s e q                                                     */
/*                                                                    */
/*    Generate a valid sub-sequence number                            */
/*--------------------------------------------------------------------*/

static char subseq( void )
{
   static char next = '0' - 1;

   switch( next )
   {
      case '9':
         next = 'A';
         break;

      case 'Z':
         next = 'a';
         break;

      default:
         next++;
   } /* switch */

   printmsg(4,"subseq: Next subsequence is %c", next);
   return next;

} /* subseq */

/*--------------------------------------------------------------------*/
/*    d o _ c o p y                                                   */
/*                                                                    */
/*    At this point only one of the systems can be remote and only    */
/*    1 hop away.  All the rest have been filtered out                */
/*--------------------------------------------------------------------*/

int   do_copy(char *src_syst,
              char *src_file,
              char *dest_syst,
              char *dest_file)
{
      char        *p;
      KWBoolean wild_flag = KWFalse;
      KWBoolean write_flag;
      char        tmfile[15];       /* Unix style name for c file     */
      char        idfile[15];       /* Unix style name for data file copy  */
      char        work[FILENAME_MAX];   /* temp area for filename hacking  */
      char        search_file[FILENAME_MAX];
      char        source_path[FILENAME_MAX];
      char        icfilename[FILENAME_MAX]; /* our hacked c file path  */
      char        idfilename[FILENAME_MAX]; /* our hacked d file path  */

      struct  stat    statbuf;
      DIR *dirp = NULL;
      struct direct *dp = NULL;
      KWBoolean makeDirectory = KWTrue;  /* May need to build spool dir  */

      unsigned long    sequence = getSeq();
      char    *remote_syst;   /* Non-local system in copy              */
      char    *sequence_s;
      FILE        *cfile;
      static  char    *spool_fmt = SPOOLFMT;

      sequence_s = jobNumber( sequence, 3, bflag[F_ONECASE] );

      remote_syst =  equal(src_syst, E_nodename) ? dest_syst : src_syst;
      sprintf(tmfile, spool_fmt, 'C', remote_syst, grade, sequence_s);
      importpath(work, tmfile, remote_syst);
      mkfilename(icfilename, E_spooldir, work);

      if (!equal(src_syst, E_nodename))
      {
         if (expand_path(dest_file, NULL, E_homedir, NULL) == NULL)
            exit(1);
         strcpy( dest_file, normalize( dest_file ));
         p  = src_file;
         while (*p)
         {
            if (*p ==  '\\')
               *p = '/';
            p++;
         }
         printmsg(1, "uucp - from \"%s\" - control = %s", src_syst,
                  tmfile);

         if ((cfile = FOPEN(icfilename, "a",TEXT_MODE )) == NULL)
         {
            printerr( icfilename );
            fprintf(stderr, "uucp: cannot append to %s\n", icfilename);
            panic();
         }

         fprintf(cfile, "R %s %s %s -%s %s 0777 %s\n", src_file, dest_file,
               E_mailbox, flags, *spool_file ? spool_file : "dummy", remote_user);
         fclose(cfile);
         return(1);
      }
      else if (!equal(dest_syst, E_nodename))
      {
         printmsg(1,"uucp - spool %s - mkdir %s - execute %s",
                spool_flag ? "on" : "off",
                  dir_flag ? "on" : "off", xeqt_flag ? "do" : "don't");

         printmsg(1,"     - dest m/c = %s  sequence = %ld  control = %s",
                  dest_syst, sequence, tmfile);

         if (expand_path(src_file, NULL, E_homedir, NULL) == NULL)
            exit(1);

         normalize( src_file );

         p  = dest_file;

         while (*p)
         {
            if (*p ==  '\\')
               *p = '/';
            p++;
         }

         if (strcspn(src_file, "*?") == strlen(src_file))
         {
            wild_flag = KWFalse;

            if (stat(src_file, &statbuf) != 0)
            {
               printerr( src_file );
               exit(1);
            }

            if (statbuf.st_mode & S_IFDIR)
            {
               printf("uucp - directory name \"%s\" illegal\n",
                       src_file );
               exit(1);
            }

         } /* if (strcspn(src_file, "*?") == strlen(src_file))  */
         else  {

            wild_flag = KWTrue;

            appendSlash(dest_file); /* Target must be directory   */

            strcpy(source_path, src_file);
            p = strrchr(source_path, '/');
            strcpy(search_file, p+1);
            *++p = '\0';

            dirp = opendirx(source_path,search_file);

            if (dirp == NULL)
            {
               printf("uucp - unable to open directory %s\n",source_path);
               exit(1);
            } /* if */

            if ((dp = readdir(dirp)) == nil(struct direct))
            {
               printf("uucp - can't find any file %s\n", search_file);
               exit(1);
            }
         } /* else */

         write_flag = KWTrue;

         while (write_flag)
         {
            if (wild_flag)
            {
               strcpy(src_file, source_path);
               strlwr( dp->d_name );
               strcat( strcpy(src_file, source_path), dp->d_name );
               strcpy( src_file, normalize( src_file ));
               printf("Queueing file %s for %s!%s\n", src_file, dest_syst,
                        dest_file);
            }

            if (spool_flag)
            {
               sprintf(idfile, spool_fmt, 'D', E_nodename, (char) subseq(),
                           sequence_s);

               importpath(work, idfile, remote_syst);
               mkfilename(idfilename, E_spooldir, work);

               if ( makeDirectory )
               {
                  char *lastPath = strrchr( idfilename , '/' );

                  if ( lastPath != NULL )
                  {

                     *lastPath = '\0';
                     MKDIR( idfilename );
                     *lastPath = '/';     /* Restore last segment of name  */
                     makeDirectory = KWFalse;

                  } /* if ( lastPath != NULL ) */

               } /* if ( makeDirectory ) */

               if (cp(src_file, idfilename) != 0)
               {
                  printmsg(0, "copy \"%s\" to \"%s\" failed",
                              src_file,
                              idfilename);

                  if (dirp != NULL )
                     closedir( dirp );

                  exit(1);
               }
            }
            else
               strcpy(idfile, "D.0");

            if ((cfile = FOPEN(icfilename, "a",TEXT_MODE)) == NULL)
            {
               printerr( icfilename );
               printf("uucp: cannot append to %s\n", icfilename);

               if (dirp != NULL )
                  closedir( dirp );

               exit(1);
            }

            fprintf(cfile, "S %s %s %s -%s %s 0644 %s\n",
                           src_file,
                           dest_file,
                           E_mailbox,
                           flags,
                           idfile,
                           remote_user);
            fclose(cfile);

            if (wild_flag)
            {
               dp = readdir(dirp);
               if ( dp == NULL )
                  write_flag = KWFalse;
            }
            else
               write_flag = KWFalse;
         }
         if (dirp != NULL )
            closedir( dirp );
         return(1);
      }
      else  {

         if (expand_path(src_file, NULL, E_homedir, NULL) == NULL)
            exit(1);

         if (expand_path(dest_file, NULL, E_homedir, NULL) == NULL)
            exit(1);

         if (strcmp(src_file, dest_file) == 0)
         {
            fprintf(stderr, "%s %s - same file; can't copy\n",
                  src_file, dest_file);
            exit(1);
         }

         cp(src_file, dest_file);
         return(1);
      }

} /* do_copy */

/*--------------------------------------------------------------------*/
/*    m a i n                                                         */
/*                                                                    */
/*    main program, of course                                         */
/*--------------------------------------------------------------------*/

main(int argc, char *argv[])
{
      int         i;
      int         option;
      char        src_system[20], dest_system[20];
      char        src_inter[20],  dest_inter[20];
      char        src_file[FILENAME_MAX],   dest_file[FILENAME_MAX];

/*--------------------------------------------------------------------*/
/*                             Initialize                             */
/*--------------------------------------------------------------------*/

      debuglevel = 0;

      banner( argv );

      if (!configure(B_UUCP))
         exit(1);

/*--------------------------------------------------------------------*/
/*                        Process option flags                        */
/*--------------------------------------------------------------------*/

      while ((option = getopt(argc, argv, "Ccdfg:jmn:rs:x:")) != EOF)
      {
         switch(option)
         {
            case 'c':               /* don't spool                    */
               spool_flag = KWFalse;
               break;

            case 'C':               /* force spool                    */
               spool_flag = KWTrue;
               break;

            case 'd':               /* make directories               */
               dir_flag = KWTrue;
               break;

            case 'f':               /* don't make directories         */
               dir_flag = KWFalse;
               break;

            case 'g':               /* set grade of transfer           */
               grade = *optarg;
               break;

            case 'j':               /* output job id to stdout        */
               j_flag = KWTrue;
               break;

            case 'm':               /* send mail when copy completed  */
               mail_me = KWTrue;
               break;

            case 'n':               /* notify remote user file was sent  */
               mail_them = KWTrue;
               sprintf(remote_user, "%.8s", optarg);
               break;

            case 'r':               /* queue job only                 */
               xeqt_flag = KWFalse;
               break;

            case 's':               /* report status of transfer to file  */
               strcpy( spool_file, optarg);
               expand_path( spool_file, NULL, E_pubdir , NULL);
               break;

            case 'x':               /* set debug level                 */
               debuglevel = atoi(optarg);
               break;

            default:
               usage();
               exit(1);
               break;

         }
      }

      flags[0] = (char)(dir_flag ? 'd' : 'f');
      flags[1] = (char)(spool_flag ? 'C' : 'c');

      i = 2;

      if (mail_them)
         flags[i++] = 'n';

      flags[i] = '\0';

      if (remote_user[0] == '\0')
      {
         /* copy the string taking care not to overrun the buffer */

         strncpy(remote_user, E_mailbox, sizeof(remote_user) - 1 );
         remote_user[sizeof(remote_user) - 1] = '\0';
      }

      if (argc - optind < 2)
      {
         usage();
         exit(1);
      }

#if defined(_Windows)
   openlog( NULL );
   atexit( CloseEasyWin );               /* Auto-close EasyWin on exit  */
#endif

/*--------------------------------------------------------------------*/
/*       Now - posibilities:                                          */
/*       Sources - 1 or more, local or 1 hop away (NOT > 1 hop!)      */
/*       Dest    - normal cp rules, single only, local, 1 hop or >1   */
/*                 hop                                                */
/*       Wildcards possible on sources.                               */
/*                                                                    */
/*    Actions depend on these - so we need to split the pathnames     */
/*    for more info.                                                  */
/*--------------------------------------------------------------------*/

      split_path(argv[argc - 1], dest_system, dest_inter, dest_file);

/*--------------------------------------------------------------------*/
/*        OK - we have a destination system - do we know him?         */
/*--------------------------------------------------------------------*/

      if (*dest_system != '\0')
      {
         if (checkreal(dest_system) == BADHOST)
         {
            fprintf(stderr, "uucp - bad system: %s\n", dest_system);
            exit(1);
         }
      }
      else        /* make sure we have a system name for destination */
         strcpy(dest_system, E_nodename);

      printmsg(9, "destination: system \"%s\", inter \"%s\", file \"%s\"",
                  dest_system,
                  dest_inter,
                  dest_file);

/*--------------------------------------------------------------------*/
/*    Now - if there is more than 1 source then normal cp rules,      */
/*          i.e. dest must be a directory                             */
/*--------------------------------------------------------------------*/

      if (argc - optind > 2)
         appendSlash(dest_file);

      for (i = optind; i < (argc - 1); i++)
      {

         split_path(argv[i], src_system, src_inter, src_file);

/*--------------------------------------------------------------------*/
/*            We need to winnow out various combinations -            */
/*                        so lets get at them                         */
/*                                                                    */
/*                   Do we know the source system?                    */
/*--------------------------------------------------------------------*/

         if (*src_system != '\0')
         {
            if (checkreal(src_system) == BADHOST)
            {
               fprintf(stderr, "uucp - bad system %s\n", src_system);
               exit(1);
            }
         }

/*--------------------------------------------------------------------*/
/*                    Source can't be >1 hop away                     */
/*--------------------------------------------------------------------*/

         if (*src_inter != '\0')
         {
            fprintf(stderr, "uucp - illegal syntax %s\n", argv[i]);
            exit(1);
         }

/*--------------------------------------------------------------------*/
/*        if source is remote AND wildcarded then we need uux         */
/*--------------------------------------------------------------------*/

         if ((*src_system != '\0') && (strcspn(src_file, "*?[") < strlen(src_file)))
         {
            do_uux(src_system, src_system, src_file, dest_system, dest_inter, dest_file);
            continue;
         }

/*--------------------------------------------------------------------*/
/*            if dest requires forwarding then we need uux            */
/*--------------------------------------------------------------------*/

         if (*dest_inter != '\0')
         {
            do_uux(dest_system, src_system, src_file, "", dest_inter, dest_file);
            continue;
         }

/*--------------------------------------------------------------------*/
/*         if both source & dest are remote then we need uux          */
/*--------------------------------------------------------------------*/

         if ((*src_system != '\0') && (!equal(src_system, E_nodename)) &&
             (*dest_system != '\0') && (!equal(dest_system, E_nodename)))
         {
            do_uux(dest_system, src_system, src_file, "", dest_inter, dest_file);
            continue;
         }

/*--------------------------------------------------------------------*/
/*          We have left 3 options:                                   */
/*          1) src remote (non-wild) & dest local                     */
/*          2) src local & dest remote 1 hop                          */
/*          3) src & dest both local                                  */
/*                                                                    */
/*               fill up the src system if not already                */
/*--------------------------------------------------------------------*/

         if (*src_system == '\0')
            strcpy(src_system, E_nodename);

         printmsg(4, "source: system \"%s\", file \"%s\"", src_system,
                     src_file);

         do_copy(src_system, src_file, dest_system, dest_file);

      }  /* for (i = optind; i < (argc - 1); i++) */

      if (xeqt_flag)
         printmsg(1, "Call uucico");

      if (j_flag)
         printmsg(1,"j_flag");

      return 0;

} /* main */

/*--------------------------------------------------------------------*/
/*       a p p p e n d  S l a s h                                     */
/*                                                                    */
/*       Insure a path name ends in a slash                           */
/*--------------------------------------------------------------------*/

static void appendSlash( char *s )
{
   size_t last = strlen( s );

   if ( last )
   {
      last--;                    /* Step back to last character */
      if ( s[last] != '/' )
         strcat( s + last, "/");
   }
   else {
      printmsg(0,"Empty string for file destination");
      panic();
   } /* else */

}  /* appendSlash */
