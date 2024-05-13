/*
    Autores:
        - Marc Link Cladera
        - Carlos Gálvez Mena
        - Jesús Castillo Benito
*/

#include "directorios.h"

#define DEBUGN7 0

#if USARCACHE==1
    static struct UltimaEntrada UltimaEntradaEscritura;
    static struct UltimaEntrada UltimaEntradaLectura;
#endif

#if USARCACHE==2
    static struct CacheFIFO writeCache;
    static struct CacheFIFO readCache;
    static char initWriteCache = 0;
    static char initReadCache = 0;
#endif
#if USARCACHE==3
    static struct CacheLRU writeCache;
    static struct CacheLRU readCache;
    static char initWriteCache = 0;
    static char initReadCache = 0;
#endif

//----------------------------- NIVEL 7 (11/04/2023 - 01/05/2024) -----------------------------

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
        *p_entrada = 0;

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
        int found = 0;
        memset(buffer_lec, 0, BLOCKSIZE);
        mi_read_f(*p_inodo_dir, buffer_lec, nbloc * BLOCKSIZE, BLOCKSIZE);

        while(entry_inode_number < cant_entries_inode)
        {
            for (size_t j = 0; j < cant_entries_inode; j++)
            {
                //Increment the number of revisde entries
                *p_entrada = *p_entrada + 1;
                //Check if it's the entry that we are searching
                if (strcmp(buffer_lec[j].nombre, beginning) == 0)
                {
                    found = 1;
                    *p_inodo = buffer_lec[j].ninodo;
                    break;
                }
                entry_inode_number++;
                if (entry_inode_number == (BLOCKSIZE / sizeof(struct entrada)) ) break;
            }
            if (found == 1) break;
            nbloc++;
            memset(buffer_lec, 0, BLOCKSIZE);
            mi_read_f(*p_inodo_dir, buffer_lec, nbloc * BLOCKSIZE, BLOCKSIZE);
        }
    }

    const int index = entry_inode_number % (BLOCKSIZE / sizeof(struct entrada));
    if (strcmp(beginning, buffer_lec[index].nombre) != 0 && entry_inode_number == cant_entries_inode)
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

            strcpy(buffer_lec[index].nombre, beginning);
            if(type == 'd')
            {
                if (strcmp(end, "/") == 0)
                {
                    buffer_lec[index].ninodo = reservar_inodo('d', permisos);
#if DEBUGN7
                    fprintf(
                            stderr,
                            GRAY
                            "[buscar_entrada() -> reservado inodo %d tipo %c con permisos %d para %s]\n"
                            RESET,
                            buffer_lec[index].ninodo, type, permisos, beginning
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
                buffer_lec[index].ninodo = reservar_inodo('f', permisos);
#if DEBUGN7
                fprintf(
                        stderr,
                        GRAY
                        "[buscar_entrada() -> reservado inodo %d tipo %c con permisos %d para %s]\n"
                        RESET,
                        buffer_lec[index].ninodo, type, permisos, beginning
                );
#endif
            }
            //Write entry in the father directory
            if(mi_write_f(
                *p_inodo_dir,
                &buffer_lec[index],
                *p_entrada * sizeof(struct entrada),
                sizeof(struct entrada)
            ) == FALLO)
            {
                if (buffer_lec[index].ninodo != FALLO)
                {
                    liberar_inodo(buffer_lec[index].ninodo);
                }
                return FALLO;
            }
#if DEBUGN7
                fprintf(
                        stderr,
                        GRAY
                        "[buscar_entrada() -> creada entrada: %s, %d]\n"
                        RESET,
                        buffer_lec[index].nombre, buffer_lec[index].ninodo
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
        *p_inodo = buffer_lec[index].ninodo;
        *p_entrada = entry_inode_number;

        return EXITO;
    }
    *p_inodo_dir = buffer_lec[index].ninodo;
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

//----------------------------- NIVEL 8 (25/04/2023 - 05/05/2024) -----------------------------

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
    int error = buscar_entrada(camino, &p_inode_dir, &n_inode, &p_entry, 0, 6);
    if (error < 0)
    {
        mostrar_error_buscar_entrada(error);
        return error;
    }

    // Read inode with the assigned value of the function buscar_entrada()
    struct inodo inode;
    leer_inodo(n_inode, &inode);
    unsigned long cant_entries_inode = inode.tamEnBytesLog / sizeof(struct entrada);
    if (inode.tipo != tipo)
    {
        fprintf(stderr, RED "ERROR: the type of the file doesn't correspond with the path\n" RESET);
        return FALLO;
    }
    
    if (tipo == 'd' && cant_entries_inode > 0)
    {
        printf("Total: %lu\n", cant_entries_inode);
        if (flag == 'l')
        {
            printf("\nTYPE\tPERMISSIONS\tmTIME\t\t\tSIZE\tNAME\n"
                       "---------------------------------------------------------------------\n");
        }
        int nbloc = 0;
        int entry_inode_number = 0;

        struct entrada buffer_lec[BLOCKSIZE/sizeof(struct entrada)];
        mi_read_f(n_inode, buffer_lec, nbloc * BLOCKSIZE, BLOCKSIZE);

        while(entry_inode_number < cant_entries_inode)
        {

            for (size_t j = 0; j < cant_entries_inode; j++)
            {
                if (flag == 'l') build_extended_buffer(buffer_lec[j], buffer);
                else build_buffer(buffer_lec[j], buffer);
                entry_inode_number++;
                if (entry_inode_number == BLOCKSIZE / sizeof(struct entrada)) break;
            }
            nbloc++;
            memset(buffer_lec, 0, BLOCKSIZE);
            mi_read_f(n_inode, buffer_lec, nbloc * BLOCKSIZE, BLOCKSIZE);
        }
    } else if (tipo == 'f')
    {
        struct entrada entry;
        mi_read_f(p_inode_dir, &entry, p_entry * sizeof(struct entrada), sizeof(struct entrada));

        if (flag == 'l')
        {
            printf("\nTYPE\tPERMISSIONS\tmTIME\t\t\tSIZE\tNAME\n"
                   "---------------------------------------------------------------------\n");
            build_extended_buffer(entry, buffer);
        }
        else build_buffer(entry, buffer);
    }

    return EXITO;
}

int build_buffer(struct entrada entry, char *buffer)
{
    struct inodo inode;
    leer_inodo(entry.ninodo, &inode);
    char tmp[sizeof(entry.nombre) + 20];
    char *aux;

    if (inode.tipo == 'd') sprintf(tmp, NEGRITA BLUE "%s\t" RESET, entry.nombre);
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
    if (inode.tipo == 'd') sprintf(name, NEGRITA BLUE "%s" RESET, entry.nombre);
    else sprintf(name, "%s", entry.nombre);
    strcat(buffer, name);
    strcat(buffer, "\n");

    return EXITO;
}

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
    printf("Nº de inodo: %d\n", p_inodo);
    return mi_stat_f(p_inodo, p_stat);
}

