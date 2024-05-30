/*
    Autores:
        - Marc Link Cladera
        - Carlos Gálvez Mena
        - Jesús Castillo Benito
*/

#include "bloques.h"
#include "semaforo_mutex_posix.h"

#include <sys/mman.h>

// Global variable for the file descriptor of the virtual device.
static int fd = 0;
static unsigned int inside_sc = 0;
static sem_t *mutex;
#if MMAP
    static int tamSFM = 0;
    static void *ptrSFM;
#endif

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
    if (fd > 0) close(fd);
    fd = open(camino, O_RDWR | O_CREAT, 0666);
    #if MMAP
        ptrSFM = do_mmap(fd);
    #endif
    if (fd < 0)
    {
        perror(RED "ERROR" RESET);
        return FALLO;
    }

    return fd;
}

int bumount()
{
    #if MMAP
        msync(ptrSFM, tamSFM, MS_SYNC);
        munmap(ptrSFM, tamSFM);
    #endif
    deleteSem();
    if ((fd = close(fd)) < 0)
    {
        perror(RED "ERROR" RESET);
        return FALLO;
    }

    return EXITO;
}

int bwrite(unsigned int nbloque, const void *buf)
{
    #if MMAP==0
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
    #endif
    #if MMAP
        int s;
        if (nbloque * BLOCKSIZE + BLOCKSIZE <= tamSFM) s = BLOCKSIZE;
        else s = tamSFM - nbloque * BLOCKSIZE;
        if (s > 0) memcpy(ptrSFM + (nbloque*BLOCKSIZE), buf, s);

        return s;
    #endif
}

int bread(unsigned int nbloque, void *buf)
{
    // Set the pointer to the block we want to read
    #if MMAP==0
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
    #endif
    #if MMAP
        int s;
        if (nbloque * BLOCKSIZE + BLOCKSIZE <= tamSFM) s = BLOCKSIZE;
        else s = tamSFM - nbloque * BLOCKSIZE;
        if (s > 0) memcpy(buf, ptrSFM + (nbloque*BLOCKSIZE), s);

        return s;
    #endif

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

#if MMAP
    void *do_mmap(int fd) {
        struct stat st;
        void *ptr;
        fstat(fd, &st);
        tamSFM = st.st_size; //static int tamSFM: tamaño memoria compartida
        if ((ptr = mmap(NULL, PROT_WRITE, MAP_SHARED, tamSFM, fd, 0))== (void *)-1)
            fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
        return ptr;
    }
#endif
