/*
    Autores:
        - Marc Link Cladera
        - Carlos Gálvez Mena
        - Jesús Castillo Benito
*/

#include "ficheros.h"

//----------------------------- NIVEL 5 (26/02/2023 - 29/03/2024) -----------------------------

/// @brief Writes the content of a buffer on a file/directory.
/// @param ninodo id of the inode where we will write.
/// @param buf_original buffer that which content will be written to the
/// file.
/// @param offset initial position of writing.
/// @param nbytes number of bytes that will be written.
/// @return the number of bytes written. FALLO otherwise.
int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes)
{
    int written_bytes = 0;

    // Get the specified inode.
    struct inodo inode;
    leer_inodo(ninodo, &inode);
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
        mi_waitSem();
        int nbfisico = traducir_bloque_inodo(&inode, primerBL, 1);
        char buf_bloque[BLOCKSIZE];
        if (bread(nbfisico, buf_bloque) == FALLO)
        {
            mi_signalSem();
            return FALLO;
        }
        mi_signalSem();

        // Copy the new data of buf_original to the buffer that contains
        // the data of the block that we read. Then, write the updated block.       
        memcpy(buf_bloque + desp1, buf_original, nbytes);

        mi_waitSem();
        if (bwrite(nbfisico, buf_bloque) == FALLO) 
        {
            mi_signalSem();
            return FALLO;
        }
        mi_signalSem();
        written_bytes = nbytes;
    }
    // Case where the writing affects to more than 1 block.
    else
    {
        // FASE 1: First logical block
        mi_waitSem();
        int nbfisico = traducir_bloque_inodo(&inode, primerBL, 1);
        char buf_bloque[BLOCKSIZE];
        if (bread(nbfisico, buf_bloque) == FALLO)
        {
            mi_signalSem();
            return FALLO;
        }
        mi_signalSem();

        // Copy the bytes that belong to the first block to the buffer
        //that contains the data of the block.
        memcpy(buf_bloque + desp1, buf_original, BLOCKSIZE - desp1);

        mi_waitSem();
        if (bwrite(nbfisico, buf_bloque) == FALLO) 
        {
            mi_signalSem();
            return FALLO;
        }
        mi_signalSem();

        written_bytes += BLOCKSIZE - desp1;
        // FASE 2: Intermediate logical blocks.
        for (size_t i = primerBL + 1; i < ultimoBL; i++)
        {
            mi_waitSem();
            
            nbfisico = traducir_bloque_inodo(&inode, i, 1);
            if ((written_bytes += bwrite(nbfisico, buf_original + (BLOCKSIZE - desp1) + (i - primerBL - 1) * BLOCKSIZE)) == FALLO)
            {
                mi_signalSem();
                return FALLO;
            }

            mi_signalSem();
        }
        
        // FASE 3: Last logical block.
        mi_waitSem();
        nbfisico = traducir_bloque_inodo(&inode, ultimoBL, 1);
        if (bread(nbfisico, buf_bloque) == FALLO)
        {
            mi_signalSem();
            return FALLO;
        }
        mi_signalSem();
        
        memcpy(buf_bloque, buf_original + (nbytes - (desp2 + 1)), desp2 + 1);

        mi_waitSem();
        if (bwrite(nbfisico, buf_bloque) == FALLO)
        {
            mi_signalSem();
            return FALLO;
        }
        mi_signalSem();

        written_bytes += desp2 + 1;
    }
    // If we have written further than the end of the file
    // update tamEnBytesLog.
    if (inode.tamEnBytesLog < (offset + written_bytes))
    {
        inode.tamEnBytesLog = (offset + written_bytes);
        inode.ctime = time(NULL);
    }
    inode.mtime = time(NULL);

    mi_waitSem();
    if (escribir_inodo(ninodo, &inode) == FALLO)
    {
        mi_signalSem();
        return FALLO;
    }
    mi_signalSem();
    
    return written_bytes;
} 

