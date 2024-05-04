#include "directorios.h"

#define DEBUGN7 1
static struct UltimaEntrada UltimaEntradaEscritura;

//----------------------------- NIVEL 7 (11/04/2023 - 01/05/2024) -----------------------------

/// Given a string that starts with '/', divides its content in two parts:
/// * inicial: content inside the first two '/' (directory name) | if there's not a second '/' (file name);
/// * final: the rest of the content of the string.
/// \param camino string that contains the path
/// \param inicial first part of the path
/// \param final second part of the path
/// \param tipo 'd' if the path refers to a directory; 'f' if the path refers to a file.
/// \return EXITO if the operation was successfull.
int extraer_camino(const char *camino, char *inicial, char *final, char *tipo)
{
    //If the path is null or if path doesn't start with '/'
    if (camino == NULL || inicial == NULL || final == NULL || tipo == NULL || *camino != '/') 
    {
        return FALLO;
    }

    char *aux = (char *)camino + 1;
    char *barPos = strchr(aux, '/');

    if(barPos == NULL)
    {
        strcpy(inicial, aux);
        memset(final, 0, strlen(final));
        *tipo = 'f';
    }
    else
    {
        strcpy(final, barPos);
        const size_t aux_len = strlen(aux);
        const size_t final_len = strlen(final);
        strncpy(inicial, aux, aux_len - final_len);
        inicial[aux_len - final_len] = '\0';
        *tipo = 'd';
    }

    return EXITO;
}

/// Search an entry in our file system given a path
/// \param camino_parcial path to the entry
/// \param p_inodo_dir number of the inode of actual directory
/// \param p_inodo inode number of the specified entry
/// \param p_entrada number of entry
/// \param reservar 0 if we only want to consult; 1 if we want to create a new entry
/// \param permisos permissions of the new entries
/// \return the inode number of the desired entry
int buscar_entrada(
    const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, 
    unsigned int *p_entrada, char reservar, unsigned char permisos
)
{
    struct entrada buffer_lec[BLOCKSIZE/sizeof(struct entrada)];
    struct inodo inode;
    char beginning[sizeof(buffer_lec[0].nombre)];
    char end[strlen(camino_parcial)];
    char type;

    if (strcmp("/", camino_parcial) == 0)
    {
        struct superbloque SB;
        if (bread(posSB, &SB) == FALLO) return FALLO;
        *p_inodo = SB.posInodoRaiz;
        p_entrada = 0;

        return 0;
    }

    if (extraer_camino(camino_parcial, beginning, end, &type) == FALLO)
        return ERROR_CAMINO_INCORRECTO;
#if DEBUGN7
    fprintf(
            stderr,
            GRAY
            "[buscar_entrada() -> inicial: %s, final: %s, reservar: %d]\n"
            RESET,
            beginning, end, reservar
    );
#endif

    leer_inodo(*p_inodo_dir, &inode);
    if ((inode.permisos & 4) != 4) return ERROR_PERMISO_LECTURA;

    memset(buffer_lec, 0, BLOCKSIZE);
    //Calculate the number of entrys in the inode
    int cant_entries_inode = inode.tamEnBytesLog / sizeof(struct entrada), entry_inode_number = 0;

    if (cant_entries_inode > 0)
    {
        int nbloc = 0;
        
        while((entry_inode_number < cant_entries_inode) && (strcmp(beginning, buffer_lec[entry_inode_number].nombre) != 0))
        {
            entry_inode_number = 0;
            memset(buffer_lec, 0, BLOCKSIZE);
            mi_read_f(*p_inodo_dir, buffer_lec, nbloc * (BLOCKSIZE / sizeof(struct entrada)), BLOCKSIZE);
            for (size_t j = 0; j < cant_entries_inode; j++)
            {
                //Increment the number of revisde entries
                *p_entrada = *p_entrada + 1;
                //Check if it's the entry that we are searching
                if (strcmp(buffer_lec[j].nombre, beginning) == 0)
                {
                    *p_inodo = buffer_lec[j].ninodo;
                    break;  
                }
                entry_inode_number++;
            }
            nbloc++;
        }
    }

    if (strcmp(beginning, buffer_lec[entry_inode_number].nombre) != 0 && entry_inode_number == cant_entries_inode)
    {
        switch (reservar)
        {
        case 0:
            return ERROR_NO_EXISTE_ENTRADA_CONSULTA;

        case 1:
            
            //Create entry in the directory refered by *p_inodo_dir
            //If it's a file, don't allow writing
            if(inode.tipo == 'f') 
            {
                return ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO;
            }
            
            if((inode.permisos & 2) == 0)
            {
                return ERROR_PERMISO_ESCRITURA;
            }

            strcpy(buffer_lec[entry_inode_number].nombre, beginning);
            if(type == 'd')
            {
                if (strcmp(end, "/") == 0)
                {
                    buffer_lec[entry_inode_number].ninodo = reservar_inodo('d', permisos);
#if DEBUGN7
                    fprintf(
                            stderr,
                            GRAY
                            "[buscar_entrada() -> reservado inodo %d tipo %c con permisos %d para %s]\n"
                            RESET,
                            buffer_lec[entry_inode_number].ninodo, type, permisos, beginning
                    );
#endif
                }
                else
                {
                    return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;
                }
            }
            else
            {
                buffer_lec[entry_inode_number].ninodo = reservar_inodo('f', permisos);
#if DEBUGN7
                fprintf(
                        stderr,
                        GRAY
                        "[buscar_entrada() -> reservado inodo %d tipo %c con permisos %d para %s]\n"
                        RESET,
                        buffer_lec[entry_inode_number].ninodo, type, permisos, beginning
                );
#endif
            }
            //Write entry in the father directory
            if(mi_write_f(
                *p_inodo_dir,
                &buffer_lec[entry_inode_number],
                *p_entrada * sizeof(struct entrada),
                sizeof(struct entrada)
            ) == FALLO)
            {
                if (buffer_lec[entry_inode_number].ninodo != FALLO)
                {
                    liberar_inodo(buffer_lec[entry_inode_number].ninodo);
                }
                return FALLO;
            }
#if DEBUGN7
                fprintf(
                        stderr,
                        GRAY
                        "[buscar_entrada() -> creada entrada: %s, %d]\n"
                        RESET,
                        buffer_lec[entry_inode_number].nombre, buffer_lec[entry_inode_number].ninodo
                );
#endif

            break;
        default:
            return FALLO;
        }
    }

    if(strcmp(end, "/") == 0 || strcmp(end, "") == 0)
    {
        if((entry_inode_number < cant_entries_inode) && (reservar == 1))
        {
            return ERROR_ENTRADA_YA_EXISTENTE;
        }
        *p_inodo = buffer_lec[entry_inode_number].ninodo;
        *p_entrada = entry_inode_number;

        return EXITO;
    }
    *p_inodo_dir = buffer_lec[entry_inode_number].ninodo;
    *p_inodo = 0;
    *p_entrada = 0;

    return buscar_entrada(end, p_inodo_dir, p_inodo, p_entrada, reservar, permisos);
}

