/*--------------------------------------------------------------------*/
/*    c a t c h e r . h                                               */
/*                                                                    */
/*    Ctrl-Break handler prototypes for UUPC/extended                 */
/*--------------------------------------------------------------------*/

void ctrlchandler( void );

extern boolean terminate_processing;
extern boolean norecovery;
extern boolean interactive_processing;
extern int panic_rc;
