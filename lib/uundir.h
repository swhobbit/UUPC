/*
   ndir.h for MS-DOS by Samuel Lam <skl@van-bc.UUCP>, June/87
*/

#define MSDOS_MAXNAMLEN 12
#define MAXNAMLEN MSDOS_MAXNAMLEN

struct direct {
   long d_ino;
   short d_reclen;
   short d_namlen;
   char d_name[MAXNAMLEN + 1];
};

typedef struct {
   char filereserved[21];
   char fileattr;
   int  filetime, filedate;
   long filesize;
   char filename[MSDOS_MAXNAMLEN + 1];
} DTA;

typedef struct {
   char dirid[4];
   struct direct dirent;
   DTA dirdta;
   int dirfirst;
} DIR;

extern DIR *opendirx(const char *dirname, char *pattern);

#define opendir(x) opendirx(x, "*.*")

extern struct direct *readdir(DIR *dirp);
extern void closedir(DIR *dirp);