/// Prints the corresponding error given its number
/// \param error number of the error.
void mostrar_error_buscar_entrada(int error)
{
    switch (error)
    {
        case -2: fprintf(stderr, RED "Error: Camino incorrecto.\n" RESET); break;
        case -3: fprintf(stderr, RED "Error: Permiso denegado de lectura.\n" RESET); break;
        case -4: fprintf(stderr, RED "Error: No existe el archivo o el directorio.\n" RESET); break;
        case -5: fprintf(stderr, RED "Error: No existe algún directorio intermedio.\n" RESET); break;
        case -6: fprintf(stderr, RED "Error: Permiso denegado de escritura.\n" RESET); break;
        case -7: fprintf(stderr, RED "Error: El archivo ya existe.\n" RESET); break;
        case -8: fprintf(stderr, RED "Error: No es un directorio.\n" RESET); break;
    }
}

//----------------------------- NIVEL 8 (25/04/2023 - ) -----------------------------

int mi_creat(const char *camino, unsigned char permisos)
{
    struct superbloque SB;
    bread(posSB, &SB);
    unsigned int p_inode_dir = SB.posInodoRaiz;
    unsigned int n_inode = 0;
    unsigned int p_entry = 0;
    mostrar_error_buscar_entrada(
        buscar_entrada(camino, &p_inode_dir, &n_inode, &p_entry, 1, permisos)
    );

    return EXITO;
}

int mi_dir(const char *camino, char *buffer, char tipo, char flag)
{
    struct superbloque SB;
    bread(posSB, &SB);

    unsigned int p_inode_dir = 0;
    unsigned int n_inode = 0;
    unsigned int p_entry = 0;
    mostrar_error_buscar_entrada(
        buscar_entrada(camino, &p_inode_dir, &n_inode, &p_entry, 0, 6)
    );

    // Read inode with the assigned value of the function buscar_entrada()
    struct inodo inode;
    leer_inodo(n_inode, &inode);
    unsigned long cant_entries_inode = inode.tamEnBytesLog / sizeof(struct entrada);
    if (inode.tipo != tipo)
    {
        fprintf(stderr, RED "ERROR: the type of the file doesn't correspond with the path\n" RESET);
        return FALLO;
    }

    if (cant_entries_inode > 0)
    {
        if (flag == 'l')
        {
            printf("Total: %lu\n", cant_entries_inode);
            printf("\nTYPE\tPERMISSIONS\tmTIME\t\t\tSIZE\tNAME\n"
                   "---------------------------------------------------------------------\n");
        }
        int nbloc = 0;
        int entry_inode_number = 0;

        while(entry_inode_number < cant_entries_inode)
        {
            entry_inode_number = 0;
            struct entrada buffer_lec[BLOCKSIZE/sizeof(struct entrada)];
            mi_read_f(n_inode, buffer_lec, nbloc * (BLOCKSIZE / sizeof(struct entrada)), BLOCKSIZE);
            for (size_t j = 0; j < cant_entries_inode; j++)
            {
                if (flag == 'l') build_extended_buffer(buffer_lec[j], buffer);
                else build_buffer(buffer_lec[j], buffer);
                entry_inode_number++;
            }
            nbloc++;
        }
    }

    return EXITO;
}

