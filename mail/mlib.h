/*
   mlib.h


   Update History:

      01 Oct 89      Add prototypes for Console_fgets, Is_Console    ahd
      07 Sep 90      More prototypes                                 ahd
*/

extern int Get_One( void );

extern boolean Console_fgets(char *buff,
                              int buflen, char *prompt);          /* ahd   */

int Invoke_Editor(const char *ecmd,
                  const char *filename);

int L_invoke_pager(const char *ecmd,
                   const char *filename);

extern boolean Is_Console(FILE *stream);                          /* ahd   */

extern void ClearScreen( void );                                  /* ahd   */
