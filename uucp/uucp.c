/*--------------------------------------------------------------------*/
/*    u u c p . c                                                     */
/*                                                                    */
/*    UUCP lookalike for IBM PC.                                      */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1993 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: uucp.c 1.6 1993/08/02 03:24:59 ahd Exp $
 *
 *    Revision history:
 *    $Log: uucp.c $
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

#include  <ctype.h>
#include  <direct.h>
#include  <fcntl.h>
#include  <io.h>
#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  <sys/types.h>
#include  <sys/stat.h>

#ifdef _Windows
#include <windows.h>
#endif

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include  "lib.h"
#include  "expath.h"
#include  "getopt.h"
#include  "getseq.h"
#include  "hlib.h"
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

static boolean       spool_flag = FALSE;
static char          spool_file[FILENAME_MAX]; /* alt spool file name */
static boolean       dir_flag = TRUE;
static boolean       xeqt_flag = TRUE;    /* Triggered by -r option */
static char          grade = 'n';         /* Default grade of service */
static boolean       mail_me = FALSE;     /* changes with -m */
static boolean       mail_them = FALSE;   /* changes with -n */
static char  remote_user[10];             /* user to mail with -n */
static char  *destn_file;
static char  flags[16];

currentfile();

/*--------------------------------------------------------------------*/
/*    u s a g e                                                       */
/*                                                                    */
/*    Report flags used by program                                    */
/*--------------------------------------------------------------------*/

static         void    usage(void)
{
      fprintf(stderr, "Usage: uucp\t[-c|-C] [-d|-f] [-gGRADE] [-j] [-m] [-nUSER] [-r] [-sFILE]\\\n\
\t\t[-xDEBUG_LEVEL] source-files destination-file\n");
}

/*--------------------------------------------------------------------*/
/*    c p                                                             */
/*                                                                    */
/*    Copy a file                                                     */
/*--------------------------------------------------------------------*/

static int cp(char *from, char *to)
{
      int         fd_from, fd_to;
      int         nr, nw = -1;
      char        buf[BUFSIZ*4]; /* faster if we alloc a big buffer */

      if ((fd_from = open(from, O_RDONLY | O_BINARY)) == -1)
         return(1);        /* failed */
      /* what if the to is a directory? */
      /* possible with local source & dest uucp */
      if ((fd_to = open(to, O_CREAT | O_BINARY | O_WRONLY, S_IWRITE | S_IREAD)) == -1) {
         close(fd_from);
         return(1);        /* failed */
         /* NOTE - this assumes all the required directories exist!  */
      }
      while  ((nr = read(fd_from, buf, sizeof buf)) > 0 &&
         (nw = write(fd_to, buf, nr)) == nr)
         ;
      close(fd_to);
      close(fd_from);
      if (nr != 0 || nw == -1)
         return(1);        /* failed in copy */
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

      *system = *inter = *file = '\0';    /* init to nothing */
      for (p = path;; p = p_left + 1)  {
         p_left = strchr(p, '!');         /* look for the first bang */
         if (p_left == NULL)  {           /* not a remote path */
            strcpy(file, p);              /* so just return filename */
            return;
         }
         /* now check if the system was in fact us.
       If so strip it and restart */
         if (equaln(E_nodename, p, p_left - p) &&
            (E_nodename[p_left - p] == '\0'))
            continue;

         p_right = strrchr(p, '!');      /* look for the last bang */
         strcpy(file, p_right + 1);      /* and thats our filename */
         strncpy(system, p, p_left - p); /* and we have a system thats not us */
         system[p_left - p] = '\0';
         /* now see if there is an intermediate path */
         if (p_left != p_right)  {        /* yup - there is */
            strncpy(inter, p_left + 1, p_right - p_left - 1);
            inter[p_right - p_left - 1] = '\0';
         }
         return;                 /* and we're done */
      }        /* never get here :-)  */
}

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
      char        xcmd[BUFSIZ];        /* buffer for assembling the UUX command */
      char        *ex_flg;

/*--------------------------------------------------------------------*/
/*                 First - lets get the basic command                 */
/*--------------------------------------------------------------------*/

      ex_flg = xeqt_flag ? "" : "-r";
      sprintf(xcmd, "-C %s %s!uucp -C ", ex_flg, remote);
                              /* but what about mailing the guy? */

