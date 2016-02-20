/* Functions to implement link layer protocol - simple versions
	can only detect errors, no acknowledgements or re-transmission:
   LL_connect() connects to another computer;
   LL_discon()  disconnects;
   LL_send()    sends a block of data;
   LL_receive() waits to receive a block of data.
   All functions take a debug argument - if 1, they print
   messages explaining what is happening.
   Regardless of debug, functions print messages on errors.
   All functions return negative values on error or failure.
   Definitions of constants are in the header file.  */

typedef unsigned char byte;

#include <stdio.h>      // input-output library: print & file operations
#include <time.h>       // for timing functions
#include "physical.h"   // physical layer functions
#include "linklayer.h"  // these functions

static int seqNumTx;        // transmit frame sequence number
static int connected = 0;   // keep track of state of connection
static int framesSent = 0;      // count of frames sent
static int badFrames = 0;       // count of bad frames received
static int goodFrames = 0;      // count of good frames received
static int timeouts = 0;        // count of timeouts
static long timerRx;            // time value for timeouts

// ===========================================================================
/* Function to connect to another computer.
   It just calls PHY_open() and reports any error.
   It also initialises counters for debug purposes.  */
int LL_connect(int debug)
{
    // Try to connect - set suitable parameters here...
    int retCode = PHY_open(1,4800,8,0,1000,50,PROB_ERR);
    if (retCode == 0)   // check if succeeded
    {
        connected = 1;      // record that we are connected
        seqNumTx = 0;       // set first sequence number
        framesSent = 0;     // initialise counters for debug
        badFrames = 0;
        goodFrames = 0;
        timeouts = 0;
        if (debug) printf("LL: Connected\n");
        return 0;
    }
    else  // failed
    {
        connected = 0;  // record lack of connection
        printf("LL: Failed to connect, PHY returned code %d\n",retCode);
        return -retCode;  // return negative error code
    }
}


// ===========================================================================
/* Function to disconnect from other computer.
   It just calls PHY_close() and prints debug info.  */
int LL_discon(int debug)
{
    int retCode = PHY_close();  // try to disconnect
    connected = 0;  // assume no longer connected
    if (retCode == 0)   // check if succeeded
    {
        if (debug) // print all the counters
        {
            printf("LL: Disconnected.  Sent %d data frames\n", framesSent);
            printf("LL: Received %d good and %d bad frames, had %d timeouts\n",
                   goodFrames, badFrames, timeouts);
        }
        return 0;

    }
    else  // failed
    {
        printf("LL: Failed to disconnect, PHY returned code %d\n", retCode);
        return -retCode;  // return negative error code
    }
}


// ===========================================================================
/* Function to send a block of data in a frame.
   Arguments:  Data block as array of bytes,
               number of bytes to send, debug.
   Return value is 0 on success, negative on failure.
   If connected, builds a frame, then sends the frame using PHY_send.
   This simple version does nothing else...  */
int LL_send(byte *dataTx, int nData, int debug)
{
    byte frameTx[3*MAX_BLK];  // array large enough for frame
    int nFrame = 0;           // size of frame
    int retVal;  // return value from other functions

    // First check if connected
    if (connected == 0)
    {
        printf("LL: Attempt to send while not connected\n");
        return -10;  // error code
    }

    // Then check if block size OK - adjust limit for your design
    if (nData > MAX_BLK)
    {
        printf("LL: Cannot send block of %d bytes, max %d\n", nData, MAX_BLK);
        return -11;  // error code
    }

    // Build the frame
    nFrame = buildDataFrame(frameTx, dataTx, nData, seqNumTx);

    // Send the frame, then check for problems
    retVal = PHY_send(frameTx, nFrame);  // send frame bytes
    if (retVal != nFrame)  // problem!
    {
        printf("LL: Block %d, failed to send frame\n", seqNumTx);
        return -12;  // error code
    }
    if (debug) printf("LL: Sent frame %d bytes, block %d\n",
                      nFrame, seqNumTx);

    framesSent++;  // increment frame counter (for debug)
    seqNumTx = next(seqNumTx);  // increment sequence number
    return 0;

}  // end of LL_send


// ===========================================================================
/* Function to receive a frame and extract a block of data.
   Arguments:  array to hold data block,
               max size of data block.
   Return value is actual size of data block, or negative on error.
   If connected, tries to get frame from received bytes.
   If frame found, check if good frame.
   If good, extract data and return number of data bytes.
   If bad, return with error code.  */
