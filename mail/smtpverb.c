/*--------------------------------------------------------------------*/
/*       s m t p v e r b . c                                          */
/*                                                                    */
/*       SMTP verb parser for UUPC/extended                           */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1997 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: uusmtpd.c 1.1 1997/05/20 03:55:46 ahd v1-12s $
 *
 *    $Log: uusmtpd.c $
 *    Revision 1.1  1997/05/20 03:55:46  ahd
 *    Initial revision
 *
 */

/*--------------------------------------------------------------------*/
/*                         Standard includes                          */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"
#include "../uucico/commlib.h"

#include "smtpverb.h"

/*--------------------------------------------------------------------*/
/*                      Global defines/variables                      */
/*--------------------------------------------------------------------*/

RCSID("$Id$");

currentfile();

/*--------------------------------------------------------------------*/
/*       S M T P R e s p o n s e                                      */
/*                                                                    */
/*       Send a response to a remote client                           */
/*--------------------------------------------------------------------*/

KWBoolean
SMTPResponse( SMTPClient *client, int code, const char *text )
{
  char buf[5];

  sprintf( buf, "%03.3d%c", code < 0 ? - code : code,
                        code < 0 ? '-' : ' ' );


  printmsg( 2,">>> %s%.75s", buf, text);

  if( !swrite( buf, strlen(buf)) )
  {
     printmsg(0,"Error sending response code to remote host");
     return KWFalse;
  }

  if( !swrite( text, strlen(text)) )
  {
     printmsg(0,"Error sending response text to remote host");
     return KWFalse;
  }

  if( !swrite( "\r\n", 2) == EOF)
  {
    printmsg(0, "Error sending CR/LF to remote host" );
    return KWFalse;
  }

  return KWTrue;

} /* SMTPResponse */

/*--------------------------------------------------------------------*/
/*       S M T P R e a d                                              */
/*                                                                    */
/*       Read an SMTP command from a remote system                    */
/*--------------------------------------------------------------------*/

KWBoolean
SMTPRead( SMTPClient *client, char *buf, size_t len, unsigned int timeout)
{
   size_t i;

  for( i=0; i <(len-1); i++)
  {
    if ( ! sread( buf + i, 1, timeout ))
    {
       printmsg( 0, "SMTPRead: Read timeout after %d seconds",
                 timeout);
       return KWFalse;
    }

    /* After we have at least two characters, check for CR/LF pair */
    if ( i && ! memcmp( buf + i - 1, "\r\n", 2 ))
    {
        buf[i-1] = '\0';
        return KWTrue;
    }
  }

  buf[len - 1] = '\0';
  return KWTrue;

} /* SMTPRead */

/*--------------------------------------------------------------------*/
/*       g e t M o d e T i m e o u t                                  */
/*                                                                    */
/*       Determine timeout for specified client mode                  */
/*--------------------------------------------------------------------*/

time_t
getModeTimeout( SMTPMode time )
{
   return 10;
}