/*--------------------------------------------------------------------*/
/*                  Now we sort out the source name                   */
/*--------------------------------------------------------------------*/

      if ((*src_syst == '\0') || equal(src_syst, E_nodename))
         sprintf(xcmd + strlen(xcmd), " !%s ", src_file);
      else  {
         if (!equal(remote, src_syst))
            sprintf(xcmd + strlen(xcmd), " (%s!%s) ", src_syst, src_file);
         else
            sprintf(xcmd + strlen(xcmd), " (%s) ", src_file);
      } /* else */

/*--------------------------------------------------------------------*/
/*                   Now to do the destination name                   */
/*--------------------------------------------------------------------*/

      if (*dest_inter != '\0')  {
         if (*dest_syst != '\0')
            sprintf(xcmd + strlen(xcmd), " (%s!%s!%s) ", dest_syst, dest_inter, dest_file);
         else
            sprintf(xcmd + strlen(xcmd), " (%s!%s) ", dest_inter, dest_file);
      }
      else  {
         if ((*dest_syst == '\0') || equal(dest_syst, E_nodename))
            sprintf(xcmd + strlen(xcmd), " (%s!%s) ", E_nodename, dest_file);
      }
      printmsg(2, "xcmd: %s", xcmd);

/*--------------------------------------------------------------------*/
/*                              OK - GO!                              */
/*--------------------------------------------------------------------*/

   execute("uux", xcmd, NULL, NULL, FALSE, TRUE);

   return(1);

} /* do_uux */

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
      boolean wild_flag = FALSE;
      boolean write_flag;
      char        tmfile[15];       /* Unix style name for c file */
      char        idfile[15];       /* Unix style name for data file copy */
      char        work[FILENAME_MAX];   /* temp area for filename hacking */
      char        search_file[FILENAME_MAX];
      char        source_path[FILENAME_MAX];
      char        icfilename[FILENAME_MAX]; /* our hacked c file path */
      char        idfilename[FILENAME_MAX]; /* our hacked d file path */

      struct  stat    statbuf;
      DIR *dirp = NULL;
      struct direct *dp = NULL;
      char subseq = 'A';

      long    int     sequence;
      char    *remote_syst;   /* Non-local system in copy            */
      char    *sequence_s;
      FILE        *cfile;
      static  char    *spool_fmt = SPOOLFMT;

      sequence = getseq();
      sequence_s = JobNumber( sequence );
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
         if ((cfile = FOPEN(icfilename, "a",TEXT_MODE )) == NULL)  {
            printerr( icfilename );
            fprintf(stderr, "uucp: cannot append to %s\n", icfilename);
            panic();
         }
         fprintf(cfile, "R %s %s %s -%s %s 0777 %s\n", src_file, dest_file,
               E_mailbox, flags, *spool_file ? spool_file : "dummy", remote_user);
         fclose(cfile);
         return(1);
      }
      else if (!equal(dest_syst, E_nodename))  {
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
            wild_flag = FALSE;

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
            wild_flag = TRUE;
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

         write_flag = TRUE;

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
            if (spool_flag)  {
               sprintf(idfile , spool_fmt, 'D', E_nodename, (char) subseq++,
                           sequence_s);
               importpath(work, idfile, remote_syst);
               mkfilename(idfilename, E_spooldir, work);
               /* Do we need a MKDIR here for the system? */
               if (cp(src_file, idfilename) != 0)  {
                  printmsg(0, "copy \"%s\" to \"%s\" failed",
                     src_file, idfilename);           /* copy data */
                  closedir( dirp );
                  exit(1);
               }
            }
            else
               strcpy(idfile, "D.0");
            if ((cfile = FOPEN(icfilename, "a",TEXT_MODE)) == NULL)  {
               printerr( icfilename );
               printf("uucp: cannot append to %s\n", icfilename);
               if (dirp != NULL )
                  closedir( dirp );
               exit(1);
            }
            fprintf(cfile, "S %s %s %s -%s %s 0666 %s\n", src_file, dest_file,
                     E_mailbox, flags, idfile, remote_user);
            fclose(cfile);
            if (wild_flag)  {
               dp = readdir(dirp);
               if ( dp == NULL )
                  write_flag = FALSE;
            }
            else
               write_flag = FALSE;
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
}

