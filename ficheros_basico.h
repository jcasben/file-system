/*
    Autores:
        - Marc Link Cladera
        - Carlos Gálvez Mena
        - Jesús Castillo Benito
*/

#include "bloques.h"
#include <limits.h>
#include <time.h>

#define posSB 0 // el superbloque se escribe en el primer bloque de nuestro FS
#define tamSB 1
#define NPUNTEROS (BLOCKSIZE / sizeof(unsigned int))   // 256 punteros por bloque
#define DIRECTOS 12
#define INDIRECTOS0 (NPUNTEROS + DIRECTOS)    // 268
#define INDIRECTOS1 (NPUNTEROS * NPUNTEROS + INDIRECTOS0)    // 65.804
#define INDIRECTOS2 (NPUNTEROS * NPUNTEROS * NPUNTEROS + INDIRECTOS1) // 16.843.020

struct superbloque
{
    unsigned int posPrimerBloqueMB;                      // Posición absoluta del primer bloque del mapa de bits
    unsigned int posUltimoBloqueMB;                      // Posición absoluta del último bloque del mapa de bits
    unsigned int posPrimerBloqueAI;                      // Posición absoluta del primer bloque del array de inodos
    unsigned int posUltimoBloqueAI;                      // Posición absoluta del último bloque del array de inodos
    unsigned int posPrimerBloqueDatos;                   // Posición absoluta del primer bloque de datos
    unsigned int posUltimoBloqueDatos;                   // Posición absoluta del último bloque de datos
    unsigned int posInodoRaiz;                           // Posición del inodo del directorio raíz (relativa al AI)
    unsigned int posPrimerInodoLibre;                    // Posición del primer inodo libre (relativa al AI)
    unsigned int cantBloquesLibres;                      // Cantidad de bloques libres (en todo el disco)
    unsigned int cantInodosLibres;                       // Cantidad de inodos libres (en el AI)
    unsigned int totBloques;                             // Cantidad total de bloques del disco
    unsigned int totInodos;                              // Cantidad total de inodos (heurística)
    char padding[BLOCKSIZE - 12 * sizeof(unsigned int)]; // Relleno para ocupar el bloque completo
    };

#define INODOSIZE 128 // tamaño en bytes de un inodo

struct inodo
{                          // comprobar que ocupa 128 bytes haciendo un sizeof(inodo)!!!
    unsigned char tipo;     // Tipo ('l':libre, 'd':directorio o 'f':fichero)
    unsigned char permisos; // Permisos (lectura y/o escritura y/o ejecución)

    /* Por cuestiones internas de alineación de estructuras, si se está utilizando
        un tamaño de palabra de 4 bytes (microprocesadores de 32 bits):
    unsigned char reservado_alineacion1 [2];
    en caso de que la palabra utilizada sea del tamaño de 8 bytes
    (microprocesadores de 64 bits): unsigned char reservado_alineacion1 [6]; */
    unsigned char reservado_alineacion1[6];

    time_t atime; // Fecha y hora del último acceso a datos
    time_t mtime; // Fecha y hora de la última modificación de datos
    time_t ctime; // Fecha y hora de la última modificación del inodo

    /* comprobar que el tamaño del tipo time_t para vuestra plataforma/compilador es 8:
    printf ("sizeof time_t is: %ld\n", sizeof(time_t)); */

    unsigned int nlinks;             // Cantidad de enlaces de entradas en directorio
    unsigned int tamEnBytesLog;      // Tamaño en bytes lógicos (EOF)
    unsigned int numBloquesOcupados; // Cantidad de bloques ocupados zona de datos

    unsigned int punterosDirectos[12];  // 12 punteros a bloques directos
    unsigned int punterosIndirectos[3]; /* 3 punteros a bloques indirectos:
    1 indirecto simple, 1 indirecto doble, 1 indirecto triple */

    /* Utilizar una variable de alineación si es necesario  para vuestra plataforma/compilador   */
    char padding[INODOSIZE - 2 * sizeof(unsigned char) - 3 * sizeof(time_t) - 18 * sizeof(unsigned int) - 6 * sizeof(unsigned char)];
    // Fijarse que también se resta lo que ocupen las variables de alineación utilizadas!!!
};

// Declaramos las funciones de ficheros_basico.c