//----------------------------- NIVEL 9 (02/05/2023 - ) -----------------------------

int mi_write(const char *camino, const void *buf, unsigned int offset, unsigned int nbytes)
{
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;

    #if USARCACHE==0
        int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 6);
        if (error  < 0) mostrar_error_buscar_entrada(error);
    #endif
    #if USARCACHE==1
        if (strcmp(UltimaEntradaEscritura.camino, camino) == 0)
        {
            p_inodo = UltimaEntradaEscritura.p_inodo;
        } else {
            fprintf(stderr, ORANGE "[mi_write() → Actualizamos la caché de escritura]" RESET "\n");
            int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 6);
            if (error  < 0) mostrar_error_buscar_entrada(error);
            strcpy(UltimaEntradaEscritura.camino, camino);
            UltimaEntradaEscritura.p_inodo = p_inodo;
        }
    #endif
    #if (USARCACHE==2 || USARCACHE==3)
        if (initWriteCache == 0) {
            initWriteCache = 1;
            #if USARCACHE==3
            writeCache.last = 0;
            #endif
            writeCache.size = 0;
        }
        unsigned int pos;
        if((pos = searchEntry(camino, &writeCache)) >= 0)
        {
            p_inodo = writeCache.lastEntries[pos].p_inodo;
        }
        else
        {
            fprintf(stderr, ORANGE "[mi_write() → Actualizamos la caché de escritura]" RESET "\n");
            int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 6);
            if (error  < 0) mostrar_error_buscar_entrada(error);

            updateCache(&writeCache, camino, &p_inodo);
        }
    #endif

    int written_bytes = mi_write_f(p_inodo, buf, offset, nbytes);

    return written_bytes;
}


