/* EEEN20060 Communication Systems, Link Layer test program
   This program opens a file and reads block of bytes from it.
   It uses the link layer functions to send each block through
   a simulated physical layer, and to receive the result.
   It writes the result to an output file for inspection. */

typedef unsigned char byte;

#include <stdio.h>  // standard input-output library
#include <windows.h>  // needed for sleep function
#include "linklayer.h"  // link layer functions

#define DEBUG 1 // flag to make link layer functions print more
#define BLK_SIZE 50  // max data block size in bytes
// this value is small for testing - can increase

// Function prototypes
int sendFile(char *fName, int debug);
int receiveFile(int debug);


int main()
{
    char fName[80];  // string to hold filename
    int retVal;     // return value from functions
    FILE *fpi, *fpo;  // file handles for input and output files
    byte dataSend[BLK_SIZE+2];  // array of bytes to send
    byte dataReceive[BLK_SIZE+2];  // bytes received
    int nByte, nRx, nWrite;  // byte counts
    long SendCount = 0, RxCount = 0; // more byte counters

    printf("Link Layer Test Program\n");  // welcome message

    // Ask user for file name to send
    printf("\nEnter name of file to send (name.ext): ");
    scanf("%70s", fName);  // get filename
    printf("\n");  // blank line

    // Open the input file and check for failure
    printf("\nMain: Opening %s for input\n", fName);
    fpi = fopen(fName, "rb");  // open for binary read
    if (fpi == NULL)
    {
        perror("Main: Error opening input file");
        return 1;
    }

    // Open the output file and check for failure
    printf("\nMain: Opening output.txt for output\n");
    fpo = fopen("output.txt", "wb");  // open for binary write
    if (fpo == NULL)
    {
        perror("Main: Error opening output file");
        return 1;
    }

    // Ask link layer to connect to other computer
    printf("Main: Connecting...\n");
    retVal = LL_connect(DEBUG);  // try to connect
    if (retVal < 0)  // problem connecting
    {
        fclose(fpi);     // close input file
        fclose(fpo);    // close ouptut file
        return retVal;  // pass back the error
    }

    // Send the contents of the file, one block at a time
    do  // loop block by block
    {
        // read bytes from file, store in array
        nByte = (int) fread(dataSend, 1, BLK_SIZE, fpi);
        if (ferror(fpi))  // check for error
        {
            perror("Main: Error reading input file");
            break;
        }
        printf("\nMain: Read %d bytes, sending...\n", nByte);
        SendCount += nByte;  // add to byte count

        // send bytes to link layer
        retVal = LL_send(dataSend, nByte, DEBUG);  // retVal is 0 if succeeded
        if (retVal != 0) break; // need to get out of loop

        Sleep(10);  // Short delay to allow progress to be seen

        // receive bytes from link layer, up to size of array
        nRx = LL_receive(dataReceive, BLK_SIZE+2, DEBUG);
        // nRx will be number of bytes received, or negative if error
        if (nRx < 0 ) printf("Main: Error receiving data, code %d\n",nRx);
        else if (nRx == 0) printf("Main: Zero bytes received\n");
        else // we got some data - write to output file
        {
            printf("Main: Received %d bytes, writing\n", nRx);
            nWrite = (int) fwrite(dataReceive, 1, nRx, fpo);
            if (ferror(fpo))  // check for error
            {
                perror("Main: Error writing output file");
                break;
            }
            RxCount += nWrite;
        }

        Sleep(30);  // longer delay
    }
    while (feof(fpi) == 0);  // until input file ends

    // Check why the loop ended, and print message
    if (feof(fpi) == 0) printf("\nMain: Error in loop\n");
    else printf("\nMain: End of input file\n");

    // Print statistics
    printf("Read %ld bytes from input file, wrote %ld bytes to output\n",
           SendCount, RxCount);

    fclose(fpi);    // close input file
    fclose(fpo);    // close output file
    LL_discon(DEBUG);  // disconnect
    return 0;

}  // end of receiveFile
