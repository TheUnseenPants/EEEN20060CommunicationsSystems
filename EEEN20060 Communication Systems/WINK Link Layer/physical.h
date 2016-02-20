#ifndef PHYSICAL_H_INCLUDED
#define PHYSICAL_H_INCLUDED

/*  Physical Layer functions using serial port.
       PHY_open        opens and configures the port
       PHY_close       closes the port
       PHY_send        sends bytes
       PHY_receive     gets received bytes
    All functions print explanatory messages if there is
    a problem, and return values to indicate failure. */

/* PHY_open function - to open and configure the serial port.
   Arguments are port number, bit rate, number of data bits, parity,
   receive timeout constant, rx timeout interval, rx probability of error.
   See comments in function for more details of timeouts.
   Returns zero if it succeeds - anything non-zero is a problem.*/
int PHY_open(int portNum,       // port number: e.g. 1 for COM1, 5 for COM5
             int bitRate,       // bit rate: e.g. 1200, 4800, etc.
             int nDataBits,     // number of data bits: 7 or 8
             int parity,        // parity: 0 = none, 1 = odd, 2 = even
             int rxTimeConst,   // rx timeout constant in ms: 0 waits forever
             int rxTimeIntv,    // rx timeout interval in ms: 0 waits forever
             double probErr);   // rx probability of error: 0.0 for none

/* PHY_close function, to close the serial port.
   Takes no arguments, returns 0 always.  */
int PHY_close();

/* PHY_send function, to send bytes.
   Arguments: pointer to array holding bytes to be sent;
              number of bytes to send.
   Returns number of bytes sent, or negative value on error.  */
int PHY_send(byte *dataTx, int nBytesToSend);

/* PHY_get function, to get received bytes.
   Arguments: pointer to array to hold received bytes;
              maximum number of bytes to get.
   Returns number of bytes actually got, or negative value on error. */
int PHY_get(byte *dataRx, int nBytesToGet);

/* Function to print informative error messages
   when something goes wrong...  */
void printError(void);

#endif // PHYSICAL_H_INCLUDED
