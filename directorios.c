#include "directorios.h"

#define DEBUGN7 1

/// Given a string that starts with '/', divides its content in two parts:
/// * inicial: content inside the first two '/' (directory name) | if there's not a second '/' (file name);
/// * final: the rest of the content of the string.
///
/// \param camino string that contains the path
/// \param inicial
/// \param final
/// \param tipo
/// \return
int extraer_camino(const char *camino, char *inicial, char *final, char *tipo)
{
    if (camino == NULL || inicial == NULL || final == NULL || tipo == NULL || *camino != '/') 
    {
        return FALLO; // Error: puntero nulo o camino no empieza por '/'
    }

    char *aux = (char *)camino + 1;
    char *barPos = strchr((aux + 1), '/');

    if(barPos == NULL)
    {
        strcpy(inicial, aux + 1);
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

///
/// \param camino_parcial
/// \param p_inodo_dir
/// \param p_inodo
/// \param p_entrada
/// \param reservar
/// \param permisos
/// \return
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
            "[buscar_entrada() -> inicial: %s, end: %s, reservar: %d]\n"
            RESET,
            beginning, end, reservar
    );
#endif

    leer_inodo(*p_inodo_dir, &inode);
    if ((inode.permisos & 4) != 4) return ERROR_PERMISO_LECTURA;

    memset(buffer_lec, 0, BLOCKSIZE);
    //calcular cantidad de entradas inodo
    int cant_entries_inode = inode.tamEnBytesLog / sizeof(struct entrada), entry_inode_number = 0;

    if (cant_entries_inode > 0)
    {
        int found = 0;
        int nbloc = 0;
        
        while((entry_inode_number < cant_entries_inode) && (strcmp(beginning, buffer_lec[entry_inode_number].nombre) != 0))
        {
            entry_inode_number = 0;
            memset(buffer_lec, 0, BLOCKSIZE);
            mi_read_f(*p_inodo_dir, buffer_lec, nbloc * (BLOCKSIZE / sizeof(struct entrada)), BLOCKSIZE);
            for (size_t j = 0; (j < BLOCKSIZE / sizeof(struct entrada)) && !found ; j++)
            {
                // incrementar el numero de entradas revisadas.
                *p_entrada = *p_entrada + 1;
                //comparar nombre con entrada
                if (strcmp(buffer_lec[j].nombre, beginning) == 0)
                {
                    found = 1;
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
            
            //Creamos la entrada en el directorio referenciado por *p_inodo_dir
            //si es fichero no permitir escritura
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
            //escribir entrada en el directorio padre
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

int mi_creat(const char *camino, unsigned char permisos)
{
    struct superbloque SB;
    bread(posSB, &SB);
    mostrar_error_buscar_entrada(buscar_entrada(camino, &SB.posInodoRaiz, 0, 0, 1, permisos));

    return EXITO;
}

int mi_dir(const char *camino, char *buffer, char tipo, char flag)
{
    struct superbloque SB;
    bread(posSB, &SB);

    unsigned int p_inode_dir;
    unsigned int n_inode = 0;
    unsigned int p_entry = 0;
    buscar_entrada(camino, &p_inode_dir, &n_inode, &p_entry, 0, 6);
    // Read inode with the assigned value of the function buscar_entrada()
    struct inodo inode;
    leer_inodo(n_inode, &inode);

    char header[] = "TYPE\t\tPERMISSIONS\t\tmTIME\t\tNAME"
        "\n-----------------------------------------------------\n";
    
    if (inode.tipo == 'd')
    {
        
    }

    return EXITO;
}