/// @brief Reads information from a file/directory and stores it in a memory buffer.
/// @param ninodo number of inode which corresponds the file/directory.
/// @param buf_original buffer where the information will be stored. The buffer must be inizialized with 0's.
/// @param offset the initial reading position with respect to the inode.
/// @param nbytes the number of bytes to read.
/// @return the number of bytes actually read. FALLO otherwise.
int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes)
{
    int read_bytes = 0;

    // Get the specified inode.
    struct inodo inode;
    leer_inodo(ninodo, &inode);
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
    if ((offset + nbytes) >= inode.tamEnBytesLog)
    {
        nbytes = inode.tamEnBytesLog - offset;
    }

    unsigned int primerBL = (offset / (unsigned) BLOCKSIZE);
    unsigned int ultimoBL = (offset + nbytes - 1) / (unsigned) BLOCKSIZE;
    unsigned int desp1 = offset % (unsigned) BLOCKSIZE;
    unsigned int desp2 = (offset + nbytes - 1) % (unsigned) BLOCKSIZE;

    if(primerBL == ultimoBL)
    {
        int nbfisico = traducir_bloque_inodo(&inode, primerBL, 0);
        
        if (nbfisico != -1)
        {
            char buf_bloque[BLOCKSIZE];
            mi_waitSem();
            // Read the block where the data is stored.
            if(bread(nbfisico, buf_bloque) == FALLO)
            {
                mi_signalSem();
                return FALLO;
            }
            mi_signalSem();
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
            mi_signalSem();
            return nbytes;
        }

    }
    // Case where the reading affects to more than 1 block.
    else
    {
        // FASE 1: First logical block (the important info is BLOCSIZE - desp1)
        int nbfisico = traducir_bloque_inodo(&inode, primerBL, 0);
        
        // Buffer that we will use to read the slize of the fist and last block.
        char buf_bloque[BLOCKSIZE];
        
        // If the block doesn't exist, we don't read it.
        if (nbfisico != -1)
        {
            mi_waitSem();
            if (bread(nbfisico, buf_bloque) == FALLO) 
            {
                mi_signalSem();
                return FALLO;
            }
            mi_signalSem();

            memcpy(buf_original, buf_bloque + desp1, BLOCKSIZE - desp1);   
        }
        
        // Add the number of bytes read to the total number of bytes read.
        read_bytes += BLOCKSIZE - desp1;

        // FASE 2: Intermediate logical blocks (all the block is info that we want to read).
        for (size_t i = primerBL + 1; i < ultimoBL; i++)
        {
            if ((nbfisico = traducir_bloque_inodo(&inode, i, 0)) != -1)
            {
                // Read the block, storing it in the buffer.
                mi_waitSem();
                if (bread(nbfisico, buf_original + (BLOCKSIZE - desp1) + (i - primerBL - 1) * BLOCKSIZE) == FALLO) 
                {
                    mi_signalSem();
                    return FALLO;   
                }
                mi_signalSem();
            }
            // Add the number of bytes read (BLOCKSIZE) to the total number of bytes read.
            read_bytes += BLOCKSIZE;
        }
        
        // FASE 3: Last logical block.

        if ((nbfisico = traducir_bloque_inodo(&inode, ultimoBL, 0)) != -1)
        {
            mi_waitSem();
            // Read the last block where the data is stored.
            if (bread(nbfisico, buf_bloque) == FALLO) 
            {
                mi_signalSem();
                return FALLO;
            }
            mi_signalSem();
            // Copy the slice of the block that we want to read to the buffer.
            memcpy(buf_original + read_bytes, buf_bloque, desp2 + 1);
        }
        // Add the number of bytes read to the total number of bytes read.
        read_bytes += desp2 + 1;

        return read_bytes;
    }
}

/// @brief Returns the meta-information of a file/directory: type, permissions, etc.
/// This information is placed at p_stat.
/// @param ninodo inode number
/// @param p_stat STAT where the meta-information is stored
/// @return EXITO if completed succesfully, FALLO if there is an error
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

/// @brief Change the permissions of a file/directory
/// @param ninodo inode number
/// @param permisos permissions to change
/// @return EXITO if completed succesfully, FALLO if there is an error
int mi_chmod_f(unsigned int ninodo, unsigned char permisos)
{
    struct inodo inode;
    if(leer_inodo(ninodo, &inode) == FALLO) return FALLO;
    inode.permisos = permisos;
    inode.ctime = time(NULL);
    if(escribir_inodo(ninodo, &inode) == FALLO) return FALLO;

    return EXITO;
}

//----------------------------- NIVEL 6 (03/02/2023 - ) -----------------------------

/// @brief Truncates a file/directory of the corresponding inode to the
/// given bytes
/// @param ninodo inode number
/// @param nbytes desired size
/// @return Number of released block if there is no error. FALLO if there is an error.
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