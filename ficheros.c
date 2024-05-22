/*
    Autores:
        - Marc Link Cladera
        - Carlos Gálvez Mena
        - Jesús Castillo Benito
*/

#include "ficheros.h"

//----------------------------- NIVEL 5 (26/02/2023 - 29/03/2024) -----------------------------

int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes)
{
    int written_bytes = 0;

    // Get the specified inode.
    struct inodo inode;
    mi_waitSem(); // CRITICAL SECTION -----------------------------------
    leer_inodo(ninodo, &inode);
    mi_signalSem(); // CRITICAL SECTION ---------------------------------

    // Look if the inode has the right writing permissions.
    if ((inode.permisos & 2) != 2)
    {
        fprintf(stderr, RED "ERROR: The specified inode doesn't have writing permissions\n" RESET);
        return FALLO;
    }

    unsigned int primerBL = (offset / (unsigned) BLOCKSIZE);
    unsigned int ultimoBL = (offset + nbytes - 1) / (unsigned) BLOCKSIZE;
    unsigned int desp1 = offset % (unsigned) BLOCKSIZE;
    unsigned int desp2 = (offset + nbytes - 1) % (unsigned) BLOCKSIZE;

    // Treat the case in which the buffer fits inside 1 block.
    if (primerBL == ultimoBL)
    {
        mi_waitSem(); // CRITICAL SECTION -----------------------------------
        int nbfisico = traducir_bloque_inodo(ninodo, primerBL, 1);
        mi_signalSem(); // CRITICAL SECTION ---------------------------------

        char buf_bloque[BLOCKSIZE];
        if (bread(nbfisico, buf_bloque) == FALLO) return FALLO;

        // Copy the new data of buf_original to the buffer that contains
        // the data of the block that we read. Then, write the updated block.       
        memcpy(buf_bloque + desp1, buf_original, nbytes);
        if (bwrite(nbfisico, buf_bloque) == FALLO) return FALLO;
        written_bytes = nbytes;
    }
    // Case where the writing affects to more than 1 block.
    else
    {
        // FASE 1: First logical block
        mi_waitSem(); // CRITICAL SECTION -----------------------------------
        int nbfisico = traducir_bloque_inodo(ninodo, primerBL, 1);
        mi_signalSem(); // CRITICAL SECTION ---------------------------------

        char buf_bloque[BLOCKSIZE];
        if (bread(nbfisico, buf_bloque) == FALLO) return FALLO;

        // Copy the bytes that belong to the first block to the buffer
        //that contains the data of the block.
        memcpy(buf_bloque + desp1, buf_original, BLOCKSIZE - desp1);
        if (bwrite(nbfisico, buf_bloque) == FALLO) return FALLO;

        written_bytes += BLOCKSIZE - desp1;
        // FASE 2: Intermediate logical blocks.
        for (size_t i = primerBL + 1; i < ultimoBL; i++)
        {
            mi_waitSem(); // CRITICAL SECTION -----------------------------------
            nbfisico = traducir_bloque_inodo(ninodo, i, 1);
            mi_signalSem(); // CRITICAL SECTION ---------------------------------

            if ((written_bytes += bwrite(nbfisico, buf_original + (BLOCKSIZE - desp1) + (i - primerBL - 1) * BLOCKSIZE)) == FALLO)
                return FALLO;
        }
        
        // FASE 3: Last logical block.
        mi_waitSem(); // CRITICAL SECTION -----------------------------------
        nbfisico = traducir_bloque_inodo(ninodo, ultimoBL, 1);
        mi_signalSem(); // CRITICAL SECTION ---------------------------------

        if (bread(nbfisico, buf_bloque) == FALLO) return FALLO;
        memcpy(buf_bloque, buf_original + (nbytes - (desp2 + 1)), desp2 + 1);

        if (bwrite(nbfisico, buf_bloque) == FALLO) return FALLO;
        written_bytes += desp2 + 1;
    }

    // Reset the content of inode
    memset(&inode, 0, sizeof(struct inodo));

    mi_waitSem(); // CRITICAL SECTION -----------------------------------
    if (leer_inodo(ninodo, &inode) == FALLO)
    {
        mi_signalSem();
        return FALLO;
    }
    // If we have written further than the end of the file
    // update tamEnBytesLog.
    if (inode.tamEnBytesLog < (offset + written_bytes))
    {
        inode.tamEnBytesLog = (offset + written_bytes);
        inode.ctime = time(NULL);
    }
    inode.mtime = time(NULL);

    if (escribir_inodo(ninodo, &inode) == FALLO)
    {
        mi_signalSem();
        return FALLO;
    }
    mi_signalSem(); // CRITICAL SECTION ---------------------------------

    return written_bytes;
} 

