/*
      UUPC/Extended host dependent functions header

      Change history:

      08 Sep 90   -  Created from local\host.h                    ahd
  */

#define SAME 0

#define SFILENAME "SEQF"
#define DOTFORWARD "forward"

#define SEPCHAR '/'

#define CONSOLE "con"   /* "filename" of the console */

#define NAME "NAME"
#define NODENAME "NODENAME"

#define TEXT_MODE    "t"
#define BINARY_MODE  "b"

#define MAXADDR   128            /* Max length of address strings      ahd */
#define MAXLIST   99             /* Max number of entries in arg
                                    arrays                                 */

void mkfilename(char *pathname,
                const char *path,
                const char *name);                             /* ahd   */

char *mktempname( char *input,
                  char *extension);                            /* ahd   */

char *mkmailbox(char *buf, const char *userid);

#ifdef __GNUC__
#define filebkup(x) \
        printmsg(0,"Cannot backup %s in GNU C yet ... yell at Drew", x)
#else
int filebkup( const char *input );
#endif
