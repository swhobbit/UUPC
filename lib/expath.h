/*--------------------------------------------------------------------*/
/*    UUPC/extended                                                   */
/*                                                                    */
/*    e x p a t h . h                                                 */
/*                                                                    */
/*    Header file for expath.c                                        */
/*--------------------------------------------------------------------*/

char *expand_path(char *path,          /* Input/output path name     */
                  const char *cur_dir, /* Default directory path     */
                  const char *home,    /* Default home directory     */
                  const char *ftype);  /* Default extension          */