int mi_read(const char *camino, void *buf, unsigned int offset, unsigned int nbytes)
{
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;

    #if USARCACHE==0
        int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 6);
        if (error  < 0) mostrar_error_buscar_entrada(error);
    #endif
    #if USARCACHE==1
        if (strcmp(UltimaEntradaLectura.camino, camino) == 0)
            {
                p_inodo = UltimaEntradaLectura.p_inodo;
            } else {
                fprintf(stderr, ORANGE "[mi_read() → Actualizamos la caché de lectura]" RESET "\n");
                int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 6);
                if (error  < 0) mostrar_error_buscar_entrada(error);
                strcpy(UltimaEntradaLectura.camino, camino);
                UltimaEntradaLectura.p_inodo = p_inodo;
            }
    #endif
    #if (USARCACHE==2 || USARCACHE==3)
        if (initReadCache == 0) {
            initReadCache = 1;
            #if USARCACHE==3
            readCache.last = 0;
            #endif
            readCache.size = 0;
        }
        unsigned int pos;
        if((pos = searchEntry(camino, &readCache)) >= 0)
        {
            p_inodo = readCache.lastEntries[pos].p_inodo;
        }
        else
        {
            fprintf(stderr, ORANGE "[mi_read() → Actualizamos la caché de lectura]" RESET "\n");
            int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 6);
            if (error  < 0) mostrar_error_buscar_entrada(error);

            updateCache(&readCache, camino, &p_inodo);
        }
    #endif

    int read_bytes = mi_read_f(p_inodo, buf, offset, nbytes);

    return read_bytes;
}

#if USARCACHE==2
    int searchEntry(const char *camino, struct CacheFIFO *cache)
    {
        for (int i = 0; i < cache->size; i++) {
            if(strcmp(cache->lastEntries[i].camino, camino) == 0)
            {
                cache->lastEntries[i].a = 1;
                return i;
            }
        }

        return -1;
    }
#endif
#if USARCACHE==3
int searchEntry(const char *camino, struct CacheLRU *cache)
    {
        for (int i = 0; i < cache->size; i++) {
            if(strcmp(cache->lastEntries[i].camino, camino) == 0)
            {
                gettimeofday(&cache->lastEntries[i].ultima_consulta, NULL);
                return i;
            }
        }
        return -1;
    }
#endif
#if USARCACHE==2
    void updateCache(struct CacheFIFO *cache, const char *camino, const unsigned int *p_inodo)
    {

        if(cache->size < CACHE_SIZE)
        {
            strcpy(cache->lastEntries[cache->size].camino, camino);
            cache->lastEntries[cache->size].p_inodo = *p_inodo;
            cache->lastEntries[cache->size].a = 1;
            cache->size++;
        }
        else
        {
            if(cache->lastEntries[cache->last % CACHE_SIZE].a == 0)
            {
                strcpy(cache->lastEntries[cache->last % CACHE_SIZE].camino, camino);
                cache->lastEntries[cache->last % CACHE_SIZE].p_inodo = *p_inodo;
                cache->lastEntries[cache->last % CACHE_SIZE].a = 1;
                cache->last++;
            }
            else
            {
                do
                {
                    cache->lastEntries[cache->last % CACHE_SIZE].a = 0;
                    cache->last++;
                } while(cache->lastEntries[cache->last % CACHE_SIZE].a == 1);
            }
        }
    }
#endif
#if USARCACHE==3
    void updateCache(struct CacheLRU *cache, const char *camino, const unsigned int *p_inodo)
    {
        if(cache->size < CACHE_SIZE)
        {
            strcpy(cache->lastEntries[cache->size].camino, camino);
            cache->lastEntries[cache->size].p_inodo = *p_inodo;
            gettimeofday(&cache->lastEntries[cache->size].ultima_consulta, NULL);
            cache->size++;
        }
        else
        {
            unsigned int posMin = 0;
            for(int i = 1; i < cache->size; i++)
            {
                if(compareTimevals(&cache->lastEntries[i].ultima_consulta, &cache->lastEntries[posMin].ultima_consulta) < 0)
                {
                    posMin = i;
                }
            }
            strcpy(cache->lastEntries[posMin].camino, camino);
            cache->lastEntries[posMin].p_inodo = *p_inodo;
            gettimeofday(&cache->lastEntries[posMin].ultima_consulta, NULL);
        }
    }

    int compareTimevals(const struct timeval *t1, const struct timeval *t2)
    {
        if(t1->tv_sec > t2->tv_sec) return 1;
        else if(t1->tv_sec < t2->tv_sec) return -1;
        else
        {
            if(t1->tv_usec > t2->tv_usec) return 1;
            else if(t1->tv_usec < t2->tv_usec) return -1;
        }
        return 0;
    }
