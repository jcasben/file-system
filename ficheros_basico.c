#include "ficheros_basico.h"

/// @brief Calculate the size in blocks needed for the bitmap
/// @param nbloques Number of blocks
/// @return The size of the bitmap in blocks
int tamMB(unsigned int nbloques)
{
    unsigned int size = ((nbloques/8)/BLOCKSIZE);
    if (((nbloques/8)%BLOCKSIZE) != 0)
    {
        size++;
    }

    return size;
}

/// @brief Calculate the size in blocks of the inodes array
/// @param ninodos Number of inodes.
/// @return Size of the inodes array in blocks
int tamAI(unsigned int ninodos)
{
    unsigned int size = (ninodos * INODOSIZE) / BLOCKSIZE;

    if((ninodos * INODOSIZE) % BLOCKSIZE != 0)
    {
        size++;
    }
   
    return size;
}

/// @brief Initializes the data of the superblock and writes it to the disk.
/// @param nbloques Number of blocks of the disk.
/// @param ninodos Number of inodes of the disk. It follow 
/// the following heuristic: ninodos = nbloques/4. 
/// @return FALLO if an error occurs, EXITO otherwise.
int initSB(unsigned int nbloques, unsigned int ninodos)
{
    struct superbloque SB = {
        posPrimerBloqueMB: posSB + tamSB,
        posUltimoBloqueMB: SB.posPrimerBloqueMB + tamMB(nbloques) - 1,
        posPrimerBloqueAI: SB.posUltimoBloqueMB + 1,
        posUltimoBloqueAI: SB.posPrimerBloqueAI + tamAI(ninodos) - 1,
        posPrimerBloqueDatos: SB.posUltimoBloqueAI + 1,
        posUltimoBloqueDatos: nbloques -1,
        posInodoRaiz: 0,
        posPrimerInodoLibre: 0,
        cantBloquesLibres: nbloques,
        cantInodosLibres: ninodos,
        totBloques: nbloques,
        totInodos: ninodos,
    };

    // Write the superblock structure to the disk at the position posSB
    if(bwrite(posSB, &SB) < 0)
    {
        perror(RED "ERROR" RESET);
        return FALLO;
    }

    return EXITO;
}

/// @brief this function makes the pow of a number.
/// @param base the base of the pow.
/// @param exponent the exponent of the pow.
/// @return the base powered to the exponent.
int power(int base, int exponent)
{
    int result = 1;
    for (size_t i = 0; i < exponent; i++)
    {
        result = result * base;
    }
    
    return result;
}

/// @brief Initializes de bitmap that represents with 1 the
/// metadata.
/// @return FALLO if an error occurs, EXITO otherwise.
int initMB()
{
    struct superbloque SB;
    bread(posSB, &SB);
    int meta_blocks = tamMB(SB.totBloques) + tamAI(SB.totInodos) + tamSB;
    
    int bytes_to_1 = meta_blocks / 8;
    char bufferMB[1024];
    for (size_t i = 0; i < bytes_to_1; i++)
    {
        bufferMB[i] = 255;
    }
    
    // Setting the necesary bits of the last byte to 1 (meta_blocks % 8)
    int bits_to_1 = meta_blocks % 8;
    int last_byte = 0;
    for (size_t i = 7; i > 7 - bits_to_1; i--)
    {
        last_byte += power(2, i);
    }
    bufferMB[bytes_to_1] = last_byte;

    // Setting the rest of the block to 0, from the byte bytes_to_1 + 1 
    // to BLOCKSIZE
    for (size_t i = bytes_to_1 + 1; i < BLOCKSIZE; i++)
    {
        bufferMB[i] = 0;
    }
    
    // Write the buffer to the virtual device at the first block
    // of the bitmap
    bwrite(SB.posPrimerBloqueMB, bufferMB);
    // Recalculate the number of free blocks at the super block.
    SB.cantBloquesLibres = SB.cantBloquesLibres - meta_blocks;
    bwrite(posSB, &SB);

    return EXITO;
}

/// @brief Initialize the array of inodes
/// @return FALLO if an error occurs, EXITO otherwise.
int initAI()
{
    struct inodo inodos[BLOCKSIZE/INODOSIZE];
    struct superbloque SB;
    bread(posSB, &SB);
    
    unsigned int contInodos = SB.posPrimerInodoLibre + 1;

    for (unsigned int i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++)
    {
        if(bread(i, inodos) == FALLO)
        {
            return FALLO;
        }

        for (size_t j = 0; j < BLOCKSIZE/INODOSIZE; j++)
        {
            inodos[j].tipo = 'l';
            
            if(contInodos < SB.totInodos)
            {
                inodos[j].punterosDirectos[0] = contInodos;
                contInodos++;
            } 
            else
            {
                inodos[j].punterosDirectos[0] = UINT_MAX;
                break;
            }
        }
        
        if(bwrite(i, inodos) < 0)
        {
            return FALLO;
        }
    }
    
    return EXITO;
}