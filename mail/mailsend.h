/*--------------------------------------------------------------------*/
/* m a i l s e n d . h                                                */
/*                                                                    */
/* Header file for UUPC functions for sending mail                    */
/*                                                                    */
/* CHange history:                                                    */
/*                                                                    */
/*    15 Sep 1990   Created                                       ahd */
/*--------------------------------------------------------------------*/

KWBoolean Send_Mail(FILE *datain,
               int argc,
               char *argv[],
               char *subject,
               const KWBoolean resent);

KWBoolean Collect_Mail(FILE *handle,
                  int argc,
                  char **argv,
                  const int current_msg,
                  const KWBoolean reply);
