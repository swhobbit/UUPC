/*
   ulib.h
*/


extern boolean port_active;         /* Port active flag for error handler   */

extern int openline(char *name, BPS baud, const boolean direct);

extern unsigned int sread(char *buffer,
                          unsigned int wanted,
                          unsigned int timeout);

int swrite(const char *data, unsigned int len);

void ssendbrk(unsigned int duration);

void closeline(void);

void SIOSpeed(BPS baud);

void flowcontrol( boolean );

void hangup( void );

BPS GetSpeed( void );

boolean CD( void );
