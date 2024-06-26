/*
    Autores:
        - Marc Link Cladera
        - Carlos Gálvez Mena
        - Jesús Castillo Benito
*/

#include "ficheros.h"
#include <sys/time.h>

#define ERROR_CAMINO_INCORRECTO (-2)
#define ERROR_PERMISO_LECTURA (-3)
#define ERROR_NO_EXISTE_ENTRADA_CONSULTA (-4)
#define ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO (-5)
#define ERROR_PERMISO_ESCRITURA (-6)
#define ERROR_ENTRADA_YA_EXISTENTE (-7)
#define ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO (-8)
#define TAMFILA 100
#define TAMBUFFER (TAMFILA * 1000)
#define TAMNOMBRE 60 //tamaño del nombre de directorio o fichero, en Ext2 = 256
#define PROFUNDIDAD 32 //profundidad máxima del árbol de directorios
#define USARCACHE 2 // Nivel de cache -> 0:sin caché, 1: última L/E, 2:tabla FIFO, 3:tabla LRU
#if (USARCACHE==2 || USARCACHE==3)
    #define CACHE_SIZE 3
#endif

struct entrada
{
    char nombre[TAMNOMBRE];
    unsigned int ninodo;
};

struct UltimaEntrada
{
    char camino [TAMNOMBRE*PROFUNDIDAD];
    unsigned int p_inodo; 
    #if USARCACHE==2 // tabla FIFO (segunda oportunidad)
        unsigned char a;
    #endif
    #if USARCACHE==3 // tabla LRU
        struct timeval ultima_consulta;
    #endif
};

#if USARCACHE==2
    struct CacheFIFO
    {
        struct UltimaEntrada lastEntries[CACHE_SIZE];
        unsigned int last;
        unsigned int size;
    };
#endif
#if USARCACHE==3
    struct CacheLRU
    {
        struct UltimaEntrada lastEntries[CACHE_SIZE];
        unsigned int size;
    };
#endif


/// Given a string that starts with '/', divides its content in two parts:
/// * inicial: content inside the first two '/' (directory name) | if there's not a second '/' (file name);
/// * final: the rest of the content of the string.
/// \param camino string that contains the path
/// \param inicial first part of the path
/// \param final second part of the path
/// \param tipo 'd' if the path refers to a directory; 'f' if the path refers to a file.
/// \return EXITO if the operation was successfull.
int extraer_camino(const char *camino, char *inicial, char *final, char *tipo);
/// Search an entry in our file system given a path
/// \param camino_parcial path to the entry
/// \param p_inodo_dir number of the inode of actual directory
/// \param p_inodo inode number of the specified entry
/// \param p_entrada number of entry
/// \param reservar 0 if we only want to consult; 1 if we want to create a new entry
/// \param permisos permissions of the new entries
/// \return EXITO if the entry was found, FALLO error code otherwise
int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos);
/// Prints the corresponding error given its number
/// \param error number of the error.
void mostrar_error_buscar_entrada(int error);
/// Creates an entry given its path
/// \param camino path of the new entry
/// \param permisos permissions
/// \return EXITO
int mi_creat(const char *camino, unsigned char permisos);
/// Builds a buffer with the information of the entries of a directory or a file.
/// \param camino path to the file or directory
/// \param buffer buffer to save the information
/// \param tipo type of the entry (directory or file)
/// \param flag l if an extended view is wanted
/// \return EXITO
int mi_dir(const char *camino, char *buffer, char tipo, char flag);
/// Writes the normal version of an entry onto a buffer
/// \param entry entry to be written
/// \param buffer destination buffer
/// \return EXITO
int build_buffer(struct entrada entry, char *buffer);
/// Writes the extended version of an entry onto a buffer
/// \param entry entry to be written
/// \param buffer destination buffer
/// \return EXITO
int build_extended_buffer(struct entrada, char *buffer);
/// Modify the permissions of a file
/// \param camino the path of the file
/// \param permisos the new permissions of the file
/// \return EXITO if the permissions were modified successfully, FALLO otherwise
int mi_chmod(const char *camino, unsigned char permisos);
/// Gets the stats of a file
/// \param camino the path of the file
/// \param p_stat pointer to the struct where the stats will be stored
/// \return inode number of the entry, FALLO if the stats were not obtained successfully
int mi_stat(const char *camino, struct STAT *p_stat);
/// Auxiliar function to test functionality
/// \param camino path to the file or directory
/// \param reservar 1 if we want to reserve the entry; 0 if we want to consult
void mostrar_buscar_entrada(char *camino, char reservar);
/// Writes the content of a buffer in the given file
/// \param camino file's path
/// \param buf data to write
/// \param offset offset for written data
/// \param nbytes number of bytes to write
/// \return number of written bytes
int mi_write(const char *camino, const void *buf, unsigned int offset, unsigned int nbytes);
#if USARCACHE==2
/// Searches an entry in a FIFO Cache
/// \param camino path to the entry
/// \param cache structure where the cache is stored
/// \return the index of the entry in the cache; -1 if it doesn't exist in the cache
int searchEntry(const char *camino, struct CacheFIFO *cache);
/// Adds an entry to the LRU Cache
/// \param cache structure where the FIFO Cache is stored
/// \param camino path to the entry
/// \param p_inodo inode number of the entry
unsigned int updateCache(struct CacheFIFO *cache, const char *camino, const unsigned int *p_inodo);
#endif
#if USARCACHE==3
/// Searches an entry in the LRU Cache
/// \param camino path of the entry
/// \param cache structure where the LRU Cache is stored
/// \return the index of the entry in the cache; -1 if it doesn't exist in the cache
int searchEntry(const char *camino, struct CacheLRU *cache);
/// Adds an entry to the LRU Cache
/// \param cache structure where the LRU Cache is stored
/// \param camino path to the entry
/// \param p_inodo inode number of the entry
unsigned int updateCache(struct CacheLRU *cache, const char *camino, const unsigned int *p_inodo);
/// Compares two time stamps
/// \param t1 time stamp 1
/// \param t2 time stamp 2
/// \return 1 if t1 is greater; -1 if t2 is greater
int compareTimevals(const struct timeval *t1, const struct timeval *t2);
#endif
/// Reads the content of a file and stores it in a buffer
/// \param camino file's path
/// \param buf storing for the read data
/// \param offset offset for read data
/// \param nbytes number of bytes to read
/// \return number of read bytes
int mi_read(const char *camino, void *buf, unsigned int offset, unsigned int nbytes);
/// Creates a link to the directory entry of the given path
/// \param camino1 path to the entry to be linked
/// \param camino2 path of the new link
/// \return EXITO
int mi_link(const char *camino1, const char *camino2);
/// Deletes the entry of the specified path
/// \param camino entry to be deleted
/// \return EXITO
int mi_unlink(const char *camino);
/// Checks if a path is an existing directory
/// \param camino path to the entry
/// \return EXITO if it is an existing directory
int is_directory(const char *camino);