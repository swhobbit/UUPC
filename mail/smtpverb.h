#ifndef _SMTPVERB_H
#define _SMTPVERB_H

#include "smtpclnt.h"

typedef enum
{
   SR_TE_SHUTDOWN,
   SR_PE_NOTIMPL
} SR_VERB;

KWBoolean SMTPResponse( SMTPClient *client, int code, const char *text );

KWBoolean SMTPRead( SMTPClient *client,
                    char *buf, size_t size,
                    unsigned int timeout);

time_t getModeTimeout( SMTPMode );

#endif /* _SMTPVERB_H */