int LL_receive(byte *dataRx, int maxData, int debug)
{
    byte frameRx[3*MAX_BLK];  // create array to hold frame
    int nData = 0;  // number of data bytes received
    int nFrame = 0;  // number of bytes in frame received
    int seqNumRx = 0;  // sequence number of received frame
    int i;  // for use in loop

    // First check if connected
    if (connected == 0)
    {
        printf("LL: Attempt to receive while not connected\n");
        return -10;  // error code
    }

    // First get a frame, up to maximum size of array.
    // Function returns number of bytes in frame, or negative if error
    nFrame = getFrame(frameRx, 3*MAX_BLK, RX_WAIT);
    if (nFrame < 0)  // some problem receiving
    {
        return -9;  // quit if error
    }
    if (nFrame == 0)  // timeout
    {
        printf("LL: Timeout trying to receive frame\n");
        timeouts++; // increment timeout counter
        return -5;  // report this as an error for now
    }

    // If we get this far, we have received a frame
    if (debug) printf("LL: Got frame, %d bytes\n", nFrame);

    // Next step is to check it for errors
    if (checkFrame(frameRx, nFrame) == 0 ) // frame is bad
    {
        if (debug) printf("LL: Bad frame received\n");
        printFrame(frameRx, nFrame);
        badFrames++;  // increment bad frame counter
        for (i=0; i<10; i++) dataRx[i] = 35; // # symbol
        return 10;  // return 10 dummy bytes for error
    }
    else  // we have a good frame - process it
    {
        if (debug) printf("LL: Good frame received\n");
        goodFrames++;  // increment good frame counter
        nData = processFrame(frameRx, nFrame,
                             dataRx, maxData, &seqNumRx);
        if (debug) printf("LL: Received block %d with %d data bytes\n",
                          seqNumRx, nData);
        return nData;   // return number of data bytes
    }  // end of received frame processing
}  // end of LL_receive


// ===========================================================================
/* Function to build a frame from a block of data.
   This function puts the header bytes into the frame,
   then copies in the data bytes.  Then it adds the trailer bytes.
   It keeps track of the total number of bytes in the frame,
   and returns this value to the calling function.
   Arguments: array to hold frame,
              array of data,
              number of data bytes to be sent,
              sequence number to include in header.
   Return value is number of bytes in the frame.  */
int buildDataFrame(byte *frameTx, byte *dataTx, int nData, int seq)
{
    int i = 0;  // for use in loop

    int checkSum = 0;

    // Build the header
    frameTx[0] = STARTBYTE;  // start of frame marker
    frameTx[BYTECOUNTPOS] = (byte) (HEADERSIZE+nData+TRAILERSIZE);
    frameTx[SEQNUMPOS] = (byte) seq;  // sequence number


    // Copy data bytes into frame
    for (i = 0; i < (nData); i++)
    {
        frameTx[i + HEADERSIZE] = dataTx[i];  // copy the data byte
		checkSum += (int) dataTx[i];
    }

	checkSum = (256 - (checkSum%256))%256;

    frameTx[HEADERSIZE+nData] = (byte) checkSum;



    // Build the trailer - just end marker for now
    frameTx[HEADERSIZE+nData + 1] = ENDBYTE; // end of frame marker byte

    // Return the size of the frame
    return HEADERSIZE+nData+TRAILERSIZE;
}


// ===========================================================================
/* Function to get a frame from the received bytes.
   Arguments: pointer to array of bytes to hold frame,
              maximum number of bytes to receive,
              time limit for receiving those bytes.
   Return value is number of bytes recovered, or negative if error. */
int getFrame(byte *frameRx, int maxSize, float timeLimit)
{
    int nRx = 0;  // number of bytes received so far
    int retVal = 0;  // return value from other functions
    int byteCount;

    timerRx = timeSet(timeLimit);  // set time limit to wait for frame

    // First search for the start of frame marker
    do
    {
        retVal = PHY_get(frameRx, 1); // get one byte at a time
        // Return value is number of bytes received, or negative for error
        if (retVal < 0) return retVal;  // check for error and give up
     }
    while (((retVal < 1) || (frameRx[0] != STARTBYTE)) && !timeUp(timerRx));
    // until we get a byte which is start of frame marker, or timeout

    /*
        Get one more byte to find out how many more bytes are needed.
    */
    retVal = PHY_get((frameRx + 1), 1); // get one byte at a time
    // Return value is number of bytes received, or negative for error
    if (retVal < 0) return retVal;  // check for error and give up

    byteCount = (int) frameRx[BYTECOUNTPOS];


    // If we are out of time, return 0 - no useful bytes received
    if (timeUp(timerRx))
    {
        printf("LLGF: Timeout with %d bytes received\n", nRx);
        return 0;
    }

    // If still within time limit, collect more bytes, until end marker
    nRx = 2;  // got 1 byte already
    do
    {
            PHY_get((frameRx + nRx), 1);  // get one byte at a time
        if (retVal < 0) return retVal;  // check for error and give up
        else nRx += retVal;  // update the bytes received count
     }
    while ((nRx < byteCount) && !timeUp(timerRx));
    // until we get end of frame marker or timeout

    // If time up, no end marker, so bad frame, return 0
    if (timeUp(timerRx))
    {
        printf("LLGF: Timeout with %d bytes received\n", nRx);
        return 0;
    }

    // Otherwise, we found the end marker
    return nRx;  // return number of bytes in frame
}  // end of getFrame


