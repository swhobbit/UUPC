/*

   getopt.h

   Update history:

      01Oct89 Add function prototype for getopt                      ahd
  */

#define getopt(x,y,z)   GETOPT(x,y,z)  /* Avoid GNU conflict   */
extern int getopt(int argc, char **argv, char *opts);

extern int optind;
extern char *optarg;
