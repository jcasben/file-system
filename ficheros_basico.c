/*
    Autores:
        - Marc Link Cladera
        - Carlos Gálvez Mena
        - Jesús Castillo Benito
*/

#include "ficheros_basico.h"
#define DEBUGN5 1
#define DEBUGN6 1

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
    int posMB = SB.posPrimerBloqueMB;
    int meta_blocks = tamMB(SB.totBloques) + tamAI(SB.totInodos) + tamSB;
    // Number of blocks occupied by the bits.
    int occupied_blocks = meta_blocks / BLOCKSIZE / 8;

    int bytes_to_1 = meta_blocks / 8;
    unsigned char bufferMB[BLOCKSIZE];
    //memset
    if (occupied_blocks > 0)
    {
        memset(bufferMB, 255, BLOCKSIZE);
        for (size_t i = 0; i < occupied_blocks; i++)
        {
            if (bwrite(posMB++, bufferMB) == FALLO) return FALLO;
        }
        
        bytes_to_1 -= BLOCKSIZE * occupied_blocks;
        memset(bufferMB, 0, BLOCKSIZE);
    }

    memset(bufferMB, 255, bytes_to_1);
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
    if (bwrite(posMB, bufferMB) == FALLO) return FALLO;
    // Recalculate the number of free blocks at the super block.
    SB.cantBloquesLibres -= meta_blocks;
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

//----------------------------- NIVEL 3 (21/02/2023 - 25/02/2024) -----------------------------

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
    unsigned char bufferMB[BLOCKSIZE];

    // Create the auxiliar buffer with all the bits set to 1
    unsigned char bufferAux[BLOCKSIZE];
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
        fprintf(
            stderr,
            RED 
            "ERROR: there aren't free inodes." 
            RESET
            );
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

//----------------------------- NIVEL 4 (25/02/2023 - 26/02/2024) -----------------------------

/// @brief Optain the range of pointers where the logical block we
//         are searching is located. We also obtain the adress 
//         stored in the corresponding inode pointer.
/// @param inodo inode where we are going to search the logical block.
/// @param nblogico number of the logical block.
/// @param ptr adress stored in the corresponding inode pointer.
/// @return the range of pointers where the logical block
int obtener_nRangoBL(struct inodo *inodo, unsigned int nblogico, unsigned int *ptr)
{
    if (nblogico < DIRECTOS)
    {
        *ptr = inodo->punterosDirectos[nblogico];
        return 0;
    }
    else if (nblogico < INDIRECTOS0)
    {
        *ptr = inodo->punterosIndirectos[0];
        return 1;
    }
    else if (nblogico < INDIRECTOS1)
    {
        *ptr = inodo->punterosIndirectos[1];
        return 2;
    }
    else if (nblogico < INDIRECTOS2)
    {
        *ptr = inodo->punterosIndirectos[2];
        return 3;
    }
    else
    {
        *ptr = 0;
        fprintf(
            stderr,
            RED 
            "Bloque lógico fuera de rango" 
            RESET
            );
        return FALLO;
    }
}