/// Calculate the size in blocks needed for the bitmap
/// \param nbloques Number of blocks
/// \return The size of the bitmap in blocks
int tamMB(unsigned int nbloques);
/// Calculate the size in blocks of the inodes array
/// \param ninodos Number of inodes.
/// \return Size of the inodes array in blocks
int tamAI(unsigned int ninodos);
/// Initializes the data of the superblock and writes it to the disk.
/// \param nbloques Number of blocks of the disk.
/// \param ninodos Number of inodes of the disk. It follow
/// the following heuristic: ninodos = nbloques/4.
/// \return FALLO if an error occurs, EXITO otherwise.
int initSB(unsigned int nbloques, unsigned int ninodos);
/// Initializes de bitmap that represents with 1 the
/// metadata.
/// \return FALLO if an error occurs, EXITO otherwise.
int initMB();
/// Initialize the array of inodes
/// \return FALLO if an error occurs, EXITO otherwise.
int initAI();
/// Makes the pow of a number.
/// \param base the base of the pow.
/// \param exponent the exponent of the pow.
/// \return the base powered to the exponent.
int power(int base, int exponent);
/// Modify the value of a bit in the bitmap(MB).
/// \param nbloque The block number where the bit we want to modify is located.
/// \param bit The value of the bit we want to write.
/// \return FALLO if an error occurs, EXITO otherwise.
int escribir_bit(unsigned int bloque, unsigned int);
/// Reads the value of a bit in the bitmap(MB).
/// \param nbloque The block number where the bit we want to read is located.
/// \return returns the value of the bit we want to read.
char leer_bit(unsigned int nbloque);
/// Finds the first free block, quering with MB, ocupates it and
/// return the position of that block.
/// \return the position of the first free block.
int reservar_bloque();
/// Frees the specified block.
/// \param nbloque number of the block to be freed.
/// \return the number of the freed block (nbloque); FALLO otherwise.
int liberar_bloque(unsigned int nbloque);
/// Writes an inodo at the specified inode of the array.
/// \param ninodo position of the array of the inodes where we
/// will write the new inode.
/// \param inodo inode to be written.
/// \return FALLO if an error occurs, EXITO otherwise.
int escribir_inodo(unsigned int ninodo, struct inodo *inodo);
/// Reads an inode from the array of the inodes and dumps
/// it into a struct inode variable.
/// \param ninodo number of the inode.
/// \param inodo variable of the type struct inode.
/// \return FALLO if an error occurs, EXITO otherwise.
int leer_inodo(unsigned int ninodo, struct inodo *inodo);
/// Finds the first free inode, reserves it, returns its number
/// and updates the linked list of free inodes.
/// \param tipo type of the inode.
/// \param permisos permissions of the inode.
/// \return the number of the first free inode; FALLO otherwise.
int reservar_inodo(unsigned char tipo, unsigned char permisos);
/// Optain the range of pointers where the logical block we
/// are searching is located. We also obtain the adress
/// stored in the corresponding inode pointer.
/// \param inodo inode where we are going to search the logical block.
/// \param nblogico number of the logical block.
/// \param ptr adress stored in the corresponding inode pointer.
/// \return the range of pointers where the logical block
int obtener_nRangoBL(struct inodo *inodo, unsigned int nblogico, unsigned int *ptr);
/// Obtains the index of the given logic block with the level
/// \param nblogico position of the logical block
/// \param nivel_punteros level of the indirect pointer
/// \return position of the physical block
int obtener_indice(unsigned int nblogico, int nivel_punteros);
/// Obtains the number of the physical block corresponding
/// to a specified logical block of the indicated inode.
/// \param inodo indicated inode.
/// \param nblogico number of the logical block.
/// \param reservar behavior of the function.
/// \return if reservar = 0 -> the function is used only to consult if the physical block
/// exists. If exists returns its position, if not returns FALLO;
/// if 1 -> the function is used to consult if the physical block exists and to
/// reserve a physical block. If the physical block exists, it returns its position.
/// If it doesn't exist, it reserves memory for it and returns its position.
int traducir_bloque_inodo(struct inodo *inodo, unsigned int nblogico, unsigned char reservar);
/// Frees an inode given its number
/// \param ninodo number of the inode to be freed
/// \return the number of the freed inode; FALLO otherwise
int liberar_inodo(unsigned int ninodo);
/// Frees all the occupied blocks from the logical block indicated by primerBL
/// (this one included).
/// \param primerBL first logical block to be freed.
/// \param inodo inode where we are going to free the blocks.
/// \return the number of freed blocks.
int liberar_bloques_inodo(unsigned int primerBL, struct inodo *inodo);
/// Frees the direct pointer blocks
/// \param nBL number of logical block
/// \param ultimoBL las logical block
/// \param inodo inode number
/// \param eof 0 if not eof; 1 otherwise
/// \return the number of freed blocks
int liberar_directos(unsigned int *nBL, unsigned int ultimoBL, struct inodo *inodo, int *eof);
/// Frees the indirect pointer blocks
/// \param nBL number of logical block
/// \param primerBL first logical block
/// \param ultimoBL las logical block
/// \param inodo inode
/// \param nRangoBL number of range
/// \param nivel_punteros level of poiters
/// \param ptr pointer
/// \param eof 0 if not eof; 1 otherwise
/// \return the number of freed blocks
int liberar_indirectos_recursivo(
    unsigned int *nBL, unsigned int primerBL, unsigned int ultimoBL, struct inodo *inodo, 
    int nRangoBL, unsigned int nivel_punteros, unsigned int *ptr, int *eof
);