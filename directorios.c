#include "directorios.h"

int extraer_camino(const char *camino, char *inicial, char *final, char *tipo)
{
    if (camino == NULL || inicial == NULL || final == NULL || tipo == NULL || *camino != '/') 
    {
        return FALLO; // Error: puntero nulo o camino no empieza por '/'
    }

    char *aux = (char *)camino;
    char *barPos = strchr((aux + 1), '/');

    if(barPos == NULL)
    {
        strcpy(inicial, aux + 1);
        memset(final, 0, strlen(final));
        *tipo = 'f';
    }
    else
    {
        strcpy(inicial, strtok(aux, "/"));
        *final = '/'; 

        char *trim = strtok(NULL, "\0");
        if(trim != NULL) strcpy(final+1, trim);
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
        p_inodo = SB.posInodoRaiz;
        p_entrada = 0;

        return 0;
    }

    if (extraer_camino(camino_parcial, start, final, &type) == FALLO)
        return ERROR_CAMINO_INCORRECTO;

    leer_inodo(*p_inodo_dir, &inode);
    if (inode.permisos & 4 != 4) return ERROR_PERMISO_LECTURA;

    struct entrada buffer_lec[BLOCKSIZE/sizeof(struct entrada)];
    memset(buffer_lec, 0, BLOCKSIZE);
    //calcular cantidad de entradas inodo
    num_entries_inode = inode.tamEnBytesLog / sizeof(struct entrada);
    entry_inode = 0;

    if (num_entries_inode > 0)
    {
                int found = 0;
        for (size_t i = 0; (i < 12) && !found; i++)
        {
            // mi_read_f(*p_inodo_dir, &buffer_lec, BLOCKSIZE * punterosDirectos[i], BLOCKSIZE);
            mi_read_f(inode.punterosDirectos[i], &buffer_lec, 0, BLOCKSIZE);
            for (size_t j = 0; (j < BLOCKSIZE / sizeof(struct entrada)) && !found ; j++)
            {
                // incrementar el numero de entradas revisadas.
                *p_entrada = *p_entrada + 1;
                //comparar nombre con entrada
                if (strcmp(buffer_lec[j].nombre, start) == 0)
                {
                    found = 1;
                    entry_inode++;
                    *p_inodo = buffer_lec[j].ninodo;
                    break;  
                }
            }
        }
        
        
    }

    if ()
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
