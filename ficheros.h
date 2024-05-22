/*
    Autores:
        - Marc Link Cladera
        - Carlos Gálvez Mena
        - Jesús Castillo Benito
*/

#include "ficheros_basico.h"

struct STAT
{
    time_t atime; // Fecha y hora del último acceso a datos
    time_t mtime; // Fecha y hora de la última modificación de datos
    time_t ctime; // Fecha y hora de la última modificación del inodo
    
    unsigned int nlinks;             // Cantidad de enlaces de entradas en directorio
    unsigned int tamEnBytesLog;      // Tamaño en bytes lógicos (EOF)
    unsigned int numBloquesOcupados; // Cantidad de bloques ocupados zona de datos
    
    unsigned char tipo;     // Tipo ('l':libre, 'd':directorio o 'f':fichero)
    unsigned char permisos; // Permisos (lectura y/o escritura y/o ejecución)

};

/// Writes the content of a buffer on a file/directory.
/// \param ninodo id of the inode where we will write.
/// \param buf_original buffer that which content will be written to the
/// file.
/// \param offset initial position of writing.
/// \param nbytes number of bytes that will be written.
/// \return the number of bytes written. FALLO otherwise.
int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes);
/// Reads information from a file/directory and stores it in a memory buffer.
/// \param ninodo number of inode which corresponds the file/directory.
/// \param buf_original buffer where the information will be stored. The buffer must be inizialized with 0's.
/// \param offset the initial reading position with respect to the inode.
/// \param nbytes the number of bytes to read.
/// \return the number of bytes actually read. FALLO otherwise.
int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes);
/// Returns the meta-information of a file/directory: type, permissions, etc.
/// This information is placed at p_stat.
/// \param ninodo inode number
/// \param p_stat STAT where the meta-information is stored
/// \return EXITO if completed succesfully, FALLO if there is an error
int mi_stat_f(unsigned int ninodo, struct STAT *p_stat);
/// Change the permissions of a file/directory
/// \param ninodo inode number
/// \param permisos permissions to change
/// \return EXITO if completed succesfully, FALLO if there is an error
int mi_chmod_f(unsigned int ninodo, unsigned char permisos);
/// Truncates a file/directory of the corresponding inode to the
/// given bytes
/// \param ninodo inode number
/// \param nbytes desired size
/// \return Number of released block if there is no error. FALLO if there is an error.
int mi_truncar_f(unsigned int ninodo, unsigned int nbytes);