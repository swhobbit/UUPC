/*
** declarations for comm.asm
**
** compilation must use the Ze switch to enable the
** "far" keyword for the small memory model
**
** Robin Rohlicek   3/86
** skl@van-bc.UUCP    5/88 (more prototypes and com_errors fix)
*/

void far select_port(int); /* select active port (1 or 2) */

void far save_com(void);   /* save the interupt vectors */

void far restore_com(void);   /* restore those vectors */

int far install_com(void); /* install our vectors */

void far open_com(         /* open com port */
   int,  /* baud */
   int,  /* 'M'odem or 'D'irect */
   int,  /* Parity 'N'one, 'O'dd, 'E'ven, 'S'pace, 'M'ark */
   int,  /* stop bits (1 or 2) */
   int); /* Xon/Xoff 'E'nable, 'D'isable */

void far ioctl_com( int flags, int operand, ...);

void far close_com(void);  /* close com port */

void far dtr_off(void);    /* clear DTR */

void far dtr_on(void);     /* set DTR */

long far r_count(void);    /* receive counts */
   /* high word = total size of receive buffer */
   /* low word = number of pending chars */
#define r_count_size() ((int)(r_count() >> 16))
#define r_count_pending() ((int)r_count())

int far receive_com(void); /* get one character */
   /* return -1 if none available */

long far s_count(void);    /* send counts */
   /* high word = total size of transmit buffer */
   /* low word = number of bytes free in transmit buffer */
#define s_count_size() ((int)(s_count() >> 16))
#define s_count_free() ((int)s_count())

void far send_com(int);    /* send a character */

void far send_local(int);  /* simulate receive of char */

void far sendi_com(int);   /* send immediately */


void far break_com(void);  /* send a BREAK */

int far * far com_errors(void);  /* return far pointer to error counts
                                    (in static area) */
#define COM_EOVFLOW 0   /* buffer overflows */
#define COM_EOVRUN  1   /* receive overruns */
#define COM_EBREAK  2   /* break chars */
#define COM_EFRAME  3   /* framing errors */
#define COM_EPARITY 4   /* parity errors */
#define COM_EXMIT   5   /* transmit erros */
#define COM_EDSR    6   /* data set ready errors */
#define COM_ECTS    7   /* clear to send errors */


/*--------------------------------------------------------------------*/
/*                    Macros based on modem status                    */
/*--------------------------------------------------------------------*/

int far modem_status( void ); /* Return current modem status         */

#define is_dsr_high() ( modem_status() & MDM_DSR )
#define is_cts_high() ( modem_status() & MDM_CTS )
#define is_cd_high()  ( modem_status() & MDM_CD  )

/*--------------------------------------------------------------------*/
/*                   Bits returned by modem status                    */
/*--------------------------------------------------------------------*/

#define MDM_CD    0x80        /* CD  (Carrier Detect)                */
#define MDM_RI    0x40        /* RI  (Ring Indicator)                */
#define MDM_DSR   0x20        /* DSR (Data Set Ready)                */
#define MDM_CTS   0x10        /* CTS (Clear to Send)                 */
#define MDM_CDC   0x08        /* Delta Carrier Detect  (CD changed)  */
#define MDM_TRI   0x04        /* Trailing edge of RI   (RI went OFF) */
#define MDM_DSRC  0x02        /* Delta DSR             (DSR changed) */
#define MDM_CTSC  0x01        /* Delta CTS             (CTS changed) */
