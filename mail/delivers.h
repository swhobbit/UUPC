#ifndef _DELIVERS_H
#define _DELIVERS_H
#define _DELIVERS_H

int ConnectSMTP(
   IMFILE *imf,                     /* Temporary input file          */
   const char *relay,               /* SMTP host to connect to       */
   const char *fromAddress,         /* Originating (error) address   */
   const char **toAddress,          /* List of target addressess     */
   int   count,                     /* Number of addresses           */
   const KWBoolean validate         /* Perform bounce on failure     */
);

#endif /* _DELIVERS_H */