#endif
//----------------------------- NIVEL 10 (06/05/2024 - 12/05/2024) -----------------------------

int mi_link(const char *camino1, const char *camino2)
{
    struct inodo inode1;
    unsigned int p_inode_dir1 = 0;
    unsigned int n_inode1 = 0;
    unsigned int p_entry1 = 0;
    int error = buscar_entrada(camino1, &p_inode_dir1, &n_inode1, &p_entry1, 0, 6);
    if (error < 0)
    {
        mostrar_error_buscar_entrada(error);
        return FALLO;
    }
    leer_inodo(n_inode1, &inode1);
    if((inode1.permisos & 1) == 0)
    {
        fprintf(stderr,
                RED
                "ERROR: the entry to be linked doesn't have read permissions\n"
                RESET
        );
    }

    unsigned int n_inode2 = 0;
    unsigned p_inode_dir2 = 0;
    unsigned p_entry2 = 0;
    error = buscar_entrada(camino2, &p_inode_dir2, &n_inode2, &p_entry2, 1, 6);
    if (error < 0 && error != ERROR_ENTRADA_YA_EXISTENTE)
    {
        mostrar_error_buscar_entrada(error);
        return FALLO;
    }

    struct entrada entry;
    mi_read_f(p_inode_dir2, &entry, p_entry2 * sizeof(struct entrada), sizeof(struct entrada));

    unsigned int inode_to_delete = entry.ninodo;
    entry.ninodo = n_inode1;
    mi_write_f(p_inode_dir2, &entry, p_entry2 * sizeof(struct entrada), sizeof(struct entrada));
    liberar_inodo(inode_to_delete);
    inode1.nlinks = inode1.nlinks + 1;
    inode1.ctime = time(NULL);
    escribir_inodo(n_inode1, &inode1);

    return EXITO;
}

int mi_unlink(const char *camino)
{
    struct inodo inode;
    unsigned int p_inode_dir = 0;
    unsigned int n_inode = 0;
    unsigned int p_entry = 0;

    int error = buscar_entrada(camino, &p_inode_dir, &n_inode, &p_entry, 0, 6);
    if (error < 0)
    {
        mostrar_error_buscar_entrada(error);
        return FALLO;
    }

    struct entrada entry;
    mi_read_f(p_inode_dir, &entry, p_entry * sizeof(struct entrada), sizeof(struct entrada));

    struct inodo inode_entry;
    leer_inodo(entry.ninodo, &inode_entry);
    if (inode_entry.tipo == 'd' && inode_entry.tamEnBytesLog > 0)
    {
        fprintf(stderr, RED "ERROR: can not delete not empty directory" RESET);
        return FALLO;
    }

    struct inodo inode_dir;
    leer_inodo(p_inode_dir, &inode_dir);
    unsigned int cant_entries_dir = inode_dir.tamEnBytesLog / sizeof(struct entrada);
    if (p_entry == cant_entries_dir - 1)
        mi_truncar_f(p_inode_dir, inode_dir.tamEnBytesLog - sizeof(struct entrada));
    else
    {
        struct entrada last_entry;
        mi_read_f(p_inode_dir, &last_entry, (cant_entries_dir - 1) * sizeof(struct entrada), sizeof(struct entrada));
        mi_write_f(p_inode_dir, &last_entry, p_entry * sizeof(struct entrada), sizeof(struct entrada));
        mi_truncar_f(p_inode_dir, inode_dir.tamEnBytesLog - sizeof(struct entrada));
    }

    inode_entry.nlinks = inode_entry.nlinks - 1;
    if (inode_entry.nlinks == 0) liberar_inodo(entry.ninodo);
    else
    {
        inode_entry.ctime = time(NULL);
        escribir_inodo(entry.ninodo, &inode_entry);
    }

    return EXITO;
}

int is_directory(const char *camino)
{
    unsigned int p_inode_dir = 0;
    unsigned int n_inode = 0;
    unsigned int p_entry = 0;
    int error = buscar_entrada(camino, &p_inode_dir, &n_inode, &p_entry, 0, 6);
    if (error < 0)
    {
        mostrar_error_buscar_entrada(error);
        return FALLO;
    }
    struct inodo inode;
    leer_inodo(n_inode, &inode);

    if (inode.tipo != 'd')
    {
        fprintf(stderr, RED "ERROR: the specified path doesn't refer to a directory\n" RESET);
        return FALLO;
    }

    return EXITO;
}