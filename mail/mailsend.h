/*--------------------------------------------------------------------*/
/* m a i l s e n d . h                                                */
/*                                                                    */
/* Header file for UUPC functions for sending mail                    */
/*                                                                    */
/* CHange history:                                                    */
/*                                                                    */
/*    15 Sep 1990   Created                                       ahd */
/*--------------------------------------------------------------------*/

boolean Send_Mail(FILE *datain,
               int argc,
               char *argv[],
               char *subject,
               const boolean resent);

boolean Collect_Mail(FILE *handle,
                  int argc,
                  char **argv,
                  const int current_msg,
                  const boolean reply);
