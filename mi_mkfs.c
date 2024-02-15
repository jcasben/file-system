#include "bloques.h"

int main(int argc, char **argv)
{
    // Check if the all the arguments are passed
    if (argc != 3) 
    {
        fprintf(stderr, RED "Too few arguments. Usage: ./mi_mkfs <device name> <nblocks>\n" RESET);
        return FALLO;
    }
    
    // Mounting the device with bmount(<device_name>).
    if(bmount(argv[1]) < 0)
    {
        fprintf(stderr, RED "Error mounting the device\n" RESET);
        return FALLO;
    }
    
    // Parse the number of blocks from the arguments
    int nblocks = atoi(argv[2]);
    
    if (nblocks < 0)
    {
        fprintf(stderr, RED "The number of blocks must be greater than 0.\n" RESET);
        return FALLO;
    }

    // Set the elements of the buffer to 0
    unsigned char buffer[BLOCKSIZE];
    if(memset(buffer, 0, BLOCKSIZE) < 0)
    {
        perror(RED "ERROR" RESET);
        return FALLO;
    }

    // Write the blocks to the device
    for (size_t i = 0; i < nblocks; i++)
    {
        bwrite(i, buffer);
    }
    

    // Unmounting the device
    if(bumount() < 0)
    {
        fprintf(stderr, RED "Error unmounting the device\n" RESET);
        return FALLO;
    }
}