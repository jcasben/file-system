#include "ficheros_basico.h"

//----------------------------- NIVEL 2 (15/02/2023 - 21/02/2023) -----------------------------

/// @brief Calculate the size in blocks needed for the bitmap
/// @param nbloques Number of blocks
/// @return The size of the bitmap in blocks
int tamMB(unsigned int nbloques)
{
    unsigned int size = ((nbloques / 8) / BLOCKSIZE);
    if (((nbloques / 8) % BLOCKSIZE) != 0) size++;

    return size;
}

/// @brief Calculate the size in blocks of the inodes array
/// @param ninodos Number of inodes.
/// @return Size of the inodes array in blocks
int tamAI(unsigned int ninodos)
{
    unsigned int size = (ninodos * INODOSIZE) / BLOCKSIZE;
    if ((ninodos * INODOSIZE) % BLOCKSIZE != 0) size++;
    
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
        posPrimerBloqueMB : posSB + tamSB,
        posUltimoBloqueMB : SB.posPrimerBloqueMB + tamMB(nbloques) - 1,
        posPrimerBloqueAI : SB.posUltimoBloqueMB + 1,
        posUltimoBloqueAI : SB.posPrimerBloqueAI + tamAI(ninodos) - 1,
        posPrimerBloqueDatos : SB.posUltimoBloqueAI + 1,
        posUltimoBloqueDatos : nbloques - 1,
        posInodoRaiz : 0,
        posPrimerInodoLibre : 0,
        cantBloquesLibres : nbloques,
        cantInodosLibres : ninodos,
        totBloques : nbloques,
        totInodos : ninodos,
    };

    // Write the superblock structure to the disk at the position posSB
    if (bwrite(posSB, &SB) < 0)
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
    for (size_t i = 0; i < exponent; i++) result = result * base;

    return result;
}

/// @brief Initializes de bitmap that represents with 1 the
/// metadata.
/// @return FALLO if an error occurs, EXITO otherwise.
int initMB()
{
    struct superbloque SB;
    if (bread(posSB, &SB) == FALLO) return FALLO;
    int meta_blocks = tamMB(SB.totBloques) + tamAI(SB.totInodos) + tamSB;

    int bytes_to_1 = meta_blocks / 8;
    char bufferMB[1024];
    for (size_t i = 0; i < bytes_to_1; i++) bufferMB[i] = 255;

    // Setting the necesary bits of the last byte to 1 (meta_blocks % 8)
    int bits_to_1 = meta_blocks % 8;
    int last_byte = 0;
    for (size_t i = 7; i > 7 - bits_to_1; i--) last_byte += power(2, i);
    bufferMB[bytes_to_1] = last_byte;

    // Setting the rest of the block to 0, from the byte bytes_to_1 + 1
    // to BLOCKSIZE
    for (size_t i = bytes_to_1 + 1; i < BLOCKSIZE; i++) bufferMB[i] = 0;

    // Write the buffer to the virtual device at the first block
    // of the bitmap
    if (bwrite(SB.posPrimerBloqueMB, bufferMB) == FALLO) return FALLO;
    // Recalculate the number of free blocks at the super block.
    SB.cantBloquesLibres = SB.cantBloquesLibres - meta_blocks;
    if (bwrite(posSB, &SB) == FALLO) return FALLO;

    return EXITO;
}

