/*--------------------------------------------------------------------*/
/*    s u s p e n d . h                                               */
/*                                                                    */
/*    suspend/resume uupoll/uucico daemon                             */
/*                                                                    */
/*    Author: Kai Uwe Rommel                                          */
/*                                                                    */
/*--------------------------------------------------------------------*/

extern boolean suspend_processing;

void suspend_init(void);
int suspend_other(boolean suspend);
CONN_STATE suspend_wait(void);
