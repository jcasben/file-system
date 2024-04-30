#include "directorios.h"

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
        /*
        const char *delim = "/";
        const char *slice = strtok(aux, delim);
        strcpy(inicial, slice);
        *final = '/';

        const char *trim = strtok(NULL, "\0");
        if(trim != NULL) strcpy(final+1, trim);
        *tipo = 'd';
        */
        strcpy(final, barPos);
        const int aux_len = strlen(aux);
        const int final_len = strlen(final);
        strncpy(inicial, aux, aux_len - final_len);
        inicial[aux_len - final_len] = '\0';
        *tipo = 'd';
    }

    return EXITO;
}

int buscar_entrada(
    const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, 
    unsigned int *p_entrada, char reservar, unsigned char permisos
)
{
    struct entrada entry;
    struct inodo inode;
    char start[sizeof(entry.nombre)];
    char final[strlen(camino_parcial)];
    char type;
    int num_entries_inode, entry_inode;

    if (strcmp("/", camino_parcial) == 0)
    {
        struct superbloque SB;
        if (bread(posSB, &SB) == FALLO) return FALLO;
        *p_inodo = SB.posInodoRaiz;
        p_entrada = 0;

        return 0;
    }

    if (extraer_camino(camino_parcial, start, final, &type) == FALLO)
        return ERROR_CAMINO_INCORRECTO;

    leer_inodo(*p_inodo_dir, &inode);
    if ((inode.permisos & 4) != 4) return ERROR_PERMISO_LECTURA;

    struct entrada buffer_lec[BLOCKSIZE/sizeof(struct entrada)];
    memset(buffer_lec, 0, BLOCKSIZE);
    //calcular cantidad de entradas inodo
    num_entries_inode = inode.tamEnBytesLog / sizeof(struct entrada);
    entry_inode = 0;

    if (num_entries_inode > 0)
    {
        int found = 0;
        int nbloc = 0;
        
        while((entry_inode < num_entries_inode) && (strcmp(start, entry.nombre) != 0))
        {
            mi_read_f(*p_inodo_dir, buffer_lec, nbloc * (BLOCKSIZE / sizeof(struct entrada)), BLOCKSIZE);
            for (size_t j = 0; (j < BLOCKSIZE / sizeof(struct entrada)) && !found ; j++)
            {
                // incrementar el numero de entradas revisadas.
                *p_entrada = *p_entrada + 1;
                //comparar nombre con entrada
                if (strcmp(buffer_lec[j].nombre, start) == 0)
                {
                    found = 1;
                    *p_inodo = buffer_lec[j].ninodo;
                    break;  
                }
            }
            memset(buffer_lec, 0, BLOCKSIZE);
            entry_inode++;
        }
        
    }

    if ((strcmp(start, entry.nombre) != 0) && entry_inode == num_entries_inode)
    {
        switch (reservar)
        {
        case 0:
            return ERROR_NO_EXISTE_ENTRADA_CONSULTA;
            break;
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
            else
            {
                strcpy(entry.nombre, start);
                if(type == 'd')
                {
                    if (strcmp(final, "/") == 0)
                    {
                        entry.ninodo = reservar_inodo('d', permisos);
                    }
                    else
                    {
                        return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;
                    }
                }
                else
                {
                    entry.ninodo = reservar_inodo('f', permisos);
                }
                //escribir entrada en el directorio padre
                if(mi_write_f(*p_inodo_dir, &entry, *p_entrada * sizeof(struct entrada), sizeof(struct entrada)) == FALLO)
                {
                    if (entry.ninodo != FALLO)
                    {
                        liberar_inodo(entry.ninodo);
                    }
                    return FALLO;
                }
            }            
            break;
        }
    }

    if(strcmp(final, "/") == 0 || strcmp(final, "") == 0)
    {
        if((entry_inode < num_entries_inode) && (reservar == 1))
        {
            return ERROR_ENTRADA_YA_EXISTENTE;
        }
        *p_inodo = entry.ninodo;
        *p_entrada = entry_inode;
        return EXITO;
    }
    else
    {
        *p_inodo_dir = entry.ninodo;
        return buscar_entrada(final, p_inodo_dir, p_inodo, p_entrada, reservar, permisos); 
    }
    return EXITO;
}

void mostrar_error_buscar_entrada(int error)
{
    switch (error)
    {
        case -2: fprintf(stderr, "Error: Camino incorrecto.\n"); break;
        case -3: fprintf(stderr, "Error: Permiso denegado de lectura.\n"); break;
        case -4: fprintf(stderr, "Error: No existe el archivo o el directorio.\n"); break;
        case -5: fprintf(stderr, "Error: No existe algún directorio intermedio.\n"); break;
        case -6: fprintf(stderr, "Error: Permiso denegado de escritura.\n"); break;
        case -7: fprintf(stderr, "Error: El archivo ya existe.\n"); break;
        case -8: fprintf(stderr, "Error: No es un directorio.\n"); break;
    }
}

int mi_creat(const char *camino, unsigned char permisos)
{
    struct superbloque SB;
    bread(posSB, &SB);
    mostrar_error_buscar_entrada(buscar_entrada(camino, SB.posInodoRaiz, 0, 0, 1, permisos));
}

int mi_dir(const char *camino, char *buffer, char tipo, char flag)
{
    struct superbloque SB;
    bread(posSB, &SB);

    int ninodo = 0;
    // permisos??
    buscar_entrada(camino, SB.posInodoRaiz, &ninodo, 0, 0, 0);
    struct inodo inode;
    leer_inodo(ninodo, &inode);
    char header[] = "TYPE\t\tPERMISSIONS\t\tmTIME\t\tNAME"
        "\n-----------------------------------------------------\n";
    
    if (inode.tipo == 'd')
    {
        
    }
}