/// @brief Initialize the array of inodes
/// @return FALLO if an error occurs, EXITO otherwise.
int initAI()
{
    struct inodo inodos[BLOCKSIZE / INODOSIZE];
    struct superbloque SB;
    if (bread(posSB, &SB) == FALLO) return FALLO;

    unsigned int contInodos = SB.posPrimerInodoLibre + 1;
    for (unsigned int i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++)
    {
        if (bread(i, inodos) == FALLO) return FALLO;
        for (size_t j = 0; j < BLOCKSIZE / INODOSIZE; j++)
        {
            inodos[j].tipo = 'l';

            if (contInodos < SB.totInodos)
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
        if (bwrite(i, inodos) < 0) return FALLO;
    }

    return EXITO;
}

//----------------------------- NIVEL 3 (21/02/2023 - )-----------------------------

/// @brief Modify the value of a bit in the bitmap(MB).
/// @param nbloque The block number where the bit we want to modify is located.
/// @param bit The value of the bit we want to write.
/// @return FALLO if an error occurs, EXITO otherwise.
int escribir_bit(unsigned int nbloque, unsigned int bit)
{
    // Read the superblock
    struct superbloque SB;
    if (bread(posSB, &SB) == FALLO) return FALLO;
    int posbyte = nbloque / 8;
    int posbit = nbloque % 8;
    int nbloqueMB = posbyte / BLOCKSIZE;
    int nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB;
    unsigned char bufferMB[BLOCKSIZE];

    // Read the block where the bit is located
    if (bread(nbloqueabs, bufferMB) == FALLO) return FALLO;

    // Modify the bit in the byte(buferMB)
    posbyte = posbyte % BLOCKSIZE;
    unsigned char mascara = 128;
    mascara >>= posbit;

    if (bit == 1) bufferMB[posbyte] |= mascara;
    else bufferMB[posbyte] &= ~mascara;

    // Write the block with the bit modified
    if (bwrite(nbloqueabs, bufferMB) == FALLO) return FALLO;

    return EXITO;
}

/// @brief Reads the value of a bit in the bitmap(MB).
/// @param nbloque The block number where the bit we want to read is located.
/// @return returns the value of the bit we want to read.
char leer_bit(unsigned int nbloque)
{
    // Read the superblock
    struct superbloque SB;
    if (bread(posSB, &SB) == FALLO) return FALLO;

    int posbyte = nbloque / 8;
    int posbit = nbloque % 8;
    int nbloqueMB = posbyte / BLOCKSIZE;
    int nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB;
    unsigned char bufferMB[BLOCKSIZE];

    // Read the block where the bit is located
    if (bread(nbloqueabs, bufferMB) == FALLO) return FALLO;

    // Read the bit in the byte(buferMB)
    posbyte = posbyte % BLOCKSIZE;
    unsigned char mascara = 128;
    mascara >>= posbit;
    mascara &= bufferMB[posbyte];
    mascara >>= (7 - posbit);

    return mascara;
}

/// @brief Finds the first free block, quering with MB, ocupates it and
/// return the position of that block.
/// @return the position of the first free block.
int reservar_bloque()
{
    struct superbloque SB;
    if (bread(posSB, &SB) == FALLO) return FALLO;

    // Check the amount of free blocks
    // If the amount is zero or fewer, there is no free block
    if (SB.cantBloquesLibres <= 0) return FALLO;

    // Create the buffer to read from MB
    char bufferMB[BLOCKSIZE];

    // Create the auxiliar buffer with all the bits set to 1
    char bufferAux[BLOCKSIZE];
    memset(bufferAux, 255, BLOCKSIZE);

    // Iterates through nbloqueMB and finds a block that contains at least one 0
    unsigned int nbloqueMB = 0;
    if (bread(nbloqueMB + SB.posPrimerBloqueMB, bufferMB) == FALLO) return FALLO;

    // Compares the content of bufferMB with the content of the auxiliar buffer
    while (
            memcmp(bufferMB, bufferAux, BLOCKSIZE) == 0 
            && (nbloqueMB + SB.posPrimerBloqueMB <= SB.posUltimoBloqueMB)
        )
    {
        nbloqueMB++;
        if (bread(nbloqueMB + SB.posPrimerBloqueMB, bufferMB) == FALLO) return FALLO;
    }

    unsigned int posbyte = 0;
    // Locates on bufferMB the position of the first byte that contains any 0
    while(bufferMB[posbyte] == 255 && posbyte < BLOCKSIZE) posbyte++;
    
    unsigned char mascara = 128;
    unsigned int posbit = 0;
    // Find on the byte the position of the first 0
    while(bufferMB[posbyte] & mascara)
    {
        bufferMB[posbyte] <<= 1;
        posbit++;
    }

    // Calculates the number of the physic block that we can reserve
    unsigned int nbloque = (nbloqueMB * BLOCKSIZE + posbyte) * 8 + posbit;

    // Set to 1 the bit that represents the block that we
    // want to reserve at the bitmap
    if (escribir_bit(nbloque, 1) == FALLO) return FALLO;
    
    // Decrease the number of free block and save the superblock
    SB.cantBloquesLibres--;
    if (bwrite(posSB, &SB) == FALLO) return FALLO;

    // Clean the block setting all the bytes to 0
    char clean[BLOCKSIZE];
    memset(clean, 0, BLOCKSIZE);
    if (bwrite(nbloque, clean) == FALLO) return FALLO;
    
    return nbloque;
}

/// @brief frees the specified block.
/// @param nbloque number of the block to be freed.
/// @return the number of the freed block (nbloque); FALLO otherwise.
int liberar_bloque(unsigned int nbloque)
{
    // Set to 0 the bit that represent the block that we
    // want to free at the bitmap.
    if (escribir_bit(nbloque, 0) == FALLO) return FALLO;

    // Read the superblock and increase the number of free blocks.
    struct superbloque SB;
    if (bread(posSB, &SB) == FALLO) return FALLO;
    SB.cantBloquesLibres++;
    if (bwrite(posSB, &SB) == FALLO) return FALLO;

    return nbloque;
}

/// @brief Writes an inodo at the specified inode of the array.
/// @param ninodo position of the array of the inodes where we
/// will write the new inode.
/// @param inodo inode to be written.
/// @return FALLO if an error occurs, EXITO otherwise.
int escribir_inodo(unsigned int ninodo, struct inodo *inodo)
{
    struct superbloque SB;
    if (bread(posSB, &SB) == FALLO) return FALLO;

    // Calculate the block where belongs the position of the inode
    // that we want to write.
    unsigned int ninode_block = ninodo / (BLOCKSIZE / INODOSIZE);
    unsigned int absolute_block = SB.posPrimerBloqueAI + ninode_block;

    // Use the absolute block number that we calculated to read that
    // block
    struct inodo inodos[BLOCKSIZE / INODOSIZE];
    if (bread(absolute_block, inodos) == FALLO) return FALLO;

    // Write the new inode at the relative position on the block.
    inodos[ninodo % (BLOCKSIZE / INODOSIZE)] = *inodo;
    if (bwrite(absolute_block, inodos) == FALLO) return FALLO;

    return EXITO;
}

/// @brief Reads an inode from the array of the inodes and dumps
/// it into a struct inode variable.
/// @param ninodo number of the inode.
/// @param inodo variable of the type struct inode.
/// @return FALLO if an error occurs, EXITO otherwise.
int leer_inodo(unsigned int ninodo, struct inodo *inodo)
{
    struct superbloque SB;
    if (bread(posSB, &SB) == FALLO) return FALLO;

    // Calculate the block where belongs the position of the inode
    // that we want to read.
    unsigned int ninode_block = ninodo / (BLOCKSIZE / INODOSIZE);
    unsigned int absolute_block = SB.posPrimerBloqueAI + ninode_block;

    // Use the absolute block number that we calculated to read that
    // block
    struct inodo inodos[BLOCKSIZE / INODOSIZE];
    if (bread(absolute_block, inodos) == FALLO) return FALLO;

    // Dump the specified inode into the variable
    *inodo = inodos[ninodo % (BLOCKSIZE / INODOSIZE)];

    return EXITO;
}

/// @brief Finds the first free inode, reserves it, returns its number
/// and updates the linked list of free inodes.
/// @param tipo type of the inode.
/// @param permisos permissions of the inode.
/// @return the number of the first free inode; FALLO otherwise.
int reservar_inodo(unsigned char tipo, unsigned char permisos)
{
    // Read the superblock
    struct superbloque SB;
    if (bread(posSB, &SB) == FALLO) return FALLO;

    // Check if there are free inodes
    if (SB.cantInodosLibres == 0)
    {
        printf(RED "ERROR: there aren't free inodes." RESET);
        return FALLO;
    }
    // Modify the position of the first free inode and save the
    // position of the first free inode
    int posInodoReservado = SB.posPrimerInodoLibre;
    SB.posPrimerInodoLibre = SB.posPrimerInodoLibre + 1;

    // Initialize the new inode
    struct inodo new_inode = {
        tipo : tipo,
        permisos : permisos,
        nlinks : 1,
        tamEnBytesLog : 0,
        atime : time(NULL),
        mtime : time(NULL),
        ctime : time(NULL),
        numBloquesOcupados : 0,
    };

    // Write the new inode to the disk
    if (escribir_inodo(posInodoReservado, &new_inode) == FALLO) return FALLO;

    // Update the cantInodosLibres at the superblock and rewrite it.
    SB.cantInodosLibres = SB.cantInodosLibres - 1;
    if (bwrite(posSB, &SB) == FALLO) return FALLO;

    return posInodoReservado;
}