// ===========================================================================
/* Function to check a received frame for errors.
   Arguments: pointer to array of bytes holding frame,
              number of bytes in frame.
   As a minimum, should check error detecting code.
   This example also checks start and end markers.
   Returns 1 if frame is good, 0 otherwise.   */
int checkFrame(byte *frameRx, int nFrame)
{
    int i;
    int nData = nFrame -(HEADERSIZE + TRAILERSIZE);
    int checkSum = 0;


    if (frameRx[0] != STARTBYTE)  // check start merker
    {
        printf("LLCF: Frame bad - start marker\n");
        return 0;
    }

    // Check the end-of-frame marker
    if (frameRx[nFrame-1] != ENDBYTE)
    {
        printf("LLCF: Frame bad - end marker\n");
        return 0;
    }

	// Need to check the error-detecting code here
	// return 0 if the check fails...
	for(i = HEADERSIZE; i < (HEADERSIZE + nData + 1); i++) {
        checkSum += frameRx[i];
	}



	if((checkSum%256)  != 0) {
        printf("LLCF: Frame bad - checksum failed\n");
        return 0;
	}

	if(nFrame != frameRx[BYTECOUNTPOS]) {
        printf("LLCF: Frame bad - byte count failed\n");
        return 0;

	}

    // If all tests passed, return 1
    return 1;
}  // end of checkFrame


// ===========================================================================
/* Function to process a received frame, to extract data.
   Frame has already been checked for errors, so this simple
   implementation assumes everything is where is should be.
   Arguments: pointer to array holding frame,
              number of bytes in the frame,
              pointer to array to hold data bytes,
              max number of bytes to extract,
              pointer to sequence number.
   Return value is number of bytes extracted. */
int processFrame(byte *frameRx, int nFrame,
                 byte *dataRx, int maxData, int *seqNum)
{
    int i = 0;  // for use in loop
    int nData;  // number of data bytes in frame

    // First get sequence number from its place in header
    *seqNum = frameRx[SEQNUMPOS];

    // Calculate number of data bytes, based on frame size
    nData = nFrame - HEADERSIZE - TRAILERSIZE;
    if (nData > maxData) nData = maxData;  // safety check

    // Now copy data bytes from middle of frame
    for (i = 0; i < nData; i++)
    {
        dataRx[i] = frameRx[HEADERSIZE + i];  // copy each byte
    }

    return nData;  // return size of block extracted
}  // end of processFrame


// ===========================================================================
/* Function to send an acknowledgement - this version does nothing. */
int sendAck(int type, int seq)
{
        return 0;
}

// ===========================================================================
/* Function to advance the sequence number,
   wrapping around at maximum value.  */
int next(int seq)
{
    return ((seq + 1) % MOD_SEQNUM);
}


// ===========================================================================
/* Function to set time limit at a point in the future.
   limit   is time limit in seconds (from now)  */
long timeSet(float limit)
{
    long timeLimit = clock() + (long)(limit * CLOCKS_PER_SEC);
    return timeLimit;
}  // end of timeSet


// ===========================================================================
/* Function to check if time limit has elapsed.
   timer  is timer variable to check
   returns 1 if time has reached or exceeded limit,
           0 if time has not yet reached limit.   */
int timeUp(long timeLimit)
{
    if (clock() < timeLimit) return 0;  // still within limit
    else return 1;  // time limit has been reached or exceeded
}  // end of timeUP


// ===========================================================================
/* Function to print bytes of a frame, in groups of 10.
   For small frames, print all the bytes,
   for larger frames, just the start and end.  */
void printFrame(byte *frame, int nByte)
{
    int i, j;

    if (nByte <= 50)  // small frame - print all the bytes
    {
        for (i=0; i<nByte; i+=10)  // step in groups of 10 bytes
        {
            for (j=0; (j<10)&&(i+j<nByte); j++)
            {
                printf("%3d ", frame[i+j]);  // print as number
            }
            printf(":  ");  // separator
            for (j=0; (j<10)&&(i+j<nByte); j++)
            {
                printf("%c", frame[i+j]);   // print as character
            }
            printf("\n");   // new line
        }  // end for
    }
    else  // large frame - print start and end
    {
        for (j=0; (j<10); j++)  // first 10 bytes
            printf("%3d ", frame[j]);  // print as number
        printf(":  ");  // separator
        for (j=0; (j<10); j++)
            printf("%c", frame[j]); // print as character
        printf("\n - - -\n");   // new line, separator
        for (j=nByte-10; (j<nByte); j++)  // last 10 bytes
            printf("%3d ", frame[j]);  // print as number
        printf(":  ");  // separator
        for (j=nByte-10; (j<nByte); j++)
            printf("%c", frame[j]); // print as character
        printf("\n");   // new line
    }

}  // end of printFrame
