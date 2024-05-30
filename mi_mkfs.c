/*
    Autores:
        - Marc Link Cladera
        - Carlos Gálvez Mena
        - Jesús Castillo Benito
*/

#include "directorios.h"

int main(int argc, char **argv)
{
    // Check if the all the arguments are passed
    if (argc != 3)
    {
        fprintf(stderr, RED "ERROR: invalid syntax. Usage: ./mi_mkfs <device name> <nblocks>\n" RESET);
        return FALLO;
    }

    #if MMAP
        unsigned int finDV = atoi(argv[2]) * BLOCKSIZE - 1;
        FILE *fp = fopen(argv[1], "w");
        fseek(fp, finDV, SEEK_SET);
        fputc('\0', fp);
        fclose(fp);
    #endif

    // Mounting the device with bmount(<device_name>).
    if (bmount(argv[1]) < 0) return FALLO;

    // Parse the number of blocks from the arguments
    int nblocks = atoi(argv[2]);

    if (nblocks < 0)
    {
        fprintf(stderr, RED "ERROR: the number of blocks must be greater than 0.\n" RESET);
        return FALLO;
    }

    // Set the elements of the buffer to 0
    unsigned char buffer[BLOCKSIZE];
    if (memset(buffer, 0, BLOCKSIZE) < 0)
    {
        perror(RED "ERROR" RESET);
        return FALLO;
    }

    // Write the blocks to the device
    for (size_t i = 0; i < nblocks; i++)
    {
        bwrite(i, buffer);
    }

    initSB(nblocks, nblocks / 4);
    initMB();
    initAI();

    reservar_inodo('d', 7);

    // Unmounting the device
    if (bumount() < 0) return FALLO;
}