
void mult(unsigned char *number,
      const unsigned range,
      const unsigned digits);

void add(unsigned char *number,
      const unsigned range,
      const unsigned digits);

boolean adiv( unsigned char *number,
             const unsigned divisor,
                   unsigned *remain,
             const unsigned digits);

#define MAX_DIGITS 20         /* Number of digits for arb math */