int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes)
{
    int read_bytes = 0;

    // Get the specified inode.
    mi_waitSem(); // CRITICAL SECTION -----------------------------------
    struct inodo inode;
    if (leer_inodo(ninodo, &inode) == FALLO)
    {
        mi_signalSem();
        return FALLO;
    }
    inode.atime = time(NULL);
    if (escribir_inodo(ninodo, &inode) == FALLO)
    {
        mi_signalSem();
        return FALLO;
    }
    mi_signalSem(); // CRITICAL SECTION ---------------------------------

    // Look if the inode has the right reading permissions.
    if ((inode.permisos & 4) != 4) 
    {
        fprintf(stderr, RED "ERROR: The specified inode doesn't have reading permissions\n" RESET);
        fprintf(stderr, "\nbytes read: %d\n", read_bytes);
        fprintf(stderr, "logical byte size: %d\n", inode.tamEnBytesLog);
        return FALLO;
    }

    // If offset is greather than inode.tamEnBytesLog we cant read (read_bytes will be 0)
    if (offset >= inode.tamEnBytesLog) return read_bytes;

    // If we want to read more bytes than the file has, we read from offset to the end of the file.
    // So we modify the nbytes to read.
    if ((offset + nbytes) >= inode.tamEnBytesLog) nbytes = inode.tamEnBytesLog - offset;

    unsigned int primerBL = (offset / (unsigned) BLOCKSIZE);
    unsigned int ultimoBL = (offset + nbytes - 1) / (unsigned) BLOCKSIZE;
    unsigned int desp1 = offset % (unsigned) BLOCKSIZE;
    unsigned int desp2 = (offset + nbytes - 1) % (unsigned) BLOCKSIZE;

    if(primerBL == ultimoBL)
    {
        int nbfisico = traducir_bloque_inodo(ninodo, primerBL, 0);
        
        if (nbfisico != -1)
        {
            char buf_bloque[BLOCKSIZE];
            // Read the block where the data is stored.
            if(bread(nbfisico, buf_bloque) == FALLO) return FALLO;
            // Copy the slice of the block that we want to read to the buffer.
            memcpy(buf_original, buf_bloque + desp1, nbytes);
            // Return the number of bytes read.
            return nbytes;
        } 
        else
        {
            // If the block is not allocated, we dont change the
            // buffer (because it is full of 0's) and return nbytes
            // as the number of bytes read.
            return nbytes;
        }
    }
    // Case where the reading affects to more than 1 block.
    else
    {
        // FASE 1: First logical block (the important info is BLOCSIZE - desp1)
        int nbfisico = traducir_bloque_inodo(ninodo, primerBL, 0);
        // Buffer that we will use to read the slize of the fist and last block.
        char buf_bloque[BLOCKSIZE];
        
        // If the block doesn't exist, we don't read it.
        if (nbfisico != -1)
        {
            if (bread(nbfisico, buf_bloque) == FALLO) return FALLO;
            memcpy(buf_original, buf_bloque + desp1, BLOCKSIZE - desp1);   
        }
        
        // Add the number of bytes read to the total number of bytes read.
        read_bytes += BLOCKSIZE - desp1;

        // FASE 2: Intermediate logical blocks (all the block is info that we want to read).
        for (size_t i = primerBL + 1; i < ultimoBL; i++)
        {
            if ((nbfisico = traducir_bloque_inodo(ninodo, i, 0)) != -1)
            {
                // Read the block, storing it in the buffer.
                if (bread(nbfisico, buf_original + (BLOCKSIZE - desp1) + (i - primerBL - 1) * BLOCKSIZE) == FALLO) return FALLO;
            }
            // Add the number of bytes read (BLOCKSIZE) to the total number of bytes read.
            read_bytes += BLOCKSIZE;
        }
        
        // FASE 3: Last logical block.

        if ((nbfisico = traducir_bloque_inodo(ninodo, ultimoBL, 0)) != -1)
        {
            // Read the last block where the data is stored.
            if (bread(nbfisico, buf_bloque) == FALLO) return FALLO;
            // Copy the slice of the block that we want to read to the buffer.
            memcpy(buf_original + read_bytes, buf_bloque, desp2 + 1);
        }
        // Add the number of bytes read to the total number of bytes read.
        read_bytes += desp2 + 1;

        return read_bytes;
    }
}

int mi_stat_f(unsigned int ninodo, struct STAT *p_stat)
{
    struct inodo inode;
    if(leer_inodo(ninodo, &inode) == FALLO) return FALLO;
    
    p_stat->atime = inode.atime;
    p_stat->ctime = inode.ctime;
    p_stat->mtime = inode.mtime;
    p_stat->nlinks = inode.nlinks;
    p_stat->numBloquesOcupados = inode.numBloquesOcupados;
    p_stat->permisos = inode.permisos;
    p_stat->tamEnBytesLog = inode.tamEnBytesLog;
    p_stat->tipo = inode.tipo;

    return EXITO;
}

int mi_chmod_f(unsigned int ninodo, unsigned char permisos)
{
    mi_waitSem();
    struct inodo inode;
    if(leer_inodo(ninodo, &inode) == FALLO)
    {
        mi_signalSem();
        return FALLO;
    }
    inode.permisos = permisos;
    inode.ctime = time(NULL);
    if(escribir_inodo(ninodo, &inode) == FALLO)
    {
        mi_signalSem();
        return FALLO;
    }
    mi_signalSem();

    return EXITO;
}

//----------------------------- NIVEL 6 (03/02/2023 - 06/04/2024) -----------------------------

int mi_truncar_f(unsigned int ninodo, unsigned int nbytes)
{   
    struct inodo inode;
    int releasedBlocks;

    // Loads the inode
    if(leer_inodo(ninodo, &inode) == FALLO) return FALLO;

    // Check if the inode has write permissions
    if((inode.permisos & 2) != 2) return FALLO;

    // Check if nbytes is greater than the inode's EOF
    if(inode.tamEnBytesLog < nbytes) return FALLO;

    // Calculates the first block to release
    unsigned int primerBL = (nbytes % BLOCKSIZE == 0) ? 
                                nbytes/BLOCKSIZE : 
                                nbytes/BLOCKSIZE + 1;

    // Relase the inode's blocks specified by primerBL
    if((releasedBlocks = liberar_bloques_inodo(primerBL, &inode)) == FALLO) return FALLO;
    
    // Updates numBloquesOcupados, mtime, and ctime
    inode.tamEnBytesLog = nbytes;
    inode.numBloquesOcupados -= releasedBlocks;
    inode.mtime = time(NULL);
    inode.ctime = time(NULL);

    // Saves the inode
    if((escribir_inodo(ninodo, &inode)) == FALLO) return FALLO;

    return releasedBlocks;
}