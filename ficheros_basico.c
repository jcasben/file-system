#include "ficheros_basico.h"

struct superbloque *SB;

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
    SB = malloc(sizeof(struct superbloque));
    SB->posPrimerBloqueMB = posSB + tamSB;
    SB->posUltimoBloqueMB = SB->posPrimerBloqueMB + tamMB(nbloques) - 1;
    SB->posPrimerBloqueAI = SB->posUltimoBloqueMB + 1;
    SB->posUltimoBloqueAI = SB->posPrimerBloqueAI + tamAI(ninodos) - 1;
    SB->posPrimerBloqueDatos = SB->posUltimoBloqueAI + 1;
    SB->posUltimoBloqueDatos = nbloques-1;
    SB->posInodoRaiz = 0;
    SB->posPrimerInodoLibre = 0;
    SB->cantBloquesLibres = nbloques;
    SB->cantInodosLibres = ninodos;
    SB->totBloques = nbloques;
    SB->totInodos = ninodos;

    // Write the superblock structure to the disk at the position posSB
    if(bwrite(posSB, SB) < 0)
    {
        perror(RED "ERROR" RESET);
        return FALLO;
    }

    return EXITO;
}

/// @brief Initializes de bitmap that represents with 1 the
/// metadata.
/// @return 
int initMB()
{
    //Leer el superbloque y hacer magia.
}

/// @brief Initialize the array of inodes
/// @return 
int initAI()
{
    struct inodo inodos[BLOCKSIZE/INODOSIZE];  
    
    unsigned int contInodos = SB->posPrimerInodoLibre + 1;

    for (unsigned int i = SB->posPrimerBloqueAI; i <= SB->posUltimoBloqueAI; i++)
    {
        if(bread(i, inodos) == FALLO)
        {
            return FALLO;
        }

        for (size_t j = 0; j < BLOCKSIZE/INODOSIZE; j++)
        {
            inodos[j].tipo = 'l';
            
            if(contInodos < SB->totInodos)
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
        
        if(bwrite(i, inodos) == FALLO)
        {
            return FALLO;
        }
    }
    
    return EXITO;
}