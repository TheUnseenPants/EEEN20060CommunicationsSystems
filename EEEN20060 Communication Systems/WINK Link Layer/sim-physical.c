/*  Simulated Physical Layer functions for serial port communication.
       PHY_open    initialises
       PHY_close   does nothing
       PHY_send    puts bytes into an array, with random bytes at start
       PHY_get     gets bytes from the array, adding random errors
    All functions print explanatory messages if there is
    a problem, and return values to indicate failure. */


#include <stdio.h>   // needed for printf
#include <stdlib.h>  // for random number functions
#include <time.h>    // for time function, used to seed rand
#include <windows.h>    // for Sleep function
#include "physical.h"  // header file for these functions

#define BUFSIZE 2000    // size of array to hold bytes

/* Creating a variable this way allows it to be shared
   by the functions in this file only.  */
static byte buffer[BUFSIZE];    // array to hold bytes
static int nBytesWritten = 0;   // number of bytes written to buffer
static int nBytesUsed = 0;      // number of bytes read from buffer
static int rxTimeLimit = 0;     // time limit for PHY_get()
static double rxProbErr = 0.0;  // probability of error for PHY_get()

/* PHY_open function - would open and configure the serial port.
   Arguments are port number, bit rate, number of data bits, parity,
   receive timeout constant, rx timeout interval, rx probability of error.
   See comments below for more detail on timeouts.
   Returns zero if it succeeds - anything non-zero is a problem.*/
int PHY_open(int portNum,       // port number: e.g. 1 for COM1, 5 for COM5
             int bitRate,       // bit rate: e.g. 1200, 4800, etc.
             int nDataBits,     // number of data bits: 7 or 8
             int parity,        // parity: 0 = none, 1 = odd, 2 = even
             int rxTimeConst,   // rx timeout constant in ms: 0 waits forever
             int rxTimeIntv,    // rx timeout interval in ms: 0 waits forever
             double probErr)    // rx probability of error: 0.0 for none
{
    // Set the byte counters to 0
    nBytesWritten = 0;
    nBytesUsed = 0;

    // Set up receive time limit - very rough approximation!
    rxTimeLimit = rxTimeConst + rxTimeIntv;

    /* Set up simulated errors on receive path:
       Set the seed for the random number generator,
       and check the probability of error value. */
    srand(time(NULL));  // get time and use as seed
    if ((probErr>=0.0) && (probErr<=1.0))  // check valid
        rxProbErr = probErr; // pass value to shared variable

    // In simulation, this always succeeds
    return 0;
}

//===================================================================
/* PHY_close function, would close the serial port,
    but does nothing in simulation.
   Takes no arguments, returns 0 always.  */
int PHY_close()
{
    return 0;
}

//===================================================================
/* PHY_send function, to send bytes.
   Arguments: pointer to array holding bytes to be sent;
              number of bytes to send.
   Returns number of bytes sent, or negative value on error.  */
int PHY_send(byte *dataTx, int nBytesToSend)
{
     int nBytesSent;    // number of bytes actually sent
     int i;             // used in for loops
     int threshold = 0;  // threshold for error simulation
     int flip;          // bits to change in simulating error
     byte byteTx;

    // If this is start of frame, put some random bytes in array
    if (nBytesWritten == 0)
    {
        nBytesWritten = 4 + (rand() % 16);  // number of bytes to add
        for (i=0; i<nBytesWritten; i++)
        {
            buffer[i] = rand() % 200;  // put random bytes in buffer
        }
    }

    // Check if there is room in the array
    if (nBytesWritten + nBytesToSend > BUFSIZE) // not enough room
    {
        nBytesSent = BUFSIZE - nBytesWritten;  // send what we can
        printf("PHY SIM: Buffer full\n");
    }
    else
    {
        nBytesSent = nBytesToSend;  // will send all the bytes
    }

    // Set threshold for adding errors
    if (rxProbErr != 0.0)
        // set threshold as fraction of max, scaling for 8 bit bytes
        threshold = 1 + (int)(8.0 * (double)RAND_MAX * rxProbErr);
    else
        threshold = 0;


    // Now copy the bytes into the storage array, adding errors
    for (i=0; i<nBytesSent; i++)
    {
        byteTx = dataTx[i];
        if (rand() < threshold)  // want to cause an error
        {
            flip = rand() % 8;  // random integer 0 to 7
            flip = 1 << flip; // bit pattern: single 1 in random place
            byteTx ^= (byte) flip;  // invert one bit
            printf("PHY_send: #### Simulated error... ####\n");
        }
        buffer[nBytesWritten+i] = byteTx;
    }

    nBytesWritten += nBytesSent; // update nBytesWritten

    return nBytesSent; // return number of bytes sent
}

//===================================================================
/* PHY_get function, to get received bytes.
   Arguments: pointer to array to hold received bytes;
              maximum number of bytes to get.
   Returns number of bytes actually got, or negative value on error.  */
int PHY_get(byte *dataRx, int nBytesToGet)
{
     int nBytesGot;      // number of bytes actually got
     int i;             // for use in loop
     int nBytesAvailable;

    // Check if there are bytes available
    nBytesAvailable = nBytesWritten - nBytesUsed;
    if (nBytesAvailable == 0)  // no bytes available
    {
        dataRx[0] = rand() % 256;   // get one random byte
        if (rxTimeLimit == 0) // no time limit set
            Sleep(10000);   // should wait forever!
        else Sleep(rxTimeLimit);  // if limit set, wait that long
        return 1;   // and return with just one byte
    }

    // Check if there are enough bytes waiting
    if (nBytesToGet > nBytesAvailable)  // not enough bytes
        nBytesGot = nBytesAvailable;   // will return what is available
    else
        nBytesGot = nBytesToGet;    // will return what is requested


    // Copy bytes from storage array to receive data array
    for (i = 0; i < nBytesGot; i++)
    {
        dataRx[i] = buffer[nBytesUsed+i]; // copy byte
    }

    nBytesUsed += nBytesGot;    // update the used byte counter

    // If we have used all the bytes, reset counters
    if (nBytesUsed == nBytesWritten)
    {
        nBytesWritten = 0;
        nBytesUsed = 0;
    }

    return nBytesGot; // if no problem, return number of bytes we got
}

/* Function to print informative error messages
   when something goes wrong...  - does nothing here*/
void printError(void)
{

}