/*--------------------------------------------------------------------*/
/*    m a i n                                                         */
/*                                                                    */
/*    main program, of course                                         */
/*--------------------------------------------------------------------*/

void  main(int argc, char *argv[])
{
      int         i;
      int         option;
      boolean j_flag = FALSE;
      char        src_system[100], dest_system[100];
      char        src_inter[100],  dest_inter[100];
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

      while ((option = getopt(argc, argv, "Ccdfg:jmn:rs:x:")) != EOF)  {
         switch(option)  {
            case 'c':               /* don't spool */
               spool_flag = FALSE;
               break;
            case 'C':               /* force spool */
               spool_flag = TRUE;
               break;
            case 'd':               /* make directories */
               dir_flag = TRUE;
               break;
            case 'e':               /* send uucp command to sys */
               /* This one is in Sams but nowhere else - I'm ignoring it */
               break;
            case 'f':               /* don't make directories */
               dir_flag = FALSE;
               break;
            case 'g':               /* set grade of transfer */
               grade = *optarg;
               break;
            case 'j':               /* output job id to stdout */
               j_flag = TRUE;
               break;
            case 'm':               /* send mail when copy completed */
               mail_me = TRUE;
               break;
            case 'n':               /* notify remote user file was sent */
               mail_them = TRUE;
               sprintf(remote_user, "%.8s", optarg);
               break;
            case 'r':               /* queue job only */
               xeqt_flag = FALSE;
               break;
            case 's':               /* report status of transfer to file */
               strcpy( spool_file, optarg);
               expand_path( spool_file, NULL, E_pubdir , NULL);
               break;
            case 'x':               /* set debug level */
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
   atexit( CloseEasyWin );               // Auto-close EasyWin on exit
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

      if (*dest_system != '\0')  {
         if (checkreal(dest_system) == BADHOST)  {
            fprintf(stderr, "uucp - bad system: %s\n", dest_system);
            exit(1);
         }
      }
      else        /* make sure we have a system name for destination */
         strcpy(dest_system, E_nodename);
      printmsg(9, "destination: system \"%s\", inter \"%s\", file \"%s\"",
            dest_system, dest_inter, dest_file);

/*--------------------------------------------------------------------*/
/*    Now - if there is more than 1 source then normal cp rules,      */
/*          i.e. dest must be a directory                             */
/*--------------------------------------------------------------------*/

      if (argc - optind > 2)
         strcat(dest_file, "/");
      destn_file = argv[argc - 1];
      for (i = optind; i < (argc - 1); i++)  {
         split_path(argv[i], src_system, src_inter, src_file);

/*--------------------------------------------------------------------*/
/*            We need to winnow out various combinations -            */
/*                        so lets get at them                         */
/*                                                                    */
/*                   Do we know the source system?                    */
/*--------------------------------------------------------------------*/

         if (*src_system != '\0')  {
            if (checkreal(src_system) == BADHOST)  {
               fprintf(stderr, "uucp - bad system %s\n", src_system);
               exit(1);
            }
         }

/*--------------------------------------------------------------------*/
/*                    Source can't be >1 hop away                     */
/*--------------------------------------------------------------------*/

         if (*src_inter != '\0')  {
            fprintf(stderr, "uucp - illegal syntax %s\n", argv[i]);
            exit(1);
         }

/*--------------------------------------------------------------------*/
/*        if source is remote AND wildcarded then we need uux         */
/*--------------------------------------------------------------------*/

         if ((*src_system != '\0') && (strcspn(src_file, "*?[") < strlen(src_file)))  {
            do_uux(src_system, src_system, src_file, dest_system, dest_inter, dest_file);
            continue;
         }

/*--------------------------------------------------------------------*/
/*            if dest requires forwarding then we need uux            */
/*--------------------------------------------------------------------*/

         if (*dest_inter != '\0')  {
            do_uux(dest_system, src_system, src_file, "", dest_inter, dest_file);
            continue;
         }

/*--------------------------------------------------------------------*/
/*         if both source & dest are remote then we need uux          */
/*--------------------------------------------------------------------*/

         if ((*src_system != '\0') && (!equal(src_system, E_nodename)) &&
               (*dest_system != '\0') && (!equal(dest_system, E_nodename)))  {
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
      }
      if (xeqt_flag)
         printmsg(1, "Call uucico");
      if (j_flag)
         printmsg(1,"j_flag");
      exit(0);
}
