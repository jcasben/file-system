/*
    Autores:
        - Marc Link Cladera
        - Carlos Gálvez Mena
        - Jesús Castillo Benito
*/

#include "bloques.h"
#include "semaforo_mutex_posix.h"

// Global variable for the file descriptor of the virtual device.
static int fd = 0;
static unsigned int inside_sc = 0;
static sem_t *mutex;

/// @brief Mounts the virtual disk, opening it and setting the file descriptor.
/// @param camino path to the virtual disk.
/// @return the file descriptor of the virtual disk or -1 (FALLO) if an error occurs.
int bmount(const char *camino)
{
    if (!mutex)
    {
        mutex = initSem();
        if (mutex == SEM_FAILED) return -1;
    }

    if (camino == NULL)
    {
        fprintf(stderr, RED "ERROR: unvalid path\n" RESET);
        return FALLO;
    }

    // Using umask for giving the right permissions to the file
    umask(000);
    fd = open(camino, O_RDWR | O_CREAT, 0666);
    if (fd < 0)
    {
        perror(RED "ERROR" RESET);
        return FALLO;
    }

    return fd;
}

/// @brief Unmounts the virtual disk, closing it.
/// @return 0 (EXITO) if the disk is unmounted successfully, -1 (FALLO)
/// if an error occurs.
int bumount()
{
    deleteSem();
    if (close(fd) < 0)
    {
        perror(RED "ERROR" RESET);
        return FALLO;
    }

    return EXITO;
}

/// @brief Writes a block to the virtual device, on the physical block
/// specified by nbloque.
/// @param nbloque physical block we want to write.
/// @param buf buffer with the data we want to write.
/// @return the number of bytes written (BLOCKSIZE) or -1 (FALLO) if an error occurs.
int bwrite(unsigned int nbloque, const void *buf)
{
    // Set the pointer to the block we want to write
    if (lseek(fd, (nbloque * BLOCKSIZE), SEEK_SET) < 0)
    {
        perror(RED "ERROR" RESET);
        return FALLO;
    }
    // We write the block
    if (write(fd, buf, BLOCKSIZE) != BLOCKSIZE)
    {
        perror(RED "ERROR" RESET);
        return FALLO;
    }

    return BLOCKSIZE;
}

/// @brief Reads a block from the virtual device, from the physical block
/// specified by nbloque.
/// @param nbloque physical block we want to read.
/// @param buf buffer where we want to store the data we read.
/// @return the number of bytes read (BLOCKSIZE) or -1 (FALLO) if an error occurs.
int bread(unsigned int nbloque, void *buf)
{
    // Set the pointer to the block we want to read
    if (lseek(fd, (nbloque * BLOCKSIZE), SEEK_SET) < 0)
    {
        perror(RED "ERROR" RESET);
        return FALLO;
    }
    // We read the block
    if (read(fd, buf, BLOCKSIZE) == FALLO)
    {
        perror(RED "ERROR" RESET);
        return FALLO;
    }

    return BLOCKSIZE;
}


void mi_waitSem()
{
    // It is made so that you don't have to do wait twice
    if (!inside_sc)
    {
        waitSem(mutex);
    }
    inside_sc++;
}

void mi_signalSem()
{
    // It is made so that you don't have to do signal twice
    inside_sc--;
    if (!inside_sc)
    {
        signalSem(mutex);
    }
    
}