#ifndef LINKLAYER_H_INCLUDED
#define LINKLAYER_H_INCLUDED

// Link Layer Protocol definitions - adjust all these to match your design
#define MAX_BLK 255 // largest number of data bytes allowed in a block
#define MOD_SEQNUM 16 // modulo for sequence numbers

// Frame marker byte values
#define STARTBYTE 206     // start of frame marker
#define ENDBYTE 204     // end of frame marker
#define STUFFBYTE 220   // stuff byte

// Frame header byte positions
#define SEQNUMPOS 2     // position of sequence number
#define BYTECOUNTPOS 1 // position of byte count

// Header and trailer size
#define HEADERSIZE 3		// number of bytes in frame header
#define TRAILERSIZE 2	// number of bytes in frame trailer

// Acknowledgement values
#define GOOD 1          // type is good - positive ack
#define BAD 26          // type is bad, nak
#define ACK_SIZE 5      // number of bytes in ack frame

// Time limits
#define TX_WAIT 5.0   // sender waiting time in seconds
#define RX_WAIT 20.0  // receiver waiting time in seconds
#define MAX_TRIES 6   // number of times to re-try (either end)

// Simulated errors
#define PROB_ERR 3.0E-4  // probability of simulated error on receive


/* Functions to implement link layer protocol.
   All functions take a debug argument - if 1, they print
   messages explaining what is happening.
   Regardless of debug, functions print messages on errors.
   All functions return negative values on error or failure.  */

// Function to connect to another computer.
int LL_connect(int debug);

// Function to disconnect from other computer.
int LL_discon(int debug);

// Function to send a block of data in a frame.
int LL_send(byte *dataTx, int nData, int debug);

// Function to receive a frame and return a block of data.
int LL_receive(byte *dataRx, int maxData, int debug);


// ==========================================================
// Functions called by the four link layer functions above

// Function to build a frame from a block of data.
int buildDataFrame(byte *frameTx, byte *dataTx, int nData, int seq);

// Function to get a frame from the received bytes.
int getFrame(byte *frameRx, int maxSize, float timeLimit);

// Function to check a received frame for errors.
int checkFrame(byte *frameRx, int nFrame);

// Function to process received frame.
int processFrame(byte *frameRx, int nFrame,
                 byte *dataRx, int maxData, int *seqNum);

// Function to send an acknowledgement - positive or negative.
int sendAck(int type, int seq);

// ==========================================================
// Helper functions used by various other functions

// Function to advance the sequence number
int next(int seq);

// Function to set time limit at a point in the future.
long timeSet(float limit);

// Function to check if time limit has elapsed.
int timeUp(long timeLimit);

// Function to check if byte is a protocol byte.
int special(byte b);

// Function to print bytes of a frame, in groups of 10.
void printFrame(byte *frame, int nByte);

#endif // LINKLAYER_H_INCLUDED
