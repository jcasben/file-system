#include "ficheros.h"

//----------------------------- NIVEL 5 (26/02/2023 - ) -----------------------------

/// @brief Writes the content of a buffer on a file/directory.
/// @param ninodo id of the inode where we will write.
/// @param buf_original buffer that which content will be written to the
/// file.
/// @param offset initial position of writing.
/// @param nbytes number of bytes that will be written.
/// @return the number of bytes written. FALLO otherwise.
int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes)
{
    // Get the specified inode.
    struct inodo inode;
    leer_inodo(ninodo, &inode);
    // Look if the inode has the right writing permissions.
    if ((inode.permisos & 2) != 2)
    {
        fprintf(stderr, RED "ERROR: The specified inode doesn't have writing permissions\n" RESET);
        return FALLO;
    }

    int primerBL = offset / BLOCKSIZE;
    int ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;
    int desp1 = offset % BLOCKSIZE;
    int desp2 = (offset + nbytes - 1) % BLOCKSIZE;

    // Treat the case in which the buffer fits inside 1 block.
    if (primerBL == ultimoBL)
    {
        int nbfisico = traducir_bloque_inodo(&inode, primerBL, 1);
        char buf_bloque[BLOCKSIZE];
        if (bread(nbfisico, buf_bloque) == FALLO) return FALLO;
        // Copy the new data of buf_original to the buffer that contains
        // the data of the block that we read. Then, write the updated block.       
        memcpy(buf_bloque + desp1, buf_original, nbytes);
        if (bwrite(nbfisico, buf_bloque) == FALLO) return FALLO;
    }
    // Case where the writing affects to more than 1 block.
    else
    {
        // FASE 1: First logical block
        int nbfisico = traducir_bloque_inodo(&inode, primerBL, 1);
        char buf_bloque[BLOCKSIZE];
        if (bread(nbfisico, buf_bloque) == FALLO) return FALLO;
        // Copy the bytes that belong to the first block to the buffer
        //that contains the data of the block.
        memcpy(buf_bloque + desp1, buf_original, BLOCKSIZE - desp1);
        if (bwrite(nbfisico, buf_bloque) == FALLO) return FALLO;

        // FASE 2: Intermediate logical blocks.
        for (size_t i = primerBL + 1; i < ultimoBL; i++)
        {
            if (bwrite(nbfisico, buf_original + (BLOCKSIZE - desp1) + (i - primerBL - 1)) == FALLO) return FALLO;
        }
        
        // FASE 3: Last logical block.
        nbfisico = traducir_bloque_inodo(&inode, ultimoBL, 1);
        if (bread(nbfisico, buf_bloque) == FALLO) return FALLO;
        memcpy(buf_bloque, buf_original + (nbytes - (desp2 + 1)), desp2 + 1);
        if (bread(buf_bloque, nbfisico) == FALLO) return FALLO;
        // TODO: actualizar la metainfo del inodo
        // TODO: devolver la cantidad de bytes escritos
    }
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
        return FALLO;
    }

    int primerBL = offset / BLOCKSIZE;
    int ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;
    int desp1 = offset % BLOCKSIZE;
    int desp2 = (offset + nbytes - 1) % BLOCKSIZE;

    // if offset is greather than inode.tamEnBytesLog we cant read (read_bytes will be 0)
    if (offset >= inode.tamEnBytesLog) return read_bytes;
    if ((offset + nbytes) >= inode.tamEnBytesLog)
    {
        nbytes = inode.tamEnBytesLog - offset;
        //leer solo 
    }
    //
    if(primerBL == ultimoBL)
    {
        int nbfisico = traducir_bloque_inodo(&inode, primerBL, 0);
        if (nbfisico != -1)
        {
            char buf_bloque[BLOCKSIZE];
            if(bread(nbfisico, buf_bloque) == FALLO) return FALLO;
            memcpy(buf_bloque + desp1, buf_original, nbytes); // es nbytes?
            // not finished
        } 
        else
        {
            read_bytes += BLOCKSIZE - desp1;
            //not finished
        }

    }
    
    else
    {
        //not finished
    }
    
}

/// @brief Returns the meta-information of a file/directory: type, permissions, etc.
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
    inode.ctime = time(NULL);
    if(escribir_inodo(ninodo, &inode) == FALLO) return FALLO;
    return EXITO;
}