/// @brief Obtains the index of the given logic block with the level
/// @param nblogico position of the logical block
/// @param nivel_punteros level of the indirect pointer
/// @return position of the physical block
int obtener_indice(unsigned int nblogico, int nivel_punteros)
{
    if(nblogico < DIRECTOS) return nblogico;  
    else if (nblogico < INDIRECTOS0) return nblogico - DIRECTOS;
    else if (nblogico < INDIRECTOS1)
    {
        if(nivel_punteros == 2) return (nblogico - INDIRECTOS0) / NPUNTEROS;
        else if(nivel_punteros == 1) return (nblogico - INDIRECTOS0) % NPUNTEROS;
    }
    else if (nblogico < INDIRECTOS2)
    {
        if(nivel_punteros == 3) return (nblogico - INDIRECTOS1) / (NPUNTEROS*NPUNTEROS);
        else if(nivel_punteros == 2) 
            return ((nblogico - INDIRECTOS1) % (NPUNTEROS*NPUNTEROS)) / NPUNTEROS;   
        else if(nivel_punteros == 1) 
            return ((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) % NPUNTEROS;
    }
    
    return FALLO;
}

/// @brief Obtains the number of the physical block corresponding
/// to a specified logical block of the indicated inode.
/// @param inodo indicated inode.
/// @param nblogico number of the logical block.
/// @param reservar behavior of the function.
/// @return if reservar = 0 -> the function is used only to consult if the physical block 
/// exists. If exists returns its position, if not returns FALLO;
/// if 1 -> the function is used to consult if the physical block exists and to
/// reserve a physical block. If the physical block exists, it returns its position.
/// If it doesn't exist, it reserves memory for it and returns its position.
int traducir_bloque_inodo(struct inodo *inodo, unsigned int nblogico, unsigned char reservar)
{
    unsigned int ptr = 0;
    unsigned int ptr_ant = 0;
    int nRangoBL = obtener_nRangoBL(inodo, nblogico, &ptr);
    int nivel_punteros = nRangoBL;
    int indice;
    unsigned int buffer[NPUNTEROS];

    while (nivel_punteros > 0)
    {
        if (ptr == 0)
        {
            if (reservar == 0) return FALLO;
            else
            {
                ptr = reservar_bloque();
                
                inodo->numBloquesOcupados++;
                inodo->ctime = time(NULL);
                if (nivel_punteros == nRangoBL)
                {
                    inodo->punterosIndirectos[nRangoBL - 1] = ptr;
                    #if DEBUGN5
                        fprintf(
                            stderr, 
                            GRAY 
                            "[traducir_bloque_inodo() -> inodo.punterosIndirectos[%d] = %d "
                            "(reservado BF %d para punteros_nivel%d)]\n" 
                            RESET, 
                            nivel_punteros - 1, ptr, ptr, nivel_punteros
                        );
                    #endif
                }
                else
                {
                    buffer[indice] = ptr;
                    #if DEBUGN5
                        fprintf(stderr,
                            GRAY
                            "[traducir_bloque_inodo() -> punteros_nivel%d [%d] = %d "
                            "(reservado BF %d para punteros_nivel%d)]\n"
                            RESET,
                            nivel_punteros + 1, indice, ptr, ptr, nivel_punteros
                        );
                    #endif
                    if (bwrite(ptr_ant, buffer) == FALLO) return FALLO;
                }
                memset(buffer, 0, BLOCKSIZE);
            }
        }
        else if (bread(ptr, buffer) == FALLO) return FALLO;

        indice = obtener_indice(nblogico, nivel_punteros);
        ptr_ant = ptr;
        ptr = buffer[indice];
        nivel_punteros--;
    }
    
    if (ptr == 0)
    {
        if (reservar == 0) return FALLO;
        else
        {
            ptr = reservar_bloque();
            
            inodo->numBloquesOcupados++;
            inodo->ctime = time(NULL);

            if (nRangoBL == 0)
            {
                inodo->punterosDirectos[nblogico] = ptr;
                #if DEBUGN5
                    fprintf(stderr,
                        GRAY 
                        "[traducir_bloque_inodo() -> inodo.punterosDirectos[%d] = %d "
                        "(reservado BF %d para BL %d)]\n"
                        RESET,
                        nblogico, ptr, ptr, nblogico
                    );
                #endif
            }
            else
            {
                buffer[indice] = ptr;
                #if DEBUGN5
                    fprintf(stderr,
                        GRAY
                        "[traducir_bloque_inodo() -> punteros_nivel%d [%d] = %d "
                        "(reservado BF %d para BL %d)]\n"
                        RESET,
                        nivel_punteros + 1, indice, ptr, ptr, nblogico
                    );
                #endif
                if (bwrite(ptr_ant, buffer) == FALLO) return FALLO;
            }
        }
    }

    return ptr;
}

//-------------------------- NIVEL 6 (03/02/2023 - 06/04/2024) --------------------------

/// @brief
/// @param ninodo
/// @return
int liberar_inodo(unsigned int ninodo)
{
    struct inodo inodo;
    if (leer_inodo(ninodo, &inodo) == FALLO) return FALLO;
    int aux = liberar_bloques_inodo(0, &inodo);
    // If it is correct inodo.numBloquesOcupados should be 0.
    inodo.numBloquesOcupados -= aux;
    // Put the tipe of the inode to 'l' (free)
    inodo.tipo = 'l';
    // Update the size of the inode in bytes to 0.
    inodo.tamEnBytesLog = 0;

    // Update the linked list of free inodes and update the superblock
    struct superbloque SB;
    if (bread(posSB, &SB) == FALLO) return FALLO;
    inodo.punterosDirectos[0] = SB.posPrimerInodoLibre;
    SB.posPrimerInodoLibre = ninodo;
    SB.cantInodosLibres++;

    // Write the superblock and the inode updated to the disk
    if (bwrite(posSB, &SB) == FALLO) return FALLO;
    inodo.ctime = time(NULL);
    if (escribir_inodo(ninodo, &inodo) == FALLO) return FALLO;

    return ninodo;
}

#if DEBUGN6
    int contador_breads = 0, contador_bwrites = 0;
#endif

/// @brief Frees all the occupied blocks from the logical block indicated by primerBL
/// (this one included).
/// @param primerBL first logical block to be freed.
/// @param inodo inode where we are going to free the blocks.
/// @return the number of freed blocks.
int liberar_bloques_inodo(unsigned int primerBL, struct inodo *inodo) 
{
    unsigned int ultimoBL, ptr = 0, nivel_punteros = 0;
    int nRangoBL = 0, liberados = 0, eof = 0;
    
    if (inodo->tamEnBytesLog == 0) return 0; // el fichero está vacío
    if (inodo->tamEnBytesLog % BLOCKSIZE == 0) ultimoBL = inodo->tamEnBytesLog / BLOCKSIZE - 1;
    else ultimoBL = inodo->tamEnBytesLog / BLOCKSIZE;
    #if DEBUGN6
        fprintf(
            stderr, 
            BLUE 
            "[liberar_bloques_inodo() -> primerBL: %d, ultimoBL: %d]\n" 
            RESET, 
            primerBL, ultimoBL
        );
    #endif
    
    unsigned int nBL = primerBL;

    nRangoBL = obtener_nRangoBL(inodo, nBL, &ptr);
    if (nRangoBL == 0) liberados += liberar_directos(&nBL, ultimoBL, inodo, &eof);

    while (!eof)
    {
        nRangoBL = obtener_nRangoBL(inodo, nBL, &ptr);
        nivel_punteros = nRangoBL;
        
        liberados += liberar_indirectos_recursivo(
            &nBL, primerBL, ultimoBL, inodo, 
            nRangoBL, nivel_punteros, &ptr, &eof
        );
    }
    #if DEBUGN6
        fprintf(
            stderr, 
            BLUE NEGRITA 
            "[liberar_bloques_inodo() -> total bloques liberados: %d, total_breads: %d, total_bwrites: %d]\n" 
            RESET, 
            liberados, contador_breads, contador_bwrites
        );
    #endif
    return liberados;
}

int liberar_directos(unsigned int *nBL, unsigned int ultimoBL, struct inodo *inodo, int *eof)  
{
    int liberados = 0;

    for (int d = *nBL; d < DIRECTOS && !*eof; d++)
    {
        if (inodo->punterosDirectos[*nBL])  
        {
            liberar_bloque(inodo->punterosDirectos[*nBL]);
            #if DEBUGN6
                fprintf(
                    stderr, 
                    GRAY 
                    "[liberar_directos() -> liberado BF %d de datos para BL %d]\n" 
                    RESET, 
                    inodo->punterosDirectos[*nBL], *nBL
                );
            #endif
            inodo->punterosDirectos[*nBL] = 0;
            liberados++;
        }
        *nBL = *nBL+1;
        if (*nBL > ultimoBL) *eof = 1;      // Fin del archivo
    }

    return liberados;
}

int liberar_indirectos_recursivo(
    unsigned int *nBL, unsigned int primerBL, unsigned int ultimoBL, struct inodo *inodo, 
    int nRangoBL, unsigned int nivel_punteros, unsigned int *ptr, int *eof
) 
{
    int liberados = 0;
    unsigned int bloquePunteros[NPUNTEROS];
    unsigned int bloquePunteros_Aux[NPUNTEROS];
    unsigned int bufferCeros[NPUNTEROS];
    int indice_inicial;

    memset(bufferCeros, 0, BLOCKSIZE);

    if (*ptr) { //si cuelga un bloque de punteros
        indice_inicial = obtener_indice(*nBL, nivel_punteros);
        if (indice_inicial==0 || *nBL == primerBL) //solo leemos bloque si no estaba cargado
        {
            if (bread(*ptr, bloquePunteros) == FALLO) return FALLO;
            #if DEBUGN6 
                contador_breads++; 
            #endif
            memcpy(bloquePunteros_Aux, bloquePunteros, BLOCKSIZE); // Guardamos copia del bloque para ver si hay cambios
        }   

        for (int i = indice_inicial; i < NPUNTEROS && !*eof; i++) //exploramos el bloque de punteros
        {
            
            if (bloquePunteros[i] != 0) 
            {
                if (nivel_punteros == 1) 
                {
                    liberar_bloque(bloquePunteros[i]); //de datos
                    #if DEBUGN6
                        fprintf(
                            stderr, 
                            GRAY
                            "[liberar_indirectos_recursivo() -> liberado BF %d de datos para BL %d]\n" 
                            RESET, 
                            bloquePunteros[i], *nBL
                        );
                    #endif
                    bloquePunteros[i] = 0;
                    liberados++;
                    *nBL = *nBL + 1;
                } 
                else liberados += liberar_indirectos_recursivo(
                        nBL, primerBL, ultimoBL, inodo, nRangoBL, 
                        nivel_punteros-1, &bloquePunteros[i], eof);
            } 
            else 
            {
                switch (nivel_punteros)
                {
                    case 1: *nBL = *nBL+1 ; break;
                    case 2: *nBL += NPUNTEROS; break;
                    case 3: *nBL += NPUNTEROS*NPUNTEROS; break;
                }
            }
            if (*nBL > ultimoBL) *eof = 1;      // Fin del archivo
        }
        if (memcmp(bloquePunteros, bloquePunteros_Aux, BLOCKSIZE) != 0) // Si el bloque es distinto al original
        {   
            if (memcmp(bloquePunteros, bufferCeros, BLOCKSIZE) != 0) //si quedan punteros en el bloque lo salvamos
            {
                bwrite(*ptr, bloquePunteros);
                #if DEBUGN6
                    contador_bwrites++;
                    fprintf(
                        stderr, 
                        ORANGE 
                        "[liberar_indirectos_recursivo() -> salvado BF %d de punteros_nivel%d correspondiente al BL %d]\n" 
                        RESET, 
                        *ptr, nivel_punteros, *nBL
                    ); 
                #endif
            } 
            else 
            {
                liberar_bloque(*ptr); //de punteros
                #if DEBUGN6
                    fprintf(
                        stderr, 
                        GRAY 
                        "[liberar_indirectos_recursivo() -> liberado BF %d de punteros_nivel%d correspondiente al BL %d]\n" 
                        RESET, 
                        *ptr, nivel_punteros, *nBL
                    );
                #endif
                *ptr = 0;
                liberados++;
            }
        }
    }
    else 
    {
        switch (nRangoBL) 
        {
            case 1: *nBL = INDIRECTOS0; break;
            case 2: *nBL = INDIRECTOS1; break;
            case 3: *nBL = INDIRECTOS2; break;
        } 
    }
    
    return liberados;
}