int build_buffer(struct entrada entry, char *buffer)
{
    struct inodo inode;
    leer_inodo(entry.ninodo, &inode);
    char tmp[sizeof(entry.nombre) + 20];
    char *aux;

    if (inode.tipo == 'd') sprintf(tmp, BLUE "%s\t" RESET, entry.nombre);
    else sprintf(tmp, "%s\t", entry.nombre);

    // Check if we have to create a new line
    size_t num_rows = strlen(buffer) / TAMFILA;
    aux = buffer + (num_rows * TAMFILA);
    if (strlen(aux + strlen(tmp)) >= TAMFILA + 20) strcat(buffer, "\n");

    strcat(buffer, tmp);

    return EXITO;
}

int build_extended_buffer(struct entrada entry, char *buffer)
{
    struct inodo inode;
    leer_inodo(entry.ninodo, &inode);
    char type[2];
    strcpy(type, (char[2]) { (char) inode.tipo, '\0' });

    // Entry type
    strcat(buffer, type);
    strcat(buffer, "\t");
    // Entry permissions
    if (inode.permisos & 4) strcat(buffer, "r"); else strcat(buffer, "-");
    if (inode.permisos & 2) strcat(buffer, "w"); else strcat(buffer, "-");
    if (inode.permisos & 1) strcat(buffer, "x"); else strcat(buffer, "-");
    strcat(buffer, "\t");
    strcat(buffer, "\t");
    // Entry mTIME
    char tmp[20];
    struct tm *tm; //ver info: struct tm
    tm = localtime(&inode.mtime);
    sprintf(tmp, "%d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min,  tm->tm_sec);
    strcat(buffer, tmp);
    strcat(buffer, "\t");
    // Entry size
    char tmp2[20];
    sprintf(tmp2, "%u", inode.tamEnBytesLog);
    strcat(buffer, tmp2);
    strcat(buffer, "\t");
    // Entry name
    char name[80];
    if (inode.tipo == 'd') sprintf(name, BLUE "%s" RESET, entry.nombre);
    else sprintf(name, "%s", entry.nombre);
    strcat(buffer, name);
    strcat(buffer, "\n");

    return EXITO;
}

/// Modify the permissions of a file
/// \param camino the path of the file
/// \param permisos the new permissions of the file
/// \return EXITO if the permissions were modified successfully, FALLO otherwise
int mi_chmod(const char *camino, unsigned char permisos)
{
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 6);
    if (error < 0) {
        mostrar_error_buscar_entrada(error);
        return FALLO;
    }
    return mi_chmod_f(p_inodo, permisos);
}

/// Gets the stats of a file
/// \param camino the path of the file
/// \param p_stat pointer to the struct where the stats will be stored
/// \return EXITO if the stats were obtained successfully, FALLO otherwise
int mi_stat(const char *camino, struct STAT *p_stat)
{
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 6);
    if (error < 0) {
        mostrar_error_buscar_entrada(error);
        return FALLO;
    }
    printf("Nº de inodo: %d", p_inodo);
    return mi_stat_f(p_inodo, p_stat);
}

//----------------------------- NIVEL 9 (02/05/2023 - ) -----------------------------

/// Writes the content of a buffer in the given file
/// \param camino file's path
/// \param buf data to write
/// \param offset offset for written data
/// \param nbytes number of bytes to write
/// \return number of written bytes
int mi_write(const char *camino, const void *buf, unsigned int offset, unsigned int nbytes)
{

    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 6);
    if (error  < 0) {
        mostrar_error_buscar_entrada(error);
    }

    int written_bytes = mi_write_f(p_inodo, buf, offset, nbytes);


    return written_bytes;
}

/// Reads the content of a file and stores it in a buffer
/// \param camino file's path
/// \param buf storing for the read data
/// \param offset offset for read data
/// \param nbytes number of bytes to read
/// \return number of read bytes
int mi_read(const char *camino, void *buf, unsigned int offset, unsigned int nbytes)
{
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 6);
    if (error  < 0) {
        mostrar_error_buscar_entrada(error);
    }

    int read_bytes = mi_read_f(p_inodo, buf, offset, nbytes);


    return read_bytes;
}